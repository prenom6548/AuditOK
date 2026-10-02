/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef TEST_MANAGER_H
#define TEST_MANAGER_H

#include <QObject>
#include <QQuickView>
#include <QRect>

class TestManager : public QObject
{
    Q_OBJECT
public:
    explicit TestManager(QObject *parent = nullptr);
    ~TestManager();

private slots:
    void testConstructor();
    void testManagerViewInitialized();
    void testManagerViewGeometry();
    void testManagerViewVisibility();

private:
    void cleanup();
};

#endif // TEST_MANAGER_H
