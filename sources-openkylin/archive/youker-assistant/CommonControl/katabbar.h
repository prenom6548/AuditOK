/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KATABBAR_H
#define KATABBAR_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QButtonGroup>
#include <QScrollArea>
#include <qgsettings.h>

#include "katabbutton.h"

class KATabBar : public QWidget
{
    Q_OBJECT
public:
    explicit KATabBar(QWidget *parent = nullptr);
    virtual ~KATabBar();

    void initUI();
    void initConnections();

    int addTabButton(KATabButton *tabBtn);
    int getCurIndex();

public slots:
    void onChangeTabPage();
    void checkArrowNeedShow();
    void onChangeTabIndex(int nIndex);
    void onTabItemClicked();
    void onTabIndexPosChange(int nIndex);
    void onChkSwitchItem(int nDirection);

signals:
    void changeTabIndex(int nIndex);
    void tabIndexPosChange(int nIndex);
    void chkSwitchItem(int nDirection);

protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *e) override;

private:
    void initStyleTheme();
    void adjustScroll(int nLength);

private:
    QVBoxLayout *m_mainLayout = nullptr;
    QHBoxLayout *m_topLayout = nullptr;
    QHBoxLayout *m_topMidLayout = nullptr;
    QPushButton *m_leftArrowBtn = nullptr;
    QPushButton *m_rightArrowBtn = nullptr;

    QButtonGroup *m_btnGroup = nullptr;
    QScrollArea *m_scrollWidget = nullptr;
    QGSettings *m_styleSettings = nullptr;

    QString m_strStyleName;
    int m_nCurIndex;
    unsigned m_uDirectionBtnWidth = 0;
    float m_fFontSize = 0;
};

#endif // KATABBAR_H
