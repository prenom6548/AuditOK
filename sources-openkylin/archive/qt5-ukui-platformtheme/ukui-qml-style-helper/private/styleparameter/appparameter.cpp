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
 */

#include "appparameter.h"
#include <QDebug>
#include <QApplication>
#include <QGSettings>
#include <QFontDatabase>
#include <QIcon>
#include <QDBusConnection>
#include <QIcon>
#include <QPainter>
#include <QPair>
#include <qpa/qplatformscreen.h>
#include <qpa/qplatformcursor.h>
#include <private/qhighdpiscaling_p.h>

#include "../../../libqt5-ukui-style/settings/ukui-style-settings.h"
#include "../../../qt5-ukui-platformtheme/platform-theme-fontdata.h"

using namespace UKUIQQC2Style;
APPParameter::APPParameter(QQuickItem *parent) : QQuickItem(parent)
{  
    if(!qApp)
        return;
    m_font =qApp->font();
    m_palette = qApp->palette();


    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        auto settings = UKUIStyleSettings::globalInstance();

        auto opacity = settings->get("menuTransparency").toInt()/100.0;
        setMenuTransparency(opacity);

        //set font
        auto fontName = settings->get("systemFont").toString();
        auto fontSize = settings->get("systemFontSize").toString().toDouble();
        if (qApp->property("noChangeSystemFontSize").isValid() && qApp->property("noChangeSystemFontSize").toBool())
            fontSize = 11;
        QFont tempFont = m_font;
        tempFont.setFamily(fontName);
        tempFont.setPointSizeF(fontSize);
        setFont(tempFont);

        auto styleName = settings->get("styleName").toString();

        if (styleName == "ukui-default" || styleName == "ukui-light" || styleName == "ukui-white" || styleName == "ukui" || styleName == "ukui-config")
            setIsDark(false);
        else if(styleName == "ukui-black" || styleName == "ukui-dark" )
        {
            setIsDark(true);

        }
        if(qApp->property("useLightTheme").isValid() && qApp->property("useLightTheme").toBool()){
            setIsDark(false);
        }

        /*!
         * \bug
         * if we set app font, qwizard title's font will
         * become very small. I handle the wizard title
         * in ProxyStyle::polish().
         */
        //Skip QGuiApplication avoid it crash when we setfont
        if (qApp->applicationName().toLower().contains(QLatin1String("kwin"))) {
            QDBusConnection::sessionBus().connect(QString(),
                                                  QStringLiteral("/UKUIPlatformTheme"),
                                                  QStringLiteral("org.ukui.UKUIPlatformTheme"),
                                                  QStringLiteral("refreshFonts"),
                                                  this,
                                                  SLOT(slotChangeStyle(QString)));
        }

        connect(settings, &QGSettings::changed, this, [&](const QString& key){
                slotChangeStyle(key);
                TokenParameter * token = qApp->property("qqc2-globaltoken").value<TokenParameter *>();
                if(token){
                    setPalette(token->getInstance()->appPalette());
                }
                else
                    setPalette(qApp->palette());
                emit parametryChanged();
            }, Qt::UniqueConnection);
        connect(settings, &QGSettings::changed, this, [&](const QString& key){
            if(key == "iconThemeName" || key == "icon-theme-name"){
                emit iconThemeChanged();
            }
        }, Qt::UniqueConnection);
    }

    connect(qApp, &QGuiApplication::paletteChanged, this, [&](const QPalette &pal){
        setPalette(pal);
        emit parametryChanged();
    }, Qt::UniqueConnection);

    if(!qApp || !qApp->property("qqc2-globaltoken").isValid())
        return;
    TokenParameter * token = qApp->property("qqc2-globaltoken").value<TokenParameter *>();
    if(token){
        setWindowColor(token->getInstance()->windowActive());
        setFocusBorderColor(token->getInstance()->kBrandFocus());
        setFocusBorderWidth(token->getInstance()->focusline());
        connect(token->getInstance(), &UKUIGlobalDTConfig::GlobalDTConfig::tokenChanged, this, [=](){
            setPalette(token->getInstance()->appPalette());
            setWindowColor(token->getInstance()->windowActive());
            setFocusBorderColor(token->getInstance()->kBrandFocus());
            setFocusBorderWidth(token->getInstance()->focusline());
            emit parametryChanged();
        }, Qt::UniqueConnection);
    }
}

APPParameter::~APPParameter()
{

}

APPParameter* APPParameter::qmlAttachedProperties(QObject* parent)
{
    auto p = qobject_cast<QQuickItem*>(parent);
    return new APPParameter(p);
}

const QFont &APPParameter::font() const
{
    return m_font;
}

void APPParameter::setFont(const QFont &newFont)
{
    if (m_font == newFont)
        return;
    m_font = newFont;
    emit fontChanged();
}

