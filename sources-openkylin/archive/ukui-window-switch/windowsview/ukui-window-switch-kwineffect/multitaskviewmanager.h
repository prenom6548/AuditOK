/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
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

#ifndef MULTITASKVIEWMANAGER_H
#define MULTITASKVIEWMANAGER_H

#include <memory>
#include <QQuickView>
#include <QQmlEngine>

#include <kwineffects.h>

#include "dbusservice.h"
#include "../desktoplistmodel.h"
#include "../appwindowlistmodel.h"
#include "../screens-manager.h"

class QTranslator;

class MultitaskViewManager : public KWin::Effect
{
    Q_OBJECT
    Q_PROPERTY(bool shouldShowTabBorder READ shouldShowTabBorder WRITE setShouldShowTabBorder NOTIFY shouldShowTabBorderChanged)

public:
    MultitaskViewManager();
    ~MultitaskViewManager() override;
    enum ProjectType {
        TABLET,
        MASTER,
        TWOINONE,
    };
    Q_INVOKABLE void open();
    Q_INVOKABLE void close();
    Q_INVOKABLE void setShowingDesktop(bool showing);
    Q_INVOKABLE bool isMultitaskViewVerticalLayout() const { return m_isViewVerticalLayout; }
    Q_INVOKABLE bool shouldShowTabBorder() const { return m_isShowTabBorder; }
    Q_INVOKABLE void setShouldShowTabBorder(bool isShow);
    Q_INVOKABLE void activeAreaChangeDataCollect(const QString &areaName);
    Q_INVOKABLE bool isWlcom() const {return false;};

    // 跨窗口键盘导航
    Q_INVOKABLE void activateNextTaskView(const int currentIndex);
    Q_INVOKABLE void activatePreviousTaskView(const int currentIndex);

    void grabbedKeyboardEvent(QKeyEvent* e) override;
    static bool m_isOpenGLBackend;

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
    bool isCursorNavigationKey(QKeyEvent *e) const;
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


    // 窗口选择界面 此处不涉及，前后端交互完全由kwin处理
    // 多任务视图界面
    QQuickView *createTaskView(Screen* screen);
    void showTaskViews();
    void hideTaskViews();

private:
    std::unique_ptr<QQmlEngine> m_qmlEngine = nullptr;
    QMap<Screen*, QQuickView*> m_taskViews;
    std::unique_ptr<DBusService> m_dbusService = nullptr;
    std::unique_ptr<DesktopListModel> m_desktopListModel = nullptr;
    std::unique_ptr<AppWindowListModel> m_appWindowListModel = nullptr;
    bool isOpen = false;
    bool m_isKeyboardGrab = false;
    ProjectType m_projectType = MultitaskViewManager::MASTER;
    QUrl m_qmlUrl = QUrl("qrc:/qml/multitaskview.qml");
    // 记录旋转模式是否打开
    bool m_isAutoRotateBeforeShowMultiTask = false;
    // 记录是否加载平板模式的qml
    bool m_isLoadTabletModeQML = false;
    // 记录多任务视图整体布局方向是否为垂直方向
    bool m_isViewVerticalLayout = false;
    // 记录走焦边框是否显示
    bool m_isShowTabBorder = false;
    QTranslator *m_translator = nullptr;

signals:
    void shouldShowTabBorderChanged();

private Q_SLOTS:
    void updateDesktopBackground();
};

#endif // MULTITASKVIEWMANAGER_H
