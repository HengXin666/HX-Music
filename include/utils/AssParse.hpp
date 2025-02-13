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

namespace HX {

class AssParse {
public:
    explicit AssParse()
        : _assLibrary(::ass_library_init())
        , _assRenderer(::ass_renderer_init(_assLibrary))
        , _assTrack(nullptr)
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

    /**
     * @brief 设置渲染器输出字幕图像的参考尺寸 即(ASS 字幕坐标系统的宽度和高度)
     * @param w 宽度
     * @param h 高度
     */
    void setFrameSize(int w, int h) {
        ::ass_set_frame_size(_assRenderer, w, h);
    }

    /**
     * @brief 加载本地ass文件
     * @param filePath 文件路径
     * @param encoded 文件编码
     */
    void readFile(const char* filePath, const char* encoded = "UTF-8") {
        if (_assTrack) {
            ::ass_free_track(_assTrack);
        }
        _assTrack = ::ass_read_file(_assLibrary, filePath, encoded);
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
    ASS_Image* rendererFrame(long long now, int& change) {
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
};

} // namespace HX

#endif // !_HX_ASS_PARSE_H_