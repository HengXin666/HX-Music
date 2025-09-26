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

#include <memory>
#include <filesystem>

#include <pybind11/pybind11.h>
#include <pybind11/embed.h> // pybind11::scoped_interpreter

#include <HXLibs/log/Log.hpp>
#include <HXLibs/container/ThreadPool.hpp>

namespace HX {

namespace internal {

inline std::string getPythonVersion(const std::filesystem::path& venvPath) {
#ifdef _WIN32
    std::filesystem::path pythonExe = venvPath / "Scripts" / "python.exe";
#else
    std::filesystem::path pythonExe = venvPath / "bin" / "python3";
#endif

    std::string versionCmd = pythonExe.string() + " -c \"import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')\"";
    FILE* pipe = popen(versionCmd.c_str(), "r");
    if (!pipe) return "";
    char buffer[16];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);
    result.erase(result.find_last_not_of(" \n\r\t")+1); // 去掉换行
    return result;
}

} // namespace internal

struct ToKaRaOKAss {
private:
    struct PyData {
        pybind11::scoped_interpreter _guard;
        pybind11::object _kaRaOKAss;
    };
    container::UninitializedNonVoidVariant<PyData, void> _pyData;
    container::ThreadPool _pool;
public:
    ToKaRaOKAss()
        : _pyData{}
        , _pool{}
    {
        _pool.setFixedThreadNum(1);
        _pool.run<container::ThreadPool::Model::FixedSizeAndNoCheck>();
        _pool.addTask([this]() mutable {
            namespace py = pybind11;
            _pyData.emplace<0>(PyData{py::scoped_interpreter{}, py::object{}});
            auto& [_, _kaRaOKAss] = _pyData.get<0>();
            // 添加 Python 脚本目录到 sys.path
            py::module sys = py::module::import("sys");
            sys.attr("path").cast<py::list>().append("../pyTool");
            // 导入模块
            py::module myscript = py::module::import("main");
            // 获取 py 类
            _kaRaOKAss = myscript.attr("KaRaOKAss")();
            if (!_kaRaOKAss || _kaRaOKAss.is_none()) [[unlikely]] {
                throw std::runtime_error("KaRaOKAss ctor failed");
            }
        }).wait();
    }

    ToKaRaOKAss& operator=(ToKaRaOKAss&&) noexcept = delete;

    /**
     * @brief 释放
     * @warning 之后再次调度就是ub
     */
    void release() {
        log::hxLog.warning("释放ing..");
        if (_pyData.index()) [[unlikely]] {
            return;
        }
        _pool.addTask([this] {
            log::hxLog.info("raii pybind");
            auto& [_guard, _kaRaOKAss] = _pyData.get<0>();
            _kaRaOKAss.release();
            _pyData.reset();
            log::hxLog.info("raii pybind: ok");
        }).wait();
    }

    /**
     * @brief 从本地文件在网络上匹配歌词并保存到指定路径
     * @param absolutePath 本地歌曲的绝对路径
     * @param outputPath 输出路径
     */
    container::FutureResult<> findLyricsFromNet(
        std::filesystem::path absolutePath,
        std::filesystem::path outputPath
    ) {
        return _pool.addTask([_absolutePath = std::move(absolutePath),
                              _outputPath = std::move(outputPath), this]() {
            auto const& [_, _kaRaOKAss] = _pyData.get<0>();
            _kaRaOKAss.attr("findLyricsFromNet")(_absolutePath.string(), _outputPath.string(), 0);
        });
    }

    /**
     * @brief 对歌词进行日语注音
     * @param absolutePath 歌词的绝对路径
     */
    container::FutureResult<> doJapanesePhonetics(std::filesystem::path absolutePath) {
        return _pool.addTask([_absolutePath = std::move(absolutePath), this]() {
            auto& [_, _kaRaOKAss] = _pyData.get<0>();
            _kaRaOKAss.attr("doJapanesePhonetics")(_absolutePath.string());
        });
    }

    /**
     * @brief 转变为双行卡拉ok样式
     * @param absolutePath 歌词的绝对路径
     */
    container::FutureResult<> toTwoLineKaraokeStyle(std::filesystem::path absolutePath) {
        return _pool.addTask([_absolutePath = std::move(absolutePath), this]() {
            auto& [_, _kaRaOKAss] = _pyData.get<0>();
            _kaRaOKAss.attr("toTwoLineKaraokeStyle")(_absolutePath.string());
        });
    }

    /**
     * @brief 应用卡拉ok模板
     * @param absolutePath 歌词的绝对路径
     */
    container::FutureResult<> callApplyKaraokeTemplateLua(std::filesystem::path absolutePath) {
        return _pool.addTask([_absolutePath = std::move(absolutePath), this]() {
            auto& [_, _kaRaOKAss] = _pyData.get<0>();
            _kaRaOKAss.attr("callApplyKaraokeTemplateLua")(_absolutePath.string());
        });
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