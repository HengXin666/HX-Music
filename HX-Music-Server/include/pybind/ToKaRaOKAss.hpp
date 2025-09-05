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

#include <mutex>
#include <memory>
#include <filesystem>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h> // pybind11::scoped_interpreter

#include <HXLibs/log/Log.hpp>

namespace HX {

struct ToKaRaOKAss {
private:
    pybind11::scoped_interpreter _guard;
    pybind11::object _kaRaOKAss;
    std::mutex _mtx;
public:
    ToKaRaOKAss()
        : _guard{}
        , _kaRaOKAss{}
        , _mtx{}
    {
        namespace py = pybind11;
        py::gil_scoped_acquire gil{};
        // 添加 Python 脚本目录到 sys.path
        py::module sys = py::module::import("sys");
        sys.attr("path").cast<py::list>().append("../pyTool");
        // 导入模块
        py::module myscript = py::module::import("main");
        // 获取 py 类
        _kaRaOKAss = myscript.attr("KaRaOKAss")();
        if (!_kaRaOKAss || _kaRaOKAss.is_none()) {
            throw std::runtime_error("KaRaOKAss ctor failed");
        }
    }

    ToKaRaOKAss& operator=(ToKaRaOKAss&&) noexcept = delete;

    /**
     * @brief 从本地文件在网络上匹配歌词并保存到指定路径
     * @param absolutePath 本地歌曲的绝对路径
     * @param outputPath 输出路径
     */
    ToKaRaOKAss& findLyricsFromNet(
        std::filesystem::path const& absolutePath,
        std::filesystem::path const& outputPath
    ) {
        pybind11::gil_scoped_acquire gil{};
        log::hxLog.info("begin findLyricsFromNet");
        try {
            _kaRaOKAss.attr("findLyricsFromNet")(absolutePath.string(), outputPath.string());
        } catch (std::exception const& e) {
            log::hxLog.error("err:", e.what());
            throw ;
        }
        return *this;
    }

    /**
     * @brief 对歌词进行日语注音
     * @param absolutePath 歌词的绝对路径
     */
    ToKaRaOKAss& doJapanesePhonetics(std::filesystem::path const& absolutePath) {
        std::unique_lock _{_mtx};
        _kaRaOKAss.attr("doJapanesePhonetics")(absolutePath.string());
        return *this;
    }

    /**
     * @brief 转变为双行卡拉ok样式
     * @param absolutePath 歌词的绝对路径
     */
    ToKaRaOKAss& toTwoLineKaraokeStyle(std::filesystem::path const& absolutePath) {
        _kaRaOKAss.attr("toTwoLineKaraokeStyle")(absolutePath.string());
        return *this;
    }

    /**
     * @brief 应用卡拉ok模板
     * @param absolutePath 歌词的绝对路径
     */
    void callApplyKaraokeTemplateLua(std::filesystem::path const& absolutePath) {
        _kaRaOKAss.attr("callApplyKaraokeTemplateLua")(absolutePath.string());
    }
};

/**
 * @brief 获取 Ass 卡拉Ok 处理对象
 * @return std::shared_ptr<ToKaRaOKAss> 
 */
inline std::shared_ptr<ToKaRaOKAss> getToKaRaOKAssPtr() {
    static auto ptr = std::make_shared<ToKaRaOKAss>();
    return ptr;
}

} // namespace HX