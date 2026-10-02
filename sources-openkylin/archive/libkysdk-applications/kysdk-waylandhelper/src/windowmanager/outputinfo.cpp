/*
 * libkysdk-waylandhelper's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "outputinfo.h"

class Q_DECL_HIDDEN OutputInfo::Private
{
public:
    Private(OutputInfo *q);
    ~Private();

    QRect m_usableArea;
    QString m_name;
    OutputInfo *q;
};

OutputInfo::OutputInfo(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

void OutputInfo::setOutputName(const QString &outputName)
{
    d->m_name = outputName;
}

void OutputInfo::setUsableArea(const QRect &usableArea)
{
    d->m_usableArea = usableArea;
}

OutputInfo::~OutputInfo()
{
}

QString OutputInfo::outputName()
{
    return d->m_name;
}

QRect OutputInfo::usableArea()
{
    return d->m_usableArea;
}

OutputInfo::Private::Private(OutputInfo *q)
    : q(q)
{
}

OutputInfo::Private::~Private()
{
}
