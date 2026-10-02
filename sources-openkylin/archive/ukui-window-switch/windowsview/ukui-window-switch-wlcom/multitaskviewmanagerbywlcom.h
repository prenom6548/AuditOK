/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef MULTITASKVIEWMANAGERBYWLCOM_H
#define MULTITASKVIEWMANAGERBYWLCOM_H

#include <memory>
#include <QQuickView>
#include <QQmlEngine>

//ukui-quick
#include <window-helper.h>

#include "dbusservice.h"
#include "../desktoplistmodel.h"
#include "../appwindowlistmodel.h"
#include "../screens-manager.h"

class QTranslator;
class MultitaskViewManagerByWlcom : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QScreen* currentScreen READ getCurrentScreen NOTIFY updateCurrentScreen)
    Q_PROPERTY(QRect currentScreenRect READ currentScreenRect NOTIFY currentScreenRectChanged)
    Q_PROPERTY(bool shouldShowTabBorder READ shouldShowTabBorder WRITE setShouldShowTabBorder NOTIFY shouldShowTabBorderChanged)
    Q_PROPERTY(bool isBackTab READ isBackTab)
    Q_PROPERTY(bool altTabVisible READ altTabVisible NOTIFY altTabVisibleChanged)
    Q_PROPERTY(bool winTabVisible READ winTabVisible NOTIFY winTabVisibleChanged)
    Q_PROPERTY(int desktopTipPos READ desktopTipPos NOTIFY desktopTipPosChanged)

public:
    MultitaskViewManagerByWlcom();
    ~MultitaskViewManagerByWlcom();
    enum ProjectType {
        TABLET,
        MASTER,
        TWOINONE,
    };
    Q_INVOKABLE void open();
    Q_INVOKABLE void close();
    Q_INVOKABLE void openTab();
    Q_INVOKABLE void openBackTab();
    Q_INVOKABLE void setShowingDesktop(bool showing);
    Q_INVOKABLE bool isMultitaskViewVerticalLayout() const { return m_isViewVerticalLayout; }
    Q_INVOKABLE bool shouldShowTabBorder() const { return m_isShowTabBorder; }
    Q_INVOKABLE void setShouldShowTabBorder(bool isShow);
    Q_INVOKABLE void activeAreaChangeDataCollect(const QString &areaName);
    Q_INVOKABLE bool isWlcom() const {return true;};
    Q_INVOKABLE bool isAltKeyPressed();
    Q_INVOKABLE void keyNavigationDataCollect(const int &keyValue);

    Q_INVOKABLE void activateNextTaskView(const int currentIndex);
    Q_INVOKABLE void activatePreviousTaskView(const int currentIndex);

    QScreen *getCurrentScreen();
    QRect currentScreenRect();
    QList<Screen *> getScreens();
    bool isBackTab();
    bool altTabVisible();
    bool winTabVisible();
    int desktopTipPos();

    static bool m_isOpenGLBackend;
    static bool isOpen;

private:
    void initModel();
    void initializeProjectType();
    void setQmlUrl();
    void registerQmlTypes();
    void registerShortcut();
    void initDBusService();
    void installTranslator();
    void initView();
    void destroyView();
    void connectSignals();
    bool isCursorNavigationKey(const int &key) const;
    QString getProjectCodeName();
    QString getProjectSubName();
    QString getProductFeatures();
    //!
    //! \brief getRotationModeStatus 获取当前旋转模式是否打开
    //!
    bool getRotationModeStatus();
    ///!
    //! \brief setRotationModeStatus
    //!
    void setRotationModeStatus(bool openRotationMode);

    // 窗口选择界面
    void showTabView();
    void hideTabView();

    // 多任务视图界面
    QQuickView *createTaskView(Screen* screen);
    void showTaskViews();
    void hideTaskViews();

private:
    std::unique_ptr<QQmlEngine> m_qmlEngine = nullptr;
    QMap<Screen*, QQuickView*> m_taskViews;
    std::unique_ptr<QQuickView> m_tabView = nullptr;
    std::unique_ptr<DBusService> m_dbusService = nullptr;
    std::unique_ptr<DesktopListModel> m_desktopListModel = nullptr;
    std::unique_ptr<AppWindowListModel> m_appWindowListModel = nullptr;
    ProjectType m_projectType = MultitaskViewManagerByWlcom::MASTER;
    QUrl m_qmlUrl = QUrl("qrc:/qml/MultitaskView.qml");
    QUrl m_tabQmlUrl = QUrl("qrc:/qml/Tabbox.qml");
    // 记录旋转模式是否打开
    bool m_isAutoRotateBeforeShowMultiTask = false;
    // 记录是否加载平板模式的qml
    bool m_isLoadTabletModeQML = false;
    // 记录多任务视图整体布局方向是否为垂直方向
    bool m_isViewVerticalLayout = false;
    // 记录走焦边框是否显示
    bool m_isShowTabBorder = false;
    QTranslator *m_translator = nullptr;
    UkuiQuick::WindowProxy *m_tabWindowProxy = nullptr;
    UkuiQuick::WindowProxy *m_taskWindowProxy = nullptr;
    bool m_isBackTab = false;
    bool m_showDesktopTipFlag = false;

signals:
    void currentScreenRectChanged();
    void shouldShowTabBorderChanged();
    void taskViewShowed();
    void altTabVisibleChanged();
    void winTabVisibleChanged();
    void backgroundChanged();

    void showDesktopTipSignal();
    void desktopTipPosChanged();
    void updateCurrentScreen();

private Q_SLOTS:
    void updateDesktopBackground();
    void updateAltTabView();
    void onTabletModeChanged();
    // 工作区切换时的编号指示器
    void showDesktopBallonTip();
    void onScreenAdd(Screen *screen);
    void onScreenRemove(Screen *screen);
};

#endif // MULTITASKVIEWMANAGERBYWLCOM_H
