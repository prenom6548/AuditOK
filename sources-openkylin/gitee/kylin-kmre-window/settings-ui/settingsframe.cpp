/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
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

#include "settingsframe.h"
#include "settingscrollcontent.h"
#include "dockeripwidget.h"
#include "gamewidget.h"
#include "glesversionwidget.h"
#include "displaymodewidget.h"
#include "camerawidget.h"
#include "cleanerwidget.h"
#include "logwidget.h"
#include "appmultiplierwidget.h"
#include "utils.h"
#include "phoneinfowidget.h"
#include "generalsettingwidget.h"
#include "developerwidget.h"
#include "advancedwidget.h"
#include "messagebox.h"
#include "popuptip.h"
#include "dbusclient.h"
#include "appsettingspanel.h"

#ifdef UKUI_WAYLAND
#include "ukui-wayland/ukui-decoration-manager.h"
#endif

#include <QApplication>
#include <QStackedWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusInterface>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QGSettings>
#include <QMouseEvent>
#include <QDebug>
#include <syslog.h>
#define ORG_UKUI_STYLE      "org.ukui.style"

using namespace kmre;
namespace {

static QString getDisplayInformation()
{
    QDBusInterface interface("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "cn.kylinos.Kmre.Manager", QDBusConnection::sessionBus());
    QString value;
    QDBusMessage response = interface.call("getDisplayInformation");
    if (response.type() == QDBusMessage::ReplyMessage) {
        value = response.arguments().takeFirst().toString();
    }

    return value;
}

static bool fbMatched(const char* fbName)
{
    FILE* fp = NULL;
    char line[256] = {0};
    bool matched = false;

    fp = fopen("/proc/fb", "r");
    if (!fp) {
        fprintf(stderr, "GPU detection open /proc/fb error!");
        return false;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strcasestr(line, fbName)) {
            matched = true;
            break;
        }
    }

    fclose(fp);

    return matched;
}

bool isAMDGraphicCard()
{
    return (fbMatched("amdgpudrmfb") || fbMatched("radeondrmfb"));
}

}

SettingsFrame::SettingsFrame(QWidget * parent)
    : QWidget(parent)
{
#ifdef UKUI_WAYLAND
    this->setWindowFlags(windowFlags() | Qt::Tool);
#endif

    m_loginUserName = kmre::utils::getUserName();
    m_loginUserId = kmre::utils::getUid();

    this->setWindowTitle(tr("KMRE-Preference"));
    this->setContentsMargins(0, 0, 0, 0);
    this->setFixedSize(800, 590);
    this->setWindowIcon(QIcon(":/res/kmre.svg"));//wayland下设置无效
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
    this->setFocusPolicy(Qt::NoFocus);
    this->setMouseTracking(true);
    //disable style window manager
    this->setProperty("useStyleWindowManager", false);//for UKUI 3.0

    this->initDBus();
    this->initInfos();

#ifdef UKUI_WAYLAND
    m_mainVLayout = new QVBoxLayout();
    m_titleLayout = new QHBoxLayout();
    m_mainVLayout->setContentsMargins(0,4,4,0);
    m_mainVLayout->setSpacing(10);
    m_titleLayout->setContentsMargins(4,0,0,0);
    m_titleLayout->setSpacing(0);

    QIcon titleIcon = QIcon::fromTheme("kmre");
    if (titleIcon.isNull()) {
        titleIcon = QIcon(":/res/kmre.svg");
    }

    m_iconLabel  = new QLabel();
    m_iconLabel->setPixmap(titleIcon.pixmap(titleIcon.actualSize(QSize(24, 24))));

    m_titleLabel = new QLabel(tr("KMRE-Preference"));

    m_minBtn = new QPushButton(this);
    m_minBtn->setFocusPolicy(Qt::NoFocus);
    // /usr/share/icons/ukui-icon-theme-default/scalable/actions/
    m_minBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    m_minBtn->setProperty("useIconHighlightEffect", 0x2);
    m_minBtn->setProperty("isWindowButton", 0x01);
    m_minBtn->setFlat(true);
    m_minBtn->setToolTip(tr("Minimize"));
    m_minBtn->setFixedSize(30, 30);

    m_closeBtn = new QPushButton(this);
    m_closeBtn->setFocusPolicy(Qt::NoFocus);
    m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    m_closeBtn->setProperty("isWindowButton", 0x02);
    m_closeBtn->setProperty("useIconHighlightEffect", 0x08);
    m_closeBtn->setFlat(true);
    m_closeBtn->setToolTip(tr("Close"));
    m_closeBtn->setFixedSize(30, 30);

    m_titleLayout->addWidget(m_iconLabel);
    m_titleLayout->addSpacing(8);
    m_titleLayout->addWidget(m_titleLabel);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_minBtn);
    m_titleLayout->addSpacing(4);
    m_titleLayout->addWidget(m_closeBtn);
