/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2026 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef WAYLANDFUNCMAPPER_H
#define WAYLANDFUNCMAPPER_H

#include <QMap>
#include <QHash>
#include <QString>

namespace FuncMapper {
    extern QMap<uint, QString> id2name;
    extern QHash<QString, uint> name2id;

    void init();
    QString funcName(uint id, const QString& def = "none");
    uint funcId(const QString& name, uint def = 0);
    QString getGifPathByFuncId(uint id);
}

#endif // WAYLANDFUNCMAPPER_H
