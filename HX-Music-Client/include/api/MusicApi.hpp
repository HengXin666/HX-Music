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
private:
    struct _not_cb_ {};
public:
    inline static constexpr auto NotCbFunc
        = [](
            [[maybe_unused]] std::size_t all,
            [[maybe_unused]] double progress,
            [[maybe_unused]] std::size_t uploadSpeed
        ) -> _not_cb_ {
            return {};
        };
    
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
     * @param cb 回调函数
     *  - @param all 总传输字节
     *  - @param progress 进度百分比
     *  - @param uploadSpeed 上传速度, 单位: 字节 / 秒 (B/s)
     * @return container::FutureResult<container::Try<uint64_t>> 新歌曲的id
     */
    template <typename Cb>
    static container::FutureResult<container::Try<uint64_t>> uploadMusic(
        std::string localPath,
        std::string pushId,
        Cb&& cb
    ) {
        return NetSingleton::get().wsReq("/music/upload/push/" + std::move(pushId),
            [_localPath = std::move(localPath), _cb = std::forward<Cb>(cb)](
                net::WebSocketClient ws
            ) mutable -> coroutine::Task<uint64_t> {
                using namespace std::chrono;
                utils::AsyncFile file{ws.getIO()};
                co_await file.open(_localPath, utils::OpenMode::Read);
                std::vector<char> buf;
                buf.resize(1 << 22); // 4 MB
                std::queue<std::tuple<decltype(std::chrono::system_clock::now()), std::size_t>> q;
                std::size_t sum = 0; // 过去一秒内的总传输
                std::size_t all = 0; // 总传输
                double progress = 0;
                container::Try<> err;
                try {
                    auto offsetStr = co_await ws.recvText();
                    uint64_t offset = 0;
                    reflection::Numer::fromNumer(offset, offsetStr.begin(), offsetStr.end());
                    file.setOffset(offset);
                    all += offset;
                    for (;;) {
                        // 发
                        int len = co_await file.read(buf);
                        if (len) [[likely]] {
                            co_await ws.sendBytes({buf.data(), static_cast<std::size_t>(len)});
                        } else {
                            break; // 我发完了
                        }
                        // 同步进度
                        auto progressStr = co_await ws.recvText();
                        if constexpr (!std::is_same_v<std::invoke_result_t<Cb, std::size_t, double, std::size_t>, _not_cb_>) {
                            auto now = std::chrono::system_clock::now();
                            reflection::Numer::fromNumer(progress, progressStr.begin(), progressStr.end());
                            decltype(std::chrono::system_clock::now()) mae = now;
                            std::size_t size = 0;
                            if (q.size()) [[likely]] {
                                std::tie(mae, size) = q.front();
                            }
                            sum += static_cast<std::size_t>(len);
                            all += static_cast<std::size_t>(len);
                            q.push({now, static_cast<std::size_t>(len)});
                            while (now - mae >= 1s) {
                                sum -= size;
                                q.pop();
                                std::tie(mae, size) = q.front();
                            }
                            if constexpr (requires {
                                { _cb(all, progress, sum) } -> std::convertible_to<bool>;
                            }) {
                                if (_cb(all, progress, sum)) [[unlikely]] {
                                    co_await ws.close();
                                    throw std::runtime_error{"stop"};
                                }
                            } else {
                                _cb(all, progress, sum);
                            }
                        }
                        log::hxLog.debug("上传进度:", progressStr, "(+add:", 1.0 * len / (1 << 20), "MB)",
                            "总:", 1.0 * (all) / (1 << 20), "MB"
                        );
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
                } catch (...) {
                    // read 也是可能会抛出异常的~
                    err.setException(std::current_exception());
                }
                co_await file.close();
                err.rethrow();
                co_return -1; // 你警告你🐎呢
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

    /**
     * @brief 扫描歌曲
     * @tparam Cb 
     * @param cb 
     * @return 
     */
    template <typename Cb>
        requires (requires (Cb&& cb) {
            { cb(std::string{}) };
        })
    static container::FutureResult<container::Try<>> startScan(Cb&& cb) {
        return NetSingleton::get().wsReq(
            "/music/runScan/ws",
            [_cb = std::forward<Cb>(cb)](net::WebSocketClient ws) -> coroutine::Task<> {
                for (;;) {
                    _cb(co_await ws.recvText());
                }
            }
        );
    }
};

} // namespace HX