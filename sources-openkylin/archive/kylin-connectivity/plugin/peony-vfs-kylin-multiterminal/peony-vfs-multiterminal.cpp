/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 * Authors: huheng <huheng@kylinos.cn>
 *
 */

#include "peony-vfs-multiterminal.h"
#include "multiterminal-vfs-register.h"

#include <QTranslator>
#include <QLocale>
#include <QApplication>

Peony::MultVFSInternalPlugin *global_instance = nullptr;

PeonyVFSMult::PeonyVFSMult(QObject *parent) : QObject(parent)
{
    QTranslator *t = new QTranslator(this);
    t->load("/usr/share/kylin-connectivity/translations/peony-vfs-kylin-multiterminal_" + QLocale::system().name());
    QApplication::installTranslator(t);
}

void PeonyVFSMult::initVFS()
{
    if (!global_instance)
        global_instance = new Peony::MultVFSInternalPlugin;
    global_instance->initVFS();
}

void *PeonyVFSMult::parseUriToVFSFile(const QString &uri)
{
    if (!global_instance)
        global_instance = new Peony::MultVFSInternalPlugin;
    return global_instance->parseUriToVFSFile(uri);
}
