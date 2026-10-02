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

#ifndef BLANKPAGE_H
#define BLANKPAGE_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QIcon>

#include "global/utils/inisetting.h"
#include "view/common/globalsizedata.h"
#include "klabel.h"

class BlankPage : public QWidget
{
    Q_OBJECT
    friend class TestBlankPage;
public:
    BlankPage(QWidget *parent = nullptr);
    ~BlankPage();
    void init();
    void initGsetting();
    void changeTheme();

    void changeSearchText();
    void changeBlankText();

private:
    QLabel *m_blankLabel;
    ::kdk::KLabel *m_blankTextLabel;

    // 空白页面布局
    QHBoxLayout *m_hBlankLayout;
    QVBoxLayout *m_vBlankLayout;

    int searchblank;
};


#endif // BLANKPAGE_H
