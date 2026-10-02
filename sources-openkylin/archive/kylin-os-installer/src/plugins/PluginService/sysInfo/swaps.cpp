/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "swaps.h"
#include "../base_unit/file_unit.h"

namespace KServer {

SwapItemList getprocSwaps()
{
    SwapItemList swapItemList = {};
    const QString strSwap(KReadFile("/proc/swaps"));

    for(const QString& items : strSwap.split('\n')) {
        if((!items.isEmpty()) && (!items.startsWith("Filename"))) {
            const QStringList proc(items.split(QRegExp("\\s+")));
            if(proc.length() == 5) {
                SwapItem data = {
                    proc.at(0),
                    proc.at(1) == "partition" ? SwapType::Partiton : SwapType::File,
                    proc.at(2).toLongLong() * 1000,
                    proc.at(3).toLongLong() * 1000,
                    proc.at(4).toInt(),
                };
                swapItemList.append(data);
            }
        }
    }
    return (SwapItemList)swapItemList;
}

}
