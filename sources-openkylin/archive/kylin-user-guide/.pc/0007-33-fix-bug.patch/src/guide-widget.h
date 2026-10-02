#ifndef GUIDEWIDGET_H
#define GUIDEWIDGET_H
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
#pragma once

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QFileInfo>
#include <QEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QtWebKitWidgets/QWebView>
#include <QtWebKitWidgets/QWebFrame>
#include <QtWebKit/QWebSettings>
#include <QGSettings/QGSettings>
#include <QWebEngineView>
#include <QtWebChannel>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include "bridge.h"
#include <QPainterPath>

//#include "aboutwidget.h"
//#include <QtWebChannel/QtWebChannel>
class GuideWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GuideWidget(QWidget *parent = 0);
    ~GuideWidget();
    void initUI();
    void initSettings();
    void showPosition();
    void jump_app(QString);
    void set_Cursor(QPoint &event);
    QString system_name();
    QString system_version();
    QStringList getDirAndPng();

    bool eventFilter(QObject *watched, QEvent *event);

protected:
    void closeEvent(QCloseEvent *);
    void mousePressEvent(QMouseEvent * );
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *e);
    QString converRGB2HexStr(QColor _rgb);
    QPoint dragPos;
    bool mouseinwidget =false;
    bool isClose = false;
    bool mCanDrag = false;
    enum LocaTion{top,bottom,left,right,left_top,left_bottm,right_top,right_botm,middle};
    LocaTion site_flag;
    int Padding = 6;
    bool windowsflag = true;
    //原理上，只要英文名称和dbus调用参数一致，则可以自动读取，但目前并未进行统一。所以暂时采用指定的方式
    std::map<std::string, std::string> keywordMap={};
    QRect m_rect;

private:
    QWebView *m_pWebView = nullptr;
    QLabel *m_pTitleLabel = nullptr;
    QLabel *m_pIconLabel = nullptr;
    QString mJumpAppName = nullptr;
    QWidget *m_yWidget = nullptr;
    QDesktopWidget *desktop = nullptr;
    QToolButton *menuOffButton = nullptr;
    QMenu *m_menu = nullptr;
    // KAboutDialog *about_widget = nullptr;
    QGSettings *settings;
    QLineEdit *search_Line = nullptr;
    QLabel *m_queryIcon = nullptr;
    QWebEngineView *m_pWebEngineView = nullptr;
    QWebChannel *m_pWebChannel = nullptr;
    Bridge *bridge = nullptr;
    QDBusInterface *m_statusSessionDbus;
    QDBusReply<bool> is_tabletmode;
private slots:
    void slot_backOffButton();
    void slot_javaScriptFromWinObject();
    void slot_loadFinished(bool);
    void slot_onClicked_minOffButton();
    void slot_onClicked_maxOffButton();
    void slot_onClicked_closeOffButton();
    void slot_webGoto(QUrl);
    void slot_MenuActions(QAction *action);
    void slot_SettingsChange(const QString &key);
    void slot_searchLine();
    void slot_onClicked_completerAutoAdd(const QString& appName);
    void slot_getSarchKeyword();
    void slot_showBackOffButton();
    void tabletStatusChanged(bool tabletmode);
public slots:
    Q_INVOKABLE QString js_getIndexMdFilePath(QString);
    Q_INVOKABLE QString js_getIndexMdFilePathOther(QString);
    Q_INVOKABLE QString js_getIndexMdFileContent(QString);
    Q_INVOKABLE QString js_getIndexMdFileTitle(QString);
    Q_INVOKABLE QStringList js_getIntoFilename();
    Q_INVOKABLE QStringList js_getBigPngSize(QString);
    Q_INVOKABLE bool js_getCpuArchitecture();
    Q_INVOKABLE QString JudgmentSystem();
    Q_INVOKABLE QString js_getFileSystemInformation(QString);
    Q_INVOKABLE void js_executeCommand(QString);
    Q_INVOKABLE void js_changeNavHighlight();
public:

signals:
    Q_INVOKABLE void sig_backOff2js();
};
#endif //GUIDEWIDGET_H

