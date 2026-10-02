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

#include "mainwindow.h"

#include <QDebug>
#include <QPluginLoader>
#include <QEventLoop>
#include <QGSettings>
#include "size_para.h"
#include "buried_point.hpp"
#include "kom-ukui-gsettings.h"
#include "kom-utils.h"
#include "toolutils.h"
#ifdef HAS_SOUNDEFFECTS
#include "ksoundeffects.h"
#endif

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    mainIcon = new QPushButton(this);
    mainIcon->setFixedSize(64, 64);
    contentTitleLabel = new kom::KomLabel(this);
    contentTitleLabel->setFontSize(24);
    contentTitleLabel->setBold(true);
    contentTitleLabel->setFirstNumColor(QColor("#F3222D"));
    contentTitleLabel->setSecondNumColor(QColor("#ffaa00"));
    contentTitleLabel->setText(tr("Detect Network Faults"));
    statusLabel = new CustomLabel(this);
    statusLabel->setFixedSize(420, 26);
    statusLabel->setText(tr("Detect and resolve Network Faults"));

    QVBoxLayout *mVLayoutLab = new QVBoxLayout;
    mVLayoutLab->setSpacing(0);
    mVLayoutLab->setContentsMargins(0, 0, 0, 5);
    mVLayoutLab->addStretch();
    mVLayoutLab->addWidget(contentTitleLabel);
    mVLayoutLab->addWidget(statusLabel);
    mVLayoutLab->addStretch();

    cancelBtn = new QPushButton(this);
    cancelBtn->setFixedHeight(36);
    cancelBtn->setMinimumWidth(96);
    cancelBtn->setText(tr("Cancel"));
    checkAgainBtn = new QPushButton(this);
    checkAgainBtn->setFixedHeight(36);
    checkAgainBtn->setMinimumWidth(96);
    checkAgainBtn->setText(tr("Restart"));
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(manualStopCheckProcess()));
    connect(checkAgainBtn, SIGNAL(clicked()), this, SLOT(reCheckProcess()));

    QVBoxLayout *mVLayoutBtn = new QVBoxLayout;
    mVLayoutBtn->setSpacing(0);
    mVLayoutBtn->setContentsMargins(0, 0, 0, 0);
    mVLayoutBtn->addWidget(cancelBtn);
    mVLayoutBtn->addWidget(checkAgainBtn);

    returnBtn = new QPushButton(this);
    returnBtn->setFixedHeight(36);
    returnBtn->setMinimumWidth(96);
    returnBtn->setText(tr("Return"));
    connect(returnBtn, &QPushButton::clicked, this, &MainWindow::slotReturnBtnClick);

    QHBoxLayout *mHLayoutTop = new QHBoxLayout;
    mHLayoutTop->setSpacing(0);
    mHLayoutTop->setContentsMargins(40, 24, 40, 0);
    mHLayoutTop->addWidget(mainIcon);
    mHLayoutTop->addSpacing(16);
    mHLayoutTop->addLayout(mVLayoutLab);
    mHLayoutTop->addStretch();
    mHLayoutTop->addWidget(returnBtn);
    mHLayoutTop->addSpacing(8);
    mHLayoutTop->addLayout(mVLayoutBtn);

    QWidget *midWidget = new QWidget(this);
    midWidget->setFixedHeight(20);
    line = new QFrame(midWidget);
    line->setFixedHeight(2);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->raise();
    progressBar = new KProgressBar(midWidget);
    progressBar->setState(ProgressBarState::NormalProgress);
    progressBar->setOrientation(Qt::Horizontal);
    progressBar->setMinimum(0);
    progressBar->setMaximum(120);
    progressBar->setValue(0);
    progressBar->setFixedHeight(4);
    progressBar->setTextVisible(false);

    QVBoxLayout *mVLayoutMid = new QVBoxLayout;
    mVLayoutMid->setSpacing(0);
    mVLayoutMid->setContentsMargins(0, 0, 0, 0);
    mVLayoutMid->addStretch();
    mVLayoutMid->addWidget(progressBar);
    mVLayoutMid->addWidget(line);
    mVLayoutMid->addStretch();
    midWidget->setLayout(mVLayoutMid);

    QLabel *mIconLabel = new QLabel(this);
    mIconLabel->setFixedSize(16, 16);
    QIcon ico(QIcon::fromTheme("ukui-network-agent-symbolic"));
    QPixmap pix = ico.pixmap(ico.actualSize(QSize(16, 16)));
    mIconLabel->setPixmap(pix);
    mIconLabel->setProperty("useIconHighlightEffect", 0x2);

    QFont font;
    mTitleLabel = new CustomLabel(this);
    font.setBold(true);
    font.setPixelSize(16);
    mTitleLabel->setFont(font);
    mTitleLabel->setText(tr("NetCheck"));

    mItemSumLabel = new CustomLabel(this);
    font.setBold(false);
    font.setPixelSize(12);
    mItemSumLabel->setFont(font);
    mItemSumLabel->setText(tr("total 6 items"));

    QPushButton *expandBtn = new QPushButton(this);
    expandBtn->setFixedSize(16, 16);
    expandBtn->setIcon(QIcon::fromTheme("ukui-up-symbolic"));
    expandBtn->setVisible(false);

    QHBoxLayout *mTitleLayout = new QHBoxLayout;
    mTitleLayout->setSpacing(0);
    mTitleLayout->setContentsMargins(40, 0, 40, 7);
    mTitleLayout->addWidget(mIconLabel);
    mTitleLayout->addSpacing(10);
    mTitleLayout->addWidget(mTitleLabel);
    mTitleLayout->addSpacing(12);
    mTitleLayout->addWidget(mItemSumLabel);
    mTitleLayout->addStretch();
    mTitleLayout->addWidget(expandBtn);

    // 网络检测单项区域界面
    checkWidget = new QWidget();
    widgetView = new QVBoxLayout(checkWidget);
    widgetView->setSpacing(0);
    widgetView->setContentsMargins(0, 0, 0, 0);
    widgetView->setAlignment(Qt::AlignTop);

    scrollAreaWidgetContents = new QWidget(this);
    allView = new QVBoxLayout(scrollAreaWidgetContents);
    allView->setSpacing(0);
    allView->setContentsMargins(0, 0, 0, 0);
    allView->setAlignment(Qt::AlignTop);
    allView->addWidget(checkWidget);

    scrollArea = new QScrollArea(this);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setFixedHeight(400);
    scrollArea->setWidget(scrollAreaWidgetContents);
    scrollArea->setWidgetResizable(true);

    QHBoxLayout *mScroHLayout = new QHBoxLayout;
    mScroHLayout->setSpacing(0);
    mScroHLayout->setContentsMargins(40, 0, 40, 0);
    mScroHLayout->addSpacing(26);
    mScroHLayout->addWidget(scrollArea);

    // 界面整体布局
    QVBoxLayout *mAllLayout = new QVBoxLayout;
    mAllLayout->setSpacing(0);
    mAllLayout->setContentsMargins(0, 0, 0, 40);
    mAllLayout->addLayout(mHLayoutTop);
    mAllLayout->addSpacing(14);
    mAllLayout->addWidget(midWidget);
    mAllLayout->addSpacing(14);
    mAllLayout->addLayout(mTitleLayout);
    mAllLayout->addLayout(mScroHLayout);
    mAllLayout->addStretch();
    this->setLayout(mAllLayout);

    qRegisterMetaType<CHECKRESULT>("CHECKRESULT");
    qRegisterMetaType<statusStruct>("statusStruct");
    qRegisterMetaType<InnerNetCheck>("InnerNetCheck&");
    qRegisterMetaType<QMap<QString, bool>>("QMap<QString,bool>");
    qRegisterMetaType<QMap<QString, QMap<QString, bool>>>("QMap<QString,QMap<QString,bool>>");

    m_variableItems.clear();
    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    this->move((availableGeometry.width() - this->width()) / 2, (availableGeometry.height() - this->height()) / 2);

    loadingPlugins();
    initDBus();
    initSettings();
    initUI();
}

