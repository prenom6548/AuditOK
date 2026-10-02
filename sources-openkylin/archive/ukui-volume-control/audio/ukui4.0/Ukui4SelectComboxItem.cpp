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

#include "Ukui4SelectComboxItem.h"
#include <QHBoxLayout>
#include <QDBusMessage>
#include <iostream>
#include <QDebug>

#include "ClientMethod.h"
#include "Ukcc5DevicePortItem.h"

#include <kysdk/applications/accessinfohelper.h>

using namespace kdk;

Ukui4SelectCombox::Ukui4SelectCombox(QComboBox* box, const QString& objName)
    : m_pCombox(box)
{
    KDK_EXTEND_ALL_INFO_FORMAT(m_pCombox, "Audio", objName, "");
}

void Ukui4SelectCombox::setCurrentIndex(int idx)
{
    // check idx vaild
    if (idx >= m_pCombox->count()) {
        std::cout << "invaild index: " << idx << std::endl;
        return;
    }

    m_pCombox->blockSignals(true);
    m_pCombox->setCurrentIndex(idx);
    m_pCombox->blockSignals(false);
}

void Ukui4SelectCombox::setEnabled(bool enable)
{
    m_pCombox->setEnabled(enable);
}

bool Ukui4SelectCombox::isEnabled() const
{
    return m_pCombox->isEnabled();
}

int Ukui4SelectCombox::findData(const QVariant& value, int role)
{
    return m_pCombox->findData(value, role);
}

int Ukui4SelectCombox::findText(const QString& text)
{
    return m_pCombox->findText(text);
}

void Ukui4SelectCombox::insertItem(const QString& text, const QVariant& userData)
{
    m_pCombox->blockSignals(true);
    m_pCombox->insertItem(m_pCombox->count(), text, userData);
    m_pCombox->blockSignals(false);
}

void Ukui4SelectCombox::removeItem(int index)
{
    m_pCombox->blockSignals(true);
    m_pCombox->removeItem(index);
    m_pCombox->blockSignals(false);
}

QVariant Ukui4SelectCombox::currentData(int role) const
{
    return m_pCombox->currentData();
}

std::list<QVariant> Ukui4SelectCombox::getDataList() const
{
    std::list<QVariant> list;

    for (int i = 0; i < m_pCombox->count(); ++i) {

        QList<QVariant> v;
        auto data = m_pCombox->itemData(i).toStringList();
        v.push_back(i);
        v.push_back(data.at(0));
        v.push_back(data.at(1));
        list.push_back(QVariant(v));
    }

    return list;
}

int Ukui4SelectCombox::count() const
{
    return m_pCombox->count();
}

void Ukui4SelectCombox::clear()
{
    m_pCombox->clear();
}

QWidget* Ukui4SelectCombox::getWidget()
{
    return m_pCombox;
}

void Ukui4SelectCombox::setParent(QWidget* parent)
{
    m_pCombox->setParent(parent);
}

Ukui4SelectComboxItem::Ukui4SelectComboxItem(std::shared_ptr<ISelectBox> box, QLabel* label, SelectComboxType type, QWidget* parent)
    : ISelectComboxItem(box, label), m_type(type), QFrame(parent)
{
    initUi();
    initSlots();
}

void Ukui4SelectComboxItem::initUi()
{
    setFixedHeight(60);

    m_pDisplayLabel->setFixedHeight(28);
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hLayout->addWidget(m_pDisplayLabel);
    hLayout->addItem(new QSpacerItem(93, 20, QSizePolicy::Fixed));
    hLayout->addWidget(m_pSelectBox->getWidget());
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(hLayout);
//    setFrameShape(QFrame::Shape::Box);
}

void Ukui4SelectComboxItem::initSlots()
{
}

QWidget* Ukui4SelectComboxItem::getWidget()
{
    return this;
}

void Ukui4SelectComboxItem::setEnabled(bool enable)
{
    m_pSelectBox->setEnabled(enable);
}

Ukui5SelectCombox::Ukui5SelectCombox(QListWidget* box, const QString& objName)
    : m_pCombox(box)
{
    m_pCombox->setFixedHeight(130);

    auto pal = m_pCombox->palette();
    pal.setBrush(QPalette::Base, QColor(0, 0, 0, 0));
    m_pCombox->setPalette(pal);

    KDK_EXTEND_ALL_INFO_FORMAT(m_pCombox, "Audio", objName, "");
}

void Ukui5SelectCombox::setCurrentIndex(int idx)
{
    // check idx vaild
    if (idx >= m_pCombox->count()) {
        std::cout << "invaild index: " << idx << std::endl;
        return;
    }

    m_pCombox->blockSignals(true);
    m_pCombox->setCurrentIndex(m_pCombox->model()->index(idx, 0));
    m_pCombox->blockSignals(false);

    setListWidgetSelectItem(idx);
}

void Ukui5SelectCombox::setEnabled(bool enable)
{
    m_pCombox->setEnabled(enable);
}

