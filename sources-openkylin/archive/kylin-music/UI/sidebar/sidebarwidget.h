/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QScrollArea>
#include <UI/base/popupDialog.h>
#include <QPalette>
#include <QPainter>
#include <QPainterPath>
#include <QColor>
#include <QStyleOption>
#include <QGSettings>
#include <kpushbutton.h>

#include "customSiderBarScrollArea.h"
#include "customToolButton.h"
#include "leftsiderbarwidget.h"
#include "knavigationbar.h"
#include "ktoolbutton.h"
#include "UI/globalsignal.h"
#include "ukuiwindowhelper.h"
using namespace kdk;


class SideBarWidget : public LeftsiderbarWidget
{
    Q_OBJECT

    friend class unitMainWindow;

public:
    explicit SideBarWidget(QWidget *parent = nullptr);

    PopupDialog *newSonglistPup = nullptr; //新建歌单弹窗界面
    PopupDialog *renameSongListPup = nullptr; //重命名歌单弹窗界面
    CustomToolButton *playListBtn = nullptr;

    void initAddButton();
    void sidecolor();
    void initWidget();
    void initConnect();
    void getPlayListName();
    QString newPlayListName();

protected:
#ifndef WHITE_TEST
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void enterEvent(QEvent *event);
    bool eventFilter(QObject *target, QEvent *event) override;
#endif
    void paintEvent(QPaintEvent *event) override;
    void updateSiderColor(bool isWindowActive);
//private:
//    int transparency = 1.0;

public Q_SLOTS:
    void addPlayList();
    void addItemToSongList();
 #ifndef WHITE_TEST
    void playAll(QString btnText);
    void rename(QString btnText);
#endif
    void renamePlayList();
    void removePlayList(QString text);
    void slotListSearch();
    void slotSongListHigh();

    /**
     * @brief slotButtonClicked
     * @param listname 点击按钮的名称
     */
    void slotButtonClicked(QString listname);
    void setButtonStyle(QString listname);

    void freshLogoWid();

    void setThemeIcon(QString themeIcon);

    void slotLableSetFontSize(int size);

Q_SIGNALS:
    void playListBtnClicked(QString listname);
    void playListRemoved(QString listname);
    void playListRenamed(QString oldName,QString newName);
    void playListAdded(QString listName);
    void playListClick();
    void signalPlayAll(QString listName);
    void sigShowMenu();

private:
    KPushButton *myPlayListBtn = nullptr; //添加歌单按钮
    CustomSiderBarScrollArea *scrollArea; //新建歌单按钮界面(包含我喜欢在内)

    QWidget *newPlayListWidget = nullptr;
    QWidget *logoWid = nullptr;
    QWidget *mainWid = nullptr;
//    QWidget *sideWid = nullptr;
    QVBoxLayout *newPlayListLayout = nullptr;
    QLabel *logoLabel = nullptr;
    QLabel *logoNameLabel = nullptr;
    QStringList playListName; //歌单名
    QLabel *libraryLabel = nullptr; //音乐库label
    QLabel *myPlayListLabel = nullptr; //我的歌单label
    QString btnText; //获取歌单名
    CustomToolButton *m_lastBtn = nullptr; //保存程序关闭之前歌单按钮
    UkuiWindowHelper *m_ukuiWindowHelper {nullptr};
    QGSettings *m_pTransparency;
    double tranSparency;
    QColor m_pSideColor;

    void adjustBackground(bool isFirst = false);
    QString elideText(const QString &text);
    void refreshAllButtonElideText();
    QFont m_elideFont;
    int m_maxElideWidth;
};

#endif // SIDEBARWIDGET_H
