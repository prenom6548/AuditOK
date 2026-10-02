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

#ifndef NETCHECKHOMEPAGE_H
#define NETCHECKHOMEPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "custom_label.h"
#include "custom_push_button.h"
#include <kborderlessbutton.h>
using namespace kdk;

class NetCheckHomePage : public QWidget
{
    Q_OBJECT
public:
    explicit NetCheckHomePage(QWidget *parent = nullptr);
    void changeThemeColor(int color);
    void changeSystemSize(int size);

private:
    QLabel *mTitleLabel = nullptr;
    CustomLabel *mDescribLabel = nullptr;
    QLabel *mIconLabel = nullptr;
    QLabel *mItemLabel = nullptr;
    QPushButton *mStartBtn = nullptr;
    QLabel *mPicLabel = nullptr;
    KBorderlessButton *mSetButton = nullptr;

    int themeColor = 0;
    int mFontSize = 0;

signals:
    void sigChangeStackWidgetHome(int page);
    void sigStartCheck();
    void sigUpdateMainConfig();
private slots:
    void slotStartBtnClicked();
    void showConfigureWin();
};

#endif // NETCHECKHOMEPAGE_H
