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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "kcrumb.h"
#include "kborderlessbutton.h"
#include "kcustomtabbar.h"
#include "themeController.h"
#include <QApplication>
#include <QDebug>
#include <QFontMetrics>
#include <QPainter>
#include <QProxyStyle>
#include <QStyle>
#include <QStyleOptionTab>

namespace kdk
{

static int spacing = 4;

class Q_DECL_HIDDEN CrumbStyle : public QProxyStyle
{
    Q_OBJECT

public:
    CrumbStyle(QStyle *style = nullptr);

    QRect subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const override;
};

class Q_DECL_HIDDEN KCrumbPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KCrumb)

public:
    KCrumbPrivate(KCrumb *parent);

    void changeTheme();

private:
    KCrumb *q_ptr;
    QIcon m_icon;
    bool m_isFlat;
    QStringList m_stringList;
};

KCrumb::KCrumb(QWidget *parent)
    : KCustomTabBar(parent)
    , d_ptr(new KCrumbPrivate(this))
{
    Q_D(KCrumb);
    CrumbStyle *style = new CrumbStyle();
    setStyle(style);

    connect(d->m_gsetting, &QGSettings::changed, this, [=] {
        d->changeTheme();
        update();
    });
}

int KCrumb::addTab(const QString &text)
{
    Q_D(KCrumb);
    d->m_stringList.append(text);
    return KCustomTabBar::addTab(text);
}

int KCrumb::addTab(const QIcon &icon, const QString &text)
{
    Q_D(KCrumb);
    d->m_stringList.append(text);
    return KCustomTabBar::addTab(icon, text);
}

int KCrumb::insertTab(int index, const QString &text)
{
    Q_D(KCrumb);
    d->m_stringList.insert(index, text);
    return KCustomTabBar::insertTab(index, text);
}

int KCrumb::insertTab(int index, const QIcon &icon, const QString &text)
{
    Q_D(KCrumb);
    d->m_stringList.insert(index, text);
    return insertTab(index, icon, text);
}

void KCrumb::removeTab(int index)
{
    Q_D(KCrumb);
    d->m_stringList.removeAt(index);
    KCustomTabBar::removeTab(index);
}

void KCrumb::moveTab(int from, int to)
{
    Q_D(KCrumb);
    d->m_stringList.move(from, to);
    KCustomTabBar::moveTab(from, to);
}

void KCrumb::setIcon(const QIcon &icon)
{
    Q_D(KCrumb);
    d->m_icon = icon;
    update();
}

QIcon KCrumb::icon() const
{
    Q_D(const KCrumb);
    return d->m_icon;
}

bool KCrumb::isFlat() const
{
    Q_D(const KCrumb);
    return d->m_isFlat;
}

void KCrumb::setFlat(bool flat)
{
    Q_D(KCrumb);
    d->m_isFlat = flat;
    update();
}

void KCrumb::leftScroll()
{
    Q_D(KCrumb);
    leftScrollTabs();
}

void KCrumb::rightScroll()
{
    Q_D(KCrumb);
    rightScrollTabs();
}

QSize KCrumb::tabSizeHint(int index) const
{
    Q_D(const KCrumb);
    QSize size = KCustomTabBar::tabSizeHint(index);
    QFont font(QApplication::font());

    QFontMetrics fm(font);
    int width = fm.width(tabText(index));
    size.setWidth(width + 40);
    if (index == 0 && !d->m_icon.isNull())
        size.setWidth(size.width() + 50);
    return size;
}

