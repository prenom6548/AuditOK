/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
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

#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QToolButton>

class Menu;
class QLabel;
class QPushButton;
class QHBoxLayout;
class Action;
class KmreWindow;

class TitleBar : public QWidget
{
    Q_OBJECT
public:
    explicit TitleBar(KmreWindow *window);
    ~TitleBar();

    void setTitleName(const QString &titleName) {mTitleName = titleName;}
    void setIcon(const QString &iconName);
    int getMenuBtnX();
    void updateUi();
    void updateTitleName();
    void updateTitleName(const QString &titleName, bool force = false);
    void updateTitle(int width);

private:
    void initUI();
    void initConnect();
    void checkScreenshotFeature();
    void initLeftContent();
    void initMiddleContent();
    void initRightContent();
    void initShotMenu();
    void updateLayout(int width);

    void waylandMove();

protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

signals:
    void sigBack();
    void sigFullscreen();
    void sigClose();
    void sigMiniSize();
    void sigShowMenu();
    void sigScreenShot();
    void sigToTop(bool top);
    void sigToHide(bool hide);

private:
    KmreWindow* mMainWindow = nullptr;
    QHBoxLayout *m_layout = nullptr;
    QHBoxLayout *m_lLayout = nullptr;
    QHBoxLayout *m_mLayout = nullptr;
    QHBoxLayout *m_rLayout = nullptr;
    QLabel *m_iconLabel = nullptr;
    QLabel *m_titleLabel = nullptr;
    //QWidget *m_centralWidget = nullptr;
    QPushButton *m_backBtn = nullptr;
    QPushButton *m_topBtn = nullptr;
    QPushButton *m_cancletopBtn = nullptr;
    QPushButton *m_fullscreenBtn = nullptr;
    QPushButton *m_menuBtn = nullptr;
    QPushButton *m_minBtn = nullptr;
    QPushButton *m_closeBtn = nullptr;
    QToolButton *m_screenshotBtn = nullptr;
    bool m_hideWhileScreenShotSupported = false;
    Menu *HideMenu = nullptr;
    Action *m_setHideAction = nullptr;
    bool m_useUkuiTheme = true;
    bool mButtonPressed;
    QPoint mLastPos;
    int mLayoutDeltaSize;
    QString mTitleName;


    bool m_moveFlag;
};

#endif // TITLEBAR_H
