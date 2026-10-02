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

#ifndef KTEXTEDITOR_H
#define KTEXTEDITOR_H

#include "gui_g.h"
#include <QTextEdit>
#include <QWidget>

namespace kdk
{

/**
 * @defgroup TextEditModule
 * @since 2.5.2.0
 * {
 */
class KTextEditorPrivate;

class GUI_EXPORT KTextEditor : public QWidget
{
    Q_OBJECT
public:
    KTextEditor(QWidget *parent = nullptr);

    /**
     * @brief 获取QTextedit
     * @return
     */
    QTextEdit *textEdit();

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Q_DECLARE_PRIVATE(KTextEditor)
    KTextEditorPrivate *const d_ptr;
};
}
#endif // KTEXTEDITOR_H
