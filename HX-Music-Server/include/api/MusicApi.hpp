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

#include <api/Api.hpp>
#include <dao/MemoryDAOPool.hpp>

#include <HXLibs/reflection/json/JsonRead.hpp>
#include <HXLibs/utils/FileUtils.hpp>

#include <config/DbPath.hpp>
#include <dao/MusicDAO.hpp>
#include <pojo/vo/MusicVO.hpp>
#include <pojo/vo/InitUploadFileTaskVO.hpp>
#include <pojo/vo/SelectDataVO.hpp>
#include <pojo/vo/SongListVO.hpp>
#include <interceptor/TokenInterceptor.hpp>
#include <utils/DirFor.hpp>
#include <utils/MusicInfo.hpp>
#include <utils/Uuid.hpp>
#include <utils/ThreadSafeMap.hpp>

#include <api/ApiMacro.hpp>

namespace HX {

/**
 * @brief 音乐播放 相关服务 API
 */
HX_SERVER_API_BEGIN(MusicApi) {
    struct MusicFileTask {
        std::string path;               // 临时文件采用 `${path}.tmp` 在后缀, 防止服务器关闭, 而任务却进行中
                                        // 下次重启服务器就可以把 .tmp 的文件全部删除了, 让用户重新上传
        uint64_t fileSize;              // 文件大小
        std::chrono::system_clock::time_point makeTime; // 任务创建时间
        uint64_t nowOffset;             // 当前写入进度 (偏移指针)
        std::unique_ptr<std::atomic_bool> atWork;       // 记录是否开始任务
    };
    auto musicDAO 
        = dao::MemoryDAOPool::get<MusicDAO, config::MusicDbPath>();

    /**
     * @brief 扫描音乐信息, 并且保存到数据库.
     */
    auto saveMusicInfo = [=](
        std::string path,                       // 相对于 ./file/music 的路径
        const std::filesystem::path& fullPath,  // path 加上 ./file/music 的路径
        coroutine::EventLoop& loop              // 一个新建的事件循环, 不应该为 res/req .getIO!
    ) {
        MusicInfo info{fullPath};
        auto imgOpt = info.getAlbumArtAdvanced();
        log::hxLog.info("新增歌曲:", path);
        auto const& dao = musicDAO->add<MusicDO>({
            {},
            std::move(path),
            info.getTitle(),
            info.getArtistList(),
            info.getAlbum(),
            static_cast<uint64_t>(info.getLengthInMilliseconds()),
            imgOpt ? imgOpt->type : ""
        });  
        if (imgOpt) {
            auto img = *imgOpt;
            utils::AsyncFile file{loop};
            file.syncOpen(
                "./file/cover/" + std::to_string(dao.id) + std::move(img.type)
            );
            file.syncWrite(img.buf);
            file.syncClose();
        }
        return dao.id;
    };
    auto musicUploadTaskMap
        = std::make_shared<utils::ThreadSafeMap<
            std::string, MusicFileTask>>();
    HX_ENDPOINT_BEGIN
        // 断点续传下载音乐
        .addEndpoint<GET, HEAD>("/music/download/{id}", [=] ENDPOINT {
            using namespace std::string_literals;
            log::hxLog.debug("请求 Path:", req.getReqPath());
            co_await api::coTryCatch([&] CO_FUNC {
                auto idStrView = req.getPathParam(0);
                MusicDAO::PrimaryKeyType id{};
                reflection::fromJson(id, idStrView);
                co_await res.useRangeTransferFile(
                     req.getRangeRequestView(),
                     "./file/music/"s += musicDAO->at(id).path
                );
            }, [&] CO_FUNC {
                co_await api::setJsonError("歌曲id不存在", res).sendRes();
            });
        })
        // 扫描服务端音乐
        .addEndpoint<GET>("/music/runScan", [=] ENDPOINT {
            std::size_t cnt = 0;
            coroutine::EventLoop loop;
            utils::traverseDirectory("./file/music", {},
                [&](const std::filesystem::path& relativePath) {
                std::string path = relativePath.string();
                std::filesystem::path fullPath = std::filesystem::path{"./file/music"} / relativePath;
                if (!std::filesystem::is_directory(fullPath) && !musicDAO->isExist(path)) {
                    saveMusicInfo(std::move(path), fullPath, loop);
                    ++cnt;
                }
            });
            co_await api::setJsonSucceed(
                "OK: 扫描完成, 新增 " + std::to_string(cnt) + " 首音乐!",
            res).sendRes();
        }, TokenInterceptor<PermissionEnum::Administrator>{})
        // 获取音乐信息
        .addEndpoint<GET>("/music/info/{id}", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto idStrView = req.getPathParam(0);
                MusicDAO::PrimaryKeyType id{};
                reflection::fromJson(id, idStrView);
                auto const& musicDO = musicDAO->at(id);
                co_await api::setJsonSucceed<MusicVO>({
                    musicDO.id,
                    musicDO.path,
                    musicDO.musicName,
                    musicDO.singers,
                    musicDO.musicAlbum,
                    musicDO.millisecondsLen
                }, res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("歌曲 ID 不存在", res).sendRes();
            });
        }, TokenInterceptor<PermissionEnum::ReadOnlyUser>{})
        // 初始化上传音乐任务
        .addEndpoint<POST>("/music/upload/init", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                using namespace std::string_literals;
                auto vo = co_await api::getVO<InitUploadFileTaskVO>(req);
                // 1. 验证路径是否存在该文件
                std::filesystem::path filePath
                    = "./file/music" / std::filesystem::path{vo.path};
                std::filesystem::path tmpFilePath
                    = "./file/music" / std::filesystem::path{vo.path + ".tmp"s};
                if (std::filesystem::exists(filePath) 
                 || std::filesystem::exists(tmpFilePath)
                ) [[unlikely]] {
                    if (std::filesystem::is_regular_file(filePath)) [[likely]] {
                        co_return co_await api::setJsonError(
                            "文件已存在", res).sendRes();
                    } else if (std::filesystem::is_regular_file(tmpFilePath)) {
                        co_return co_await api::setJsonError(
                            "任务已存在", res).sendRes();
                    } {
                        co_return co_await api::setJsonError(
                            "路径被占用, 目标可能是文件夹", res).sendRes();
                    }
                }
                // 2. 创建唯一id
                std::string id;
                do {
                    id = utils::Uuid::makeV4();
                } while (!musicUploadTaskMap->try_emplace(
                    id, MusicFileTask{
                        vo.path,
                        vo.fileSize,
                        std::chrono::system_clock::now(),
                        0,
                        std::make_unique<std::atomic_bool>(false)
                    }
                ));
                // 3. 创建文件夹, 如果路径不存在
                std::filesystem::create_directories(filePath.parent_path());

                // 4. 创建占位文件
                utils::AsyncFile file{req.getIO()};
                co_await file.open(tmpFilePath.string(), utils::OpenMode::Write);
                co_await file.close();
                co_return co_await api::setJsonSucceed(
                    std::move(id), res).sendRes();
            }, [&] CO_FUNC {
                co_return co_await api::setJsonError(
                    "数据非法", res).sendRes();
            });
        }, TokenInterceptor<PermissionEnum::RegularUser>{})
        // 上传音乐主任务
        .addEndpoint<WS>("/music/upload/push/{pushId}", [=] ENDPOINT {
            using namespace std::string_literals;
            log::hxLog.debug("ws: ", req.getReqPath());
            auto [it, end] = musicUploadTaskMap->find(static_cast<std::string_view>(
                req.getPathParam(0)
            ));
            if (it == end) {
                co_return co_await api::setJsonError(
                    "任务不存在", res).sendRes();
            }
            auto& task = it->second;
            if (task.atWork->exchange(true)) {
                // 之前就是 true, 表示已经在工作了
                co_return co_await api::setJsonError(
                    "任务被占用, 任务已经在工作了", res).sendRes();
            }
            // 写文件
            utils::AsyncFile file{req.getIO()};
            std::filesystem::path tmpFilePath
                = "./file/music" / std::filesystem::path{task.path + ".tmp"s};
            auto ws = co_await net::WebSocketFactory::accept(req, res);
            // 设置为尾加
            co_await file.open(tmpFilePath.string(), utils::OpenMode::Append);
            file.setOffset(task.nowOffset);
            try {
                // 先协商进度
                co_await ws.sendText(std::to_string(task.nowOffset));
                while (task.nowOffset < task.fileSize) {
                    auto buf = co_await ws.recvBytes();
                    co_await file.write(buf);
                    task.nowOffset += buf.size();
                    // 完成百分比
                    co_await ws.sendText(
                        log::internal::FormatZipString{}.make<double, 5>(
                            static_cast<double>(task.nowOffset) / static_cast<double>(task.fileSize)
                    ));
                }
                try {
                    // 任务完成, 重命名文件
                    std::filesystem::path filePath
                        = "./file/music" / std::filesystem::path{task.path};
                    std::filesystem::rename(tmpFilePath, filePath);
                    // 刮削到数据库
                    coroutine::EventLoop loop;
                    auto id 
                        = saveMusicInfo(std::move(task.path), filePath, loop);
                    // 发送 id
                    co_await ws.sendText(std::to_string(id));
                    // 删除任务
                    musicUploadTaskMap->erase(it);
                    co_await ws.close();
                } catch (...) {
                    ;
                }
            } catch (...) {
                // 客户端ws断开了
                *task.atWork = false;
            }
            co_await file.close();
        }, TokenInterceptor<PermissionEnum::RegularUser>{})
        // 分页查找歌曲
        .addEndpoint<POST>("/music/select", [=] ENDPOINT {
            co_await api::coTryCatch([&] CO_FUNC {
                auto [beginId, maxCnt] = co_await api::getVO<SelectDataVO>(req);
                SongListVO resVO;
                musicDAO->lockSelect([&](MusicDAO::MapType const& mp) mutable {
                    auto it = mp.lower_bound(beginId + 1);
                    if (it == mp.end())
                        return;
                    for (; maxCnt && it != mp.end(); ++it, --maxCnt) {
                        auto const& musicDO = it->second;
                        resVO.songList.emplace_back<MusicVO>({
                            musicDO.id,
                            musicDO.path,
                            musicDO.musicName,
                            musicDO.singers,
                            musicDO.musicAlbum,
                            musicDO.millisecondsLen
                        });
                    }
                });
                co_await api::setJsonSucceed(std::move(resVO), res).sendRes();
            }, [&] CO_FUNC {
                co_await api::setJsonError("查找数据非法", res).sendRes();
            });
        }, TokenInterceptor<PermissionEnum::ReadOnlyUser>{})
    HX_ENDPOINT_END;
} HX_SERVER_API_END;

} // namespace HX

#include <api/UnApiMacro.hpp>