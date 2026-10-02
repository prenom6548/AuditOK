/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef KLINEEDIT_H
#define KLINEEDIT_H

#include "gui_g.h"
#include <QLineEdit>
#include <QPushButton>

namespace kdk
{

class KLineEditPrivate;

/**
 * @defgroup InputBoxModule
 * @since 2.5.2.0
 * {
 */

class GUI_EXPORT KLineEdit : public QLineEdit
{
    Q_OBJECT
public:
    KLineEdit(QWidget *parent = nullptr);

    /**
     * @brief 获取按钮
     * @return
     */
    QPushButton *pushbutton();

protected:
    void resizeEvent(QResizeEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Q_DECLARE_PRIVATE(KLineEdit)
    KLineEditPrivate *const d_ptr;
};

}
#endif // KLINEEDIT_H