#endif

    m_categoryListWidget = new QListWidget(this);
    m_categoryListWidget->setFixedSize(190, this->height());
//    m_categoryListWidget->setFixedSize(180, this->height());
    m_categoryListWidget->setSpacing(4);
    m_categoryListWidget->setContentsMargins(0, 0,0, 0);
    m_categoryListWidget->setFocusPolicy(Qt::NoFocus);
    m_categoryListWidget->setMouseTracking(true);
    m_categoryListWidget->setStyleSheet("QListWidget{background:palette(Base);line-height:40px;color:palette(Text);border-radius:6px;}QListWidget::item{padding-left:16px;border:0;}QListWidget::item:selected{background:#3790FA;color:#FFFFFF;border-radius:6px;}");
    m_categoryListWidget->setResizeMode(QListView::Adjust);
    m_categoryListWidget->setViewMode(QListView::ListMode);
    m_categoryListWidget->setMovement(QListView::Static);

    m_stack = new QStackedWidget(this);
    m_stack->setStyleSheet("QStackedWidget{background:palette(Base);}");// rgb(255, 255, 255);}");
//    m_stack->setAttribute(Qt::WA_TranslucentBackground, true);
    m_stack->setFocusPolicy(Qt::NoFocus);
    m_stack->setAutoFillBackground(true);
    //m_stack->setFixedSize(599,540);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setContentsMargins(10, 16, 0, 10);
    layout->addWidget(m_categoryListWidget);
    layout->addWidget(m_stack);

#ifdef UKUI_WAYLAND
    m_mainVLayout->addLayout(m_titleLayout);
    m_mainVLayout->addLayout(layout);
    this->setLayout(m_mainVLayout);

    connect(m_minBtn, &QPushButton::clicked, [this](){this->showMinimized();});
    connect(m_closeBtn, &QPushButton::clicked, [this]() {qApp->quit();});
#else
    this->setLayout(layout);
#endif

    this->initCategoryList();
    this->initStack();

    connect(m_categoryListWidget,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(changeInfoPage(QListWidgetItem*)));

    if (QApplication::arguments().length() > 1) {
        bootOptionsFilter(QApplication::arguments().at(1));
    }

    QDBusConnection::sessionBus().connect("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "cn.kylinos.Kmre.Manager", "currentCameraDevice", this, SLOT(updateCurrentCameraDevice(QString)));
//    QDBusInterface *mSystemBusInterface = new QDBusInterface("cn.kylinos.Kmre",
//                                             "/cn/kylinos/Kmre",
//                                             "cn.kylinos.Kmre",
//                                             QDBusConnection::systemBus());
//    QObject::connect(mSystemBusInterface, SIGNAL(Stopped(QString)), this, SLOT(onKmreDockerStopped(QString)));
    if (QGSettings::isSchemaInstalled(ORG_UKUI_STYLE)) {
        m_ukuiSettings = new QGSettings(ORG_UKUI_STYLE);
        connect(m_ukuiSettings, &QGSettings::changed, [&](QString key) {
            if (key == "systemFont" || key == "systemFontSize") {
                QFont font = this->font();
                for (auto widget : qApp->allWidgets()) {
                    widget->setFont(font);
                }
            }
        });
    }

#ifdef UKUI_WAYLAND
    installEventFilter(this);
#endif

    this->hide();
}

