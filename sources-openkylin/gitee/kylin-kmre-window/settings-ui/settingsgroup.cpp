/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "settingsgroup.h"
#include "settingsitem.h"

#include <QVBoxLayout>
#include <QEvent>
#include <QDebug>

SettingsGroup::SettingsGroup(QFrame *parent) :
    QFrame(parent),
    m_layout(new QVBoxLayout),
    m_timer(new QTimer(this))
{
    //this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    this->setStyleSheet("QFrame{border-radius:6px;background:palette(Window);}");
    m_layout->setMargin(0);
    m_layout->setSpacing(1);
    m_timer->setSingleShot(true);
    m_timer->setInterval(10);
    connect(m_timer, &QTimer::timeout, this, &SettingsGroup::updateHeight, Qt::QueuedConnection);
    this->setLayout(m_layout);
}

SettingsGroup::SettingsGroup(const QString &title, QFrame *parent)
    : SettingsGroup(parent)
{
    setAccessibleName(title);
}

SettingsGroup::~SettingsGroup()
{

}

void SettingsGroup::insertItem(const int index, SettingsItem *item)
{
    m_layout->insertWidget(index, item);
    item->installEventFilter(this);
    m_timer->start();
}

void SettingsGroup::appendItem(SettingsItem *item)
{
    insertItem(m_layout->count(), item);
}

void SettingsGroup::removeItem(SettingsItem *item)
{
    m_layout->removeWidget(item);
    item->removeEventFilter(this);

    m_timer->start();
}

int SettingsGroup::itemCount() const
{
    return m_layout->count();
}

void SettingsGroup::clear()
{
    const int count = m_layout->count();
    for (int i = 0; i != count; ++i) {
        QLayoutItem *item = m_layout->takeAt(0);
        QWidget *w = item->widget();
        w->removeEventFilter(this);
        w->setParent(nullptr);
        delete item;
    }
    m_timer->start();
}

SettingsItem *SettingsGroup::getItem(int index)
{
    if (index < 0) {
        return NULL;
    }

    if (index < itemCount()) {
        return qobject_cast<SettingsItem *>(m_layout->itemAt(index)->widget());
    }

    return NULL;
}

void SettingsGroup::updateHeight()
{
    Q_ASSERT(sender() == m_timer);
    setFixedHeight(m_layout->sizeHint().height());
}

bool SettingsGroup::eventFilter(QObject *, QEvent *event)
{
    switch (event->type())
    {
    case QEvent::Resize:
    {
        m_timer->start();
        break;
    }
    default:;
    }

    return false;
}
