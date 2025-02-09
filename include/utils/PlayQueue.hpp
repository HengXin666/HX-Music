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
#ifndef _HX_PLAY_QUEUE_H_
#define _HX_PLAY_QUEUE_H_

#include <QString>
#include <QVariant>

#include <utils/FileTree.hpp>

namespace HX {

class PlayQueue : public FileTree<QString> {
public:
    using ItOpt = typename std::optional<FileTree<QString>::iterator>;
    using Node = typename FileTree<QString>::Tp;

    explicit PlayQueue()
        : FileTree<QString>()
    {}
};

} // namespace HX

Q_DECLARE_METATYPE(HX::PlayQueue::iterator);

#endif // !_HX_PLAY_QUEUE_H_