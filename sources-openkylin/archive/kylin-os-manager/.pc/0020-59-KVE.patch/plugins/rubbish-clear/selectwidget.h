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

#ifndef SELECTWIDGET_H
#define SELECTWIDGET_H

#include <QDialog>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

#include "selectlistwidget.h"
#include "utils.h"
#include "kdialog.h"

using namespace kdk;

namespace KylinRubbishClear
{

class SelectWidget : public KDialog
{
    Q_OBJECT
public:
    SelectWidget(CleanerModuleID id = InvalidID, const QString &title = "", bool needMin = false, QWidget *parent = 0,
                 bool t = false);
    ~SelectWidget();

    void loadData(const QString &title, const QStringList &cachelist, const QStringList &baklist);
    void moveCenter();

    void paintEvent(QPaintEvent *event);
public Q_SLOTS:
    void onClose();

Q_SIGNALS:
    void notifyMainCheckBox(int status);
    void refreshSelectedItems(CleanerModuleID id, const QStringList &selecteds);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    CleanerModuleID m_id;
    QVBoxLayout *m_mainLayout = nullptr;
    SelectListWidget *m_listWidget = nullptr;
    QPoint m_dragPosition; //移动的距离
    bool flag;
    bool m_mousePressed; //按下鼠标左键
};

} // namespace KylinRubbishClear

#endif // SELECTWIDGET_H
