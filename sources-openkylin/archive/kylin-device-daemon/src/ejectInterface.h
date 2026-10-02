/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#ifndef EJECTINTERFACE_H
#define EJECTINTERFACE_H
#include <QWidget>
#include <QtWidgets>
#include <QLabel>
#include <QBoxLayout>
#include <QIcon>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QPushButton>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusObjectPath>
#include <QDBusReply>
#include <qgsettings.h>
#include <windowmanager/windowmanager.h>

#ifdef KY_SDK_WAYLANDHELPER
#include <ukuiwindowhelper/ukuiwindowhelper.h>
#endif

#include "MacroFile.h"
#include "UnionVariable.h"
#include "ksoundeffects.h"
using namespace kdk;

QT_BEGIN_NAMESPACE
namespace Ui { class ejectInterface; }
QT_END_NAMESPACE
class ejectInterface : public QWidget
{
    Q_OBJECT
public:
    ejectInterface(QWidget *parent, bool forceEject = false);
    ~ejectInterface();
    int getPanelPosition(QString str);
    int getPanelHeight(QString str);
    void initTransparentState();
    void getTransparentData();
    void showEjectInterface();

private:
    QLabel *eject_image_label;
    QIcon eject_image_icon;
    QLabel *show_text_label;
    QLabel *mount_name_label;
    QHBoxLayout *ejectinterface_h_BoxLayout;
    QHBoxLayout *mountname_h_BoxLayout;
    QVBoxLayout *main_V_BoxLayput;
    QTimer *interfaceHideTime;
    QScreen *EjectScreen;

#ifdef KY_SDK_WAYLANDHELPER
    UkuiWindowHelper* m_windowHelper = nullptr;
#endif

    double m_transparency;
    int fontSize;
    QGSettings *m_transparency_gsettings = nullptr;
    QGSettings *fontSettings = nullptr;

    void moveEjectInterfaceRight();
    void getFontSize();
    void initFontSetting();

private Q_SLOTS:
    void on_interface_hide();
protected:
    void paintEvent(QPaintEvent *event);

};

#endif
