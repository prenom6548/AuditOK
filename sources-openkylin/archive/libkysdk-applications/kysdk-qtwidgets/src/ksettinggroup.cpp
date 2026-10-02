/*
 *
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
 * Authors: Sisheng He <hesisheng@kylinos.cn>
 *
 */
#include "ksettinggroup.h"
#include <QApplication>
namespace kdk
{

class KSettingGroupPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KSettingGroup)
public:
    KSettingGroupPrivate(KSettingGroup *parent);

private:
    KSettingGroup *q_ptr;

    QVBoxLayout *m_pMainLayout;
    KBaseFrame::BorderRadiusStyle m_lastRadiusStyle = KBaseFrame::BorderRadiusStyle::Bottom;
    KBaseFrame::BorderRadiusStyle m_OneRadiusStyle = KBaseFrame::BorderRadiusStyle::Around;
    QList<KBaseFrame *> m_showItemList;
    QList<KBaseFrame *> m_allItemList;

private:
    /**
     * 递归添加显示出来的 KBaseFrame
     */
    void updateShowItemList(KBaseFrame *frame);
    /**
     * 递归获取所有 KBaseFrame
     */
    void updateAllItemList(KBaseFrame *frame);
};

KSettingGroupPrivate::KSettingGroupPrivate(KSettingGroup *parent)
    : q_ptr(parent)
{
    Q_Q(KSettingGroup);
    setParent(parent);
    m_pMainLayout = new QVBoxLayout(q);
    m_pMainLayout->setSpacing(1);
    m_pMainLayout->setContentsMargins(0, 0, 0, 0);
    q->setLayout(m_pMainLayout);
    qApp->installEventFilter(q);
    q->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}

void KSettingGroupPrivate::updateAllItemList(KBaseFrame *frame)
{
    if (frame->isContainer() && frame->frameShape() == QFrame::NoFrame && frame->layout()) {
        for (int i = 0; i < frame->layout()->count(); i++) {
            QLayoutItem *it = frame->layout()->itemAt(i);
            KBaseFrame *item = qobject_cast<KBaseFrame *>(it->widget());
            if (!item) {
                continue;
            }
            updateAllItemList(item);
        }
    } else {
        m_allItemList.append(frame);
        return;
    }
}

void KSettingGroupPrivate::updateShowItemList(KBaseFrame *frame)
{
    Q_Q(KSettingGroup);
    if (frame->isContainer() && frame->frameShape() == QFrame::NoFrame && frame->layout()) {
        for (int i = 0; i < frame->layout()->count(); i++) {
            QLayoutItem *it = frame->layout()->itemAt(i);
            KBaseFrame *item = qobject_cast<KBaseFrame *>(it->widget());
            if (!item) {
                continue;
            }
            updateShowItemList(item);
        }
    } else {
        if (frame->isVisibleTo(q)) {
            m_showItemList.append(frame);
        }
        return;
    }
}

KSettingGroup::KSettingGroup(QWidget *parent, Shape shape, BorderRadiusStyle style)
    : KBaseFrame(parent, style, false)
    , d_ptr(new KSettingGroupPrivate(this))
{
    Q_D(KSettingGroup);
    setFrameShape(shape);
}

KSettingGroup::~KSettingGroup()
{
}

void KSettingGroup::addWidget(QFrame *item, bool autoUpdateShape, bool visible)
{
    Q_D(KSettingGroup);
    d->m_pMainLayout->addWidget(item, autoUpdateShape);
    item->setVisible(visible);
    if (autoUpdateShape) {
        updateShape();
    }
}

void KSettingGroup::insertWidget(int index, QFrame *item, bool autoUpdateShape, bool visible)
{
    Q_D(KSettingGroup);
    d->m_pMainLayout->insertWidget(index, item);
    item->setVisible(visible);
    if (autoUpdateShape) {
        updateShape();
    }
}

void KSettingGroup::removeWidget(QWidget *w, bool autoUpdateShape)
{
    Q_D(KSettingGroup);
    d->m_pMainLayout->removeWidget(w);
    if (autoUpdateShape) {
        updateShape();
    }
}

void KSettingGroup::updateShape()
{
    Q_D(KSettingGroup);
    d->m_showItemList.clear();
    for (int i = 0; i < d->m_pMainLayout->count(); i++) {
        QLayoutItem *it = d->m_pMainLayout->itemAt(i);
        KBaseFrame *item = qobject_cast<KBaseFrame *>(it->widget());
        if (!item) {
            continue;
        }
        d->updateShowItemList(item);
    }

    for (int i = 0; i < d->m_showItemList.size(); i++) {
        KBaseFrame *item = d->m_showItemList.at(i);
        if (d->m_showItemList.size() == 1) {
            item->setRadiusType(d->m_OneRadiusStyle);
        } else if (i == 0) {
            item->setRadiusType(BorderRadiusStyle::Top);
        } else if (i == d->m_showItemList.size() - 1) {
            item->setRadiusType(d->m_lastRadiusStyle);
        } else {
            item->setRadiusType(BorderRadiusStyle::None);
        }
    }
}

QVBoxLayout *KSettingGroup::layout()
{
    Q_D(KSettingGroup);
    return d->m_pMainLayout;
}

void KSettingGroup::setLastRadiusStyle(BorderRadiusStyle style)
{
    Q_D(KSettingGroup);
    d->m_lastRadiusStyle = style;
}

void KSettingGroup::setOneRadiusStyle(BorderRadiusStyle style)
{
    Q_D(KSettingGroup);
    d->m_OneRadiusStyle = style;
}

bool KSettingGroup::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KSettingGroup);
    if (event->type() == QEvent::Hide || event->type() == QEvent::Show) {
        d->m_allItemList.clear();
        for (int i = 0; i < d->m_pMainLayout->count(); i++) {
            QLayoutItem *it = d->m_pMainLayout->itemAt(i);
            KBaseFrame *item = qobject_cast<KBaseFrame *>(it->widget());
            d->updateAllItemList(item);
        }
        for (int i = 0; i < d->m_allItemList.size(); i++) {
            if (d->m_allItemList.at(i) == watched) {
                updateShape();
            }
        }
    }
    return KBaseFrame::eventFilter(watched, event);
}

int KSettingGroup::showItemsCount()
{
    Q_D(KSettingGroup);
    d->m_showItemList.clear();
    for (int i = 0; i < d->m_pMainLayout->count(); i++) {
        QLayoutItem *it = d->m_pMainLayout->itemAt(i);
        KBaseFrame *item = qobject_cast<KBaseFrame *>(it->widget());
        if (!item) {
            continue;
        }
        d->updateShowItemList(item);
    }
    return d->m_showItemList.size();
}

void KSettingGroup::removeAndDeleteAllWidget()
{
    Q_D(KSettingGroup);
    while (d->m_pMainLayout->count() > 0) {
        QLayoutItem *it = d->m_pMainLayout->itemAt(0);
        KBaseFrame *item = qobject_cast<KBaseFrame *>(it->widget());
        if (!item) {
            continue;
        }
        d->m_pMainLayout->removeWidget(item);
        item->deleteLater();
        item = nullptr;
    }
    return;
}

}

#include "ksettinggroup.moc"
#include "moc_ksettinggroup.cpp"