bool Ukui5SelectCombox::isEnabled() const
{
    return m_pCombox->isEnabled();
}

int Ukui5SelectCombox::findData(const QVariant& value, int role)
{
    int idx = -1;
    for (int i = 0; i < m_pCombox->count(); ++i) {
        auto item = m_pCombox->item(i);
        auto wid = (Ukcc5DevicePortItem*) m_pCombox->itemWidget(item);
        if (2 != value.toStringList().size()) {
            std::cerr << "Invaild value!" << std::endl;
            return idx;
        }

        if (value.toStringList().at(0) == wid->getCardName() && value.toStringList().at(1) == wid->getPortName()) {
            idx = i;
            break;
        }
    }

    return idx;
}

int Ukui5SelectCombox::findText(const QString& text)
{
//    return m_pCombox->findText(text);
    return -1;
}

void Ukui5SelectCombox::insertItem(const QString& text, const QVariant& userData)
{
    qDebug() << "insertItem " << text << " data:" << userData;
    auto list = userData.toStringList();
    auto item = new QListWidgetItem(m_pCombox);
    item->setData(Qt::AccessibleTextRole, QString("Audio: item"));
    item->setData(Qt::AccessibleDescriptionRole, QString("Audio: item"));
    item->setSizeHint(QSize(200, 36));
    auto label = new KLabel;
    label->setText(text);
    auto widget = new Ukcc5DevicePortItem(label, new QLabel(), list.at(0), list.at(1));

    m_pCombox->blockSignals(true);
    m_pCombox->setItemWidget(item, widget);
    m_pCombox->insertItem(m_pCombox->count(), item);
    m_pCombox->blockSignals(false);
    m_pCombox->update();
}

void Ukui5SelectCombox::removeItem(int index)
{
    m_pCombox->blockSignals(true);
    m_pCombox->removeItemWidget(m_pCombox->takeItem(index));
    m_pCombox->blockSignals(false);
//    auto item = m_pCombox->itemAt(index);
//    m_pCombox->removeItemWidget(item);
}

QVariant Ukui5SelectCombox::currentData(int role) const
{
    auto item = m_pCombox->currentItem();
    auto wid = (Ukcc5DevicePortItem*) m_pCombox->itemWidget(item);

    return QVariant(std::initializer_list<QString> {wid->getPortName(), wid->getCardName()});
}

std::list<QVariant> Ukui5SelectCombox::getDataList() const
{
    std::list<QVariant> list;

    for (int i = 0; i < m_pCombox->count(); ++i) {
        auto item = m_pCombox->item(i);
        auto wid = (Ukcc5DevicePortItem*)m_pCombox->itemWidget(item);
        QList<QVariant> v;
        v.push_back(i);
        v.push_back(wid->getCardName());
        v.push_back(wid->getPortName());
        list.push_back(QVariant(v));
    }

    return list;
}

int Ukui5SelectCombox::count() const
{
    return m_pCombox->count();
}

void Ukui5SelectCombox::clear()
{
    m_pCombox->blockSignals(true);
    m_pCombox->clear();
    m_pCombox->blockSignals(false);
}

QWidget* Ukui5SelectCombox::getWidget()
{
    return m_pCombox;
}

void Ukui5SelectCombox::setParent(QWidget* parent)
{
    m_pCombox->setParent(parent);
}

void Ukui5SelectCombox::setListWidgetSelectItem(int row)
{
    for (int i = 0; i < m_pCombox->count(); ++i) {
        auto item = m_pCombox->item(i);
        auto wid = (Ukcc5DevicePortItem*) m_pCombox->itemWidget(item);

        if (i == row /*&& !wid->isChecked()*/) {
            wid->setChecked(true);
        }
        else {
            wid->setChecked(false);
        }
    }
}

Ukui5SelectComboxItem::Ukui5SelectComboxItem(std::shared_ptr<ISelectBox> box, QLabel* label, SelectComboxType type, QWidget* parent)
    : ISelectComboxItem(box, label), m_type(type), QFrame(parent)
{
    initUi();
    initSlots();
}

void Ukui5SelectComboxItem::initUi()
{
    setFixedHeight(190);

    m_pDisplayLabel->setFixedHeight(28);
    QVBoxLayout* vLayout = new QVBoxLayout();
    vLayout->addItem(new QSpacerItem(16, 10, QSizePolicy::Fixed));
    vLayout->addWidget(m_pDisplayLabel);
    vLayout->addItem(new QSpacerItem(93, 10, QSizePolicy::Fixed));
    vLayout->addWidget(m_pSelectBox->getWidget());
    vLayout->addItem(new QSpacerItem(16, 10, QSizePolicy::Fixed));
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(16, 0, 20, 0);
    setLayout(vLayout);
//    setFrameShape(QFrame::Shape::Box);
}

void Ukui5SelectComboxItem::initSlots()
{
}

void Ukui5SelectComboxItem::setEnabled(bool enable)
{
    m_pSelectBox->setEnabled(enable);
}

QWidget* Ukui5SelectComboxItem::getWidget()
{
    return this;
}
