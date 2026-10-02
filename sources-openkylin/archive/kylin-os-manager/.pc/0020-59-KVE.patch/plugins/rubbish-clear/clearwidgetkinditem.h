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

#ifndef CLEARWIDGETKINDITEM_H
#define CLEARWIDGETKINDITEM_H
#include <QWidget>
#include <QPushButton>
#include <QLabel>

namespace KylinRubbishClear
{

class ClearWidgetKindItem : public QWidget
{
    Q_OBJECT
public:
    explicit ClearWidgetKindItem(QWidget *parent = nullptr);
    QPushButton *mBtn = nullptr;
    QLabel *mLabelIcon = nullptr;
    QLabel *mLabelTips = nullptr;

    void setWidgetContent(const QString icon, const QString btnText, const QString tipText);
    void setWidgetItemIcon(const QString icon);
    void setWidgetItemBtnText(const QString btnText);
    void setWidgetItemTipText(const QString tipText);
    void changeSystemFontSize(int size);

private:
    QString mStrTips = "";

Q_SIGNALS:
    void sigKindItemClick();
};

} // namespace KylinRubbishClear


#endif // CLEARWIDGETKINDITEM_H
