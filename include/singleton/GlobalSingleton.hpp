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
#ifndef _HX_GLOBAL_SINGLETON_H_
#define _HX_GLOBAL_SINGLETON_H_

#include <views/InterfaceManagementProxy.h>

/**
 * @brief 全局单例
 */
struct GlobalSingleton {
    inline static GlobalSingleton& get() {
        static GlobalSingleton s{};
        return s;
    }

    /// @brief 界面管理代理类
    InterfaceManagementProxy imp;
private:
    GlobalSingleton() = default;
    GlobalSingleton& operator=(GlobalSingleton&&) = default;
};

#endif // !_HX_GLOBAL_SINGLETON_H_
