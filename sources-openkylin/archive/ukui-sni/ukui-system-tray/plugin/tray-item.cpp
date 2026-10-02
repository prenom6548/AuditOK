/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: youdiansaodongxi <guojiaqi@kylinos.cn>
 *
 */

#include "tray-item.h"
#include "status-notifier-item.h"
#include <QQuickWindow>
#include <QDebug>

QIcon extractIcon(const QIcon&icon, const QIcon&defaultValue)
{
    return icon.isNull() ? defaultValue : icon;
}

class TrayItemPrivate
{
public:
    QString m_source;
    QString m_desktop;
    UkuiSni::StatusNotifierItem* m_item {nullptr};
    bool m_fixed {false};
    bool m_recordOrder {false};
    bool m_isReady {false};
    QQuickItem *m_parentItem {nullptr};
};

TrayItem::TrayItem(const QString&itemSource, QObject* parent) : QObject(parent), d(new TrayItemPrivate)
{
    d->m_source = itemSource;
    d->m_item = UkuiSni::StatusNotifierHost::self()->itemForService(d->m_source);
    connect(d->m_item, &UkuiSni::StatusNotifierItem::dataUpdated, this, &TrayItem::dataUpdated);
    connect(d->m_item, &UkuiSni::StatusNotifierItem::contextMenuReady, this, &TrayItem::contextMenuReady, Qt::QueuedConnection);
    connect(d->m_item, &UkuiSni::StatusNotifierItem::activateResult, this, &TrayItem::activateResult);
    connect(d->m_item, &UkuiSni::StatusNotifierItem::menuStateChange, this, &TrayItem::menuStateChanged);
}

TrayItem::~TrayItem()
{
    if(d) {
        delete d;
        d = nullptr;
    }
}

QString TrayItem::service() const
{
    if (d->m_item) {
        return d->m_item->service();
    }
    return {};
}

QIcon TrayItem::attentionIcon() const
{
    if (d->m_item) {
        return d->m_item->attentionIcon().isNull() ? icon() : d->m_item->attentionIcon();
    }
    return {};
}

QString TrayItem::attentionIconName() const
{
    if(d->m_item) {
        return d->m_item->attentionIconName();
    }
    return {};
}

QString TrayItem::attentionMovieName() const
{
    if(d->m_item) {
        return d->m_item->attentionMovieName();
    }
    return {};
}

QString TrayItem::category() const
{
    if(d->m_item) {
        return d->m_item->category();
    }
    return {};
}

QIcon TrayItem::icon() const
{
    if (d->m_item) {
        return d->m_item->icon().isNull()? QIcon::fromTheme("application-x-executable") : d->m_item->icon();
    }
    return {};
}

QString TrayItem::iconName() const
{
    if(d->m_item) {
        return d->m_item->iconName();
    }
    return {};
}

QString TrayItem::iconThemePath() const
{
    if(d->m_item) {
        return d->m_item->iconThemePath();
    }
    return {};
}

QString TrayItem::id() const
{
    if(d->m_item) {
        return d->m_item->id();
    }
    return {};
}

bool TrayItem::itemIsMenu() const
{
    if(d->m_item) {
        return d->m_item->itemIsMenu();
    }
    return false;
}

QIcon TrayItem::overlayIcon() const
{
    if(d->m_item) {
        return d->m_item->overlayIcon();
    }
    return {};
}

QString TrayItem::overlayIconName() const
{
    if(d->m_item) {
        return d->m_item->overlayIconName();
    }
    return {};
}

QString TrayItem::status() const
{
    if(d->m_item) {
        return d->m_item->status();
    }
    return "Passive";
}

QString TrayItem::title() const
{
    if(d->m_item) {
        return d->m_item->title();
    }
    return {};
}

QString TrayItem::toolTipSubTitle() const
{
    if(d->m_item) {
        return d->m_item->toolTipSubTitle();
    }
    return {};
}

QString TrayItem::toolTipTitle() const
{
    if(d->m_item) {
        return d->m_item->toolTipTitle();
    }
    return {};
}

QString TrayItem::windowId() const
{
    if(d->m_item) {
        return d->m_item->windowId();
    }
    return {};
}

QString TrayItem::desktop() const
{
    return d->m_desktop;
}

uint TrayItem::pid() const
{
    return d->m_item->pid();
}
bool TrayItem::fixed() const
{
    return d->m_fixed;
}
void TrayItem::setFixed(bool fixed)
{
    d->m_fixed = fixed;
}

bool TrayItem::recordOrder() const
{
    return d->m_recordOrder;
}

void TrayItem::setRecordOrder(bool recordOrder)
{
    d->m_recordOrder = recordOrder;
}

void TrayItem::setParentItem(QQuickItem *item)
{
    d->m_parentItem = item;
}

void TrayItem::setDesktop(const QString &desktop)
{
    d->m_desktop = desktop;
}

QString TrayItem::source() const
{
    return d->m_source;
}

void TrayItem::contextMenu(int x, int y)
{
    d->m_item->contextMenu(x, y);
}

void TrayItem::activate(int x, int y)
{
    d->m_item->activate(x, y);
}

void TrayItem::secondaryActivate(int x, int y)
{
    d->m_item->secondaryActivate(x, y);
}

void TrayItem::scroll(int delta, const QString &direction)
{
    d->m_item->scroll(delta, direction);
}

void TrayItem::provideXdgActivationToken(const QString &token)
{
    d->m_item->provideXdgActivationToken(token);
}

void TrayItem::dataUpdated()
{
    if (!d->m_isReady) {
        if (d->m_item->id() != "") {
            d->m_isReady = true;
            Q_EMIT itemReady(this);
        }
    }
    Q_EMIT itemDataChanged(d->m_source);
}

void TrayItem::contextMenuReady(QMenu *menu)
{
    if(menu && !menu->isEmpty()) {
        menu->winId();
        if(menu->windowHandle() && d->m_parentItem) {
            menu->windowHandle()->setTransientParent(d->m_parentItem->window());
        }
        menu->popup(QCursor::pos());
    }
}
