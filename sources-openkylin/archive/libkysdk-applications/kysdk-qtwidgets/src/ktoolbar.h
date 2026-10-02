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

#ifndef KTOOLBAR_H
#define KTOOLBAR_H

#include "gui_g.h"
#include "ktoolbutton.h"
#include <QPushButton>
#include <QWidget>

namespace kdk
{

class KToolBarPrivate;

/**
 * @since 2.5.2.0
 */
class GUI_EXPORT KToolBar : public QWidget
{
    Q_OBJECT
public:
    KToolBar(QWidget *parent = nullptr);

    Qt::Orientation orientation();

    void setOrientation(Qt::Orientation orientation);

    KToolButton *addButton(QIcon icon);

    KToolButton *addButton(QString string);

    //    KToolButton* addButton (const QIcon&icon ,const QString text = QString());
    QList<KToolButton *> addButton(QList<QIcon> list);

    QList<KToolButton *> addButton(QList<QString> list);

    QList<KToolButton *> buttonList();

    void removeButton(KToolButton *button);

    void removeButton(int id);

    void removeButton(QList<KToolButton *> list);

    void setBorderRadius(int radius);

    int borderRadius();

    void setExclusive(bool);

    bool exclusive();

    void setCheckable(bool flag);

    bool isCheckable();

    QList<KToolButton *> checkedButton();

    void setLineVisible(bool flag);

    bool isLineVisible();

    void setBackgroundColorEnabled(bool flag);

    bool backgroundColorEnabled();

protected:
    void paintEvent(QPaintEvent *event);

private:
    Q_DECLARE_PRIVATE(KToolBar)
    KToolBarPrivate *const d_ptr;
};

}

#endif // KTOOLBAR_H
