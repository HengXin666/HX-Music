#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <filesystem>

namespace fs = std::filesystem;

// 检查是否包含绘图指令
bool containsDrawing(const std::string& line) {
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
void separateAssFile(const fs::path& inputPath) {
    // 验证输入文件
    if (!fs::exists(inputPath)) {
        throw std::runtime_error("输入文件不存在");
    }
    if (inputPath.extension() != ".ass") {
        throw std::runtime_error("仅支持ASS文件");
    }

    // 准备输出路径
    fs::path textPath = inputPath;
    textPath.replace_filename(inputPath.stem().string() + "_text" + inputPath.extension().string());

    fs::path effectPath = inputPath;
    effectPath.replace_filename(inputPath.stem().string() + "_effect" + inputPath.extension().string());

    // 打开文件
    std::ifstream inFile(inputPath);
    if (!inFile.is_open()) {
        throw std::runtime_error("无法打开输入文件");
    }

    std::ofstream textFile(textPath);
    std::ofstream effectFile(effectPath);
    if (!textFile.is_open() || !effectFile.is_open()) {
        throw std::runtime_error("无法创建输出文件");
    }

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
                        textFile << line << '\n';
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
    textFile.close();
    effectFile.close();

    std::cout << "分离完成:\n"
              << "  文本字幕: " << textPath << "\n"
              << "  特效字幕: " << effectPath << std::endl;
}

int _main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "用法: " << argv[0] << " <input.ass>" << std::endl;
        return 1;
    }

    try {
        separateAssFile(argv[1]);
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}