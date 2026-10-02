/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

#ifndef UKUIDESKTOPPORTAL_H
#define UKUIDESKTOPPORTAL_H

#include <QObject>
#include <QDBusContext>

class FileChooserPortal;
class InhibitPortal;
class NotificationPortal;
class AppChooserPortal;
class PrintPortal;

class UKUIDesktopPortal : public QObject, public QDBusContext
{
    Q_OBJECT
public:
    explicit UKUIDesktopPortal(QObject *parent = nullptr);

private:
    FileChooserPortal *const m_fileChooser;
    InhibitPortal *const m_inhibit;
    NotificationPortal *const m_notification;
    AppChooserPortal * const m_appChooser;
    PrintPortal * const m_printPortal;
};

#endif // UKUIDESKTOPPORTAL_H
