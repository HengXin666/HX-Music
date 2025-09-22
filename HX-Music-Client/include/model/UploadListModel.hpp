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

#include <filesystem>

#include <QCoreApplication>
#include <QAbstractListModel>

#include <singleton/SignalBusSingleton.h>
#include <controller/MessageController.h>
#include <api/MusicApi.hpp>
#include <api/PlaylistApi.hpp>
#include <utils/DirFor.hpp>

namespace HX {

class UploadListModel : public QAbstractListModel {
    Q_OBJECT

    enum UploadListRoles {
        NameRole = Qt::UserRole + 1,
        PathRole,
        ProgressRole,
        UploadSpeedRole,
        PlaylistIdRole,
        UploadStatusRole,
        ErrMsgRole,
        NowUploadSizeRole,
        TotalSizeRole,
    };

    enum UploadStatus {
        Waiting,
        Uploading,
        UploadCompleted,
        Error,
        Stoped, // 暂停中
    };

    struct UploadFileData {
        QString fileName;                           // 文件名
        std::filesystem::path path;                 // 文件路径
        std::filesystem::path basePath;             // 基准路径
        std::string taskUuid;                       // 任务 uuid
        double progress;                            // 上传进度百分比
        std::size_t uploadSpeed;                    // 上传速度 (B / s)
        uint64_t addToPlaylistId;                   // 需要添加的歌单id
        UploadStatus uploadStatus;                  // 上传状态
        QString errMsg;                             // 错误信息
        std::size_t nowUploadSize;                  // 已经上传的大小
        std::size_t totalSize;                      // 总大小
        std::unique_ptr<std::atomic_bool> isStop    // 是否暂停
            = std::make_unique<std::atomic_bool>(false);
    };

    // 忽略大小写并且可靠排序的
    struct CaseInsensitiveLess {
        bool operator()(std::string_view lhs, std::string_view rhs) const noexcept {
            auto lit = lhs.begin(), rend = lhs.end();
            auto rit = rhs.begin(), rend2 = rhs.end();
            for (; lit != lhs.end() && rit != rhs.end(); ++lit, ++rit) {
                unsigned char l = std::tolower(static_cast<unsigned char>(*lit));
                unsigned char r = std::tolower(static_cast<unsigned char>(*rit));
                if (l < r) 
                    return true;
                if (l > r) 
                    return false;
            }
            return lhs.size() < rhs.size();
        }
    };

    // 支持的播放格式 (得看ffmpeg)
    std::set<std::string, CaseInsensitiveLess> _supportedAudioExt {
        ".mp3", ".flac", ".wav", ".ogg",
        ".aac", ".m4a", ".wma", ".ape",
        ".mkv", ".mp4", ".avi"
    };

