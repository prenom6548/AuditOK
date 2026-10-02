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

#ifndef SELECTCATEGORYWIDGET_H
#define SELECTCATEGORYWIDGET_H

#include <QDialog>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QtMath>
#include <QPainterPath>
#include <QPainter>

#include "selectlistwidget.h"
#include "utils.h"
#include "kdialog.h"
using namespace kdk;

namespace KylinRubbishClear
{

class SelectCategoryWidget : public KDialog
{
    Q_OBJECT

public:
    SelectCategoryWidget(CleanerCategoryID id = InvalidCategory, const QString &title = "", bool needMin = false,
                         QWidget *parent = 0);
    ~SelectCategoryWidget();

    void loadData(const QStringList &arglist, const QStringList &statuslist, const QStringList &baklist);
    void moveCenter();

    void paintEvent(QPaintEvent *event);
public Q_SLOTS:
    void onClose();

Q_SIGNALS:
    void notifyMainCheckBox(int status);
    void refreshSelectedItems(CleanerCategoryID id, const QStringList &selecteds);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    CleanerCategoryID m_id;
    QVBoxLayout *m_mainLayout = nullptr;
    SelectListWidget *m_listWidget = nullptr;
    QPoint m_dragPosition; //移动的距离
    bool m_mousePressed;   //按下鼠标左键
};

} // namespace KylinRubbishClear


#endif // SELECTCATEGORYWIDGET_H