void KCrumb::paintEvent(QPaintEvent *event)
{
    Q_D(const KCrumb);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    QFontMetrics fm = p.fontMetrics();

    QColor highLightColor = ThemeController::getPaletteColorFromDT("highlight");
    QColor baseColor;
    QColor focusColor;
    if (ThemeController::themeMode() == ThemeFlag::DarkTheme) {
        if (d->m_isFlat)
            baseColor = "#D9D9D9";
        else
            baseColor = "#47474A";
        focusColor = "#D9D9D9";
    } else {
        if (d->m_isFlat)
            baseColor = "#262626";
        else
            baseColor = "#B3B3B3";
        focusColor = "#262626";
    }

    int width = 0;
    for (int i = 0; i < count(); i++) {
        width += fm.width(d->m_stringList.at(i)) + 40;
    }
    if (!d->m_icon.isNull())
        width += 50;
    int useWidth = fm.width(d->m_stringList.at(count() - 1)) + 40;

    for (int i = 0; i < count(); i++) {
        QRect rc = tabRect(i);
        QStyleOptionTab option;
        initStyleOption(&option, i);
        p.setPen(Qt::NoPen);
        p.drawRect(rc);
        if (QStyle::State_MouseOver & option.state)
            p.setPen(highLightColor);
        else if (QStyle::State_Selected & option.state)
            p.setPen(focusColor);
        else
            p.setPen(baseColor);
        p.setBrush(Qt::NoBrush);

        if (rect().width() >= width) {
            if (i == 0 && !d->m_icon.isNull()) {
                p.drawPixmap(rc.left() + 10, (rc.height() - 24) / 2, 24, 24,
                             d->m_icon.pixmap(24, 24));
                p.drawText(rc.adjusted(30, 0, -24, 0), Qt::AlignCenter, d->m_stringList.at(i));
            } else
                p.drawText(rc.adjusted(0, 0, -24, 0), Qt::AlignCenter, d->m_stringList.at(i));
            if (i != count() - 1)
                p.drawPixmap(rc.right() - 24, (rc.height() - 16) / 2, 16, 16,
                             ThemeController::drawColoredPixmap(QIcon::fromTheme("ukui-end-symbolic").pixmap(16, 16), baseColor));
        } else {
            if (i == 0) {
                if (!d->m_icon.isNull()) {
                    p.drawPixmap(rc.left() + 10, (rc.height() - 24) / 2, 24, 24,
                                 d->m_icon.pixmap(24, 24));
                    p.drawText(rc.adjusted(30, 0, -24, 0), Qt::AlignCenter, d->m_stringList.at(i));
                    useWidth += fm.width(d->m_stringList.at(0)) + 90;
                    ;
                } else {
                    p.drawText(rc.adjusted(0, 0, -24, 0), Qt::AlignCenter, d->m_stringList.at(i));
                    useWidth += fm.width(d->m_stringList.at(0)) + 40;
                }
            } else if (i == count() - 1) {
                p.drawText(rc.adjusted(0, 0, -24, 0), Qt::AlignCenter, d->m_stringList.at(i));
            } else {
                int usableWidth = 0;
                for (int j = i; j < count() - 1; j++) {
                    usableWidth += fm.width(d->m_stringList.at(j)) + 40;
                }

                if ((rect().width() - useWidth) <= usableWidth) {
                    setTabContent(i, "...");
                    setTabToolTip(i, d->m_stringList.at(i));
                    p.drawText(rc.adjusted(0, 0, -24, 0), Qt::AlignCenter, "...");
                    useWidth += fm.width("...") + 40;
                } else // 如果可用长度 大于 需用长度 则正常显示
                {

                    setTabContent(i, d->m_stringList.at(i));
                    p.drawText(rc.adjusted(0, 0, -24, 0), Qt::AlignCenter, d->m_stringList.at(i));
                    useWidth += fm.width(d->m_stringList.at(i)) + 40;
                    if (!tabToolTip(i).isEmpty())
                        setTabToolTip(i, "");
                }
            }
            if (i != count() - 1) {
                p.drawPixmap(rc.right() - 24, (rc.height() - 16) / 2, 16, 16,
                             ThemeController::drawColoredPixmap(QIcon::fromTheme("ukui-end-symbolic").pixmap(16, 16), baseColor));
            }
        }
    }
}

KCrumbPrivate::KCrumbPrivate(KCrumb *parent)
    : q_ptr(parent)
    , m_isFlat(true)
{
}

void KCrumbPrivate::changeTheme()
{
    initThemeStyle();
}

CrumbStyle::CrumbStyle(QStyle *style)
{
}

QRect CrumbStyle::subElementRect(QStyle::SubElement element, const QStyleOption *option, const QWidget *widget) const
{
    switch (element) {
    case QStyle::SE_TabBarScrollLeftButton:
        return QRect();
        break;
    case QStyle::SE_TabBarScrollRightButton:
        return QRect();
        break;
    default:
        break;
    }
    return QProxyStyle::subElementRect(element, option, widget);
}

}

#include "kcrumb.moc"
#include "moc_kcrumb.cpp"
