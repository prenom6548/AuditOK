/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef SELECTLISTITEM_H
#define SELECTLISTITEM_H

#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QCheckBox>
#include "custom_label.h"
namespace KylinRubbishClear
{

class SelectListItem : public QWidget
{
    Q_OBJECT
public:
    explicit SelectListItem(QWidget *parent = 0, QString description = "", QString tipMsg = "", bool hasTip = false,
                            int itemWidth = 0, bool check = true);
    ~SelectListItem();

    bool itemIsChecked();
    QString itemDescription();

Q_SIGNALS:
    void selectedSignal(bool checked, QString description);

private:
    QString m_description;
    QString m_tip;
    bool m_hasTip;
    QHBoxLayout *m_mainLayout = nullptr;
    QCheckBox *m_checkBox = nullptr;
    CustomLabel *m_descLabel = nullptr;
};

} // namespace KylinRubbishClear


#endif // SELECTLISTITEM_H
