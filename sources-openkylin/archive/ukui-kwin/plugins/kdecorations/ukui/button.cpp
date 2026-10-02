/*
 * KWin Style UKUI
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "button.h"
#include <QPainter>
#include <KDecoration2/DecoratedClient>
#include "xatom-helper.h"
#include <QFile>

using namespace UKUI;

Button *Button::create(KDecoration2::DecorationButtonType type, KDecoration2::Decoration *decoration, QObject *parent)
{
    auto d = static_cast<UKUI::Decoration *>(decoration);
    auto b = new Button(type, d, parent);
    switch (type) {
        case KDecoration2::DecorationButtonType::Menu:
            QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::iconChanged, b, [b]() {b->update();});
            break;

        case KDecoration2::DecorationButtonType::OnAllDesktops:
            b->setVisible(false);
            break;

        case KDecoration2::DecorationButtonType::Minimize:
            {
                bool bRet = XAtomHelper::getInstance()->checkButtonAvailable(d->client().data()->windowId(), KDecoration2::DecorationButtonType::Minimize);
                if(false == bRet)
                {
                    b->setVisible(false);
                }
                //QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::minimizeableChanged, b, &Button::setVisible );
                break;
            }

        case KDecoration2::DecorationButtonType::Maximize:
            {
                bool bRet = XAtomHelper::getInstance()->checkButtonAvailable(d->client().data()->windowId(), KDecoration2::DecorationButtonType::Maximize);
                if(false == bRet)
                {
                    b->setVisible(false);
                }
                //QObject::connect(d->client().data(), &KDecoration2::DecoratedClient::maximizeableChanged, b, &Button::setVisible );
                break;
            }

        case KDecoration2::DecorationButtonType::Close:
            {
                bool bRet = XAtomHelper::getInstance()->checkButtonAvailable(d->client().data()->windowId(), KDecoration2::DecorationButtonType::Close);
                if(false == bRet)
                {
                    b->setVisible(false);
                }
                break;
            }

        case KDecoration2::DecorationButtonType::ContextHelp:
            b->setVisible(false);

        default:
            break;
    }

    return b;
}

Button::Button(QObject *parent, const QVariantList &args) : KDecoration2::DecorationButton(args.at(0).value<KDecoration2::DecorationButtonType>(), args.at(1).value<Decoration*>(), parent)
{

}

Button::Button(KDecoration2::DecorationButtonType type, UKUI::Decoration *decoration, QObject *parent) : KDecoration2::DecorationButton(type, decoration, parent)
{

}

Button::~Button()
{

}

static QString loadThemeNameFromId(UKUI::KylinTheme themeId)
{
    QString strIconName;
    // get icon theme from theme enum
    switch(themeId) {
        case UKUI::KylinTheme::ThemeDefault:
            strIconName = "ukui-base";
            break;
        case UKUI::KylinTheme::ThemeClassical:
            strIconName = "ukui-classical";
            break;
        case UKUI::KylinTheme::ThemeFashion:
            strIconName = "ukui-fashion";
            break;
        
        // 定制版本
        case UKUI::KylinTheme::ThemeLaika:
            strIconName = "laika";
            break;
        case UKUI::KylinTheme::ThemeMavis:
            strIconName = "mavis";
            break;
        default:
        break;
    }
    return strIconName;
}

void Button::paint(QPainter *painter, const QRect &repaintRegion)
{
    if(false == isVisible())
    {
        return;
    }

    if(!decoration())
    {
        return;
    }

    auto d = qobject_cast<Decoration*>(decoration());

    //menu button
    if(type() == KDecoration2::DecorationButtonType::Menu)
    {
        const QRectF iconRect(geometry().topLeft(), geometry().size().toSize());
        decoration()->client().data()->icon().paint(painter, iconRect.toRect());
        return;
    }

    QString strPath;
    QString strIconTheme;
    QString strIconType;
    QString strIconState;
    QString strIconStyle;

    // handle icon theme
    strIconTheme = loadThemeNameFromId(d->themeId());

    // handle icon state
    if (isPressed()) {
        strIconState = "clicked";
    } else if (isHovered()) {
        strIconState = "hover";
    } else {
        if(d->themeId() == ThemeClassical) {
            if(d->client().data()->isActive()) {
                strIconState = "common";
            } else {
                strIconState = "common-inactive";
            }
        } else {
            strIconState = "common";
        }
    }

    // handle icon style
    if (d->themeMode() == Dark) {
        strIconStyle = "white";
    } else {
        strIconStyle = "black";
    }

    // handle icon type
    switch (type()) {
        case KDecoration2::DecorationButtonType::Minimize:
            strIconType = "minimize";
            break;
        case KDecoration2::DecorationButtonType::Maximize:
            strIconType = isChecked() ? "restore" : "maximize";
            break;
        case KDecoration2::DecorationButtonType::Close:
            strIconType = "close";
            break;
        default:
            return;
            break;
    }

    // construct icon path
    strPath = QStringLiteral(":/icon/%1/%2-%3-%4.svg").arg(strIconTheme).arg(strIconState).arg(strIconType).arg(strIconStyle);
    QFile ddd(strPath);
    if (!ddd.exists())
        strPath = QStringLiteral(":/icon/%1/%2-%3.svg").arg(strIconTheme).arg(strIconState).arg(strIconType);

    const QRectF iconRect(geometry().topLeft(), geometry().size().toSize());
    QIcon icon(strPath);
    icon.paint(painter, iconRect.toRect());

    return;
}


