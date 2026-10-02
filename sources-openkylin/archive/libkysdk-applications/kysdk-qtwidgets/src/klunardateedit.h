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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#ifndef KLUNARDATEEDIT_H
#define KLUNARDATEEDIT_H

#include "gui_g.h"
#include <QDateEdit>
#include <QObject>
#include <QWidget>

namespace kdk
{
class KLunarDateEditPrivate;

/**
 *  @defgroup KLunarDateEdit
 *  @since 2.5.2.0
 */

class GUI_EXPORT KLunarDateEdit : public QWidget
{
    Q_OBJECT
public:
    explicit KLunarDateEdit(QWidget *parent = nullptr);

    /**
     * @brief 是否显示农历
     * @param showLunar：true显示，false不显示
     */
    void setShowLunar(bool showLunar);

    /**
     * @brief 获取控件中的DateEdit控件
     * @return
     */
    QDateTimeEdit *getDateEdit();

protected:
    void resizeEvent(QResizeEvent *event);

signals:
    void dateChanged(QDate);

private slots:
    void slotPaletteChanged();
    void slotDateChanged(QDate date);
    void slotClicked();

private:
    Q_DECLARE_PRIVATE(KLunarDateEdit)
    KLunarDateEditPrivate *const d_ptr;
};
}

#endif // KLUNARDATEEDIT_H
