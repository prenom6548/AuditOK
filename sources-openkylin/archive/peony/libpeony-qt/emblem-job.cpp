/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "emblem-job.h"
#include "emblem-provider.h"

#include <QDebug>

using namespace Peony;

EmblemJob::EmblemJob(QStringList &list)
{
    setAutoDelete(true);
    // 相当于移动构造，避免大量文件导致的卡顿，同时也清空了当前的queryQueue，避免重复查询
    m_list.swap(list);
}

void EmblemJob::run()
{
    auto manager = EmblemProviderManager::getInstance();
    
    for (auto uri : m_list) {
        if (manager->shouldStop()) {
            qDebug() << "EmblemJob: interrupted, stopping emblem query";
            return;
        }
        
        manager->querySync(uri);
    }

    if (!manager->shouldStop()) {
        Q_EMIT manager->queueQueryFinished();
    }
}
