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

#include "ukuidesktopportal.h"
#include "ukuifilechooserportal.h"
#include "ukuisessioninhibitportal.h"
#include "notification-portal.h"
#include "appchooser/appchooser.h"
#include "print.h"


UKUIDesktopPortal::UKUIDesktopPortal(QObject *parent) : QObject(parent)
                                                      , m_fileChooser(new FileChooserPortal(this))
                                                      , m_inhibit(new InhibitPortal(this))
                                                      , m_notification(new NotificationPortal(this))
                                                      , m_appChooser(new AppChooserPortal(this))
                                                      , m_printPortal(new PrintPortal(this))
{
}
