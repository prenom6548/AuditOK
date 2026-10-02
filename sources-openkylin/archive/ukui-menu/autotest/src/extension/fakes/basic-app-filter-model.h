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

#ifndef AUTOTEST_EXTENSION_FAKE_BASIC_APP_FILTER_MODEL_H
#define AUTOTEST_EXTENSION_FAKE_BASIC_APP_FILTER_MODEL_H

#include "../../../src/data-entity.h"

namespace UkuiMenu {

class AppDatabaseInterface;

class BasicAppFilterModel
{
public:
    static BasicAppFilterModel *instance();

    const AppDatabaseInterface *databaseInterface() const;
    DataEntity appOfIndex(int row) const;
    int indexOfApp(const QString &appid) const;
    bool getAppById(const QString &appid, DataEntity &app) const;
};

} // namespace UkuiMenu

#endif
