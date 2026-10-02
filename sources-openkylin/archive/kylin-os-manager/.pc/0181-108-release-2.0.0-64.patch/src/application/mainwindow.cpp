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
#ifdef HAS_KYSDK
#include <kinputdialog.h>
#endif

#include "kom_application_interface.h"
#include "kom-configure.h"
#include "mainwindow.h"
#include "defines.h"
#include "../config.h"
#include "kom_buriedpoint.h"

extern CommandLineArgument g_commandLineArgument;
static MainWindow *g_mainWindow = nullptr;

/**
 * 用于插件回调，插件到主框架的通信接口, 每个接口遵循自己的协议.
 *
 * 1. geometry: 获取主框架的 geometry.
 *    void pluginCallback("geometry", int *x, int *y, int *width, int *height)
 * 2. SwitchToProblemFeedback: 切换到服务与支持页面
 *    void pluginCallback("SwitchToProblemFeedback")
 * 3. RunJunkClean: 运行垃圾清理功能
 *    void pluginCallback("RunJunkClean")
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

        return;
    }
    if (!strcmp(funcName, "SwitchToProblemFeedback") && g_mainWindow != nullptr) {
        g_mainWindow->ControlPlugin(MainWindow::PluginOperate::kDisplayProblemFeedback);
        return;
    }
    if (!strcmp(funcName, "RunJunkClean") && g_mainWindow != nullptr) {
        g_mainWindow->ControlPlugin(MainWindow::PluginOperate::kRunJunkClean);
        return;
    }
}

// 主框架到插件的通信接口
void MainWindow::ControlPlugin(PluginOperate operate, QVariant data) {
    switch (operate) {
        case PluginOperate::kRepairSpecificError: {
            SwitchTab("FaultDiagnosis");
            const auto iter = m_tabInterfaces.find("FaultDiagnosis");
            if (iter == m_tabInterfaces.end()) {
                qCritical() << "Repair specific error transfer data fail: no fault diagnosis plugin found.";
                return;
            }
            iter->second->communication(KomApplicationInterface::CommunicationType::kRepairSpecificError, data);
            break;
        }
        case PluginOperate::kDiagnosisSpecificType: {
            SwitchTab("FaultDiagnosis");
            const auto iter = m_tabInterfaces.find("FaultDiagnosis");
            if (iter == m_tabInterfaces.end()) {
                qCritical() << "Diagnosis specific type transfer data fail: no fault diagnosis plugin found";
                return;
            }
            iter->second->communication(KomApplicationInterface::CommunicationType::kDiagnosisSpecificType, data);
            break;
        }
        case PluginOperate::kUserExperiencePopup: {
            const auto iter = m_tabInterfaces.find("FaultDiagnosis");
            if (iter == m_tabInterfaces.end()) {
                qCritical() << "User experience popup transfer data fail: no fault diagnosis plugin found";
                return;
            }
            iter->second->communication(KomApplicationInterface::CommunicationType::kUserExperiencePopup, data);
            break;
        }
        case PluginOperate::kRunJunkClean: {
            SwitchTab("JunkClean");

            const auto iter = m_tabInterfaces.find("JunkClean");
            if (iter == m_tabInterfaces.end()) {
                qCritical() << "Control plugin: junk clean instance is not exist";
                return;
            }
            iter->second->communication(KomApplicationInterface::CommunicationType::kRunJunkClean, data);
            break;
        }
        case PluginOperate::kDisplayProblemFeedback: {
            if (!SwitchTab("ServiceSupport")) {
                return;
            }

            // 切换到问题反馈界面
            auto tab_interface_iter = m_tabInterfaces.find("ServiceSupport");
            if (tab_interface_iter == m_tabInterfaces.end()) {
                qCritical() << "Control plugin: service support instance is not exist";
                return;
            }
            auto service_support_callback = tab_interface_iter->second->getCallbackPtr();
            if (service_support_callback) {
                service_support_callback(ClassType::CallbackClickPluginProblemFeedback, QVariant());
            }
            break;
        }
        case PluginOperate::kDisplayServiceSupport: {
            if (!SwitchTab("ServiceSupport")) {
                return;
            }
            break;
        }
        case PluginOperate::kDisplayDeviceManager: {
            if (!SwitchTab("DeviceManager")) {
                return;
            }
            break;
        }
        case PluginOperate::kDisplayHardwareInfo: {
            SwitchTab("DeviceManager");
            // 切换到硬件信息页面
            const auto iter = m_tabInterfaces.find("DeviceManager");
            if (iter == m_tabInterfaces.end()) {
                qCritical() << "Switch to hardware info fail: no device manager plugin found";
                return;
            }
            iter->second->communication(KomApplicationInterface::CommunicationType::kDisplayHardwareInfo, QVariant());
            break;
        }
        case PluginOperate::kDisplayDriverManager: {
            SwitchTab("DeviceManager");
            // 切换到驱动管理页面
            const auto iter = m_tabInterfaces.find("DeviceManager");
            if (iter == m_tabInterfaces.end()) {
                qCritical() << "Switch to hardware info fail: no device manager plugin found";
                return;
            }
            iter->second->communication(KomApplicationInterface::CommunicationType::kDisplayDriverManager, QVariant());
            break;
        }
        default: {
            break;
        }
    }
}

MainWindow::MainWindow(std::vector<KomApplicationProvider *> tabProviders, QWidget *parent)
    : KWidget(parent),
      m_tabProviders(tabProviders),
      m_navigationBar(new kdk::KNavigationBar(this)),
      m_stackWidget(new QStackedWidget(this))
{
    g_mainWindow = this;

    setObjectName("main_frame_interface");
    setAttribute(Qt::WA_AlwaysShowToolTips);
    windowButtonBar()->maximumButton()->setVisible(false);
    auto *tripletButton = windowButtonBar()->menuButton();
    auto *tripletMenu = tripletButton->menu();
    tripletMenu->removeAction(tripletButton->settingAction());
    tripletMenu->removeAction(tripletButton->themeAction());
    tripletMenu->removeAction(tripletButton->feedbackAction());
    connect(tripletButton->assistAction(), &QAction::triggered, this, &MainWindow::showUserManual);
    connect(tripletButton->quitAction(), &QAction::triggered, this, &MainWindow::close);
    connect(tripletButton->aboutAction(), &QAction::triggered, this, &MainWindow::showAbout);

    setFixedSize(1024, 640);
    setIcon("kylin-os-manager");
    setWindowIcon(QIcon::fromTheme("kylin-os-manager"));
    setWidgetName(tr("Kylin Os Manager"));
    setWindowTitle(tr("Kylin Os Manager"));
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

    // 处理命令行参数
    // 页面跳转
    QModelIndex jumpIndex;
    if (!g_commandLineArgument.jumpTabName.isEmpty()) {
        auto iter = m_tabStandardItems.find(g_commandLineArgument.jumpTabName.toStdString());
        if (iter != m_tabStandardItems.end()) {
            kom::BuriedPoint::uploadMessage(kom::BuriedPoint::BuriedPointPage::kMainFrame,
                                            kom::BuriedPoint::EventCode::kJumpTo,
                                            {{"jumpTabName", g_commandLineArgument.jumpTabName}});
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

    jumpToSecondaryWidget(jumpIndex); // 处理第二参数
    moveCenter();
    kom::BuriedPoint::uploadProgramStartup();
    internal();

    // 修复模式
    if (g_commandLineArgument.repair) {
        QByteArray data;
        data.append(g_commandLineArgument.module.toLocal8Bit().toHex());
        data.append("\n");
        data.append(g_commandLineArgument.path.toLocal8Bit().toHex());
        data.append("\n");
        data.append(g_commandLineArgument.error_code.toLocal8Bit().toHex());
        if (!g_commandLineArgument.error_data.isEmpty()) {
            data.append("\n");
            data.append(g_commandLineArgument.error_data.toLocal8Bit().toHex());
        }
        ControlPlugin(PluginOperate::kRepairSpecificError, data);
    }

    // 指定诊断模式
    if (g_commandLineArgument.diagnosis) {
        ControlPlugin(PluginOperate::kDiagnosisSpecificType, g_commandLineArgument.diagnosis_type.toLocal8Bit().toHex());
    }
}

MainWindow::~MainWindow()
{
    // 规避存在QWebEnginePag,且QProcess::startDetached启动的进程未关闭时，主程序无法退出的问题
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
        navigationItem->setData(callback, NavigationItemRole::CallbackFunc);
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
    if (call != nullptr) {
        call(ClassType::CallbackClickTab, "");
    }
    QWidget *widget = index.data(NavigationItemRole::WidgetPointer).value<QWidget *>();
    m_stackWidget->setCurrentWidget(widget);
    kom::BuriedPoint::uploadMessage(kom::BuriedPoint::BuriedPointPage::kMainFrame,
                                    kom::BuriedPoint::EventCode::kNavigationClick,
                                    {{"navigationName", index.data(Name).toString()}});
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

void MainWindow::internal()
{
#ifdef HAS_KYSDK
    kom::Configure conf;
    if (conf.value(CONFIG_PLUGIN_GLOBAL, CONFIG_PLUGIN_GLOBAL_CUSTOMIZE).toString() != "internal") {
        return;
    }
    if (conf.value(CONFIG_PLUGIN_GLOBAL, CONFIG_PLUGIN_GLOBAL_INTERNAL_REGISTER, false).toBool()) {
        return;
    }
    QString lastStr = "@kylinos.cn";
    kdk::KInputDialog input;
    input.setLabelText("【内部试用版】\n请输入邮箱以便在您反馈成功时发送通知,请务必保证填写正确的邮箱地址");
    input.setTextValue(lastStr);
    input.setOkButtonText("确定");
    input.setCancelButtonText("取消");
    input.exec();
    QString mail = input.textValue();
    if (mail.isEmpty() || mail == lastStr || mail.length() < lastStr.length()) {
        return;
    }
    conf.setValue(CONFIG_PLUGIN_GLOBAL, CONFIG_PLUGIN_GLOBAL_INTERNAL_REGISTER, true);
    conf.setValue(CONFIG_PLUGIN_PROBLEM_FEEDBACK, CONFIG_PLUGIN_PROBLEM_FEEDBACK_EMAIL, mail);
#endif
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
    aboutDialog.setAppName(tr("Kylin Os Manager"));
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

void MainWindow::closeEvent(QCloseEvent *event) {
    const auto index = m_navigationBar->listview()->currentIndex();
    const auto item = m_navigationBar->model()->itemFromIndex(index);
    const auto name = item->data(NavigationItemRole::Name).toString();

    if (name == "FaultDiagnosis") {
        // 由故障诊断模块判断是否需要忽略关闭事件显示用户体验弹窗，还是接受事件关闭窗口
        QVariant data;
        data.setValue(event);
        ControlPlugin(PluginOperate::kUserExperiencePopup, data);
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

void MainWindow::jumpToSecondaryWidget(QModelIndex jumpIndex)
{
    if (g_commandLineArgument.jumpSecondaryName.isEmpty())
        return;

    // problemFeedback 问题反馈 第二参数
    if (g_commandLineArgument.jumpSecondaryName == "ProblemFeedback") {
        kom::BuriedPoint::uploadMessage(kom::BuriedPoint::BuriedPointPage::kMainFrame,
                                        kom::BuriedPoint::EventCode::kJumpTo,
                                        {{"jumpTabName", g_commandLineArgument.jumpSecondaryName}});
        CallbackFun call = jumpIndex.data(NavigationItemRole::CallbackFunc).value<CallbackFun>();
        if (call!=nullptr) {
            call(ClassType::CallbackClickPluginProblemFeedback,"");
        }
    } else if (g_commandLineArgument.jumpSecondaryName == "HardwareInfo") {
        ControlPlugin(PluginOperate::kDisplayHardwareInfo);
    } else if(g_commandLineArgument.jumpSecondaryName == "DriverManager"){
        ControlPlugin(PluginOperate::kDisplayDriverManager);
    } else if (g_commandLineArgument.jumpSecondaryName == "MaxDiskCapacity") {
        ControlPlugin(PluginOperate::kRunJunkClean, "MaxDiskCapacity");
    }
}

bool MainWindow::SwitchTab(const QString &plugin_name) {
    auto tab_standard_item_iter = m_tabStandardItems.find(plugin_name.toStdString());
    if (tab_standard_item_iter == m_tabStandardItems.end()) {
        qCritical() << "Switch tab " << plugin_name << " is not exist";
        return false;
    }
    auto index = m_navigationBar->model()->indexFromItem(tab_standard_item_iter->second);
    auto *selection_model = m_navigationBar->listview()->selectionModel();
    selection_model->clearSelection();
    selection_model->setCurrentIndex(index, QItemSelectionModel::Select);
    QMetaObject::invokeMethod(m_navigationBar->listview(), "clicked", Qt::DirectConnection, Q_ARG(QModelIndex, index));

    return true;
}
