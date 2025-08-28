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

#include <QFile>
#include <QDomDocument>
#include <QSvgRenderer>
#include <QPixmap>
#include <QPainter>
#include <QIcon>
#include <QQuickImageProvider>
#include <QUrlQuery>

#include <QDebug>

namespace HX {

class [[nodiscard]] SvgPars {
public:
    explicit SvgPars(QString const& svgPath)
        : _doc()
    {
        // 读取SVG文件
        QFile file(svgPath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning("无法打开SVG文件");
            return;
        }
        QByteArray svgData = file.readAll();
        file.close();

        // 解析SVG内容
        if (!_doc.setContent(svgData)) {
            qWarning("无法解析SVG内容");
            return;
        }
    }

    /**
     * @brief 替换`标签`下的`属性`的`值`
     * @param tagName 标签名
     * @param attributeName 属性名
     * @param newAttributeValue 新的属性值
     * @return SvgPars&& 
     */
    [[nodiscard]] SvgPars&& replaceTagAndAttributeAndVal(
        const QString& tagName, 
        const QString& attributeName,
        const QString& newAttributeValue
    ) && {
        auto dfs = [&] (
            auto&& dfs, 
            QDomElement& element
        ) -> void {
            if (element.tagName() == tagName) {
                element.setAttribute(attributeName, newAttributeValue);
            }
            for (int i = 0; i < element.childNodes().count(); ++i) {
                QDomNode childNode = element.childNodes().at(i);
                if (childNode.isElement()) {
                    auto element = childNode.toElement();
                    dfs(dfs, element);
                }
            }
        };
        auto element = _doc.documentElement();
        dfs(dfs, element);
        return std::move(*this);
    }

    /**
     * @brief 渲染SVG并生成QPixmap后返回QIcon
     * @return QIcon 
     */
    [[nodiscard]] QIcon makeIcon() && {
        return QIcon(std::move(*this).makePixmap());
    }

    /**
     * @brief 渲染SVG并生成QPixmap
     * @return QPixmap 
     */
    [[nodiscard]] QPixmap makePixmap() && {
        QSvgRenderer svgRenderer(_doc.toByteArray());
        QSize iconSize = svgRenderer.defaultSize();
        QPixmap pixmap(iconSize);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        svgRenderer.render(&painter);
        return pixmap;
    }
private:
    QDomDocument _doc;
};

class QmlSvgPars : public QQuickImageProvider {
    Q_OBJECT
public:
    QmlSvgPars()
        : QQuickImageProvider{QQuickImageProvider::Image}
    {}

    // QML 调用
    QImage requestImage(
        [[maybe_unused]] QString const& id,
        [[maybe_unused]] QSize* size,
        [[maybe_unused]] QSize const& requestedSize
    ) override {
        // 解析 id: "qrc:/icons/next.svg?color=#ff0000"
        const QStringList parts = id.split("?");
        QString path = parts[0];         // qrc:/icons/next.svg
        if (path.startsWith("qrc")) [[likely]] {
            path = path.remove(0, 3); // :/icons/next.svg
        }
        QString color = "#000000";
        if (parts.size() == 2) [[likely]] {
            const QUrlQuery query(parts[1]);
            color = query.queryItemValue("color");
        }
        auto img = SvgPars{path}
            .replaceTagAndAttributeAndVal(
                "path",
                "fill",
                color)
            .makePixmap()
            .toImage();
        if (size) {
            *size = img.size();
        }
        return img;
    }
};

} // namespace HX

