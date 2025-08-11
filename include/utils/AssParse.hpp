#pragma once
/*
 * Copyright (C) 2025 Heng_Xin. All rights reserved.
 *
 * This file is part of HX-Music.
 *
 * HX-Music is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HX-Music is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HX-Music.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef _HX_ASS_PARSE_H_
#define _HX_ASS_PARSE_H_

#include <ass/ass.h>

#include <QByteArray>

#include <QFile>
#include <fstream>
#include <sstream>
#include <regex>

namespace HX {

namespace internal {

struct AssSeparatorResult {
    std::string textAss;    // 仅文本版本ASS内容
    std::string nonTextAss; // 非文本版本ASS内容
};

namespace fs = std::filesystem;

inline std::string stripVerticalAffectTags(const std::string& line) {
    std::string out;
    out.reserve(line.size());
    bool inTag = false;
    std::string tagBuffer;

    static const std::regex verticalAffectTags(
        R"(\\(move|org|t\(.*\\frx|fry|frz|fscx|fscy|fs|an[1-9]|fad|fade))",
        std::regex::icase
    );

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];
        if (c == '{') {
            inTag = true;
            tagBuffer.clear();
            tagBuffer.push_back(c);
        } else if (c == '}') {
            tagBuffer.push_back(c);
            inTag = false;
            // 删除会影响垂直位置的 tag
            if (!std::regex_search(tagBuffer, verticalAffectTags)) {
                out += tagBuffer;
            }
        } else {
            if (inTag) tagBuffer.push_back(c);
            else out.push_back(c);
        }
    }
    return out;
}



// 检查是否包含绘图指令
inline bool containsDrawing(const std::string& line) {
    // 同时检测 {\p 和 \p# 两种格式
    for (size_t i = 0; i < line.size() - 2; ++i) {
        // 检测 {\p
        if (line[i] == '{' && line[i+1] == '\\' && 
           std::tolower(line[i+2]) == 'p') {
            return true;
        }
        // 检测 \p# (独立标签)
        if (line[i] == '\\' && std::tolower(line[i+1]) == 'p' &&
           std::isdigit(line[i+2])) {
            return true;
        }
    }
    return false;
}

// 主分离函数
inline AssSeparatorResult separateAssFile(const fs::path& inputPath) {
    // 验证输入文件
    if (!fs::exists(inputPath)) {
        throw std::runtime_error("输入文件不存在");
    }
    if (inputPath.extension() != ".ass") {
        throw std::runtime_error("仅支持ASS文件");
    }

    // 打开文件
    std::ifstream inFile(inputPath);
    if (!inFile.is_open()) {
        throw std::runtime_error("无法打开输入文件");
    }

    std::stringstream textFile{};
    std::stringstream effectFile{};

    // 状态机
    enum class State { HEADER, EVENTS, FOOTER };
    State state = State::HEADER;
    std::string line;

    while (std::getline(inFile, line)) {
        // 移除Windows换行符
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        switch (state) {
        case State::HEADER:
            // 写入两个文件
            textFile << line << '\n';
            effectFile << line << '\n';

            // 检测事件段开始
            if (line == "[Events]") {
                state = State::EVENTS;
            }
            break;

        case State::EVENTS:
            // 检测新段落开始
            if (!line.empty() && line[0] == '[') {
                state = State::FOOTER;
                textFile << line << '\n';
                effectFile << line << '\n';
                break;
            }

            // 处理事件内容
            {
                std::string trimmed = line;
                // 移除前导空白
                size_t start = trimmed.find_first_not_of(" \t");
                if (start != std::string::npos) {
                    trimmed = trimmed.substr(start);
                }

                // 分离对话行
                if (trimmed.compare(0, 9, "Dialogue:") == 0) {
                    if (containsDrawing(line)) {
                        effectFile << line << '\n';
                    } else {
                        // 下面不对! 不应该修改 ass 字幕
                        std::string stableLine = stripVerticalAffectTags(line);
                        textFile << stableLine << '\n';
                        if (stableLine != line) { // 如果有去掉的tag, 原版放到特效ASS
                            effectFile << line << '\n';
                        }
                    }
                }
                // 非对话行复制到两个文件
                else {
                    textFile << line << '\n';
                    effectFile << line << '\n';
                }
            }
            break;

        case State::FOOTER:
            // 复制剩余内容
            textFile << line << '\n';
            effectFile << line << '\n';
            break;
        }
    }

    // 关闭文件
    inFile.close();
    return {
        std::move(textFile).str(),
        std::move(effectFile).str()
    };
}

} // namespace internal

class AssParse {
public:
    explicit AssParse()
        : _assLibrary{::ass_library_init()}
        , _assRenderer{::ass_renderer_init(_assLibrary)}
        , _assTrack{nullptr}
        , _width{}
        , _height{}
    {
        // 初始化字体
        ::ass_set_fonts(
            _assRenderer,
            nullptr,     // 字体路径
            "SimHei",  // 黑体
            ASS_FONTPROVIDER_AUTODETECT,
            nullptr,
            1                  // 启用抗锯齿 (?)
        );
    }

    void closeLog() noexcept {
        ::ass_set_message_cb(
            _assLibrary,
            [](int /*level*/,
               const char* /*fmt*/,
               va_list /*args*/,
               void* /*data*/) {
                // 什么都不做, 完全屏蔽日志
            },
            nullptr);
    }

    /**
     * @brief 设置渲染器输出字幕图像的参考尺寸 即(ASS 字幕坐标系统的宽度和高度)
     * @param w 宽度
     * @param h 高度
     */
    void setFrameSize(int w, int h) noexcept {
        _width = w;
        _height = h;
        ::ass_set_frame_size(_assRenderer, w, h);
    }

    int getWidth() const noexcept {
        return _width;
    }

    int getHeight() const noexcept {
        return _height;
    }

    /**
     * @brief 加载本地ass文件
     * @param filePath 文件路径
     * @param encoded 文件编码
     */
    void readFile(const char* filePath, const char* encoded = "UTF-8") noexcept {
        // auto result = internal::separateAssFile(filePath);
        // readMemory(result.nonTextAss.data());
        // return;
        if (_assTrack) {
            ::ass_free_track(_assTrack);
        }
        _assTrack = ::ass_read_file(_assLibrary, filePath, encoded);
    }

    /**
     * @brief 加载内存中已读取的ass文件内容
     * @param buf 
     */
    void readMemory(QByteArray&& buf) noexcept {
        if (_assTrack) {
            ::ass_free_track(_assTrack);
        }
        _assTrack = ::ass_read_memory(_assLibrary, buf.data(), buf.size(), nullptr);
    }

    /**
     * @brief 渲染一帧
     * 
     * @param now 第几毫秒
     * @param change [out] 内容是否改变(与前一次调用),
     * 值含义:
     * - 没有变化: 0; 
     * - 位置变化: 1; 
     * - 内容变化: 2; 
     * @warning 请不要释放`ASS_Image*`! 它是库内部维护的!
     * @return ASS_Image* 图像链表
     */
    ASS_Image* rendererFrame(long long now, int& change) noexcept {
        return ::ass_render_frame(_assRenderer, _assTrack, now, &change);
    }

    ~AssParse() noexcept {
        if (_assTrack) {
            ::ass_free_track(_assTrack);
        }
        if (_assRenderer) {
            ::ass_renderer_done(_assRenderer);
        }
        if (_assLibrary) {
            ::ass_library_done(_assLibrary);
        }
    }

private:
    // ass库数据
    ASS_Library* _assLibrary;

    // ass渲染器
    ASS_Renderer* _assRenderer;

    // ass字幕文件内容
    ASS_Track* _assTrack;

    int _width;
    int _height; 
};

} // namespace HX

#endif // !_HX_ASS_PARSE_H_