MainWindow::~MainWindow()
{
    if (m_checkSettings) {
        delete m_checkSettings;
        m_checkSettings = nullptr;
    }
    if (m_themeSettings) {
        delete m_themeSettings;
        m_themeSettings = nullptr;
    }
    for (ItemWidget *it : m_variableItems) {
        if (it) {
            delete it;
            it = nullptr;
        }
    }
    m_variableItems.clear();
    if (m_ipTitle) {
        delete m_ipTitle;
        m_ipTitle = nullptr;
    }
    if (m_webTitle) {
        delete m_webTitle;
        m_webTitle = nullptr;
    }
    //这个补充可能是非必要的尝试
    //    for(PluginKits kit :m_pluginList){
    //        delete kit.m_pluginPtr;
    //    }
    //    this->close();
}

// 读取插件
void MainWindow::loadingPlugins()
{
    for (int i = 0; i <= static_cast<int>(CheckType::NET_CHECK); ++i) {
        pluginKitsIncrease();
    }

    static bool installed = (QCoreApplication::applicationDirPath() == QDir(("/usr/bin")).canonicalPath());
    if (installed) {
        m_pluginsDir = QDir("/usr/lib/kylin-os-manager/fault-detection/");
    } else {
        m_pluginsDir = QDir(qApp->applicationDirPath() + "/../../plugins/network-check/plugins");
        qDebug() << "application path:" << m_pluginsDir.absolutePath();
    }

    foreach (QString fileName, m_pluginsDir.entryList(QDir::Files)) {
        qDebug() << "Scan Plugin: " << fileName;

        if (!fileName.endsWith(".so")) {
            continue;
        }

        // 加载插件
        QPluginLoader loader(m_pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if (plugin) {
            LibBase *pluginInstance = dynamic_cast<LibBase *>(plugin);
            if (pluginInstance->getKYdynamicLibKey() == "KYAppCheckKey") {
                int type = static_cast<int>(pluginInstance->getPluginType());
                qDebug() << "current plugin type is :" << type;

                switch (type) {
                case static_cast<int>(CheckType::NW_HW_DEVICE_CONF): {
                    HWCheck *instance = dynamic_cast<HWCheck *>(plugin);
                    if (type < m_pluginList.size()) {
                        m_pluginList[static_cast<int>(pluginInstance->getPluginType())].setInit(instance);
                        connect(m_pluginList[static_cast<int>(pluginInstance->getPluginType())].m_pluginObserver,
                                SIGNAL(updateMainWinUI(const statusStruct &)), this,
                                SLOT(updateMainWinUISlot(const statusStruct &)));
                    }
                    break;
                }
                case static_cast<int>(CheckType::NW_CON_CONF): {
                    IPCheck *instance = dynamic_cast<IPCheck *>(plugin);
                    if (type < m_pluginList.size()) {
                        m_pluginList[static_cast<int>(pluginInstance->getPluginType())].setInit(instance);
                        connect(m_pluginList[static_cast<int>(pluginInstance->getPluginType())].m_pluginObserver,
                                SIGNAL(updateMainWinUI(const statusStruct &)), this,
                                SLOT(updateMainWinUISlot(const statusStruct &)));
                    }
                    break;
                }
                case static_cast<int>(CheckType::DHCP_CONF): {
                    DHCPCheck *instance = dynamic_cast<DHCPCheck *>(plugin);
                    if (type < m_pluginList.size()) {
                        m_pluginList[static_cast<int>(pluginInstance->getPluginType())].setInit(instance);
                        connect(m_pluginList[static_cast<int>(pluginInstance->getPluginType())].m_pluginObserver,
                                SIGNAL(updateMainWinUI(const statusStruct &)), this,
                                SLOT(updateMainWinUISlot(const statusStruct &)));
                    }
                    break;
                }
                case static_cast<int>(CheckType::DNS_CONF): {
                    DNSCheck *instance = dynamic_cast<DNSCheck *>(plugin);
                    if (type < m_pluginList.size()) {
                        m_pluginList[static_cast<int>(pluginInstance->getPluginType())].setInit(instance);
                        connect(m_pluginList[static_cast<int>(pluginInstance->getPluginType())].m_pluginObserver,
                                SIGNAL(updateMainWinUI(const statusStruct &)), this,
                                SLOT(updateMainWinUISlot(const statusStruct &)));
                    }
                    break;
                }
                case static_cast<int>(CheckType::HOST_CONF): {
                    HostCheck *instance = dynamic_cast<HostCheck *>(plugin);
                    if (type < m_pluginList.size()) {
                        m_pluginList[static_cast<int>(pluginInstance->getPluginType())].setInit(instance);
                        connect(m_pluginList[static_cast<int>(pluginInstance->getPluginType())].m_pluginObserver,
                                SIGNAL(updateMainWinUI(const statusStruct &)), this,
                                SLOT(updateMainWinUISlot(const statusStruct &)));
                    }
                    break;
                }
                    //                case static_cast<int>(CheckType::PROXY_CONF): {
                    //                    ProxyCheck *instance = dynamic_cast<ProxyCheck *>(plugin);
                    //                    if (type < m_pluginList.size()) {
                    //                        m_pluginList[static_cast<int>(pluginInstance->getPluginType())].setInit(instance);
                    //                        connect(m_pluginList[static_cast<int>(pluginInstance->getPluginType())].m_pluginObserver,
                    //                                SIGNAL(updateMainWinUI(const statusStruct &)), this,
                    //                                SLOT(updateMainWinUISlot(const statusStruct &)));
                    //                    }
                    //                    break;
                    //                }
                case static_cast<int>(CheckType::NET_CHECK): {
                    NetCheck *instance = dynamic_cast<NetCheck *>(plugin);
                    qDebug() << "++++++++++MainWindow::loadingPlugins load CheckType::NET_CHECK++++++++++++";
                    instance->setInitialCheckType(m_innerCheckArgSettings);
                    if (type < m_pluginList.size()) {
                        m_pluginList[static_cast<int>(pluginInstance->getPluginType())].setInit(instance);
                        connect(m_pluginList[static_cast<int>(pluginInstance->getPluginType())].m_pluginObserver,
                                SIGNAL(updateMainWinUI(const statusStruct &)), this,
                                SLOT(updateMainWinUISlot(const statusStruct &)));
                        connect(instance, SIGNAL(detailCheckRes(QMap<QString, QMap<QString, bool>>)), this,
                                SLOT(refreshInnerCheckRes(QMap<QString, QMap<QString, bool>>)));
                        connect(m_pluginList[static_cast<int>(pluginInstance->getPluginType())].m_UIPtr,
                                SIGNAL(showContent()), this, SLOT(showContent()));
                    }
                    break;
                }
                default:
                    qCritical() << "【异常】打开了非检测工具插件文件！文件名：" << fileName;
                    break;
                }
            } else {
                continue;
            }

        } else {
            //如果加载错误且文件后缀为so，输出错误
            if (fileName.endsWith(".so"))
                qWarning() << fileName << "Load Failed: " << loader.errorString() << "\n";
        }
    }
}

void MainWindow::initUI()
{
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);

    mainIcon->setIcon(QIcon(":/data/normal.svg"));
    mainIcon->setIconSize(TITLE_MAIN_SIZE);
    mainIcon->setFixedSize(TITLE_MAIN_SIZE);
    mainIcon->setStyleSheet(BTN_TO_LABEL_STYLE);

    setProgressBarVisible(false);

    cancelBtn->hide();
    checkAgainBtn->hide();
    checkAgainBtn->setProperty("isImportant", true);
    returnBtn->hide();

    widgetView->addSpacing(0);
    int points = m_pluginList.size();
    for (int i = 0; i < points; ++i) {
        if (m_pluginList[i].m_UIPtr) {
            widgetView->addWidget(m_pluginList[i].m_UIPtr);
            m_pluginList[i].m_pluginPtr->setInit();
        } else {
            qCritical() << "加载了非正确的动态库！";
            continue;
        }
    }
    widgetView->addSpacing(10);

    connect(this, SIGNAL(startCheckIndex(int)), this, SLOT(pluginStartCheck(int)), Qt::QueuedConnection);
}

