/*
 * Qt5-UKUI's Library
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "ukui-style-plugin.h"
#include "qt5-config-style-ukui/ukui-config-style.h"

#include "settings/black-list.h"
#include "settings/ukui-style-settings.h"
#include "effects/highlight-effect.h"
#include "settings/application-style-settings.h"

#include <QApplication>
#include <QTimer>

#include <QDebug>
#include <QStyleFactory>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnection>

UKUIStylePlugin::UKUIStylePlugin(QObject *parent) :
    QStylePlugin(parent)
{
    if (UKUIStyleSettings::isSchemaInstalled("org.ukui.style")) {
        auto settings = UKUIStyleSettings::globalInstance();
        connect(settings, &UKUIStyleSettings::changed, this, [=](const QString &key) {
            if (key == "styleName" || key == "widgetThemeName" || key == "themeColor" ||
                    key == "style-name" || key == "widget-theme-name" || key == "theme-color") {
                if (qApp->property("skipUKUITheme").isValid()&&qApp->property("skipUKUITheme").canConvert<bool>())
                    return;

                if (blackList().contains(qAppName()) || qAppName() == "kylin-software-center.py")
                    return;

                //We should not swich a application theme which use internal style.
                if (QApplication::style()->inherits("InternalStyle"))
                    return;

//                auto appStyleSettings = ApplicationStyleSettings::getInstance();
//                if (appStyleSettings->currentStyleStretagy() != ApplicationStyleSettings::Default)
//                    return;

                auto styleName = settings->get("styleName").toString();
                auto widgetThemeName = settings->get("widgetThemeName").toString();

                if (widgetThemeName != "") {
                    if (widgetThemeName == "classical") {
                        styleName ="ukui-config";
                    } else {
                        styleName = "ukui-config";
                    }
                    qApp->setStyle(new UKUIConfigStyle(styleName));

                    return;
                }

                //other style
                for (auto keys : QStyleFactory::keys()) {
                    if (styleName.toLower() == keys.toLower()) {
                        qApp->setStyle(new QProxyStyle(styleName));
                        return;
                    }
                }

                //default style fusion
                qApp->setStyle(new QProxyStyle(styleName));
                return;
            }

            if (key == "systemPalette") {
                onSystemPaletteChanged();
            }

            if (key == "useSystemPalette") {
                onSystemPaletteChanged();
            }

            if (key == "windowRadius") {
                foreach (QWidget *widget, qApp->allWidgets()) {
                    widget->update();
                }
            }

        });
    }

//    QDBusInterface *interFace = new QDBusInterface("com.kylin.statusmanager.interface",
//                                                   "/",
//                                                   "com.kylin.statusmanager.interface",
//                                                    QDBusConnection::sessionBus());
//    if(interFace->isValid()){
//        connect(interFace, SIGNAL(mode_change_signal(bool)), this,  SLOT(tableModeChanged(bool)));
//    }
}

QStyle *UKUIStylePlugin::create(const QString &key)
{
    if (blackList().contains(qAppName()))
        return new Style;

    if (qApp->property("skipUKUITheme").isValid()&&qApp->property("skipUKUITheme").canConvert<bool>()){
        bool isSkipUKUITheme = qApp->property("skipUKUITheme").toBool();
        if (isSkipUKUITheme)
            return new Style;
    }

    auto settings = UKUIStyleSettings::globalInstance();
    auto widgetThemeName = settings->get("widgetThemeName").toString();
    auto styleName = settings->get("styleName").toString();

    if (widgetThemeName != "") {
        if (widgetThemeName == "classical") {
            return new UKUIConfigStyle(styleName);
        } else {
            return new UKUIConfigStyle(styleName);
        }

    }
    return new UKUIConfigStyle(widgetThemeName);
}

const QStringList UKUIStylePlugin::blackList()
{
    return blackAppList();
}

void UKUIStylePlugin::onSystemPaletteChanged()
{
    bool useSystemPalette = UKUIStyleSettings::globalInstance()->get("useSystemPalette").toBool();
    if (useSystemPalette) {
        auto data = UKUIStyleSettings::globalInstance()->get("systemPalette");
        if (data.isNull())
            return;
        auto palette = qvariant_cast<QPalette>(data);
        QApplication::setPalette(palette);
    } else {
        auto palette = QApplication::style()->standardPalette();
        QApplication::setPalette(palette);
    }
}

void UKUIStylePlugin::tableModeChanged(bool isTableMode)
{
    if (blackList().contains(qAppName()))
        return;

    //We should not swich a application theme which use internal style.
    if (QApplication::style()->inherits("InternalStyle"))
        return;

//    auto appStyleSettings = ApplicationStyleSettings::getInstance();
//    if (appStyleSettings->currentStyleStretagy() != ApplicationStyleSettings::Default)
//        return;

    auto settings = UKUIStyleSettings::globalInstance();
    auto styleName = settings->get("styleName").toString();

    if (styleName == "ukui-default" || styleName == "ukui-dark" || styleName == "ukui-white"
            || styleName == "ukui-black" || styleName == "ukui-light" || styleName == "ukui" || styleName == "ukui-config")

    {
        if (qApp->property("customStyleName").isValid()) {
            if (qApp->property("customStyleName").toString() == "ukui-light") {
                styleName = "ukui-light";
            } else if (qApp->property("customStyleName").toString() == "ukui-dark") {
                styleName = "ukui-dark";
            }
        }

        foreach (QWidget *widget, qApp->allWidgets()) {
            widget->updateGeometry();
        }
        qApp->setProperty("isInitial", false);
        auto settings = UKUIStyleSettings::globalInstance();
        auto widgetThemeName = settings->get("widgetThemeName").toString();
        auto styleName = settings->get("styleName").toString();

        if (widgetThemeName != "") {
            if (widgetThemeName == "classical") {
                qApp->setStyle(new UKUIConfigStyle(styleName));
            } else {
                qApp->setStyle(new UKUIConfigStyle(styleName));
            }

        }
        else
            qApp->setStyle(new UKUIConfigStyle(styleName));

    }
    /*
    foreach (QObject *obj, qApp->children()) {
        if(qobject_cast<const QVBoxLayout*>(obj))//obj->inherits("QVBoxLayout"))
        {
            auto *vlayout = qobject_cast<QVBoxLayout *>(obj);
            qDebug() << "vlayout..." << vlayout->spacing();

            //layout->update();
        }
        else if(qobject_cast<QHBoxLayout *>(obj))
        {
            auto *hlayout = qobject_cast<QHBoxLayout *>(obj);
            qDebug() << "hlayout..." << hlayout->spacing();

            //layout->update();
        }
    }
    foreach (QWidget *widget, qApp->topLevelWidgets()) {
        qDebug() << "widget size...." << widget->size() << widget->minimumSizeHint() << widget->minimumSize();
        widget->adjustSize();
        if(widget->layout() != nullptr)
        {
            qDebug() << "layoutSpace...." << widget->layout()->spacing();
        }
//        if(widget->size() != widget->minimumSizeHint())
//            widget->setGeometry(widget->x(), widget->y(), widget->minimumSizeHint().width(), widget->minimumSizeHint().height());
    }
*/
    return;
}


#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(ukui-config-style, UKUIStylePlugin)
#endif // QT_VERSION < 0x050000