    // 如果是文件夹, 那么内部内容都加入到一个新建歌单, 歌单名称为: 文件夹名称
public:
    explicit UploadListModel(QObject* p = nullptr)
        : QAbstractListModel{p}
    {
        // 连接上传文件信号
        connect(
            &SignalBusSingleton::get(),
            &SignalBusSingleton::uploadFileSignal,
            this,
            [this](QString const& path, uint64_t playlistId) {
                uploadFile(path, playlistId);
            }
        );

        // 连接检查上传任务信号
        connect(
            this,
            &UploadListModel::startUploadTaskSignal,
            this,
            [this](int idx) {
                // 开始任务
                if (idx == -1) {
                    return;
                }
                // 主线程安全递增（原子）
                if (_uploadingTaskCnt.load(std::memory_order_relaxed) >= NetSingleton::CliCnt) {
                    MessageController::get().show<MsgType::Warning>("上传队列已满, 请等待");
                    return; // 队列满了, 继续等待
                }
                _uploadingTaskCnt.fetch_add(1, std::memory_order_relaxed);
                auto& data = _files[idx];
                data.uploadStatus = UploadStatus::Uploading;
                Q_EMIT dataChanged(
                    index(idx),
                    index(idx),
                    {UploadStatusRole}
                );
                namespace fs = std::filesystem;
                // 初始化任务
                MusicApi::initUploadMusic(
                    data.path.string(),
                    data.basePath == fs::path {}
                        ? data.path.filename()
                        : fs::relative(data.path, data.basePath.parent_path()) // 取相对路径
                ).thenTry([this, idx](container::Try<std::string> t) {
                    auto& _data = _files[idx];
                    if (!t) [[unlikely]] {
                        if (_data.taskUuid.empty()) {                        
                            MessageController::get().show<MsgType::Error>("初始化上传任务失败:" + t.what());
                            // 同步进度
                            QMetaObject::invokeMethod(
                                QCoreApplication::instance(),
                                [this, idx, errMsg = t.what()] {
                                    auto& _data = _files[idx];
                                    _data.uploadStatus = UploadStatus::Error;
                                    _data.errMsg = QString::fromStdString(errMsg);
                                    Q_EMIT dataChanged(
                                        index(idx),
                                        index(idx),
                                        {ErrMsgRole, UploadStatusRole}
                                    );
                                }
                            );
                            // 子线程安全递减
                            _uploadingTaskCnt.fetch_sub(1, std::memory_order_relaxed);
                            Q_EMIT startUploadTaskSignal(findTopWaitingTask());
                            t.rethrow();
                        }
                    } else {
                        _data.taskUuid = t.move(); // 记录 uuid
                    }
                    // 上传任务
                    return MusicApi::uploadMusic(
                        _data.path,
                        _data.taskUuid,
                        [this, idx](std::size_t all, double progress, std::size_t uploadSpeed) {
                            auto& _data = _files[idx];
                            // 同步进度
                            QMetaObject::invokeMethod(
                                QCoreApplication::instance(),
                                [this, all, progress, uploadSpeed, idx] {
                                    auto& _data = _files[idx];
                                    _data.progress = progress * 100;
                                    _data.uploadSpeed = uploadSpeed;
                                    _data.nowUploadSize = all;
                                    Q_EMIT dataChanged(
                                        index(idx),
                                        index(idx),
                                        {ProgressRole, UploadSpeedRole, NowUploadSizeRole}
                                    );
                                }
                            );
                            return _data.isStop->load();
                        }
                    ).thenTry([this, idx](container::Try<uint64_t> t) {
                        auto& _data = _files[idx];
                        // 子线程安全递减
                        _uploadingTaskCnt.fetch_sub(1, std::memory_order_relaxed);
                        if (!t) [[unlikely]] {
                            QMetaObject::invokeMethod(
                                QCoreApplication::instance(),
                                [this, idx, msg = t.what()] {
                                    auto& _data = _files[idx];
                                    if (msg == "stop") {
                                        _data.uploadStatus = UploadStatus::Stoped;
                                    } else {
                                        _data.uploadStatus = UploadStatus::Error;
                                        _data.errMsg = QString::fromStdString(msg);
                                    }
                                    Q_EMIT dataChanged(
                                        index(idx),
                                        index(idx),
                                        {UploadStatusRole, ErrMsgRole}
                                    );
                                }
                            );
                            return;
                        }
                        QMetaObject::invokeMethod(
                            QCoreApplication::instance(),
                            [this, idx] {
                                auto& _data = _files[idx];
                                _data.uploadStatus = UploadStatus::UploadCompleted;
                                Q_EMIT dataChanged(
                                    index(idx),
                                    index(idx),
                                    {ProgressRole, UploadSpeedRole, UploadStatusRole}
                                );
                            }
                        );
                        Q_EMIT startUploadTaskSignal(findTopWaitingTask());
                        if (!_data.addToPlaylistId) {
                            // id 为空, 啥歌单也不用添加
                            return;
                        }
                        // 获取到歌曲id
                        PlaylistApi::addMusic(
                            _data.addToPlaylistId, t.move()
                        ).thenTry([](auto t) {
                            if (!t) [[unlikely]] {
                                MessageController::get().show<MsgType::Error>(
                                    "添加到歌单失败: " + t.what()
                                );
                            }
                        });
                    });
                });
            }
        );
    }

    // 行数
    int rowCount(const QModelIndex& parent = QModelIndex()) const override {
        if (parent.isValid()) {
            return 0;
        }
        return _files.size();
    }