void MainWindow::initDBus()
{
    // TODO:最小化后拉起界面
    //    m_NWMDBus = new NmDbusAdaptor();
}

void MainWindow::initSettings()
{
    const QByteArray ukuiStyle(UKUI_STYLE);
    if (QGSettings::isSchemaInstalled(ukuiStyle)) {
        m_themeSettings = new QGSettings(ukuiStyle);
        connect(m_themeSettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "styleName") {
                m_curStyle = m_themeSettings->get("styleName").toString();
                themeUIRefresh();
            }
        });
        m_curStyle = m_themeSettings->get("styleName").toString();
        themeUIRefresh();
    }

    ToolUtils toolUtils;
    m_innerCheckArgSettings.isInnerCheck = toolUtils.getInnerCheckSetting();
    QMap<QString, QVector<QString>> settingsMap = toolUtils.getConfigFile();
    QList<QString> ipStrList;
    QList<QString> webStrList;
    if (settingsMap.contains("config-ip")) {
        QVector<QString> ipVector = settingsMap.value("config-ip");
        ipStrList.clear();
        ipStrList = ipVector.toList();
    }
    if (settingsMap.contains("config-web")) {
        QVector<QString> webVector = settingsMap.value("config-web");
        webStrList.clear();
        webStrList = webVector.toList();
    }
    m_innerCheckArgSettings.ipClear();
    for (int i = 0; i < ipStrList.size(); ++i) {
        if (!(ipStrList.at(i)).isEmpty()) {
            m_innerCheckArgSettings.ip[i] = ipStrList.at(i);
            ++m_innerCheckArgSettings.ipNum;
        }
    }
    m_innerCheckArgSettings.webClear();
    for (int i = 0; i < webStrList.size(); ++i) {
        if (!(webStrList.at(i)).isEmpty()) {
            m_innerCheckArgSettings.web[i] = webStrList.at(i);
            ++m_innerCheckArgSettings.webNum;
        }
    }
}