SettingsFrame::~SettingsFrame()
{
    if (m_ukuiSettings) {
        delete m_ukuiSettings;
        m_ukuiSettings = 0;
    }

    for (int i = 0; i< m_stack->count(); i++) {
        SettingScrollContent *content = static_cast<SettingScrollContent *> (m_stack->widget(i));

    }
    if (m_stack) {
        foreach (QObject *child, m_stack->children()) {
           QWidget *widget = static_cast<QWidget *>(child);
           widget->deleteLater();
        }
        delete m_stack;
    }

//    if (m_stack->count() > 0) {
//        m_stack->removeWidget(m_stack->widget(0));
//    }

//    QLayoutItem *child;
//    while ((child = m_layout->takeAt(0)) != 0) {
//        if (child->widget())
//            child->widget()->deleteLater();
//        delete child;
//    }
}

void SettingsFrame::initDBus()
{
    m_dbusClient = new DbusClient;
    m_dbusThread = new QThread;
    m_dbusClient->moveToThread(m_dbusThread);
    connect(m_dbusThread, SIGNAL(started()), m_dbusClient, SLOT(initDbusData()));
    connect(this,SIGNAL(requestSystemProp(int,QString)), m_dbusClient, SLOT(getSystemProp(int,QString)));
    m_dbusThread->start();
}

void SettingsFrame::initInfos()
{
    QString displayInfo = getDisplayInformation();
    if (!displayInfo.isEmpty()) {
        QJsonDocument jsonDocument = QJsonDocument::fromJson(displayInfo.toLocal8Bit().data());
        if (!jsonDocument.isNull()) {
            QJsonObject jsonObject = jsonDocument.object();
            if (!jsonObject.isEmpty() && jsonObject.size() > 0) {
                if (jsonObject.contains("display_type")) {
                    m_displayType = jsonObject.value("display_type").toString();
                }
                if (jsonObject.contains("display_types")) {
                    m_displayTypes = jsonObject.value("display_types").toString();
                }
                if (jsonObject.contains("cpu_type")) {
                    m_cpuType = jsonObject.value("cpu_type").toString();
                }
                if (jsonObject.contains("gpu_vendor")) {
                    m_gpuVendor = jsonObject.value("gpu_vendor").toString();
                }
                if (jsonObject.contains("gpu_model")) {
                    m_gpuModel = jsonObject.value("gpu_model").toString();
                }
            }
        }
    }

    m_categoryList << tr("General Setting");

    if (!m_gpuVendor.isNull() && !m_gpuVendor.isEmpty() && (m_gpuVendor == "NVIDIA" || m_gpuVendor == "AMD" || m_gpuVendor == "INTEL")) {
        if (isAMDGraphicCard() || m_gpuVendor == "INTEL") {
            if (m_displayTypes == "emugl,drm") {
                if (m_displayType == "drm"){
                    m_categoryList << tr("Display") << tr("Game Setting") << tr("PhoneInfo Setting") << tr("AppMultiplier");
                }
                else{
                    m_categoryList << tr("Display") << tr("Game Setting") << tr("PhoneInfo Setting") << tr("AppMultiplier") << tr("Images");
                }
            }
            else {
                if (m_displayType == "drm"){
                    m_categoryList << tr("Game Setting") << tr("PhoneInfo Setting") << tr("AppMultiplier");
                }
                else{
                    m_categoryList << tr("Display") << tr("Game Setting") << tr("PhoneInfo Setting") << tr("AppMultiplier");
                }
            }
        }
        else {
            if (m_displayType == "drm"){
                m_categoryList << tr("Game Setting") << tr("PhoneInfo Setting") << tr("AppMultiplier");
            }
            else{
                m_categoryList << tr("Display") << tr("Game Setting") << tr("PhoneInfo Setting") << tr("AppMultiplier");
            }
        }
    }
    else {
        bool noSupportAppMultiplier = true;
        if (isAMDGraphicCard() || m_gpuVendor == "INTEL") {
            if (m_displayTypes == "emugl,drm"){
                if (noSupportAppMultiplier) {
                    m_categoryList << tr("Display") << tr("Game Setting") << tr("PhoneInfo Setting");
                }
                else {
                    m_categoryList << tr("Display") << tr("Game Setting") << tr("PhoneInfo Setting") << tr("AppMultiplier");
                }
            }
            else {
                if (noSupportAppMultiplier) {
                    m_categoryList << tr("Game Setting") << tr("PhoneInfo Setting");
                }
                else {
                    m_categoryList << tr("Game Setting") << tr("PhoneInfo Setting") << tr("AppMultiplier");
                }
            }
        }
        else {
            if (noSupportAppMultiplier) {
                m_categoryList << tr("Game Setting") << tr("PhoneInfo Setting");
            }
            else {
                m_categoryList << tr("Game Setting") << tr("PhoneInfo Setting") << tr("AppMultiplier");
            }
        }
    }

    m_categoryList << tr("AppSettings") << tr("Advanced");

    emit requestSystemProp(1, "debug_mode");
    connect(m_dbusClient, SIGNAL(systemprop(QString)), this, SLOT(onGetSystemProp(QString)));
}

