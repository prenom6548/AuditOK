/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#ifndef TITLE_H
#define TITLE_H

#include <QHBoxLayout>
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QMenu>
#include "menumodule.h"
// sdk 搜索框
#include "ksearchlineedit.h"
#include "view/search/titlesearchcityview.h"
using namespace kdk;
class title : public QWidget
{
    Q_OBJECT
    friend class TestTitle;
public:
    explicit title(QWidget *parent = nullptr);
    QPushButton *btnMaximum;
    menuModule *g_menu = nullptr;
    KSearchLineEdit *m_searchLineEdit;
    int getSearchLinePositionX();
    int getSearchLinePositionY();
    int getSearchLineWidth();

protected:
    bool eventFilter(QObject *watch, QEvent *e) override;
    void keyPressEvent(QKeyEvent *event);
private:
    QHBoxLayout *m_titleLayout;
    QPushButton *logoBtn;
    QLabel *m_label;
    QPushButton *m_menuButton;
    QMenu *m_mainMenu;
    QPushButton *btnRefresh;
    QPushButton *btnMinimize;
    QPushButton *btnCancel;
    // sdk 搜索输入框
    //KSearchLineEdit *m_searchLineEdit;

    void initConnection();
    void initLayout();
    void initQss();
    void sendSearchInput();
    QString m_searchText;
public slots:
    void slotSearchLineEditClear();
    void slotChangeLineEditStyle();
    void slotChangeHScreenMode();   // 平板横屏
    void slotChangeVScreenMode();   // 平板竖屏
    void slotChangePCMode();   // 平板竖屏
signals:
    void recovery();  //处理主界面最大化和还原
    void setMenuModuleAllowLocationYes();
    void sigTitleSearchCity();
    void sigChangeCityFromSearch();
    void sigLineEditGetFocus();
    void sigRefreshWeather(); //刷新天气信息
};

#endif // TITLE_H