// 开始检查函数
void MainWindow::startCheckProcess()
{
    refreshUI();

    contentTitleLabel->setText(tr("Checking..."));
    statusLabel->setText(tr("Start"));
    cancelBtn->show();
    checkAgainBtn->hide();
    returnBtn->hide();
    setProgressBarVisible(true);
    progressBar->setValue(0);
    m_goOnCheck = true;

    kdk::kabase::BuriedPoint buriedPoint;
    if (buriedPoint.functionBuriedPoint(kdk::kabase::AppName::KylinNetworkCheck,
                                        kdk::kabase::BuriedPoint::PT::KylinNetworkCheckStartCheck)) {
        qCritical() << "buried point fail!pt:BaseInfo";
    }

    emit startCheckIndex(0); //从第一项开始检查
}

void MainWindow::errStopCheckProcess()
{
    cancelBtn->hide();
    returnBtn->show();
    checkAgainBtn->show();

	setCheckResultDesc();

    setProgressBarVisible(false);
    progressBar->setValue(0);
    m_correctItem = 0;
    m_warningItem = 0;
    m_errorItem = 0;
}

void MainWindow::manualStopCheckProcess()
{
    contentTitleLabel->setText(tr("Canceling..."));
    m_goOnCheck = false;
    cancelBtn->setEnabled(false);
    connect(this, SIGNAL(manualStoped()), this, SLOT(slotManualStopCheck()), Qt::UniqueConnection);
}

