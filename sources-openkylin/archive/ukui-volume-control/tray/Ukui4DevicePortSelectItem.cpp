/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "Ukui4DevicePortSelectItem.h"

#include <QHBoxLayout>
#include <QDebug>

Ukui4DevicePortItem::Ukui4DevicePortItem(QLabel* label, QPushButton* btn, const QString& devName, const QString& portName, QWidget* parent) :
    m_pDisplayLabel(label), m_pIconBtn(btn), IDevicePortItem(devName, portName), QWidget(parent)
{
    initUi();
}

void Ukui4DevicePortItem::initUi()
{
    setFixedSize(404, 48);

    QHBoxLayout* hLayout = new QHBoxLayout;
    m_pIconBtn->setFixedSize(36, 36);
    m_pIconBtn->setCheckable(true);
    m_pIconBtn->setProperty("isRoundButton", true);
    m_pIconBtn->setProperty("useButtonPalette", true);
    m_pIconBtn->setProperty("needTranslucent", true);

    hLayout->addWidget(m_pIconBtn);
    hLayout->addSpacing(12);
    hLayout->addWidget(m_pDisplayLabel);
    hLayout->setSpacing(0);
    setLayout(hLayout);
    hLayout->setContentsMargins(14, 6, 24, 6);
}

bool Ukui4DevicePortItem::isChecked()
{
    return m_pIconBtn->isChecked();
}

void Ukui4DevicePortItem::setChecked(bool checked)
{
    m_pIconBtn->setChecked(checked);
}

void Ukui4DevicePortItem::setIcon(const QIcon& icon)
{
    m_pIconBtn->setIcon(icon);
}

QWidget* Ukui4DevicePortItem::getWidget()
{
    return this;
}

Ukui4DevicePortSelectItem::Ukui4DevicePortSelectItem(QListWidget* w, QWidget* parent) :
    m_pListWidget(w), QWidget(parent)
{
    setFixedSize(412, 245);
    m_pListWidget->setParent(this);
    m_pListWidget->setFixedSize(412, 245);
    m_pListWidget->setFrameShape(QFrame::Shape::NoFrame);

    QPalette pal = m_pListWidget->palette();
    pal.setBrush(QPalette::Base, QColor(0, 0, 0, 0));
    m_pListWidget->setPalette(pal);
    m_pListWidget->setProperty("needTranslucent", true);
    m_pListWidget->setSelectionMode(QAbstractItemView:: NoSelection);
}

QListWidget* Ukui4DevicePortSelectItem::getListWidget() const
{
    return m_pListWidget;
}

void Ukui4DevicePortSelectItem::initSlots()
{
//    connect(m_pListWidget, SIGNAL(currentRowChanged(int)), this, SLOT(currentRowChangedSlots(int)));
}

void Ukui4DevicePortSelectItem::setCurrentIndex(int idx)
{
    if (m_pListWidget->count() < idx) {
        qDebug() << "invaild idx...";
        return;
    }

    QModelIndex index = m_pListWidget->model()->index(idx, 0);
    m_pListWidget->setCurrentIndex(index);

    setListWidgetSelectItem(idx);
}

void Ukui4DevicePortSelectItem::insertItem(std::shared_ptr<IDevicePortItem> portItem)
{
    m_pListWidget->update();
    auto item = new QListWidgetItem(m_pListWidget);
    item->setSizeHint(QSize(200, 48));

    m_pListWidget->setItemWidget(item, portItem->getWidget());
    m_pListWidget->insertItem(m_pListWidget->count(), item);
}

void Ukui4DevicePortSelectItem::removeItem(const QString& portName, const QString& devName)
{
    for (int i = 0; i < m_pListWidget->count(); ++i) {
        auto item = m_pListWidget->item(i);
        auto wid = (Ukui4DevicePortItem*) m_pListWidget->itemWidget(item);
        if (portName == wid->getPortName() && devName == wid->getCardName()) {
            qDebug() << "before remove listwidget.size(): " << m_pListWidget->count();
            m_pListWidget->blockSignals(true);
            auto item = m_pListWidget->takeItem(i);
            m_pListWidget->removeItemWidget(item);
            m_pListWidget->blockSignals(false);
            wid = nullptr;
            delete wid;
            qDebug() << "remove item, portName: " << portName << " cardName: " << devName
                     << " listwidget.size: " << m_pListWidget->count();
            break;
        }
    }
}

QVariant Ukui4DevicePortSelectItem::currentData(int role) const
{
    Q_UNUSED(role);
    for (int i = 0; i < m_pListWidget->count(); ++i) {
        auto item = m_pListWidget->item(i);
        auto wid = static_cast<Ukui4DevicePortItem*>(m_pListWidget->itemWidget(item));
        if (wid && wid->isChecked()) {
            return QVariant(QStringList{wid->getPortName(), wid->getCardName()});
        }
    }
    return QVariant();
}

int Ukui4DevicePortSelectItem::count() const
{
    return m_pListWidget->count();
}

QWidget* Ukui4DevicePortSelectItem::getWidget()
{
    return this;
}

void Ukui4DevicePortSelectItem::currentRowChangedSlots(int row)
{
    auto w = (Ukui4DevicePortItem*) m_pListWidget->itemWidget(m_pListWidget->item(row));
    setListWidgetSelectItem(row);
//    Q_EMIT deviceChangedSignals(w->getPortName(), w->getCardName());
}

void Ukui4DevicePortSelectItem::setListWidgetSelectItem(int row)
{
    for (int i = 0; i < m_pListWidget->count(); ++i) {
        auto item = m_pListWidget->item(i);
        auto wid = (Ukui4DevicePortItem*) m_pListWidget->itemWidget(item);
        if (i == row && !wid->isChecked()) {
            wid->setChecked(true);
        }
        else {
            wid->setChecked(false);
        }
    }
}
