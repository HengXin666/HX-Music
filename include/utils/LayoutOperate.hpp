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
#ifndef _HX_LAYOUT_OPERATE_H_
#define _HX_LAYOUT_OPERATE_H_

#include <QWidget>
#include <QLayout>

namespace HX {

struct LayoutOperate {
    /**
     * @brief 设置该布局内所有的控件是否隐藏
     * @param layout 布局
     * @param hidden true: 隐藏; false: 显示
     */
    inline static void setHidden(QLayout* layout, bool hidden) {
        if (!layout) [[unlikely]] {
            return;
        }
        const int len = layout->count();
        for (int i = 0; i < len; ++i) {
            auto* item = layout->itemAt(i);
            if (item && item->widget()) {
                item->widget()->setHidden(hidden);
            }
        }
    }
};

} // namespace HX

#endif // !_HX_LAYOUT_OPERATE_H_