void MainWindow::slotManualStopCheck()
{
    cancelBtn->hide();
    checkAgainBtn->show();
    checkAgainBtn->setEnabled(true);
    returnBtn->show();
    m_goOnCheck = false;

	setCheckResultDesc();

    m_correctItem = 0;
    m_warningItem = 0;
    m_errorItem = 0;
}

void MainWindow::reCheckProcess()
{
    startCheckProcess();
}

void MainWindow::showConfigureWin()
{
    qDebug() << "MainWindow::showConfigureWin m_goOnCheck:" << m_goOnCheck;
    //如果正在检测，则点击设置无效
    if (m_goOnCheck) {
        qDebug() << "检测过程中，禁止设置！";
        return;
    }
    //新建设置界面并移动到窗体中央
    ConfigWin configureWin;
    connect(&configureWin, SIGNAL(sigUpdateConfigFile()), this, SLOT(slotUpdateConfigFile()));
    QPoint windowCenter = QPoint(this->mapToGlobal(QPoint(0, 0)).x() + this->width() / 2,
                                 this->mapToGlobal(QPoint(0, 0)).y() + this->height() / 2);
    configureWin.move(windowCenter - configureWin.rect().center());
    configureWin.activateWindow();
    configureWin.showWin();
    //界面刷新靠gsettings变化，无需再处理
}
void MainWindow::slotUpdateConfigFile()
{
    qDebug() << "MainWindow::slotUpdateConfigFile";
    ToolUtils toolUtils;
    m_innerCheckArgSettings.isInnerCheck = toolUtils.getInnerCheckSetting();
    QMap<QString, QVector<QString>> settingsMap = toolUtils.getConfigFile();
    QList<QString> ipStrList;
    QList<QString> webStrList;
    if (settingsMap.contains("config-ip")) {
        QVector<QString> ipVector = settingsMap.value("config-ip");
        ipStrList.clear();
        ipStrList = ipVector.toList();
        qDebug() << "MainWindow::slotUpdateConfigFile config-ip:" << ipStrList;
    }
    if (settingsMap.contains("config-web")) {
        QVector<QString> webVector = settingsMap.value("config-web");
        webStrList.clear();
        webStrList = webVector.toList();
        qDebug() << "MainWindow::slotUpdateConfigFile config-web:" << webStrList;
    }
    m_innerCheckArgSettings.ipClear();
    for (int i = 0; i < ipStrList.size(); ++i) {
        if (!(ipStrList.at(i)).isEmpty()) {
            m_innerCheckArgSettings.ip[i] = ipStrList.at(i);
            ++m_innerCheckArgSettings.ipNum;
        }
    }
    m_innerCheckArgSettings.webClear();
    for (int i = 0; i < webStrList.size(); ++i) {
        if (!(webStrList.at(i)).isEmpty()) {
            m_innerCheckArgSettings.web[i] = webStrList.at(i);
            ++m_innerCheckArgSettings.webNum;
        }
    }
    refreshUI();
}
void MainWindow::allFinished()
{
#ifdef HAS_SOUNDEFFECTS
    kdk::KSoundEffects::playSound(SoundType::COMPLETE);
#endif

    cancelBtn->hide();
    checkAgainBtn->show();
    returnBtn->show();
    setProgressBarVisible(false);
    progressBar->setValue(0);

	setCheckResultDesc();
}

