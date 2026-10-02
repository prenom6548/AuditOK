/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef AUTOTEST_FAKE_LIBKYPACKAGES_H
#define AUTOTEST_FAKE_LIBKYPACKAGES_H

#include <QByteArray>

extern "C" {
bool kdk_package_is_removable_by_desktop(const char *desktop_file);
}

namespace FakeKdkPackageStore {
void reset();
void setRemovable(const QByteArray &desktopFile, bool removable);
}

#endif
