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
#include <QGSettings>

#include "qqc2styleplugin.h"
#include "kyquickstyleitem.h"
#include "kystylehelper.h"
#include "KyIcon.h"
#include "qdebug.h"
#include "styleparameter/ukuibutton.h"
#include "styleparameter/appparameter.h"
#include "styleparameter/imageprovider.h"
#include "styleparameter/ukuilabel.h"
#include "styleparameter/ukuimenuitem.h"
#include "styleparameter/ukuimenu.h"
#include "styleparameter/ukuipopup.h"
#include "styleparameter/ukuiscrollbar.h"
#include "styleparameter/ukuitabbutton.h"
#include "styleparameter/ukuitooltip.h"
#include "styleparameter/tokenparameter.h"
#include "styleparameter/ukuiradiobutton.h"
#include "styleparameter/ukuicheckbox.h"
#include "styleparameter/ukuicombobox.h"
#include "styleparameter/ukuiitemdelegate.h"
#include "styleparameter/ukuitextfiled.h"
#include "styleparameter/ukuiprogressbar.h"
#include "styleparameter/ukuislider.h"
#include "styleparameter/ukuispinbox.h"
#include "styleparameter/ukuitabbar.h"
#include "styleparameter/parsecolorinterface.h"
#include "styleparameter/ukuiswitch.h"
#include "styleparameter/ukuipopupwindowhandle.h"
#include "styleparameter/ukuitextarea.h"
#include "styleparameter/ukuiheaderview.h"
#include "styleparameter/ukuitoken.h"

using namespace UKUIQQC2Style;

Qqc2StylePlugin::~Qqc2StylePlugin()
{
    // Avoid QObject destruction during plugin unload/global teardown.
    // These objects are intentionally left for process cleanup to prevent
    // shutdown-order crashes in Qt meta type/converter internals.
    m_token = nullptr;
    m_platformthemeToken = nullptr;
}

void Qqc2StylePlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    m_token = new TokenParameter();
    qApp->setProperty("qqc2-globaltoken", QVariant::fromValue(m_token));
    m_platformthemeToken = new UKUIToken();
    m_platformthemeToken->setParent(engine);
    engine->rootContext()->setContextProperty("ukui_platformtheme_token",  QVariant::fromValue(m_platformthemeToken));
    engine->addImageProvider("imageProvider", new ImageProvider());
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        auto *settings = new QGSettings("org.ukui.style");
        connect(settings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "styleName" || key == "widgetThemeName" || key == "themeColor" ||
                    key == "style-name" || key == "widget-theme-name" || key == "theme-color" ||
                    key == "window-radius" || key == "windowRadius") {
                if(m_token){
                    m_token->getInstance()->init();
                }
            }
        });
    }
}

void Qqc2StylePlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.ukui.qqc2style.private"));

    // @uri org.ukui.qqc2style.private
    qmlRegisterType<KyStyleHelper>(uri, 1, 0, "StyleHelper");
    qmlRegisterType<KyIcon>(uri, 1, 0, "KyIcon");
    qmlRegisterType<UKUIButton>(uri, 1, 0, "UKUIButton");

    qmlRegisterType<UKUIButton>(uri, 1, 0, "UKUIButton");
    qmlRegisterType<APPParameter>(uri, 1, 0, "APPParameter");
    qmlRegisterType<UKUILabel>(uri, 1, 0, "UKUILabel");
    qmlRegisterType<UKUIMenuItem>(uri, 1, 0, "UKUIMenuItem");
    qmlRegisterType<UKUIMenu>(uri, 1, 0, "UKUIMenu");
    qmlRegisterType<UKUIPopup>(uri, 1, 0, "UKUIPopup");
    qmlRegisterType<UKUIScrollBar>(uri, 1, 0, "UKUIScrollBar");
    qmlRegisterType<UKUITabButton>(uri, 1, 0, "UKUITabButton");
    qmlRegisterType<UKUIToolTip>(uri, 1, 0, "UKUIToolTip");
    qmlRegisterType<UKUIRadioButton>(uri, 1, 0, "UKUIRadioButton");
    qmlRegisterType<UKUICheckBox>(uri, 1, 0, "UKUICheckBox");
    qmlRegisterType<UKUIComboBox>(uri, 1, 0, "UKUIComboBox");
    qmlRegisterType<UKUIItemDelegate>(uri, 1, 0, "UKUIItemDelegate");
    qmlRegisterType<UKUITextFiled>(uri, 1, 0, "UKUITextFiled");
    qmlRegisterType<UKUIProgressBar>(uri, 1, 0, "UKUIProgressBar");
    qmlRegisterType<UKUISlider>(uri, 1, 0, "UKUISlider");
    qmlRegisterType<UKUISpinBox>(uri, 1, 0, "UKUISpinBox");
    qmlRegisterType<UKUITabBar>(uri, 1, 0, "UKUITabBar");
    qmlRegisterType<UKUISwitch>(uri, 1, 0, "UKUISwitch");
    qmlRegisterType<UKUITextArea>(uri, 1, 0, "UKUITextArea");
    qmlRegisterType<UKUIHeaderView>(uri, 1, 0, "UKUIHeaderView");

    qmlRegisterType<ParseColorInterface>(uri, 1, 0, "ParseColorInterface");
    qmlRegisterType<UKUIPopupWindowHandle>(uri, 1, 0, "PopupHandle");
    qRegisterMetaType<TokenParameter>("TokenParameter");

}