void MainWindow::themeUIRefresh()
{
    if ("ukui-dark" == m_curStyle || "ukui-black" == m_curStyle) {
        QPalette linePalette = line->palette();
        QColor lineColor = QColor(255, 255, 255);
        lineColor.setAlphaF(0.08);
        linePalette.setColor(QPalette::Light, lineColor);
        line->setPalette(linePalette);

        QPalette statusLabelPalette = line->palette();
        statusLabelPalette.setColor(QPalette::Text, QColor(89, 89, 89, 255));
        statusLabel->setPalette(statusLabelPalette);
    } else {
        QPalette linePalette = line->palette();
        QColor lineColor = QColor(0, 0, 0);
        lineColor.setAlphaF(0.08);
        linePalette.setColor(QPalette::Dark, lineColor);
        line->setPalette(linePalette);
        QPalette statusLabelPalette = line->palette();
        statusLabelPalette.setColor(QPalette::Text, QColor(89, 89, 89, 255));
        statusLabel->setPalette(statusLabelPalette);
    }
}
//开启内网检测，检测完毕后，点显示详情后显示的区域
void MainWindow::showContent()
{
    //根据区域的显示状态，决定显示还是隐藏
    if (m_ipTitle) {
        if (m_ipTitle->isHidden()) {
            m_ipTitle->show();
        } else {
            m_ipTitle->hide();
        }
    }
    if (m_webTitle) {
        if (m_webTitle->isHidden()) {
            m_webTitle->show();
        } else {
            m_webTitle->hide();
        }
    }
    if (isShow) {
        for (auto &item : m_variableItems) {
            item->hide();
        }
    } else {
        for (auto &item : m_variableItems) {
            item->show();
        }
    }
    isShow = !isShow;
}
//根据检测结果，刷新详情区域内容
void MainWindow::refreshInnerCheckRes(QMap<QString, QMap<QString, bool>> resMap)
{

    //清空原区域
    for (auto &item : m_variableItems) {
        delete item;
    }
    m_variableItems.clear();
    if (m_ipTitle) {
        delete m_ipTitle;
        m_ipTitle = nullptr;
    }
    if (m_webTitle) {
        delete m_webTitle;
        m_webTitle = nullptr;
    }

    //    QMap<QString,bool> baiduRes;
    QMap<QString, bool> ipRes;
    QMap<QString, bool> webRes;
    if (resMap.isEmpty()) {
        qWarning() << "MainWindow::refreshInnerCheckRes resMap.isEmpty!";
    } else {
        //        baiduRes.clear();
        ipRes.clear();
        webRes.clear();
        //        baiduRes = resMap.value("baidu",baiduRes);
        ipRes = resMap.value("ip", ipRes);
        webRes = resMap.value("web", webRes);
    }
    //添加IP部分的窗口
    if (!ipRes.isEmpty()) {
        m_ipTitle = new ItemWidget(scrollAreaWidgetContents);
        m_ipTitle->setStatusCheck(CheckStatus::EVERTHING_IS_OK);
        m_ipTitle->setCheckRes(tr("Intranet IP"), tr(""));
        allView->addWidget(m_ipTitle);
        m_ipTitle->hide();
    }
    QMap<QString, bool>::iterator ipIter = ipRes.begin();
    while (ipIter != ipRes.end()) {
        ItemWidget *temp = new ItemWidget(scrollAreaWidgetContents);
        if (ipIter.value()) {
            temp->setCheckRes(ipIter.key(), "OK");
            temp->setStatusCheck(CheckStatus::EVERTHING_IS_OK);
        } else {
            temp->setCheckRes(ipIter.key(), "ERR");
            temp->setStatusCheck(CheckStatus::ERROR);
        }
        allView->addWidget(temp);
        m_variableItems.append(temp);
        temp->hide();
        ++ipIter;
    }
    //添加web部分的窗口
    if (!webRes.isEmpty()) {
        m_webTitle = new ItemWidget(scrollAreaWidgetContents);
        m_webTitle->setStatusCheck(CheckStatus::EVERTHING_IS_OK);
        m_webTitle->setCheckRes(tr("Intranet Web"), tr(""));
        allView->addWidget(m_webTitle);
        m_webTitle->hide();
    }
    QMap<QString, bool>::iterator webIter = webRes.begin();
    while (webIter != webRes.end()) {
        ItemWidget *temp = new ItemWidget(scrollAreaWidgetContents);
        if (webIter.value()) {
            temp->setCheckRes(webIter.key(), "OK");
            temp->setStatusCheck(CheckStatus::EVERTHING_IS_OK);
        } else {
            temp->setCheckRes(webIter.key(), "ERR");
            temp->setStatusCheck(CheckStatus::ERROR);
        }
        allView->addWidget(temp);
        m_variableItems.append(temp);
        temp->hide();
        ++webIter;
    }
}

