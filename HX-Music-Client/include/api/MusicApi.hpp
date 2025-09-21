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

#include <singleton/NetSingleton.hpp>
#include <pojo/SongInformation.hpp>

#include <HXLibs/utils/NumericBaseConverter.hpp>

#include <api/Api.hpp>
#include <pojo/vo/MusicVO.hpp>
#include <pojo/vo/InitUploadFileTaskVO.hpp>
#include <pojo/vo/SongListVO.hpp>
#include <pojo/vo/SelectDataVO.hpp>

namespace HX {

/**
 * @brief 客户端 歌曲相关请求 API
 */
struct MusicApi {
    /**
     * @brief 查询 指定歌曲id 的歌曲信息
     * @param id 
     * @return container::FutureResult<SongInformation> 
     */
    static container::FutureResult<SongInformation> selectById(uint64_t id) {
        return NetSingleton::get().getReq("/music/info/" + std::to_string(id))
            .thenTry([](container::Try<net::ResponseData> t) -> SongInformation {
                if (!t) [[unlikely]] {
                    t.rethrow();
                }
                auto res = t.move();
                auto jsonVO = api::getVO<vo::JsonVO<MusicVO>>(res);
                if (jsonVO.isError()) [[unlikely]] {
                    throw std::runtime_error{std::move(jsonVO.msg)};
                }
                return *std::move(jsonVO.data);
            }
        );
    }

    /**
     * @brief 初始化上传音乐任务
     * @param localPath 音乐的本地路径 (绝对路径)
     * @param serverPath 服务器路径 (相对于`./file/music`)
     * @return container::FutureResult<std::string> 
     */
    static container::FutureResult<std::string> initUploadMusic(
        std::string_view localPath,
        std::string serverPath
    ) {
        return NetSingleton::get().postReq<InitUploadFileTaskVO>("/music/upload/init", {
            std::move(serverPath),
            utils::FileUtils::getFileSize(localPath)
        }).thenTry([](container::Try<net::ResponseData> t) {
            if (!t) [[unlikely]] {
                t.rethrow();
            }
            auto res = t.move();
            auto jsonVO = api::getVO<vo::JsonVO<std::string>>(res);
            if (jsonVO.isError()) [[unlikely]] {
                throw std::runtime_error{std::move(jsonVO.msg)};
            }
            return *std::move(jsonVO.data);
        });
    }

    /**
     * @brief 分块上传歌曲
     * @param localPath 待上传的本地文件路径
     * @param pushId 上传任务id
     * @param progress 进度百分比
     * @param uploadSpeed 上传速度, 单位: 字节 / 秒 (B/s)
     * @return container::FutureResult<container::Try<uint64_t>> 新歌曲的id
     */
    static container::FutureResult<container::Try<uint64_t>> uploadMusic(
        std::string localPath,
        std::string pushId,
        double* progress = nullptr,
        std::size_t* uploadSpeed = nullptr
    ) {
        return NetSingleton::get().wsReq("/music/upload/push/" + std::move(pushId),
            [_localPath = std::move(localPath), progress, uploadSpeed](
                net::WebSocketClient ws
            ) mutable -> coroutine::Task<uint64_t> {
                using namespace std::chrono;
                utils::AsyncFile file{ws.getIO()};
                co_await file.open(_localPath, utils::OpenMode::Read);
                std::vector<char> buf;
                buf.resize(1 << 22); // 4 MB
                auto mae = std::chrono::system_clock::now();
                std::size_t sum = 0;
                for (;;) {
                    // 发
                    int len = co_await file.read(buf);
                    if (len) [[likely]] {
                        co_await ws.sendBytes({buf.data(), static_cast<std::size_t>(len)});
                    } else {
                        break; // 我发完了
                    }
                    if (uploadSpeed) {                    
                        sum += len;
                        if (auto now = std::chrono::system_clock::now(); now - mae >= 1s) {
                            mae = now;
                            *uploadSpeed = sum;
                            sum = 0;
                        }
                    }
                    // 同步进度
                    auto progressStr = co_await ws.recvText();
                    if (progress) {
                        reflection::fromJson(*progress, progressStr);
                    }
                    log::hxLog.debug("上传进度:", progressStr, "(+add:", 1.0 * len / (1 << 20), "MB)");
                }
                co_await file.close();
                uint64_t resId;
                try {                
                    for (;;) { 
                        // 服务器发送id后, 会关闭连接. 此处的第二次读取就是为了等待关闭连接.
                        resId = utils::NumericBaseConverter::strToNum<uint64_t, 10>(
                            co_await ws.recvText()
                        );
                    }
                } catch (...) {
                    ;
                }
                co_return resId;
            });
    }

    /**
     * @brief 分页查找歌曲
     * @param beginId  歌曲起始id
     * @param maxCnt 查找的最大数量
     * @return container::FutureResult<std::vector<SongInformation>> 
     */
    static container::FutureResult<std::vector<SongInformation>> selectMusic(
        uint64_t beginId, uint64_t maxCnt
    ) {
        return NetSingleton::get().postReq(
            "/music/select", SelectDataVO{beginId, maxCnt}
        ).thenTry([](container::Try<net::ResponseData> t) {
            return api::checkTryAndStatusAndJsonVO<SongListVO>(std::move(t)).songList;
        });
    }
};

} // namespace HX