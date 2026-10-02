/*
 *
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
 *
 * Authors: zhangjiaping <zhangjiaping@kylinos.cn>
 * Modified by: zhangpengfei <zhangpengfei@kylinos.cn>
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QPaintEvent>
#include <QPainterPath>
#include <QPainter>
#include <QtMath>
#include <QEvent>
#include <QSpacerItem>
#include <QKeyEvent>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QSettings>
#include <QPropertyAnimation>
#include <QFileInfo>
#include <QLabel>
#include <QFrame>
#include <QPushButton>
#include <QKeyEvent>
#include <QGSettings/QGSettings>
#include <QSystemTrayIcon>
#include <QTimer>
#include <window-helper.h>

#include "search-app-widget-plugin/search.h"
#include "create-index-ask-dialog.h"
#include "search-line-edit.h"
#include "search-result-page.h"

namespace UkuiSearch {
class SearchResult;
class MainWindow : public QMainWindow {
    friend class SearchResult;
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // The position which mainwindow shows in the center of screen where the cursor in.
    void centerToScreen(QWidget* widget);
    void initSettings();

protected:
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *event) override;
    void initUi();
    void initConnections();

public:
    bool eventFilter(QObject *watched, QEvent *event) override;

public Q_SLOTS:

    void ScreenGeometryChanged(QRect rect);
    void bootOptionsFilter(QString opt);                              // 过滤终端命令
    void clearSearchResult(); //清空搜索结果
    void searchKeywordSlot(const QString&);
    void resizeHeight(int height);
    void tryHide();

Q_SIGNALS:
    void setText(QString keyword);

private:
    void setSearchMethod(bool isIndexSearch);
    double getTransparentData();
    void initTimer();
    bool tryHideMainwindow();
    void setSearchMethodConfig(const bool& createIndex, const bool& noLongerAsk);
    void tryMoveUp2Panel(int windowHeight);
    static void setAutoTestAttribute(QWidget *widget, const QString &widgetType, const QString &widgetName, const QString &desc);

    SearchBarWidget *m_searchBarWidget;
    SearchResultPage *m_searchResultPage;

    CreateIndexAskDialog *m_askDialog = nullptr;
    bool m_isAskDialogVisible = false;

    QTimer *m_askTimer = nullptr; //询问是否创建索引弹窗弹出的计时器
    QTimer *m_researchTimer = nullptr; //创建索引后重新执行一次搜索的计时器
    bool m_currentSearchAsked = false; //本次搜索是否已经询问过是否创建索引了
    QGSettings *m_searchGsettings = nullptr;
    QGSettings *m_panelGSettings = nullptr;
    int m_panelSize = 0;
    bool m_panelInBottom = true;
    QSettings *m_settings = nullptr;
    AppWidgetPlugin *m_appWidgetPlugin = nullptr;
    bool m_isIndexSearch = false;
    bool m_releaseFreeMemoryTimerWorking = false;
    int m_radius = 12;
    UkuiQuick::WindowProxy *m_proxy = nullptr;
};
}

#endif // MAINWINDOW_H
