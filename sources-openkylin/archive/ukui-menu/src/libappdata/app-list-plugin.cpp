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
 * Authors: hxf <hewenfei@kylinos.cn>
 *
 */

#include "app-list-plugin.h"
#include <QDebug>
#include <utility>

namespace UkuiMenu {

// ====== AppListPluginInterface ====== //
AppListPluginInterface::AppListPluginInterface(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<UkuiMenu::LabelItem>("LabelItem");
    qRegisterMetaType<UkuiMenu::LabelBottle*>("LabelBottle*");
}

void AppListPluginInterface::search(const QString &keyword)
{
    Q_UNUSED(keyword)
}

LabelBottle *AppListPluginInterface::labelBottle()
{
    return nullptr;
}

// ====== LabelItem ====== //
LabelItem::LabelItem(QString labelName, QString displayName, LabelItem::Type type)
  : m_type(type), m_labelName(std::move(labelName)), m_displayName(std::move(displayName))
{

}

QString LabelItem::labelName() const
{
    return m_labelName;
}

LabelItem::Type LabelItem::type() const
{
    return m_type;
}

QString LabelItem::displayName() const
{
    return m_displayName;
}

// ====== LabelBottle ====== //
LabelBottle::LabelBottle(QObject *parent) : QObject(parent)
{

}

int LabelBottle::column() const
{
    return m_column;
}

bool LabelBottle::visible() const
{
    return m_visible;
}

QList<UkuiMenu::LabelItem> LabelBottle::labels() const
{
    return m_labels;
}

void LabelBottle::setColumn(int column)
{
    if (m_column == column) {
        return;
    }

    m_column = column;
    Q_EMIT columnChanged();
}

void LabelBottle::setVisible(bool visible)
{
    if (m_visible == visible) {
        return;
    }

    m_visible = visible;
    Q_EMIT visibleChanged();
}

void LabelBottle::setLabels(const QList<UkuiMenu::LabelItem> &labels)
{
    m_labels.clear();
    m_labels.append(labels);
    Q_EMIT labelsChanged();
}

} // UkuiMenu
