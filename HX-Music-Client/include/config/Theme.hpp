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
public:
    Theme(QObject* p = nullptr)
        : QObject{p}
    {}

    // 主色系
    HX_QML_QCOLOR_PROPERTY(textColor                , "#ffffff"); // 文本颜色
    HX_QML_QCOLOR_PROPERTY(paratextColor            , "#b3b3b3"); // 副文本颜色
    HX_QML_QCOLOR_PROPERTY(highlightingColor        , "#ff13ff"); // 高亮颜色

    // 悬浮色系
    
    // 背景色
    HX_QML_QCOLOR_PROPERTY(backgroundColor          , "#121212");

    // 背景图片, 默认为空, 应该使用背景色
    HX_QML_TYPE_PROPERTY(QString, backgroundImgUrl  , "qrc:/img/background.jpg");
};

#undef HX_QML_QCOLOR_PROPERTY
#undef HX_QML_TYPE_PROPERTY

} // namespace HX

