/*
 * libkysdk-qtwidgets's Library
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#include "ktag.h"
#include "ktoolbutton.h"
#include "parmscontroller.h"
#include "themeController.h"
#include <QApplication>
#include <QDebug>
#include <QFontMetrics>
#include <QPainter>
#include <QPalette>
#include <QStyleOption>
#include <QToolButton>
#include "accessinfohelper.h"

namespace kdk
{
class KTagPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KTag)

public:
    KTagPrivate(KTag *parent);
    void changeTheme();
    void updateDeleteBtnColor();

private:
    KTag *q_ptr;
    QString m_text;
    KToolButton *m_pDeleteBtn;
    TagStyle m_style;
    QColor m_backgroundColor;
    bool m_isClosable;
    bool m_isBackgroundFlag;
    bool m_isTranslucent;
};

KTag::KTag(QWidget *parent)
    : QPushButton(parent)
    , d_ptr(new KTagPrivate(this))
{
    Q_D(KTag);
    setClosable(false);
    d->changeTheme();
    connect(Parmscontroller::self(), &Parmscontroller::modeChanged, this, [=](bool flag) {
        updateGeometry();
    });
}

void KTag::setClosable(bool flag)
{
    Q_D(KTag);
    d->m_isClosable = flag;
    if (flag)
        setMinimumWidth(88);
    d->updateDeleteBtnColor();
    update();
}

bool KTag::closable()
{
    Q_D(KTag);
    return d->m_isClosable;
}

void KTag::setText(const QString &text)
{
    Q_D(KTag);
    d->m_text = text;
    update();
}

void KTag::setTagStyle(TagStyle style)
{
    Q_D(KTag);
    d->m_style = style;
    d->changeTheme();
}

TagStyle KTag::tagStyle()
{
    Q_D(KTag);
    d->updateDeleteBtnColor();
    return d->m_style;
}

void KTag::setBackgroundColor(QColor color)
{
    Q_D(KTag);
    d->m_isBackgroundFlag = true;
    d->m_backgroundColor = color;
    d->updateDeleteBtnColor();
}

void KTag::setTranslucent(bool flag)
{
    Q_D(KTag);
    d->m_isTranslucent = flag;
    if (flag)
        d->updateDeleteBtnColor();
}

bool KTag::translucent()
{
    Q_D(KTag);
    return d->m_isTranslucent;
}

QString KTag::text()
{
    Q_D(KTag);
    return d->m_text;
}

void KTag::paintEvent(QPaintEvent *event)
{
    Q_D(KTag);
    QStyleOptionButton option;
    initStyleOption(&option);
    QColor bkgColor;
    QColor highColor = ThemeController::getCustomColorFromDT("highlight-active");
    QColor iconColor;
    QColor fontColor;
    QRect iconRect;
    QRect textRect;
    if (!option.state.testFlag(QStyle::State_Enabled)) {
        if(d->m_style == IconTag)
            bkgColor = ThemeController::getCustomColorFromDT("button-disable");
        else
            bkgColor = ThemeController::getCustomColorFromDT("kbrand-disable");
        iconColor = ThemeController::getCustomColorFromDT("highlightedtext-disable");
        fontColor = ThemeController::getCustomColorFromDT("highlightedtext-disable");
    } else {
        if (d->m_isTranslucent || d->m_isBackgroundFlag) {
            iconColor = ThemeController::getCustomColorFromDT("highlight-active");
            fontColor = ThemeController::getCustomColorFromDT("highlight-active");
        } else {
            iconColor = Qt::white;
            fontColor = Qt::white;
        }

        if (!d->m_isBackgroundFlag) {
            bkgColor = highColor;
            if (option.state.testFlag(QStyle::State_MouseOver)) {
                if (option.state.testFlag(QStyle::State_Sunken)) {
                    if (d->m_isTranslucent)
                        bkgColor.setAlphaF(0.45);
                    else
                        bkgColor = ThemeController::mixColor(bkgColor, ThemeController::getCustomColorFromDT("brighttext-active"), 0.2);
                } else {
                    if (d->m_isTranslucent)
                        bkgColor.setAlphaF(0.3);
                    else
                        bkgColor = ThemeController::mixColor(bkgColor, ThemeController::getCustomColorFromDT("brighttext-active"), 0.05);
                }
            } else {
                if (d->m_isTranslucent)
                    bkgColor.setAlphaF(0.15);
            }
        } else {
            bkgColor = d->m_backgroundColor;
        }
    }
    if (closable()) {
        QRect rect = option.rect.adjusted(8, 0, -(d->m_pDeleteBtn->width()), 0);
        int drawLength = option.iconSize.width() + 4 + fontMetrics().width((d->m_text));

        if(!option.icon.isNull())
        {
            if (rect.width() > drawLength) {
                iconRect = QRect(rect.center().x() - drawLength / 2,
                                 rect.center().y() - option.iconSize.height() / 2,
                                 option.iconSize.width(),
                                 option.iconSize.height());
                textRect = QRect(iconRect.right() + 4,
                                 0,
                                 fontMetrics().width(d->m_text),
                                 rect.height());
            } else {
                iconRect = QRect(rect.x(),
                                 rect.center().y() - option.iconSize.height() / 2,
                                 option.iconSize.width(),
                                 option.iconSize.height());
                textRect = QRect(iconRect.right() + 4,
                                 0,
                                 rect.width()  - iconRect.width(),
                                 rect.height());
            }
        }
        else
        {
            if (rect.width() > drawLength) {
                textRect = QRect(rect.x(),
                                 0,
                                 rect.width() ,
                                 rect.height());
            } else {
                textRect = QRect(rect.x(),
                                 0,
                                 rect.width() ,
                                 rect.height());
            }
        }
    } else {
        QRect rect = option.rect.adjusted(8, 0, -8, 0);
        int drawLength = option.iconSize.width() + 4 + fontMetrics().width((d->m_text));
        if(!option.icon.isNull())
        {
            if (rect.width() > drawLength) {
                iconRect = QRect(rect.center().x() - drawLength / 2,
                                 rect.center().y() - option.iconSize.height() / 2,
                                 option.iconSize.width(),
                                 option.iconSize.height());
                textRect = QRect(iconRect.right() + 4,
                                 0,
                                 fontMetrics().width(d->m_text),
                                 rect.height());
            } else {
                iconRect = QRect(rect.x() ,
                                 rect.center().y() - option.iconSize.height() / 2,
                                 option.iconSize.width(),
                                 option.iconSize.height());
                textRect = QRect(iconRect.right() + 4,
                                 0,
                                 rect.width()  - iconRect.width(),
                                 rect.height());
            }
        }
        else
        {
            if (rect.width() > drawLength) {
                textRect = QRect(rect.x(),
                                 0,
                                 rect.width(),
                                 rect.height());
            } else {
                textRect = QRect( rect.x(),
                                 0,
                                 rect.width() ,
                                 rect.height());
            }
        }
    }
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    if (ThemeController::widgetTheme() == ClassicTheme) {
        if (painter.pen().width() == 1)
            painter.translate(0.5, 0.5);
    }
    QRect rect = this->rect();
    if (d->m_isClosable) {
        d->m_pDeleteBtn->show();
        d->m_pDeleteBtn->move(width() - d->m_pDeleteBtn->width(), (height() - d->m_pDeleteBtn->height()) / 2 + 2);
        rect.adjust(0, 0, -16, 0);
    } else {
        d->m_pDeleteBtn->hide();
    }
    QColor color = ThemeController::getCustomColorFromDT("highlight-active");
    QFontMetrics fm = fontMetrics();
    switch (d->m_style) {
    case HighlightTag:

        if (ThemeController::widgetTheme() == ClassicTheme) {
            QColor backColor = color;
            backColor.setAlpha(38);
            painter.setBrush(backColor);
            painter.setPen(color);
            painter.drawRoundedRect(this->rect(), 0, 0);
        } else {
            painter.setBrush(color);
            painter.setPen(Qt::PenStyle::NoPen);
            int radius = ThemeController::getRadiusFromDT("kradius-normal");
            if (radius == -1)
                radius = 6;
            painter.drawRoundedRect(this->rect(), radius, radius);
            painter.setPen(QColor("#FFFFFF"));
        }
        break;
    case BoderTag:
        painter.setBrush(QColor(Qt::transparent));
        painter.setPen(color);
        if (ThemeController::widgetTheme() == ClassicTheme)
            painter.drawRoundedRect(this->rect(), 0, 0);
        else {
            int radius = ThemeController::getRadiusFromDT("kradius-normal");
            if (radius == -1)
                radius = 6;
            painter.drawRoundedRect(this->rect(), radius, radius);
        }
        break;
    case BaseBoderTag: {
        QColor backColor = color;
        backColor.setAlpha(38);
        painter.setBrush(backColor);
        painter.setPen(color);
        if (ThemeController::widgetTheme() == ClassicTheme) {
            painter.drawRoundedRect(this->rect(), 0, 0);
        } else {
            int radius = ThemeController::getRadiusFromDT("kradius-normal");
            if (radius == -1)
                radius = 6;
            painter.drawRoundedRect(this->rect(), radius, radius);
        }
        break;
    }
    case GrayTag:

        if (ThemeController::widgetTheme() == ClassicTheme) {
            painter.setBrush(QColor(Qt::transparent));
            painter.setPen(color);
            painter.drawRoundedRect(this->rect(), 0, 0);
        } else {
            painter.setPen(Qt::PenStyle::NoPen);
            painter.setBrush(ThemeController::getCustomColorFromDT("kcomponent-normal"));
            int radius = ThemeController::getRadiusFromDT("kradius-normal");
            if (radius == -1)
                radius = 6;
            painter.drawRoundedRect(this->rect(), radius, radius);
            painter.setPen(ThemeController::getCustomColorFromDT("text-active"));
        }
        break;
    case IconTag: {
        QPixmap pixmap = option.icon.pixmap(option.iconSize);
        painter.setPen(Qt::NoPen);
        painter.setBrush(bkgColor);
        int radius = ThemeController::getRadiusFromDT("kradius-normal");
        if (radius == -1)
            radius = 6;
        painter.drawRoundedRect(this->rect(), radius, radius);
        painter.drawPixmap(iconRect, ThemeController::drawColoredPixmap(pixmap, iconColor));
        painter.setPen(fontColor);
        break;
    }
    default:
        break;
    }

    if(fm.width(d->m_text) > textRect.width())
    {
        QString str = fm.elidedText(d->m_text,Qt::TextElideMode::ElideRight,textRect.width());
        setToolTip(d->m_text);
        painter.drawText(textRect,Qt::AlignCenter,str);
    }
    else
    {
        setToolTip("");
        painter.drawText(textRect, Qt::AlignCenter, d->m_text);
    }

}

QSize KTag::sizeHint() const
{
    auto size = QPushButton::sizeHint();
    size.setHeight(Parmscontroller::parm(Parmscontroller::Parm::PM_TagHeight));
    return size;
}

KTagPrivate::KTagPrivate(KTag *parent)
    : q_ptr(parent)
    , m_isClosable(false)
    , m_text("")
    , m_style(HighlightTag)
    , m_isBackgroundFlag(false)
    , m_isTranslucent(false)
{
    Q_Q(KTag);
    setParent(parent);
    m_pDeleteBtn = new KToolButton(q);
    m_pDeleteBtn->setFixedSize(32, 32);
    m_pDeleteBtn->setIconSize(QSize(16, 16));
    connect(m_pDeleteBtn, &KToolButton::clicked, q, &KTag::close);
    m_pDeleteBtn->hide();
    QPalette btnPalette;
    btnPalette.setBrush(QPalette::Active, QPalette::Button, Qt::transparent);
    btnPalette.setBrush(QPalette::Inactive, QPalette::Button, Qt::transparent);
    btnPalette.setBrush(QPalette::Disabled, QPalette::Button, Qt::transparent);

    btnPalette.setBrush(QPalette::Active, QPalette::Highlight, Qt::transparent);
    btnPalette.setBrush(QPalette::Inactive, QPalette::Highlight, Qt::transparent);
    btnPalette.setBrush(QPalette::Disabled, QPalette::Highlight, Qt::transparent);
    m_pDeleteBtn->setAutoFillBackground(true);
    m_pDeleteBtn->setPalette(btnPalette);
    m_pDeleteBtn->setFocusPolicy(Qt::NoFocus);
    m_pDeleteBtn->setCursor(Qt::ArrowCursor);
    QIcon icon = QIcon::fromTheme("application-exit-symbolic");
    m_pDeleteBtn->setIcon(icon);
    connect(m_gsetting,&QGSettings::changed,this,[=](QString key){
        if(key == "styleName" || key == "iconThemeName" || key == "widgetThemeName")
        {
            changeTheme();
        }
    });

    KDK_ALL_INFO_FORMAT(m_pDeleteBtn,"");
}

void KTagPrivate::changeTheme()
{
    Q_Q(KTag);

    initThemeStyle();
    updateDeleteBtnColor();
}

void KTagPrivate::updateDeleteBtnColor()
{
    Q_Q(KTag);

    switch (m_style) {
    case HighlightTag:
        if(q->isEnabled())
            m_pDeleteBtn->setIconColor(true, ThemeController::getCustomColorFromDT("kfont-white"));
        else
            m_pDeleteBtn->setIconColor(true, ThemeController::getCustomColorFromDT("kfont-white-disable"));
        break;
    case BoderTag:
    case BaseBoderTag:
        if(q->isEnabled())
            m_pDeleteBtn->setIconColor(true, ThemeController::getCustomColorFromDT("kbrand-normal"));
        else
            m_pDeleteBtn->setIconColor(true, ThemeController::getCustomColorFromDT("kbrand-disable"));
        break;
    case GrayTag:
        if(q->isEnabled())
            m_pDeleteBtn->setIconColor(true, ThemeController::getCustomColorFromDT("kfont-primary"));
        else
            m_pDeleteBtn->setIconColor(true, ThemeController::getCustomColorFromDT("kfont-primary-disable"));
        break;
        break;
    case IconTag:
        if (m_isTranslucent || m_isBackgroundFlag)
            m_pDeleteBtn->setIconColor(true, ThemeController::getCustomColorFromDT("highlight-active")); // m_backgroundColor
        else
            m_pDeleteBtn->setIconColor(true, Qt::white);
        break;
    default:
        break;
    }
}
}

#include "ktag.moc"
#include "moc_ktag.cpp"
