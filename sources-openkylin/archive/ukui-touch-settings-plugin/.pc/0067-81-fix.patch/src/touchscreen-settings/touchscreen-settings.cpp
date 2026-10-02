/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "touchscreen-settings.h"

#include <ukuisdk/kylin-com4cxx.h>

#include <string>

#include <QLocale>
#include <QTranslator>
#include <QApplication>
#include <QTouchDevice>
#include <QString>
#include <QWidget>
#include <QIcon>
#include <QDebug>
#include <QGSettings>
#include <QByteArray>
#include <QBoxLayout>

#include "touchscreen.h"
#include "gestureguidance.h"
#include "devicemonitoreventfilter.h"
#include "settingscommon.h"
#include "pluginentry.h"
#include "waylandinputdevicemonitor.h"
#include "devicehelper.h"

static const std::string PLUGINS_SCHEMA = "org.ukui.control-center.plugins";
static const std::string PLUGINS_DIR = "/org/ukui/control-center/plugins/";
static const std::string KEY_SHOW = "show";

TouchscreenSettings::TouchscreenSettings() : mFirstLoad(true)
{
    QTranslator* translator = new QTranslator(this);
    translator->load("/usr/share/touchscreen-settings/translations/" + QLocale::system().name());
    QApplication::installTranslator(translator);

    if(!env::isWayland()) {
        m_deviceMonitor = new DeviceMonitorEventFilter(this);
        qApp->installNativeEventFilter(m_deviceMonitor);
        connect(m_deviceMonitor, &DeviceMonitorEventFilter::touchScreenChanged,
                this, &TouchscreenSettings::setShow);
    } else {
        WaylandInputDeviceMonitor* waylandDeviceMonitor = new WaylandInputDeviceMonitor(this);
        connect(waylandDeviceMonitor, &WaylandInputDeviceMonitor::deviceAdded,
                this, &TouchscreenSettings::waylandInputDeviceChanged);
        connect(waylandDeviceMonitor, &WaylandInputDeviceMonitor::deviceRemoved,
                this, &TouchscreenSettings::waylandInputDeviceChanged);
    }
    setShow(isEnable());
}

TouchscreenSettings::~TouchscreenSettings()
{
    //! ukui-control-center is responsible for destroying pluginWidget
}

QString TouchscreenSettings::plugini18nName()
{
    QString pluginName;
    //! \note is same UI in mavis and laika
    if (SettingsCommon::isIntelProject())
        pluginName = tr("GestureGuidance");
    else
        pluginName = tr("TouchScreen");

    if (SettingsCommon::isTabletProductFeat()) {
        pluginName = SettingsCommon::isMaxTabletSystemCategory()
                     ? tr("MaxTablet And TouchScreen")
                     : tr("Tablet And TouchScreen");
    }

    return pluginName;
}

int TouchscreenSettings::pluginTypes()
{
    return FunType::DEVICES;
}

QWidget *TouchscreenSettings::pluginUi()
{
    if (!mFirstLoad)
        return pluginWidget;

    if (SettingsCommon::isIntelProject()) {
        pluginWidget = new GestureGuidance;
    }
    else {
        //触摸校准
        pluginWidget = new QWidget();
        PluginEntry *calibrationWidget = new PluginEntry(pluginWidget);
        QVBoxLayout* layout = new QVBoxLayout(pluginWidget);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(40);

        QWidget* touchWidget = nullptr;

        //触摸屏
        if(SettingsCommon::isMaxTabletSystemCategory()) {
            touchWidget = new TouchScreen({TouchGestureInfo(TouchGestureType::rightEdgeSwipe,
                                            tr("swipe left from the right edge"),
                                            tr("show sidebar"),
                                            ":/gif/resources/右边缘左划显示侧边栏.gif")},
                                            tr("Max Tablet Mode"),
                                            tr("auto switch max tablet mode"));
        } else {
            const static QList<TouchGestureInfo> defaultGestureTypes =
            {
                TouchGestureInfo(TouchGestureType::bottomEdgeSwipe, QObject::tr("swipe up from the bottom edge"),
                            QObject::tr("open the multitasking view"), ":/gif/resources/下边缘上划显示多任务视图.gif"),
                TouchGestureInfo(TouchGestureType::fourFingersHorizontalSwipe, QObject::tr("swipe four fingers horizontally"),
                            QObject::tr("switch windows"), ":/gif/resources/四指水平滑动显示窗口切换.gif"),
                TouchGestureInfo(TouchGestureType::fourFingersSwipeDown, QObject::tr("swipe four fingers down anywhere"),
                            QObject::tr("show global search"), ":/gif/resources/四指向下划动显示搜索.gif"),
                TouchGestureInfo(TouchGestureType::rightEdgeSwipe, QObject::tr("swipe left from the right edge"),
                            QObject::tr("show sidebar"), ":/gif/resources/右边缘左划显示侧边栏.gif")

            };
            //~ contents_path /TouchScreen/Automatically switch tablet mode when plugging and unplugging keyboard
            tr("Automatically switch tablet mode when plugging and unplugging keyboard");
            touchWidget = new TouchScreen(defaultGestureTypes, tr("Tablet Mode"), tr("Automatically switch tablet mode when plugging and unplugging keyboard"), pluginWidget);
        }
        layout->addWidget(touchWidget);
        layout->addWidget(calibrationWidget->widget());
        layout->addStretch();
    }

    mFirstLoad = false;
    pluginWidget->adjustSize();
    pluginWidget->setAttribute(Qt::WA_DeleteOnClose);


//    if (mFirstLoad) {
//        mFirstLoad = false;
//        pluginWidget = new TouchScreen;
//        pluginWidget->adjustSize();
//        pluginWidget->setAttribute(Qt::WA_DeleteOnClose);
//    }
    return pluginWidget;
}

bool TouchscreenSettings::isEnable() const
{
    return !DeviceHelper::getTouchDevices().isEmpty();
}

const QString TouchscreenSettings::name() const
{
    return QStringLiteral("TouchScreen");
}

bool TouchscreenSettings::isShowOnHomePage() const
{
    return false;
}

QIcon TouchscreenSettings::icon() const
{
    return QIcon::fromTheme("ukui-gesture-symbolic");
}

QString TouchscreenSettings::translationPath() const
{
    return "/usr/share/touchscreen-settings/translations/%1.ts";
}

void TouchscreenSettings::initSearchText()
{
}

void TouchscreenSettings::waylandInputDeviceChanged()
{
    setShow(isEnable());
}

void TouchscreenSettings::setShow(bool isShow)
{
    const QByteArray id(PLUGINS_SCHEMA.c_str());
    if (!QGSettings::isSchemaInstalled(id)) {
        qWarning() << __FILE__ << __FUNCTION__ << __LINE__
                   << "schema :" << id << "is not install";
        return;
    }

    QString path(QString::fromStdString(PLUGINS_DIR + "TouchScreen/"));
    QGSettings settings(id, path.toUtf8());

    // It is signaled whenever the device list changes,
    // so it is possible that the touchscreen state has not changed

    settings.set(KEY_SHOW.c_str(), isShow);
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__
             << "Set GSetting" << id << path << KEY_SHOW.c_str() << isShow;
}
