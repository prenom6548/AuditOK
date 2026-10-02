/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: LiKe <like1@kylinos.cn>
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "maininterface.h"
#include "./fileProcess/bridge.h"
#include "./module/historywidget.h"
#include "./fileProcess/datacollect.h"

#include <QMainWindow>
#include <QApplication>
#include <QPushButton>
#include <QtWidgets>
#include <QScrollArea>
#include <QLabel>

class EntryButton : public QPushButton
{
public:
    EntryButton(QWidget *parent = nullptr);
    void setEntry(const QIcon &icon, const QString &text);
    void setTip(const QString &text);

private:
    QLabel *iconLabel;
    QLabel *textLabel;
    QLabel *tipLabel;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void initUI();
    void initEntry();
    void refresh();
    void startShowPlymouth();
    void useMaximumScale();
    void useRestoreScale();

public slots:

    void onGoHomeClicked();
signals:
    void startSecondEdit(const HistoryInfo &InfoData);
private:

    HistoryWidget *m_historywidget;
    QWidget *m_entrywidget;
    QWidget *mainWidget;
    QStackedWidget *m_stackedWidget;
    EntryButton *m_globalbtn;
    EntryButton *m_iconbtn;
    EntryButton *m_cursorbtn;
    EntryButton *m_plymouthbtn;
    EntryButton *m_grubbtn;
    EntryButton *m_bootbtn;
    MainInterface *m_maininterface;
    InfoCreateWidget *m_mainPageState;
    Bridge *m_bridge;
    FileProcess m_fileProcess;
    TitleBar*m_titlebar;

    HistoryInfo *m_infodata;
    QWidget *m_guideWidget;
    bool m_isHistoryDirUpdated = false;
    buildResultWidget *m_buildResult;

    bool m_ishistory = false;
    bool m_isKylin = false;
};
#endif // MAINWINDOW_H
