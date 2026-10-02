/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "screensaverwndadaptor.h"

#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

ScreenSaverWndAdaptor::ScreenSaverWndAdaptor(FullBackgroundWidget *parent)
    : QDBusAbstractAdaptor(parent), m_parentWidget(parent)
{
    // constructor
    setAutoRelaySignals(true);

    connect(m_parentWidget, SIGNAL(StartupModeChanged(bool)), this, SIGNAL(StartupModeChanged(bool)));
}

ScreenSaverWndAdaptor::~ScreenSaverWndAdaptor() {}

int ScreenSaverWndAdaptor::RegisteSubWnd(quint64 uWndId)
{
    int nWndCount = m_parentWidget->RegisteSubWnd(uWndId);
    if (nWndCount >= 0) {
        Q_EMIT SubWndChanged(nWndCount);
    }
    return nWndCount;
}

int ScreenSaverWndAdaptor::UnRegisteSubWnd(quint64 uWndId)
{
    int nWndCount = m_parentWidget->UnRegisteSubWnd(uWndId);
    if (nWndCount >= 0) {
        Q_EMIT SubWndChanged(nWndCount);
    }
    return nWndCount;
}

QList<quint64> ScreenSaverWndAdaptor::GetSubWndIds()
{
    return m_parentWidget->GetSubWndIds();
}

bool ScreenSaverWndAdaptor::IsStartupMode()
{
    return m_parentWidget->IsStartupMode();
}
