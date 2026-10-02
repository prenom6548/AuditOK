/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QPushButton>
#include <QLabel>
#include <QToolButton>
#include <QMenu>
#include <QGSettings>
#include <QSettings>

#include <kysdk/applications/ukuiwindowhelper/ukuiwindowhelper.h>
#include <kysdk/applications/windowmanager/windowmanager.h>
#include <kysdk/applications/kwidget.h>

#include "kleftwidget.h"
#include "krightwidget.h"
#include "process/processwidget.h"
#include "filesystem/filesystemwidget.h"
#include "service/servicewidget.h"
#include "sysresource/cpudetailswidget.h"
#include "sysresource/memdetailswidget.h"
#include "sysresource/netdetailswidget.h"
#include "sysresource/diskdetailswidget.h"
#include "sysresource/basedetailviewwidget.h"
#include "trayicon.h"

class MainWindow : public KWidget
{
    Q_OBJECT
public:
    typedef struct ModuleTpes_s {
        int process = -1;
        int service = -1;
        int disk = -1;
        int cpuDetails = -1;
        int memDetails = -1;
        int netDetails = -1;
        int diskDetails = -1;
    } ModuleTpes;

    explicit MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();
    void windowShow();

public slots:
    void handleMessage(const QString &);
    void onSwitchPage(int nIndex);
    void onMinimizeWindow();
    void onMaximizeWindow();
    void onCloseBtnClicked();
    void switchPage(int nIndex);
    void tabletmodeDbusSlot(bool isTabletmode);
    void saveSettings();
    void showUserGuide();
    void onRefreshSearchResult();
    void handleSearchTextChanged();

signals:
    void searchSignal(QString searchContent);

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void keyReleaseEvent(QKeyEvent *event) Q_DECL_OVERRIDE; //键盘松开事件
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *watched, QEvent *event);

private:
    void initUI();
    void initConnections();
    void initRightPanel();
    bool loadSettings();
    bool dblOnEdge(QMouseEvent *event);
    void moveCenter();

private:
    KSearchLineEdit *m_searchEditNew = nullptr;
    // search edit
    QString m_searchTextCache;
    QTimer *m_searchTimer = nullptr;
    bool m_isSearching = false;

    KLeftWidget *m_leftWidget = nullptr;
    KRightWidget *m_rightWidget = nullptr;

    // QGSettings
    bool m_isSurportService = false;
    QSettings *m_proSettings = nullptr;
    QSize m_lastWndSize;
    bool m_bIsWndMax;

    ProcessWidget *m_procWidget = nullptr;
    FileSystemWidget *m_fsWidget = nullptr;
    ServiceWidget *m_svcWidget = nullptr;
    CpuDetailsWidget *mCpuDetailsWidget = nullptr;
    MemDetailsWidget *mMemDetailsWidget = nullptr;
    NetDetailsWidget *mNetDetailsWidget = nullptr;
    DiskDetailsWidget *m_pDiskDetailsWidget = nullptr;

    QList<WindowId> m_listWinIds;

    int mFirstKey = 0;

    QGSettings *m_pSystemMonitorGSetting = nullptr;
    bool isShutDown = false;
    ModuleTpes m_moudleTypes;
    bool m_firstInCpuWidget = true;
    bool m_firstInMemWidget = true;
    bool m_firstInNetWidget = true;
    bool m_firstInDiskWidget = true;

    // 服务页面加载状态
    bool m_isServiceListLoaded = false;
    bool m_isFirstSwitchToService = true;

    UkuiWindowHelper *m_pUkuiWindowHelper = nullptr;
    TrayIcon *m_trayIcon = nullptr;
};

#endif // MAINWINDOW_H
