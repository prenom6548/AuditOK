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

#ifndef QUAD_BTNS_TITLE_BAR_H
#define QUAD_BTNS_TITLE_BAR_H

#include <QObject>
#include <QBoxLayout>
#include "menumodule.h"

enum class QuadBtnsShowMode : int {
    ALL = 0,
    NO_MENU_BTN,
    NO_MAX_BTN,
    ONLY_CLOSE_BTN,
};

class QuadBtnsTitleBar : public QWidget
{
    Q_OBJECT
public:
    QuadBtnsTitleBar(QWidget *mainWin, bool hasMenuBtn = true, bool hasMinBtn = true, bool hasMaxBtn = true,
                     bool hasCloseBtn = true);
    void setShowBtnsMode(QuadBtnsShowMode mode, QWidget *mainWin);
    void setQSSFontSize(QFont curFont);
    QPushButton *getCloseButton();

signals:
    void showConfigureWin();

private:
    QWidget *m_parentWin;
    QHBoxLayout *m_HwholeLayout;
    MenuModule *m_menuBtn;
    QPushButton *m_minBtn;
    QPushButton *m_maxBtn;
    QPushButton *m_closeBtn;
    void initConnect(QWidget *mainWin, bool hasMenuBtn, bool hasMinBtn, bool hasMaxBtn, bool hasCloseBtn);
    void setCloseHide();
};

#endif // QUAD_BTNS_TITLE_BAR_H
