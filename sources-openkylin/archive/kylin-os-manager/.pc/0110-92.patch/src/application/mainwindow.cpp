/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include <stdarg.h>
#include <iostream>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QString>
#include <QScreen>
#include <QStackedWidget>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QAction>
#include <QApplication>
#include <QTranslator>
#include <QProcess>

#include <knavigationbar.h>
#include <usermanual.h>
#include <kmenubutton.h>
#include <kaboutdialog.h>

#include "kom_application_interface.h"
#include "kom-configure.h"
#include "mainwindow.h"
#include "defines.h"
#include "../config.h"
#include "kom-buriedpoint.h"

extern CommandLineArgument g_commandLineArgument;
static MainWindow *g_mainWindow = nullptr;

/**
 * 用于插件回调，插件与主框架之间通信的接口, 每个接口遵循自己的协议.
 *
 * 1. geometry: 获取主框架的 geometry.
 *    void pluginCallback("geometry", int *x, int *y, int *width, int *height)
 */
static void pluginCallback(const char *funcName, ...)
{
    if (!strcmp(funcName, "geometry") && g_mainWindow != nullptr) {
        va_list args;
        va_start(args, funcName);

        int *x = va_arg(args, int *);
        int *y = va_arg(args, int *);
        int *width = va_arg(args, int *);
        int *height = va_arg(args, int *);

        va_end(args);

        const QRect rect = g_mainWindow->geometry();
        *x = rect.x();
        *y = rect.y();
        *width = rect.width();
        *height = rect.height();
    }
}

MainWindow::MainWindow(std::vector<KomApplicationProvider *> tabProviders, QWidget *parent)
    : KWidget(parent), m_tabProviders(tabProviders), m_navigationBar(new kdk::KNavigationBar(this)),
      m_stackWidget(new QStackedWidget(this))
{
    g_mainWindow = this;

    windowButtonBar()->maximumButton()->setVisible(false);
    auto *tripletButton = windowButtonBar()->menuButton();
    auto *tripletMenu = tripletButton->menu();
    tripletMenu->removeAction(tripletButton->settingAction());
    tripletMenu->removeAction(tripletButton->themeAction());
    connect(tripletButton->assistAction(), &QAction::triggered, this, &MainWindow::showUserManual);
    connect(tripletButton->quitAction(), &QAction::triggered, this, &MainWindow::close);
    connect(tripletButton->aboutAction(), &QAction::triggered, this, &MainWindow::showAbout);

    setFixedSize(1024, 640);
    setIcon("kylin-os-manager");
    setWindowIcon(QIcon::fromTheme("kylin-os-manager"));
    setWidgetName(tr("kylin-os-manager"));
    setWindowTitle(tr("kylin-os-manager"));
    setLayoutType(kdk::LayoutType::HorizontalType);

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setSpacing(0);
    vLayout->setContentsMargins(10, 0, 10, 0);
    vLayout->addSpacing(16);
    vLayout->addWidget(m_navigationBar);
    sideBar()->setLayout(vLayout);

    QVBoxLayout *vLayout_1 = new QVBoxLayout;
    vLayout_1->setSpacing(0);
    vLayout_1->setContentsMargins(0, 0, 0, 0);
    vLayout_1->addWidget(m_stackWidget);
    baseBar()->setLayout(vLayout_1);

    connect(m_navigationBar->listview(), &QListView::clicked, this, &MainWindow::onNavigationClicked);

    initUi();

    // 跳转到指定页面
    QModelIndex jumpIndex;
    if (!g_commandLineArgument.jumpTabName.isEmpty()) {
        auto iter = m_tabStandardItems.find(g_commandLineArgument.jumpTabName.toStdString());
        if (iter != m_tabStandardItems.end()) {
            kom::BuriedPoint::uploadMessage(kom::MainApplication, "JumpTo", g_commandLineArgument.jumpTabName);
            jumpIndex = m_navigationBar->model()->indexFromItem(iter->second);
        }
    }
    if (g_commandLineArgument.jumpTabName.isEmpty() || !jumpIndex.isValid()) {
        jumpIndex = m_navigationBar->model()->index(0, 0);
    }

    QItemSelectionModel *selectionModel = m_navigationBar->listview()->selectionModel();
    selectionModel->setCurrentIndex(jumpIndex, QItemSelectionModel::Select);
    QMetaObject::invokeMethod(m_navigationBar->listview(), "clicked", Qt::DirectConnection,
                              Q_ARG(QModelIndex, jumpIndex));

    moveCenter();
    kom::BuriedPoint::uploadMessage(kom::MainApplication, "APPStart");
}