void SettingsFrame::slotMessageReceived(const QString &msg)
{
    Q_UNUSED(msg)
    this->hide();
    this->show();
    showNormal();
    kmre::utils::centerToScreen(this);
    //bootOptionsFilter(msg);

#ifdef UKUI_WAYLAND
    // wayland 窗口无边框，需要在窗口show()之后执行
    UKUIDecorationManager::getInstance()->removeHeaderBar(this->windowHandle());
    UKUIDecorationManager::getInstance()->setCornerRadius(this->windowHandle(), 12, 12, 12, 12);
#endif
}

void SettingsFrame::showWindow()
{
    this->show();
    kmre::utils::centerToScreen(this);
#ifdef UKUI_WAYLAND
    // wayland 窗口无边框，需要在窗口show()之后执行
    UKUIDecorationManager::getInstance()->removeHeaderBar(this->windowHandle());
    UKUIDecorationManager::getInstance()->setCornerRadius(this->windowHandle(), 12, 12, 12, 12);
#endif
}

void SettingsFrame::bootOptionsFilter(QString opt)
{
    if (opt == "--camera") {

    }
    else if (opt == "--gles") {

    }
}

void SettingsFrame::openDeveloperWidget()
{
    QDBusInterface interface("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "org.freedesktop.DBus.Introspectable", QDBusConnection::sessionBus());
    QDBusReply<QString> reply = interface.call("Introspect");
    if (reply.isValid()) {
        QString xml_data = reply.value();
        if (xml_data.contains("setSystemProp")) {
            m_categoryList << tr("Developer Mode");
            QListWidgetItem *item = nullptr;
            QString category = tr("Developer Mode");
            item = new QListWidgetItem(category, m_categoryListWidget);
            item->setSizeHint(QSize(180, 36));//m_categoryListWidget->width()
            item->setStatusTip(category);
            item->setToolTip(category);
            item->setTextAlignment(Qt::AlignVCenter);
            if (m_DeveloperWidget == nullptr) {
                m_DeveloperWidget = new DeveloperWidget;
                this->addStackPage(m_DeveloperWidget);
            }
        }
        else {
            KylinUI::MessageBox::information(this, tr("Tips"), tr("Current manager vesion does not support developer mode."));
        }
    }
}

void SettingsFrame::initCategoryList()
{
    foreach (QString category, m_categoryList) {
        QListWidgetItem *item = nullptr;
        item = new QListWidgetItem(category, m_categoryListWidget);
        item->setSizeHint(QSize(180, 36));//m_categoryListWidget->width()
        item->setStatusTip(category);
        item->setToolTip(category);
        item->setTextAlignment(Qt::AlignVCenter);
    }
    m_categoryListWidget->setCurrentRow(0);
}

void SettingsFrame::initStack()
{
    foreach (QString category, m_categoryList) {
        if (category == tr("Display")) {
            if (m_DisplayWidget == nullptr) {
                m_DisplayWidget = new DisplayModeWidget(m_displayType, m_gpuVendor);
                this->addStackPage(m_DisplayWidget);
                connect(m_DisplayWidget, &DisplayModeWidget::restartEnv, this, &SettingsFrame::onKmreDockerStopped);
            }
        }
//        else if (category == tr("Renderer")) {
//            GlesVersionWidget *w = new GlesVersionWidget(m_gpuVendor);
//            this->addStackPage(w);
//        }
        else if (category == tr("Game Setting")) {
            GameWidget *w = new GameWidget;
            this->addStackPage(w);
        }
        else if (category == tr("Network")) {
            DockerIpWidget *w = new DockerIpWidget;
            this->addStackPage(w);
        }
        else if (category == tr("Camera")) {
            if (m_cameraWidget == nullptr) {
                m_cameraWidget = new CameraWidget;
                this->addStackPage(m_cameraWidget);
            }
        }
        else if (category == tr("Images")) {
            CleanerWidget *w = new CleanerWidget;
            this->addStackPage(w);
        }
        else if (category == tr("Log")) {
            LogWidget *w = new LogWidget(m_loginUserName, m_loginUserId);
            this->addStackPage(w);
        }
        else if (category == tr("AppMultiplier")) {
            if (m_appMultiplierWidget == nullptr) {
                m_appMultiplierWidget = new AppMultiplierWidget;
                this->addStackPage(m_appMultiplierWidget);
            }
        }
        else if (category == tr("AppSettings")) {
            if (m_appSettingsPanel == nullptr) {
                m_appSettingsPanel = new AppSettingsPanel;
                this->addStackPage(m_appSettingsPanel);
            }
        }
        else if (category == tr("PhoneInfo Setting")) {
            if (m_PhoneInfoWidget == nullptr) {
                m_PhoneInfoWidget = new PhoneInfoWidget;
                this->addStackPage(m_PhoneInfoWidget);
                connect(m_PhoneInfoWidget, &PhoneInfoWidget::restartEnv, this, &SettingsFrame::onKmreDockerStopped);
            }
        }
        else if (category == tr("General Setting")) {
            if (m_generalWidget == nullptr) {
                m_generalWidget = new GeneralSettingWidget;
                this->addStackPage(m_generalWidget);
                connect(m_generalWidget, &GeneralSettingWidget::opendeveloperWidget, this, &SettingsFrame::openDeveloperWidget);
                connect(m_generalWidget, &GeneralSettingWidget::restartEnv, this, &SettingsFrame::onKmreDockerStopped);
            }
        }
        else if (category == tr("Developer Mode")) {
            if (m_DeveloperWidget == nullptr) {
                m_DeveloperWidget = new DeveloperWidget;
                this->addStackPage(m_DeveloperWidget);
            }
        }
        else if (category == tr("Advanced")) {
            if (m_AdvancedWidget == nullptr) {
                m_AdvancedWidget = new AdvancedWidget;
                this->addStackPage(m_AdvancedWidget);
            }
        }
    }
}

void SettingsFrame::addStackPage(QWidget *widget)
{
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_stack->addWidget(widget);
}

//安卓当前使用的摄像头设备
void SettingsFrame::updateCurrentCameraDevice(const QString &deviceName)
{
    for (int i=0; i<m_categoryList.length(); i++) {
        if (m_categoryList.at(i) == tr("General Setting")) {
            m_stack->setCurrentIndex(i);
            if (m_generalWidget) {
                m_generalWidget->updateCurrentCameraDevice(deviceName);
            }
            break;
        }
    }
}

void SettingsFrame::onNavigationBarChanged(int index)
{
    m_stack->setCurrentIndex(index);

    if (index < m_categoryList.length()) {
        if (m_categoryList.at(index) == tr("General Setting")) {
            if (m_generalWidget) {
                m_generalWidget->updateCameraDevices();
            }
        }
        else if (m_categoryList.at(index) == tr("AppMultiplier")) {
            if (m_appMultiplierWidget) {
                m_appMultiplierWidget->initAppMultiplier();
            }
        }
        else if (m_categoryList.at(index) == tr("AppSettings")) {
            if (m_appSettingsPanel) {
                m_appSettingsPanel->init();
            }
        }
        else  if (m_categoryList.at(index) == tr("Developer Mode")) {
            if (m_DeveloperWidget) {
                m_DeveloperWidget->initDatas();
            }
        }
        else  if (m_categoryList.at(index) == tr("Advanced")) {
            if (m_AdvancedWidget) {
                m_AdvancedWidget->init();
            }
        }
    }
}

void SettingsFrame::changeInfoPage(QListWidgetItem *item)
{
    //QListWidget *send = qobject_cast<QListWidget *>(sender());
    if (!item) {
        return;
    }

    QString tip = item->statusTip();
    if (tip.isEmpty() || tip.isNull()) {
        return;
    }

    int index = 0;
    foreach (QString category, m_categoryList) {
        if (category == tip) {
            this->onNavigationBarChanged(index);
            break;
        }
        index ++;
    }
}

void SettingsFrame::onKmreDockerStopped()
{
    popuptip = new PopupTip();
    popuptip->setTipMessage(tr("正在重启环境..."));
    popuptip->popup(this->geometry().center(), 0);
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onRestartDocker()));
    m_timer->start(500);
}

