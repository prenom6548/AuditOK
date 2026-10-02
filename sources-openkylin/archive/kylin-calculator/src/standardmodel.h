/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef STANDARDMODEL_H
#define STANDARDMODEL_H

#include <QKeyEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFont>
#include <QLabel>
#include <QWidget>
#include <QListWidget>
#include <QDebug>

#include "basicbutton.h"
#include "widgetstyle.h"
#include "HorizontalOrVerticalMode.h"
#include "../common/picturetowhite.h"
#include "outputbase.h"

class StandardOutput : public OutputBase
{
    Q_OBJECT

public:
    explicit StandardOutput(QWidget *parent = 0);
    ~StandardOutput() {}

    // 设置组件样式
    void setWidgetStyle(bool resetFontSize = true);

    void createIntelOutput(void);

private:
    // 初始化组件
    void setWidgetUi();
};


class StandardModel : public QWidget
{
    Q_OBJECT

public:
    explicit StandardModel(QWidget *parent = 0);

    BasicButton *btnNum[10];

    BasicButton *btnClear;
    BasicButton *btnPer;
    BasicButton *btnDelete;

    BasicButton *btnDiv;
    BasicButton *btnMulti;
    BasicButton *btnSub;
    BasicButton *btnAdd;
    BasicButton *btnEqual;

    // BasicButton *btnDZero;
    BasicButton *btnPoint;

    PictureToWhite *m_picturetowhite;

    // 初始化组件
    void setWidgetUi();

    /* intel stand model ui */
    void createIntelModel(void);
    void createIntelStyle();
    void changeBtnIconSize();

public slots:
    // 键盘响应事件
    void keyPressEvent(QKeyEvent *event);
};

#endif // STANDARDMODEL_H