MainWindow::~MainWindow()
{
    //规避存在QWebEnginePag,且QProcess::startDetached启动的进程未关闭时，主程序无法退出的问题
    qint64 pid = QCoreApplication::applicationPid();
    QProcess::startDetached("/usr/bin/kill " + QString::number(pid));
}

void MainWindow::initUi(void)
{
    std::map<int, QStandardItem *> navigationItemMap;
    for (const auto &item : m_tabProviders) {
        std::shared_ptr<KomApplicationInterface> interface(item->create());
        if (!interface)
            continue;
        m_tabInterfaces.emplace(std::make_pair(interface->name(), interface));

        // 传递主框架回调函数入口
        interface->init(pluginCallback);

        QIcon icon = loadPluginIcon(interface->icon());

        QWidget *widget = interface->createWidget();
        if (!widget)
            continue;
        m_stackWidget->addWidget(widget);

        QVariant data;
        data.setValue(widget);
        QVariant callback;
        callback.setValue(interface->getCallbackPtr());
        QStandardItem *navigationItem = new QStandardItem(icon, QString::fromStdString(interface->i18nName()));
        navigationItem->setData(data, NavigationItemRole::WidgetPointer);
        navigationItem->setData(interface->name().c_str(), NavigationItemRole::Name);
        navigationItem->setData(callback,NavigationItemRole::CallbackFunc);
        navigationItemMap.emplace(std::make_pair(interface->sort(), navigationItem));
        m_tabStandardItems.emplace(std::make_pair(interface->name(), navigationItem));
    }

    for (const auto &[index, navigationItem] : navigationItemMap) {
        m_navigationBar->addItem(navigationItem);
    }
}

void MainWindow::onNavigationClicked(const QModelIndex index)
{
    CallbackFun call = index.data(NavigationItemRole::CallbackFunc).value<CallbackFun>();
    if (call!=nullptr) {
        call(ClassType::CallbackClickTab,"");
    }
    QWidget *widget = index.data(NavigationItemRole::WidgetPointer).value<QWidget *>();
    m_stackWidget->setCurrentWidget(widget);
    kom::BuriedPoint::uploadMessage(kom::MainApplication, "ToggleRootTab", index.data(Name).toString());
}

QIcon MainWindow::loadPluginIcon(std::string name)
{
    static std::string path {"/usr/share/kylin-os-manager/icons/"};

    QIcon icon;
    if (name.find("/") != std::string::npos) {
        icon.addFile(QString::fromStdString(name), QSize(16, 16));
        return icon;
    }

    icon = QIcon::fromTheme(QString::fromStdString(name));
    if (!icon.isNull())
        return icon;

    std::string selfIconPath = path + name + ".svg";
    icon.addFile(QString::fromStdString(selfIconPath), QSize(16, 16));
    return icon;
}

void MainWindow::showUserManual()
{
    kdk::UserManual userManual;
    if (!userManual.callUserManual("kylin-os-manager"))
        qCritical() << "****** kylin os manager ****** "
                    << " call user manual error !";
}

void MainWindow::showAbout()
{
    kdk::KAboutDialog aboutDialog(this);
    aboutDialog.setAppIcon(QIcon::fromTheme("kylin-os-manager"));
    aboutDialog.setAppName(tr("kylin os manager"));
    aboutDialog.setAppVersion(tr("version: %1").arg(KYLIN_OS_MANAGER_VERSION));

    // 主窗口居中
    auto mainWidgetRect = geometry();

    /*
     * XXX: sdk 的 KAboutDialog 控件, 通过 Qt 接口 geometry() 或 width() 等等获取到的大小都是错误的,
     * 所以此处硬编码控件大小, 待 sdk 修复后在做修改.
     */
    auto aboutDialogWidth = 450;
    auto aboutDialogHeight = 336;

    auto x = mainWidgetRect.x() + mainWidgetRect.width() / 2 - aboutDialogWidth / 2;
    auto y = mainWidgetRect.y() + mainWidgetRect.height() / 2 - aboutDialogHeight / 2;

    aboutDialog.move(x, y);
    aboutDialog.exec();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1) {
        showUserManual();
    }
}

void MainWindow::moveCenter()
{
    QPoint pos = QCursor::pos();
    QRect primaryGeometry;
    for (QScreen *screen : qApp->screens()) {
        if (screen->geometry().contains(pos)) {
            primaryGeometry = screen->geometry();
        }
    }

    if (primaryGeometry.isEmpty()) {
        primaryGeometry = qApp->primaryScreen()->geometry();
    }

    this->move(primaryGeometry.x() + (primaryGeometry.width() - this->width()) / 2,
               primaryGeometry.y() + (primaryGeometry.height() - this->height()) / 2);
}
