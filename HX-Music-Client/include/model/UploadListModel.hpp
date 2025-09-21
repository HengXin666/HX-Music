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
        TaskUuidRole,
        ProgressRole,
        UploadSpeedRole,
        PlaylistIdRole,
        UploadStatusRole
    };

    enum UploadStatus {
        Waiting,
        Uploading,
        uploadCompleted
    };

    struct UploadFileData {
        QString fileName;           // 文件名
        QString path;               // 文件路径
        std::string taskUuid;       // 任务uuid
        double progress;            // 上传进度百分比
        std::size_t uploadSpeed;    // 上传速度 (B / s)
        uint64_t addToPlaylistId;   // 需要添加的歌单id
        UploadStatus uploadStatus;  // 上传状态
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
        case PathRole: return file.path;
        case TaskUuidRole: return QString::fromStdString(file.taskUuid);
        case ProgressRole: return file.progress;
        case UploadSpeedRole: return static_cast<qlonglong>(file.uploadSpeed);
        case PlaylistIdRole: return static_cast<qlonglong>(file.addToPlaylistId);
        case UploadStatusRole: return file.uploadStatus;
        default: return {};
        }
    }

    // 角色名称
    QHash<int, QByteArray> roleNames() const override {
        QHash<int, QByteArray> roles;
        roles[NameRole] = "name";
        roles[PathRole] = "path";
        roles[TaskUuidRole] = "taskUuid";
        roles[ProgressRole] = "progress";
        roles[UploadSpeedRole] = "uploadSpeed";
        roles[PlaylistIdRole] = "playlistId";
        roles[UploadStatusRole] = "uploadStatus";
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
                    playlistId,
                    QString::fromStdString(file.filename().string() + " > " + relativePath.filename().string())
                );
            });
        } else {
            // 仅上传文件
            auto name = file.filename();
            addFileIfAudio(
                std::move(file),
                playlistId,
                QString::fromStdString(std::move(name))
            );
        }
    }

    // emit dataChanged(idx, idx, {ProgressRole, UploadSpeedRole});
private:
    QVector<UploadFileData> _files; // 上传队列
    
    void addFileIfAudio(std::filesystem::path path, uint64_t playlistId, QString name) {
        if (auto extension = path.extension().string(); _supportedAudioExt.find(extension) == _supportedAudioExt.end()) {
            MessageController::get().show<MsgType::Error>("不支持该拓展名: " + extension);
            return;
        }
        beginInsertRows(QModelIndex(), _files.size(), _files.size());
        _files.push_back({
            std::move(name),
            QString::fromStdString(path.string()),
            "",
            0.0,
            0,
            playlistId,
            UploadStatus::Waiting
        });
        endInsertRows();
    }
};

} // namespace HX