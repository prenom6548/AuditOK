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

#ifndef AUTOTEST_APPLICATION_FAKE_APP_PAGE_BACKEND_H
#define AUTOTEST_APPLICATION_FAKE_APP_PAGE_BACKEND_H

#include <QObject>

namespace UkuiMenu {

class LabelItem
{
    Q_GADGET
public:
    enum Type {
        Default = 0
    };
    Q_ENUM(Type)
};

class AppListPluginGroup
{
    Q_GADGET
public:
    enum Group {
        Display = 0,
        Search
    };
    Q_ENUM(Group)
};

class AppPageBackend : public QObject
{
    Q_OBJECT
public:
    static AppPageBackend *instance();

private:
    explicit AppPageBackend(QObject *parent = nullptr);
};

} // namespace UkuiMenu

#endif
