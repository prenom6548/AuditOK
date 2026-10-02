/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef KCUSTOMDATEEDIT_H
#define KCUSTOMDATEEDIT_H

#include "gui_g.h"
#include "kminicalendarwidget.h"
#include <QDateTimeEdit>
#include <QMouseEvent>

namespace kdk
{
class KCustomDateEditPrivate;

/**
 *  @defgroup KCustomDateEdit
 *  @since 2.5.2.0
 */

class GUI_EXPORT KCustomDateEdit : public QDateTimeEdit
{
    Q_OBJECT
public:
    KCustomDateEdit(QWidget *parent = nullptr);

    /**
     * @brief 触发日历控件显示
     */
    void showCalendar();

    /**
     * @brief 是否显示农历
     * @param showLunar
     */
    void setShowLunar(bool showLunar);

    /**
     * @brief 适应系统风格
     * @param isDark：是否深色模式
     */
    void updateStyle(bool isDark);

signals:
    void changeDate();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    Q_DECLARE_PRIVATE(KCustomDateEdit)
    KCustomDateEditPrivate *const d_ptr;
};
}

#endif // KCUSTOMDATEEDIT_H
