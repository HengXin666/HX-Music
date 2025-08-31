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

#include <taglib/fileref.h>
#include <taglib/flacfile.h>
#include <taglib/tpropertymap.h> // PropertyMap

#include <HXLibs/utils/FileUtils.hpp>

namespace HX {

namespace internal {

// 去掉首尾空格
inline std::string trim(const std::string& s) {
    auto begin = s.find_first_not_of(" \t\n\r");
    if (begin == std::string::npos)
        return "";
    auto end = s.find_last_not_of(" \t\n\r");
    return s.substr(begin, end - begin + 1);
}

// 字符串替换 (大小写不敏感的版本只针对 feat./ft. 简单实现)
inline void replaceAllInsensitive(
    std::string& s,
    const std::string& from,
    const std::string& to
) {
    std::string lower = s;
    std::string fromLower = from;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    std::transform(fromLower.begin(), fromLower.end(), fromLower.begin(), ::tolower);

    size_t pos = 0;
    while ((pos = lower.find(fromLower, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        lower.replace(pos, from.size(), to);
        pos += to.size();
    }
}

inline void replaceAll(std::string& s, const std::string& from, const std::string& to) {
    size_t pos = 0;
    while ((pos = s.find(from, pos)) != std::string::npos) {
        s.replace(pos, from.size(), to);
        pos += to.size();
    }
}

} // namespace internal

struct MusicInfo {
    explicit MusicInfo(std::filesystem::path path)
        : _path{std::move(path)}
        , _mpegFile{_path.c_str()}
    {}

    struct ImgRamFile {
        std::string type;
        std::string buf;
    };

    /**
     * @brief 获取音频标题
     * @return std::string `获取失败`则返回文件名
     */
    std::string getTitle() const {
        if (_mpegFile.isNull() || !_mpegFile.tag()) {
            return _path.filename().string();
        }
        auto res = _mpegFile.tag()->title().to8Bit(true);
        return res.empty() ? _path.filename().string() : res;
    }

    /**
     * @brief 获取音频歌手信息列表
     * @param errRes 
     * @return std::vector<std::string> 每一项就是一个歌手名
     */
    std::vector<std::string> getArtistList(std::vector<std::string> errRes = {}) const {
        if (_mpegFile.isNull()) {
            return errRes;
        }
        auto* tag = _mpegFile.tag();
        if (!tag) {
            return errRes;
        }
        
        const TagLib::PropertyMap& map = tag->properties();
        if (!map.contains("ARTIST")) {
            return errRes;
        }
        const auto& values = map["ARTIST"];
        if (values.size() > 1) {
            std::vector<std::string> res;
            res.reserve(values.size());
            for (const auto& val : values) {
                res.emplace_back(val.to8Bit(true));
            }
            return res;
        }
        std::string raw = values.front().to8Bit(true);
    
        // 启发式分隔
        internal::replaceAllInsensitive(raw, " feat. ", ";");
        internal::replaceAllInsensitive(raw, " ft. ", ";");
        internal::replaceAll(raw, "、", ";");
        internal::replaceAll(raw, "／", ";");
        
        std::vector<std::string> res;
        size_t start = 0;
        while (true) {
            size_t pos = raw.find(';', start);
            std::string part = raw.substr(start, pos - start);
            part = internal::trim(part);
            if (!part.empty()) {
                res.push_back(part);
            }
            if (pos == std::string::npos) {
                break;
            }
            start = pos + 1;
        }
        return res;
    }

    /**
     * @brief 获取音频专辑信息
     * @param errRes 获取失败返回的值
     * @return QString
     */
    std::string getAlbum(std::string errRes = "") const {
        if (_mpegFile.isNull() || !_mpegFile.tag()) {
            return errRes;
        }
        return _mpegFile.tag()->album().to8Bit(true);
    }

    /**
     * @brief 获取音频的总毫秒数
     * @param errRes 获取失败返回的值
     * @return int
     */
    int getLengthInMilliseconds(int errRes = 0) const {
        if (_mpegFile.isNull() || !_mpegFile.audioProperties()) {
            return errRes;
        }
        return _mpegFile.audioProperties()->lengthInMilliseconds();
    }

    /**
     * @brief 尝试解析封面
     * @return std::optional<ImgRamFile>
     */
    std::optional<ImgRamFile> getAlbumArtAdvanced() const {
        auto* baseFile = _mpegFile.file();
        if (!baseFile) {
            return {};
        }

        // 1) MP3 / MP4 via FileRef complex props (APIC, covr atoms)
        TagLib::StringList keys;
        if (auto* tag = baseFile->tag()) {
            keys = tag->complexPropertyKeys();
        }

        for (const auto& key : keys) {
            auto props = baseFile->tag()->complexProperties(key);
            for (const auto& m : props) {
                for (const auto& p : m) {
                    if (p.second.type() == TagLib::Variant::ByteVector) {
                        auto bv = p.second.value<TagLib::ByteVector>();
                        if (bv.isEmpty())
                            continue;
                        ImgRamFile res{
                            {},
                            {bv.data(), bv.size()}
                        };
                        auto type = m.value("MIME", {}).toString();
                        if (type.isEmpty()) {
                            res.type = m.contains("png") ? "png" : "jpg";
                        } else {
                            res.type = type.to8Bit(true);
                        }
                        return res;
                    }
                }
            }
        }

        // 2) FLAC / Ogg / Opus: Vorbis picture blocks
        if (auto* flac = dynamic_cast<TagLib::FLAC::File*>(baseFile)) {
            auto pics = flac->pictureList();
            if (!pics.isEmpty()) {
                auto& pic = pics.front();
                auto bv = pic->data();
                if (!bv.isEmpty()) {
                    return ImgRamFile{
                        pic->mimeType().to8Bit(true),
                        {bv.data(), bv.size()}
                    };
                }
            }
        }

        return {};
    }

    std::string filePath() const {
        return _path.string();
    }
private:
    std::filesystem::path _path;
    TagLib::FileRef _mpegFile;
};

} // namespace HX