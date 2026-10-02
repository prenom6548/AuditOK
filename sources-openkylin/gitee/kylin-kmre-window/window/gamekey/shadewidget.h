/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Zero Liu    liuzenghui1@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SHADEWIDGET_H
#define SHADEWIDGET_H
#include "joysticksinglekey.h"
#include "joystickdirectkey.h"
#include "joysticksetwidget.h"

#include <QWidget>
#include <QPushButton>
#include <QToolButton>
#include <QGridLayout>
#include <QToolButton>
#include <QSlider>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QList>
#include <mutex>

class KmreWindow;

class ShadeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ShadeWidget(KmreWindow* window);
    ~ShadeWidget();
    void updatePos();
    void showShadeWidget(bool show);

private:  
    void initShadeWidget();
    void checkPosOutsideScreen(QPoint pos);
    KmreWindow* mMainWindow = nullptr;
    QLabel* mlabel = nullptr;
    bool m_outsideScreen = false;

protected:
    bool eventFilter(QObject *obj, QEvent *evt) Q_DECL_OVERRIDE;

signals:

public slots:

};

#endif // SHADEWIDGET_H
