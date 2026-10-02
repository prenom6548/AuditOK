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
#ifndef CUSTOM_COMBO_BOX_H
#define CUSTOM_COMBO_BOX_H

#include <QComboBox>
#include <QString>

class CustomComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit CustomComboBox(QWidget *parent = nullptr);
    void setDefaultPreItem(const QString &text);
    void removeDefaultItem();
    void setDefaultAfterItem(const QString &text);
    void addItem(const QString &text);
    void addItems(const QStringList &texts);
    void checkDefaultItem();
    void clear();
    bool isEmpty();

private:
    int m_preItemSize{0};
    int m_tailItemSize{0};
};
#endif // CUSTOM_COMBO_BOX_H
