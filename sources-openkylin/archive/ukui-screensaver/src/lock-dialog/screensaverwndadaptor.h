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
#ifndef SCREENSAVERWNDADAPTOR_H
#define SCREENSAVERWNDADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "fullbackgroundwidget.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template <class T>
class QList;
template <class Key, class Value>
class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

class ScreenSaverWndAdaptor : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ScreenSaverWnd")
    Q_CLASSINFO(
        "D-Bus Introspection",
        ""
        "  <interface name=\"org.ukui.ScreenSaverWnd\">\n"
        "    <signal name=\"SubWndChanged\"/>\n"
        "    <signal name=\"StartupModeChanged\"/>\n"
        "    <method name=\"RegisteSubWnd\">\n"
        "      <arg name=\"wndid\" direction=\"in\" type=\"t\"/>\n"
        "      <arg name=\"result\" direction=\"out\" type=\"i\"/>\n"
        "    </method>\n"
        "    <method name=\"UnRegisteSubWnd\">\n"
        "      <arg name=\"wndid\" direction=\"in\" type=\"t\"/>\n"
        "      <arg name=\"result\" direction=\"out\" type=\"i\"/>\n"
        "    </method>\n"
        "    <method name=\"GetSubWndIds\">\n"
        "      <arg name=\"result\" direction=\"out\" type=\"at\"/>\n"
        "    </method>\n"
        "    <method name=\"IsStartupMode\">\n"
        "      <arg name=\"result\" direction=\"out\" type=\"b\"/>\n"
        "    </method>\n"
        "  </interface>\n"
        "")
public:
    ScreenSaverWndAdaptor(FullBackgroundWidget *parent = nullptr);
    virtual ~ScreenSaverWndAdaptor();

public Q_SLOTS:
    int RegisteSubWnd(quint64 uWndId);
    int UnRegisteSubWnd(quint64 uWndId);
    QList<quint64> GetSubWndIds();
    bool IsStartupMode();

Q_SIGNALS:
    void SubWndChanged(int nCount);
    void StartupModeChanged(bool isStartup);

private:
    FullBackgroundWidget *m_parentWidget = nullptr;
};

#endif // SCREENSAVERWNDADAPTOR_H