void SettingsFrame::onRestartDocker()
{
    QProcess process;
    QStringList options;
    QByteArray data;
    QString output;
    QString option = "ps -U $USER -u $USER -o pid,comm,cmd | grep -E \"kylin-kmre-window|kylin-kmre-manager|kylin-kmre-appstream|kylin-kmre-audio|kylin-kmre-filewatcher|kylin-kmre-fuse\" | awk '{print $1" "$2}' | grep kmre | wc -l";
    options << "-c" << option;
//    options << "-c" << "ps -U $USER -u $USER -o pid,comm,cmd | grep kmre | awk '{print $1" "$2}' | grep kmre | wc -l";

    process.start("/bin/bash", options);
    process.waitForFinished();
    process.waitForReadyRead();

    data = process.readAllStandardOutput();

    output = QString(data);
    output.replace(QString("\n"), QString(""));
    if (output.toInt() == 0) {
        m_timer->stop();
        process.start("/usr/bin/startapp", QStringList()<<"start_kmre_silently");
        process.waitForFinished();
        if (popuptip) {
            popuptip->close();
        }
        this->close();
    }
    if (stopNum == 20) {
        m_timer->stop();
        if (popuptip) {
            popuptip->close();
        }
        KylinUI::MessageBox::critical(this, tr("Error"), tr("Restarting the environment failed!"));
    }
    stopNum++;
}

void SettingsFrame::onGetSystemProp(const QString &value)
{
    if (value == "1") {
        m_categoryList << tr("Developer Mode");
        QListWidgetItem *item = nullptr;
        QString category = tr("Developer Mode");
        item = new QListWidgetItem(category, m_categoryListWidget);
        item->setSizeHint(QSize(180, 36));//m_categoryListWidget->width()
        item->setStatusTip(category);
        item->setToolTip(category);
        item->setTextAlignment(Qt::AlignVCenter);
        if (m_DeveloperWidget == nullptr) {
            m_DeveloperWidget = new DeveloperWidget;
            this->addStackPage(m_DeveloperWidget);
        }
    }
}

#ifdef UKUI_WAYLAND
void SettingsFrame::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // wayland 窗口无边框，需要在窗口show()之后执行
    UKUIDecorationManager::getInstance()->removeHeaderBar(windowHandle());
    UKUIDecorationManager::getInstance()->setCornerRadius(this->windowHandle(), 12, 12, 12, 12);
}

bool SettingsFrame::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == this) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->buttons() & Qt::LeftButton) {
                UKUIDecorationManager::getInstance()->moveWindow(this->windowHandle());
            }
        }
    }
    return false;
}
#endif
