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
#ifndef _HX_SVG_ICON_ENGINE_H_
#define _HX_SVG_ICON_ENGINE_H_

#include <QIconEngine>
#include <QSvgRenderer>
#include <QPainter>
#include <QColor>
#include <QHash>

namespace internal {

struct HashFun {
    std::size_t operator()(QColor const &c) const noexcept {
        return std::hash<unsigned int>{}(c.rgba());
    }
};

} // namespace internal

class SvgIconEngine : public QIconEngine {
    SvgIconEngine& operator=(SvgIconEngine&&) = delete;
public:
    // 构造时传入SVG文件路径
    explicit SvgIconEngine(const QString& svgPath, const QSize& size)
        : _renderer(svgPath)
        , _size(size) 
    {
        
    }

    explicit SvgIconEngine(SvgIconEngine const& o)
    {}

    // 允许动态设置颜色, 设置颜色后清空缓存
    void setSize(const QSize& size) {
        if (_size != size) {
            _size = size;
            _cache.clear();
        }
    }

    // 重写 pixmap() 方法, 根据尺寸和当前颜色返回带颜色的图标
    QPixmap switchColor(const QColor& color) {
        // 利用尺寸作为缓存键
        if (auto it = _cache.find(color); it != _cache.end())
            return it->second;

        QPixmap pixmap(_size);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        // 首先渲染原始SVG
        _renderer.render(&painter, QRectF(QPointF(0, 0), _size));
        // 然后用颜色覆盖（利用合成模式, 仅保留原图不透明区域）
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(pixmap.rect(), color);
        painter.end();

        _cache.emplace(color, pixmap);
        return pixmap;
    }

    virtual void paint(
        QPainter *painter, 
        const QRect &rect, 
        QIcon::Mode mode, 
        QIcon::State state
    ) override {
        QColor color = (mode == QIcon::Normal) ? QColor(Qt::black) : QColor(Qt::gray);
        QPixmap pix = switchColor(color);
        painter->drawPixmap(rect, pix);
    }
    
    virtual QIconEngine *clone() const override {
        SvgIconEngine* engine = new SvgIconEngine(*this);
        return engine;
    }

private:
    QSvgRenderer _renderer;
    QSize _size;
    // 用于缓存不同尺寸下生成的图标，提高性能
    mutable std::unordered_map<QColor, QPixmap, internal::HashFun> _cache;
};

#endif // !_HX_SVG_ICON_ENGINE_H_