//刷新主界面插件部分UI
//入参为各插件检测结果
void MainWindow::updateMainWinUISlot(const statusStruct &curStatus)
{
    m_pluginList[curStatus.m_index].m_UIPtr->statusChanged(curStatus);
    m_pluginList[curStatus.m_index].m_UIPtr->repaint();

    // netcheck为最后一项，需要特殊处理
    if (curStatus.m_index == static_cast<int>(CheckType::NET_CHECK)) {
        if (curStatus.m_curStutus == CheckStatus::INIT) {
            statusStruct temp = curStatus;
            if (m_innerCheckArgSettings.isInnerCheck) {
                qDebug() << "通过设置切换为内网检测！";
                temp.m_projectName = QString(tr("InnerNet Check"));
                temp.m_projectDetail = QString(tr("Check whether the intranet is smooth"));
            } else {
                qDebug() << "通过设置切换为外网检测！";
                temp.m_projectName = QString(tr("Internet access"));
                temp.m_projectDetail = QString(tr("Can user browse out net?"));
            }
            m_pluginList[curStatus.m_index].m_UIPtr->statusChanged(temp);
        }
    }

    //自动检测下一项
    if (curStatus.m_curStutus == CheckStatus::EVERTHING_IS_OK
        || curStatus.m_curStutus == CheckStatus::WARNING
        || curStatus.m_curStutus == CheckStatus::ERROR)
    {
        // 记录各种状态的数量
        if (curStatus.m_curStutus == CheckStatus::WARNING) {
            ++m_warningItem;
        } else if (curStatus.m_curStutus == CheckStatus::ERROR) {
            ++m_errorItem;
        } else {
            ++m_correctItem;
        }

        progressBar->setValue(20 + 20 * curStatus.m_index);

        //============如果要错误停止，应该在这个地方停止==================
        if (curStatus.m_index == 0 && m_errorItem != 0) {
            errStopCheckProcess();
            return;
        }

        if (m_warningItem == 0 && m_errorItem == 0) {
            statusLabel->setText(tr("checked %1 items, no issue").arg(curStatus.m_index + 1));
        } else if (m_warningItem != 0 && m_errorItem !=0 ) {
            statusLabel->setText(tr("checked %1 items, find %2 errs, %3 issues")
                                 .arg(curStatus.m_index + 1)
                                 .arg(m_errorItem)
                                 .arg(m_warningItem));
        } else if (m_errorItem != 0) {
            statusLabel->setText(tr("checked %1 items, find %2 errs")
                                 .arg(curStatus.m_index + 1)
                                 .arg(m_errorItem));
        } else if (m_warningItem != 0) {
            statusLabel->setText(tr("checked %1 items, find %2 issues")
                                 .arg(curStatus.m_index + 1)
                                 .arg(m_warningItem));
        } else {
            statusLabel->setText(tr("checked %1 items, find %2 errs, %3 issues")
                                 .arg(curStatus.m_index + 1)
                                 .arg(m_errorItem)
                                 .arg(m_warningItem));
        }

        //通过m_goOnCheck控制是否继续检测（即点击取消的flag）
        if (m_goOnCheck) {
            if (curStatus.m_index < m_pluginList.size() - 1) {
                qDebug() << "MainWindow::updateMainWinUISlot next check start :" << curStatus.m_index + 1;
                m_pluginList[curStatus.m_index + 1].m_pluginPtr->startChecking(m_innerCheckArgSettings);
            }
            if (curStatus.m_index == m_pluginList.size() - 1) {
                //设计要求进度条满了之后要停留1s，否则看不出来进度100%，之后立即变0%
                QEventLoop loop;
                QTimer::singleShot(1000, &loop, SLOT(quit()));
                loop.exec();
                allFinished();
            }
        } else {
            qDebug() << "MainWindow::updateMainWinUISlot m_goOnCheck:false";
            emit manualStoped();
        }
    } else {
        qDebug() << "第" << curStatus.m_index << "项检测中";
    }
}

//从第几项开始检测（为后续暂停检测，又继续检测的接口）
void MainWindow::pluginStartCheck(int index)
{
    m_pluginList[index].m_pluginPtr->startChecking(m_innerCheckArgSettings);
}
//新建结构体塞入list
void MainWindow::pluginKitsIncrease()
{
    PluginKits temp;
    temp.m_pluginObserver = nullptr;
    temp.m_pluginPtr = nullptr;
    temp.m_UIPtr = nullptr;
    m_pluginList.append(temp);
    qDebug() << "m_pluginList 大小为：" << m_pluginList.size();
}

