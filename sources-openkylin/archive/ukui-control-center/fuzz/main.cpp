/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#include <gtest/gtest.h>

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include <QCoreApplication>
#include <QTemporaryFile>

#include "../registeredSession/plugins/wallpaper/xmlhandle.h"

static bool runningUnderAfl()
{
    // AFL++ sets these when launching the instrumented target
    return getenv("__AFL_SHM_ID")
            || getenv("__AFL_SHM_FUZZ_ID")
            || getenv("AFL_MAP_SIZE")
            || getenv("AFL_CUSTOM_INFO_PROGRAM");
}

static int aflHarness(int argc, char **argv)
{
    // Avoid full gtest suite under AFL: DBus/GSettings tests hang or exceed dry-run timeout.
    QCoreApplication app(argc, argv);

    QByteArray data;
    char buf[4096];
    ssize_t n;
    while ((n = read(STDIN_FILENO, buf, sizeof(buf))) > 0) {
        data.append(buf, static_cast<int>(n));
        // Cap input size to keep each exec bounded
        if (data.size() > 1024 * 1024) {
            break;
        }
    }

    QTemporaryFile tmp(QStringLiteral("/tmp/ukcc-fuzz-XXXXXX.xml"));
    if (tmp.open()) {
        tmp.write(data);
        tmp.flush();
        XmlHandle handle;
        handle.xmlreader(tmp.fileName());
        (void)handle.requireXmlData();
    }

    return 0;
}

int main(int argc, char **argv)
{
    if (runningUnderAfl()) {
        return aflHarness(argc, argv);
    }

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
