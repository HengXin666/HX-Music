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

#include <QObject>
#include <QColor>

#include <HXLibs/utils/FileUtils.hpp>
#include <HXLibs/reflection/json/JsonRead.hpp>
#include <HXLibs/reflection/json/JsonWrite.hpp>

#include <singleton/GlobalSingleton.hpp>
#include <singleton/NetSingleton.hpp>

namespace HX {

/**
 * @brief 快速注册为 QML 可以用成员
 * @param type 类型
 * @param name 外露的成员名称, 内部为 _##name
 * @param defaultValue 默认颜色
 */
#define HX_QML_TYPE_PROPERTY(type, name, defaultValue)                         \
private:                                                                       \
    Q_PROPERTY(type name READ name WRITE set##name NOTIFY name##Changed)       \
public:                                                                        \
    type name() const noexcept {                                               \
        return _##name;                                                        \
    }                                                                          \
    void set##name(type const& color) noexcept {                               \
        if (_##name != color) {                                                \
            _##name = color;                                                   \
            Q_EMIT name##Changed();                                            \
        }                                                                      \
    }                                                                          \
Q_SIGNALS:                                                                     \
    void name##Changed();                                                      \
private:                                                                       \
    type _##name = defaultValue

/**
 * @brief 快速注册为 QML 可以用 QColor 成员
 * @param name 外露的成员名称, 内部为 _##name
 * @param defaultValue 默认颜色
 */
#define HX_QML_QCOLOR_PROPERTY(name, defaultValue)                             \
    HX_QML_TYPE_PROPERTY(QColor, name, defaultValue)

/**
 * @brief 主题配置
 */
class Theme : public QObject {
    Q_OBJECT

    struct ThemeConfig {
        std::string textColor;
        std::string paratextColor;
        std::string highlightingColor;
        std::string backgroundColor;
        std::string backgroundImgUrl;
    };
public:
    Theme(QObject* p = nullptr)
        : QObject{p}
    {
        try {
            ThemeConfig config;
            coroutine::EventLoop loop;
            utils::AsyncFile file{loop};
            file.syncOpen("./themeConfig.json", utils::OpenMode::Read);
            reflection::fromJson(config, file.syncReadAll());
            file.syncClose();

            _textColor = QColor{QString::fromStdString(config.textColor)};
            _paratextColor = QColor{QString::fromStdString(config.paratextColor)};
            _highlightingColor = QColor{QString::fromStdString(config.highlightingColor)};
            _backgroundColor = QColor{QString::fromStdString(config.backgroundColor)};
            _backgroundImgUrl = QString::fromStdString(config.backgroundImgUrl);

            Q_EMIT textColorChanged();
            Q_EMIT paratextColorChanged();
            Q_EMIT highlightingColorChanged();
            Q_EMIT backgroundColorChanged();
            Q_EMIT backgroundImgUrlChanged();
        } catch (std::exception const& e) {
            log::hxLog.error("错误:", e.what());
        }

        NetSingleton::get().setBackendUrl(
            GlobalSingleton::get().musicConfig.backendUrl
        );
    }

    Q_INVOKABLE QString getBackendUrl() const noexcept {
        return QString::fromStdString(NetSingleton::get().getBackendUrl());
    }

    Q_INVOKABLE void setBackendUrl(QString const& url) {
        NetSingleton::get().setBackendUrl(
            GlobalSingleton::get().musicConfig.backendUrl = url.toStdString()
        );
    }

    ~Theme() noexcept {
        coroutine::EventLoop loop;
        utils::AsyncFile file{loop};
        file.syncOpen("./themeConfig.json", utils::OpenMode::Write);
        std::string json;
        reflection::toJson<true, ThemeConfig>({
            _textColor.name(QColor::HexArgb).toStdString(),
            _paratextColor.name(QColor::HexArgb).toStdString(),
            _highlightingColor.name(QColor::HexArgb).toStdString(),
            _backgroundColor.name(QColor::HexArgb).toStdString(),
            _backgroundImgUrl.toStdString()
        }, json);
        file.syncWrite(json);
        file.syncClose();
    }

    // 主色系
    HX_QML_QCOLOR_PROPERTY(textColor                , "#ffffff"); // 文本颜色
    HX_QML_QCOLOR_PROPERTY(paratextColor            , "#b3b3b3"); // 副文本颜色
    HX_QML_QCOLOR_PROPERTY(highlightingColor        , "#ff13ff"); // 高亮颜色

    // 悬浮色系
    
    // 背景色
    HX_QML_QCOLOR_PROPERTY(backgroundColor          , "#121212");

    // 背景图片
    HX_QML_TYPE_PROPERTY(QString, backgroundImgUrl  , "qrc:/img/background.jpg");
    
    // 默认背景图片路径, 不可修改
    HX_QML_TYPE_PROPERTY(QString, defaultBackgroundImgUrl  , "qrc:/img/background.jpg");
};

#undef HX_QML_QCOLOR_PROPERTY
#undef HX_QML_TYPE_PROPERTY

} // namespace HX