void APPParameter::slotChangeStyle(const QString& key)
{
    auto settings = UKUIStyleSettings::globalInstance();
    if (!settings)
        return;
    if(key == "menuTransparency"){
    auto opacity = settings->get("menuTransparency").toInt()/100.0;
    setMenuTransparency(opacity);
    }

//    if (key == "iconThemeName" || key == "icon-theme-name") {
//        QString icontheme = settings->get("icon-theme-name").toString();

//        QIcon::setThemeName(icontheme);

//        QIcon icon = qApp->windowIcon();
//        qApp->setWindowIcon(QIcon::fromTheme(icon.name()));
//        // update all widgets for repaint new themed icons.
//        for (auto widget : QApplication::allWidgets()) {
//            widget->update();
//        }
//    }
    if(key == "styleName" || key == "style-name"){
        auto styleName = settings->get("styleName").toString();
        if(qApp && qApp->property("useLightTheme").isValid() && qApp->property("useLightTheme").toBool()){
            setIsDark(false);
        }
        else if (styleName == "ukui-default" || styleName == "ukui-light" || styleName == "ukui-white" || styleName == "ukui" || styleName == "ukui-config")
            setIsDark(false);
        else if(styleName == "ukui-black" || styleName == "ukui-dark" )
        {
            setIsDark(true);

        }
    }

    if (key == "systemFont" || key == "system-font") {
        //Skip QGuiApplication avoid it crash when we setfont
        if(!qApp)
            return;

        QString font = settings->get("system-font").toString();

        QFontDatabase db;
        int id = 0;
        if (!db.families().contains(font)) {
            PlatformThemeFontData fontData;
            QMap<QString, QString> fontMap = fontData.getAllFontInformation();
            if(fontMap.contains(font)){
                auto iter = fontMap.find(font);
                id = QFontDatabase::addApplicationFont(iter.value());
            }
        }

        QFontDatabase newDb;
        if (newDb.families().contains(font)) {
            QFont tempFont = font;
            setFont(tempFont);
        }
    }

    if (key == "systemFontStyle" || key == "system-font-style") {
        //Skip QGuiApplication avoid it crash when we setfont
        // auto *app = qobject_cast<QApplication *>(qApp);
        if(!qApp)
            return;


        QString fontStyle = settings->get("system-font-style").toString();
        if (!fontStyle.isEmpty()) {
            QFont oldFont = qApp->font();

            QFont tempFont = m_font;
            tempFont.setStyleName(fontStyle);
            setFont(tempFont);
        }
    }

    if (key == "systemFontSize" || key == "system-font-size") {
        //Skip QGuiApplication avoid it crash when we setfont
        // auto *app = qobject_cast<QApplication *>(qApp);
        if(!qApp)
            return;

        if (qApp->property("noChangeSystemFontSize").isValid() && qApp->property("noChangeSystemFontSize").toBool())
            return;
        double fontSize = settings->get("system-font-size").toString().toDouble();
        if (fontSize > 0) {
            QFont oldFont = qApp->font();

            QFont tempFont = m_font;
            tempFont.setPointSize(fontSize);
            setFont(tempFont);
        }
    }
}

const QPalette &APPParameter::palette() const
{
    return m_palette;
}


void APPParameter::setPalette(const QPalette &newPalette)
{
    if (m_palette == newPalette)
        return;
    m_palette = newPalette;
    emit paletteChanged();
}

double APPParameter::menuTransparency() const
{
    return m_menuTransparency;
}

void APPParameter::setMenuTransparency(double newMenuTransparency)
{
    if (qFuzzyCompare(m_menuTransparency, newMenuTransparency))
        return;
    m_menuTransparency = newMenuTransparency;
    emit menuTransparencyChanged();
}

const QBrush &APPParameter::windowColor() const
{
    return m_windowColor;
}

void APPParameter::setWindowColor(const QBrush &newWindowColor)
{
    if (m_windowColor == newWindowColor)
        return;
    m_windowColor = newWindowColor;
    emit windowColorChanged();
}

bool APPParameter::themeHasIcon(QString iconName)
{
    return QIcon::hasThemeIcon(iconName);
}

bool APPParameter::focusEnable() const
{
    return m_focusEnable;
}

void APPParameter::setFocusEnable(bool newFocusEnable)
{
    if (m_focusEnable == newFocusEnable)
        return;
    m_focusEnable = newFocusEnable;
    emit focusEnableChanged();
}

bool APPParameter::showToolTipWindow()
{
    if(qAppName() == "kylin-aiassistant")
        return false;
    return true;
}

QPoint APPParameter::posByCursor(int width, int height)
{
    QPoint p = QCursor::pos();
    qDebug() << "cursor pos:" << p;
    const QScreen *screen =QGuiApplication::screenAt(QCursor::pos());
    if (const QPlatformScreen *platformScreen = screen ? screen->handle() : nullptr) {
        QPlatformCursor *cursor = platformScreen->cursor();
        const QSize nativeSize = cursor ? cursor->size() : QSize(16, 16);
        const QSize cursorSize = QHighDpi::fromNativePixels(nativeSize, platformScreen);
        QPoint offset(2, cursorSize.height());
        if (cursorSize.height() > 2 * height) {
            offset = QPoint(cursorSize.width() / 2, 0);
        }
        p += offset;

        QRect screenRect = screen->availableGeometry();
        if (p.x() + width > screenRect.x() + screenRect.width())
            p.rx() -= 4 + width;
        if (p.y() + height > screenRect.y() + screenRect.height())
            p.ry() -= 24 + height;
        if (p.y() < screenRect.y())
            p.setY(screenRect.y());
        if (p.x() + width > screenRect.x() + screenRect.width())
            p.setX(screenRect.x() + screenRect.width() - width);
        if (p.x() < screenRect.x())
            p.setX(screenRect.x());
        if (p.y() + height > screenRect.y() + screenRect.height())
            p.setY(screenRect.y() + screenRect.height() - height);
    }
    qDebug() << "cursor pos1:" << p;

    return p;
}


QBrush APPParameter::focusBorderColor() const
{
    return m_focusBorderColor;
}

void APPParameter::setFocusBorderColor(QBrush newFocusBorderColor)
{
    if (m_focusBorderColor == newFocusBorderColor)
        return;
    m_focusBorderColor = newFocusBorderColor;
    emit focusBorderColorChanged();
}

int APPParameter::focusBorderWidth() const
{
    return m_focusBorderWidth;
}

void APPParameter::setFocusBorderWidth(int newFocusBorderWidth)
{
    if (m_focusBorderWidth == newFocusBorderWidth)
        return;
    m_focusBorderWidth = newFocusBorderWidth;
    emit focusBorderWidthChanged();
}
