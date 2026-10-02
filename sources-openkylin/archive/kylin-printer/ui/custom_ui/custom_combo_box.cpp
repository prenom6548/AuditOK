/*
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
 */
#include "custom_combo_box.h"
#include <QDebug>

CustomComboBox::CustomComboBox(QWidget *parent)
    : QComboBox(parent)
{

}

void CustomComboBox::setDefaultPreItem(const QString &text)
{
    QComboBox::insertItem(m_preItemSize, text);
    m_preItemSize++;


}

void CustomComboBox::removeDefaultItem()
{
    QComboBox::setCurrentIndex(-1);
    int cnt = m_preItemSize;
    while(cnt--) {
        QComboBox::removeItem(0);
    }
    m_preItemSize = 0;
    if (isEmpty()) {
        QComboBox::setCurrentIndex(-1);
    } else {
        checkDefaultItem();
    }
}

void CustomComboBox::setDefaultAfterItem(const QString &text)
{
    QComboBox::insertItem(QComboBox::count(), text);
    m_tailItemSize++;

    checkDefaultItem();
}

void CustomComboBox::addItem(const QString &text)
{
    QComboBox::insertItem(QComboBox::count() - m_tailItemSize, text);
    checkDefaultItem();
}

void CustomComboBox::addItems(const QStringList &texts)
{
    for (auto text : texts) {
        addItem(text);
    }
}

void CustomComboBox::clear()
{
    QComboBox::setCurrentIndex(-1);
    int cnt = QComboBox::count() - m_preItemSize - m_tailItemSize;
    while(cnt--) {
        QComboBox::removeItem(m_preItemSize);
    }
    checkDefaultItem();
}

void CustomComboBox::checkDefaultItem()
{
    if (QComboBox::count() == 0) {
        QComboBox::setCurrentIndex(-1);
    }
    if (isEmpty()) {
        QComboBox::setCurrentIndex(0);
    } else {
        QComboBox::setCurrentIndex(m_preItemSize);
    }
}

bool CustomComboBox::isEmpty()
{
    return !(QComboBox::count() - m_preItemSize - m_tailItemSize);
}