// 重置UI
void MainWindow::refreshUI()
{
    isShow = false;
    m_correctItem = 0;
    m_warningItem = 0;
    m_errorItem = 0;

    mainIcon->setIcon(QIcon(":/data/normal.svg"));
    statusLabel->setText(tr("Detect and resolve Network Faults"));
    contentTitleLabel->setText(tr("Detect Network Faults"));

    cancelBtn->hide();
    returnBtn->hide();
    checkAgainBtn->hide();
    cancelBtn->setEnabled(true);

    progressBar->setValue(0);
    progressBar->setState(ProgressBarState::NormalProgress);

    // 插件部分 UI 重置
    for (auto &it : m_pluginList) {
        it.m_pluginPtr->setInit();
    }

    // 详情部分 UI 重置
    for (auto &item : m_variableItems) {
        delete item;
    }
    m_variableItems.clear();

    if (m_ipTitle) {
        delete m_ipTitle;
        m_ipTitle = nullptr;
    }

    if (m_webTitle) {
        delete m_webTitle;
        m_webTitle = nullptr;
    }
}
void MainWindow::setProgressBarVisible(bool isBarShow)
{
    if (isBarShow) {
        progressBar->setVisible(true);
        line->setVisible(false);
    } else {
        progressBar->setVisible(false);
        line->setVisible(true);
    }
}
void MainWindow::slotReturnBtnClick()
{
    emit sigChangeStackWidgetMain(0);
}
void MainWindow::changeThemeColor(int color)
{
    if (color) { //深色

    } else {
    }
}

void MainWindow::changeSystemSize(int size)
{
    QFont font;

    // 网络检测标题字体
    font.setBold(true);
    font.setPointSizeF(kom::KomUtils::adaptFontSize(16));
    mTitleLabel->setFont(font);

    // 网络检测标题总项字体
    font.setBold(false);
    font.setPointSizeF(kom::KomUtils::adaptFontSize(12));
    mItemSumLabel->setFont(font);
}

void MainWindow::setCheckResultDesc(void)
{
    // 手动停止检测
    if (!m_goOnCheck) {
        if (m_errorItem == 0) {
            contentTitleLabel->setText(tr("Check interrupted, no issues found"));
            statusLabel->setText(tr("We suggest that you conduct a complete inspection again"));
            mainIcon->setIcon(QIcon(":/data/allOKFinished.svg"));
        } else {
            contentTitleLabel->setFirstNumColor(QColor("#F3222D"));
            contentTitleLabel->setText(tr("Check interruption and found %1 issues").arg(m_errorItem));
            statusLabel->setText(tr("Please repair and retest"));
            mainIcon->setIcon(QIcon(":/data/errFinished.svg"));
        }
        return;
    }

    // 正常检测结束
	if (m_errorItem == 0 && m_warningItem == 0) {
		contentTitleLabel->setText(tr("No problems found"));
		statusLabel->setText(tr("Please continue to maintain and regularly check up"));
		mainIcon->setIcon(QIcon(":/data/allOKFinished.svg"));
	} else if (m_errorItem != 0 && m_warningItem != 0) {
		contentTitleLabel->setFirstNumColor(QColor("#F3222D"));
		contentTitleLabel->setSecondNumColor(QColor("#ffaa00"));
		contentTitleLabel->setText(tr("Found %1 problem and %2 prompt problems").arg(m_errorItem).arg(m_warningItem));
		statusLabel->setText(tr("Please re-detect after repair"));
		mainIcon->setIcon(QIcon(":/data/errFinished.svg"));
	} else if (m_errorItem != 0) {
		contentTitleLabel->setFirstNumColor(QColor("#F3222D"));
		contentTitleLabel->setText(tr("Found %1 problem").arg(m_errorItem));
		statusLabel->setText(tr("Please re-detect after repair"));
		mainIcon->setIcon(QIcon(":/data/errFinished.svg"));
	} else if (m_warningItem != 0) {
		contentTitleLabel->setFirstNumColor(QColor("#ffaa00"));
		contentTitleLabel->setText(tr("Found %1 prompt problem").arg(m_warningItem));
		statusLabel->setText(tr("Please re-detect after repair"));
		mainIcon->setIcon(QIcon(":/data/errFinished.svg"));
	} else {
		contentTitleLabel->setFirstNumColor(QColor("#F3222D"));
		contentTitleLabel->setSecondNumColor(QColor("#ffaa00"));
		contentTitleLabel->setText(tr("Found %1 problem and %2 prompt problems").arg(m_errorItem).arg(m_warningItem));
		statusLabel->setText(tr("Please re-detect after repair"));
		mainIcon->setIcon(QIcon(":/data/errFinished.svg"));
	}
}