    // 数据获取
    QVariant data(const QModelIndex& index, int role) const override {
        if (!index.isValid() || index.row() < 0 || index.row() >= _files.size())
            return {};

        const auto& file = _files.at(index.row());
        switch (role) {
        case NameRole: return file.fileName;
        case PathRole: return QString::fromStdString(file.path);
        case ProgressRole: return file.progress;
        case UploadSpeedRole: return static_cast<qlonglong>(file.uploadSpeed);
        case PlaylistIdRole: return static_cast<qlonglong>(file.addToPlaylistId);
        case UploadStatusRole: return file.uploadStatus;
        case ErrMsgRole: return file.errMsg;
        case NowUploadSizeRole: return static_cast<qlonglong>(file.nowUploadSize);
        case TotalSizeRole: return static_cast<qlonglong>(file.totalSize);
        default: return {};
        }
    }

    // 角色名称
    QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles;
        roles[NameRole] = "name";
        roles[PathRole] = "path";
        roles[ProgressRole] = "progress";
        roles[UploadSpeedRole] = "uploadSpeed";
        roles[PlaylistIdRole] = "playlistId";
        roles[UploadStatusRole] = "uploadStatus";
        roles[ErrMsgRole] = "errMsg";
        roles[NowUploadSizeRole] = "nowUploadSize";
        roles[TotalSizeRole] = "totalSize";
        return roles;
    }

    /**
     * @brief 上传文件
     * @param path 上传文件的本地路径
     * @param playlistId 上传后添加到的歌单Id, 如果为 0, 表示不自动添加到歌单
     * @return Q_INVOKABLE 
     */
    Q_INVOKABLE void uploadFile(QString const& path, uint64_t playlistId) {
        namespace fs = std::filesystem;
        fs::path file{path.toStdString()};
        if (fs::is_directory(file)) {
            // 新建一个新的歌单
            utils::traverseDirectory(file, {},
                [this, file, playlistId](std::filesystem::path relativePath) {
                addFileIfAudio(
                    file / relativePath,
                    file,
                    playlistId,
                    QString::fromStdString(file.filename().string() + " > " + relativePath.filename().string())
                );
            });
        } else {
            // 仅上传文件
            auto name = file.filename();
            addFileIfAudio(
                std::move(file),
                {},
                playlistId,
                QString::fromStdString(std::move(name))
            );
        }
    }

    Q_INVOKABLE void stopTask(int idx) {
        if (idx < 0 || idx >= _files.size()) [[unlikely]] {
            return;
        }
        _files[idx].isStop->store(true);
    }

    Q_INVOKABLE void resumeTask(int idx) {
        if (idx < 0 || idx >= _files.size()) [[unlikely]] {
            return;
        }
        _files[idx].isStop->store(false);
        Q_EMIT startUploadTaskSignal(idx);
    }

Q_SIGNALS:
    // 检查上传任务信号
    void startUploadTaskSignal(int idx);

private:
    std::vector<UploadFileData> _files;         // 上传队列
    std::atomic_int8_t _uploadingTaskCnt = 0;   // 处于上传中的任务计数
    
    void addFileIfAudio(
        std::filesystem::path path,
        std::filesystem::path basePath,
        uint64_t playlistId,
        QString name
    ) {
        if (auto extension = path.extension().string(); _supportedAudioExt.find(extension) == _supportedAudioExt.end()) {
            MessageController::get().show<MsgType::Error>("不支持该拓展名: " + extension);
            return;
        }
        beginInsertRows(QModelIndex(), _files.size(), _files.size());
        auto tSize= utils::FileUtils::getFileSize(path.string());
        _files.push_back({
            std::move(name),
            std::move(path),
            std::move(basePath),
            "",
            0.0,
            0,
            playlistId,
            UploadStatus::Waiting,
            "",
            0,
            tSize
        });
        endInsertRows();
        Q_EMIT startUploadTaskSignal(findTopWaitingTask());
    }

    /**
     * @brief 查找第一个正在等待的任务
     * @return int -1 是找不到
     */
    int findTopWaitingTask() {
        int idx = 0;
        for (auto& v : _files) {
            if (v.uploadStatus == UploadStatus::Waiting) {
                return idx;
            }
            ++idx;
        }
        return -1;
    }
};

} // namespace HX