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
 * Authors: Yan Wang <wangyan@kylinos.cn>
 *
 */
#include <QQmlApplicationEngine>
#include <QQmlEngine>
#include <QGSettings>

#include "qqc2styleplugin.h"
#include "kyquickstyleitem.h"
#include "kystylehelper.h"
#include "KyIcon.h"
#include "qdebug.h"
#include "ukuiwindow.h"
#include "ukuipopuphandle.h"
#include "kyquickDialog.h"
#include "kybutton.h"
#include "kydirhelper.h"
#include "kdragfiledialog.h"

using namespace UKUISDKStyle;

Qqc2StylePlugin::~Qqc2StylePlugin()
{

}

void Qqc2StylePlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_UNUSED(uri);
}

void Qqc2StylePlugin::registerTypes(const char *uri)
{
    qDebug() << "registerTypes......." << uri;
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.ukui.platformtheme.controls"));

    qmlRegisterType<KyQuickStyleItem>(uri, 1, 0, "StyleItem");
    qmlRegisterType<KyStyleHelper>(uri, 1, 0, "StyleHelper");
    qmlRegisterType<KyIcon>(uri, 1, 0, "KyIcon");
    qmlRegisterType<UKUISDKStyle::UKUIWindow>(uri, 1, 0, "UKUIWindow");
    qmlRegisterType<UKUISDKStyle::UKUIPopupHandle>(uri, 1, 0, "UKUIPopupHandle");
    // 勿用 "KDialog"：与 qmldir 中 QML 组件 KDialog.qml（ApplicationWindow）同名会被覆盖，导致无 title 等属性
    qmlRegisterType<UKUISDKStyle::KyQuickDialog>(uri, 1, 0, "KyQuickDialog");
    qmlRegisterType<KyButton>(uri, 1, 0, "KyButton");
    qmlRegisterType<KyDirHelper>(uri, 1, 0, "KyDirHelper");
    qmlRegisterType<UKUISDKStyle::KDragFileDialog>(uri, 1, 0, "KDragFileDialog");


}
