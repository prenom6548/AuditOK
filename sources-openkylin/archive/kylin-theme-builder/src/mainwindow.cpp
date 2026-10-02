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

#include "mainwindow.h"
#include "ukuistylehelper/ukuistylehelper.h"
#include "kysdk/kysdk-system/libkysysinfo.h"

#include <QUrl>
#include <QDesktopServices>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    qApp->setApplicationName(tr("kylin-theme-builder"));
    kdk::UkuiStyleHelper *lf = kdk::UkuiStyleHelper::self();
    lf->removeHeader(this);
    this->setMouseTracking(true);
    this->setFocus();
    char* systemtype = kdk_system_get_systemName();
    qDebug()<<"system ==" << systemtype;
    if(systemtype == " Kylin"){
        m_isKylin = true;
    }else{
        m_isKylin = false;
    }
    m_bridge = Bridge::getInstance();
    FileProcess();
    m_bridge->createSavePathConfig();
    m_titlebar = new TitleBar(this);
    initUI();
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0,0 );
    layout->setSpacing(0);
    layout->addWidget(m_titlebar);
    layout->addWidget(mainWidget);
    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);
    setMouseTracking(true);
    connect(m_titlebar, &TitleBar::gohomesignal, this, &MainWindow::onGoHomeClicked);
    connect(m_titlebar, &TitleBar::maximizesignal, this, &MainWindow::useMaximumScale);
    connect(m_titlebar, &TitleBar::restoresignal, this, &MainWindow::useRestoreScale);
    connect(m_maininterface, &MainInterface::wallpaperPathChanged, m_bridge, &Bridge::wallpaperPathChanged);
    connect(m_maininterface, &MainInterface::coverPathChanged, m_bridge, &Bridge::coverPathChanged);
    connect(m_maininterface, &MainInterface::radiusChanged, m_bridge, &Bridge::radiusChanged);
    connect(m_maininterface, &MainInterface::accentColorChanged, m_bridge, &Bridge::accentColorChanged);
    connect(m_maininterface, &MainInterface::exteriorChanged, m_bridge, &Bridge::exteriorChanged);
    connect(m_maininterface, &MainInterface::transparencyChanged, m_bridge, &Bridge::transparencyChanged);
    connect(m_maininterface, &MainInterface::windowRadiusChanged, m_bridge, &Bridge::windowRadiusChanged);
//    connect(m_maininterface, &MainInterface::appIconsMapChanged, this,[=]{
//        m_bridge->updateIconCache(m_maininterface->getAppIconsMap(),"appicon");
//    });
    connect(m_maininterface, &MainInterface::appIconsMapChanged, m_bridge, &Bridge::appIconsMapChanged);
    connect(m_maininterface, &MainInterface::systemIconsMapChanged, m_bridge, &Bridge::systemIconsMapChanged);
    connect(m_maininterface, &MainInterface::placeIconsMapChanged, m_bridge, &Bridge::placeIconsMapChanged);
    connect(m_maininterface, &MainInterface::deviceIconsMapChanged, m_bridge, &Bridge::deviceIconsMapChanged);
    connect(m_maininterface, &MainInterface::cursorMapChanged, m_bridge, &Bridge::cursorMapChanged);
    connect(m_maininterface, &MainInterface::timeCursorMapChanged, m_bridge, &Bridge::timeCursorMapChanged);
    connect(m_maininterface, &MainInterface::plymouthPathChanged, m_bridge, &Bridge::plymouthPathChanged);
    connect(m_maininterface, &MainInterface::grubPathChanged, m_bridge, &Bridge::grubPathChanged);
    connect(m_maininterface, &MainInterface::refreshPlymouthDir, m_bridge, &Bridge::createPlymouth);
//    connect(m_maininterface, &MainInterface::startCopy, m_bridge, &Bridge::startCopy);
    connect(m_bridge,&Bridge::updateInfo,m_historywidget,&HistoryWidget::updateInfo);
    connect(m_bridge,&Bridge::updateThumbnail,m_maininterface,&MainInterface::updateThumbnail);

    connect(m_maininterface, &MainInterface::globalthemeChange, this, [&]( QMap<QString, bool> *globalthemeChangeMap){
        for (auto it = globalthemeChangeMap->begin(); it != globalthemeChangeMap->end(); ++it) {
            const QString& key = it.key();
            bool value = it.value();

//            qDebug() << "Key:" << key << "Value:" << value;

            // 判断键值是否为 true
            if (value) {
                GlobalThemeWidget::g_themeChange  = true;
                break; // 如果找到一个 true，就可以中断循环了
            } else {
                GlobalThemeWidget::g_themeChange = false;
            }
        }
    });
    connect(m_maininterface, &MainInterface::iconthemeChange, this, [&]( QMap<QString, bool> *iconthemeChangeMap){
        for (auto it = iconthemeChangeMap->begin(); it != iconthemeChangeMap->end(); ++it) {
            const QString& key = it.key();
            bool value = it.value();

//            qDebug() << "Key:" << key << "Value:" << value;

            // 判断键值是否为 true
            if (value) {
                IconThemeWidget::g_themeChange  = true;
                break; // 如果找到一个 true，就可以中断循环了
            } else {
                IconThemeWidget::g_themeChange = false;
            }
        }
    });
    connect(m_maininterface, &MainInterface::cursorthemeChange, this, [&]( QMap<QString, bool> *cursorthemeChangeMap){
        for (auto it = cursorthemeChangeMap->begin(); it != cursorthemeChangeMap->end(); ++it) {
            const QString& key = it.key();
            bool value = it.value();

//            qDebug() << "Key:" << key << "Value:" << value;

            // 判断键值是否为 true
            if (value) {
                CursorThemeWidget::g_themeChange  = true;
                break; // 如果找到一个 true，就可以中断循环了
            } else {
                CursorThemeWidget::g_themeChange = false;
            }
        }
    });
    connect(m_maininterface, &MainInterface::gohomesignals, this, &MainWindow::onGoHomeClicked);
    connect(m_titlebar,&TitleBar::savePathUpdate,this,[&](const QString& path){
        FileProcess::g_debPath = path;
        m_bridge->savePathConfigChanged(path);
    });
    connect(m_maininterface,&MainInterface::hasGetThemeName,m_bridge,&Bridge::ThemeNameChanged);
    this->setContentsMargins(0,5,0,0);
}



MainWindow::~MainWindow()
{
}

/**
 * @brief 初始化界面
 */
void MainWindow::initUI()
{
//    QPalette palette;
//    palette.setColor(QPalette::Background, Qt::white);
//    this->setAutoFillBackground(true);
//    this->setPalette(palette);
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);

    this->setMinimumSize(1280, 720);

    m_maininterface = new MainInterface();
    m_entrywidget = new QWidget();
    initEntry();

    QVBoxLayout* alllayout = new QVBoxLayout;

    QGridLayout* layout = new QGridLayout;

    layout->setSpacing(24);

    layout->addWidget(m_globalbtn, 1, 1);
    layout->addWidget(m_iconbtn, 1, 2);
    layout->addWidget(m_cursorbtn, 1, 3);
    layout->addWidget(m_plymouthbtn, 2, 1);
    layout->addWidget(m_grubbtn, 2, 2);

    layout->setColumnStretch(0, 1);
    layout->setRowStretch(0, 1);
    layout->setColumnStretch(4, 1);
    layout->setRowStretch(3, 1);


    m_guideWidget = new QWidget();

    QHBoxLayout *labellayout = new QHBoxLayout();
    QLabel *Label = new QLabel(m_guideWidget);
    Label->setText(tr("kylin-theme-builder"));
//    QPushButton *guideLabel = new QPushButton(m_guideWidget);
    QLabel *guideLabel = new QLabel(m_guideWidget);
    QString str = "<a href=\"https://gitee.com/openkylin/kylin-theme-builder/blob/openkylin/nile/README.md\">" + tr("User guide") + "</a>";
    guideLabel->setText(str);
    guideLabel->setTextFormat(Qt::RichText);
    guideLabel->setCursor(Qt::PointingHandCursor);
    guideLabel->setOpenExternalLinks(true); // 将此属性设置为 true

    labellayout->addStretch(1);
    labellayout->addWidget(Label);
    labellayout->addWidget(guideLabel);
    labellayout->addStretch(1);

    m_guideWidget->setLayout(labellayout);

    m_historywidget = new HistoryWidget();
//    m_historywidget->setMinimumHeight(2000);
    QDir builderDir = QDir::homePath()+"/.cache/theme-build/";
    QFileInfoList fileInfoList = builderDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);

    if(fileInfoList.isEmpty() || (fileInfoList.count() == 1 && fileInfoList.first().isFile() && fileInfoList.first().fileName() == "savepath.conf")){
        alllayout->addStretch(1);
        alllayout->addLayout(layout);
        alllayout->addSpacing(80);
        alllayout->addWidget(m_guideWidget);
        alllayout->addStretch(1);
    }else{
        alllayout->addStretch(1);
        alllayout->addLayout(layout);
        alllayout->addSpacing(80);
        alllayout->addWidget(m_guideWidget);
        alllayout->addWidget(m_historywidget);
    }


    m_entrywidget->setLayout(alllayout);
    mainWidget = new QWidget(this);
    mainWidget->setContentsMargins(0,0,0,0);
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setContentsMargins(0,0,0,0);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(m_entrywidget);
    scrollArea->setFrameStyle(QFrame::HLine);
    m_stackedWidget = new QStackedWidget(this);
    //setContentsMargins为0不生效
    m_stackedWidget->setContentsMargins(0,0,0,0);

    m_stackedWidget->addWidget(scrollArea);
    m_stackedWidget->addWidget(m_maininterface);
    m_titlebar->setGoHomeBtn(false);
    QVBoxLayout* mainLayout = new QVBoxLayout;
    QPushButton *line = new QPushButton();
    line->setDisabled(true);
    line->setFixedHeight(1);
    //titelbar line
    mainLayout->addWidget(line);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(m_stackedWidget);
    mainLayout->setContentsMargins(0,0,0,0);


    mainWidget->setLayout(mainLayout);

    mainWidget->setContentsMargins(0, 0, 0,0 );

    QButtonGroup* buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(m_globalbtn);
    buttonGroup->addButton(m_iconbtn);
    buttonGroup->addButton(m_cursorbtn);
    buttonGroup->addButton(m_plymouthbtn);
    buttonGroup->addButton(m_grubbtn);

    m_globalbtn->setProperty("useButtonPalette", true);
    m_iconbtn->setProperty("useButtonPalette", true);
    m_cursorbtn->setProperty("useButtonPalette", true);
    m_plymouthbtn->setProperty("useButtonPalette", true);
    m_grubbtn->setProperty("useButtonPalette", true);

    connect(m_historywidget,&HistoryWidget::startSecondEdit,[=](HistoryInfo InfoData){
        m_titlebar->setGoHomeBtn(true);
        m_maininterface->isHistory(true);
        m_stackedWidget->setCurrentWidget(m_maininterface);

        m_bridge->createFileManager(InfoData.historytime);

        m_maininterface->getThemeInfo(InfoData);

        m_ishistory = true;
        if(InfoData.themetype == "globalTheme"){
            m_maininterface->hideNavigation(true);
            m_maininterface->setBuildBtnText(true);

        }else{
            m_maininterface->hideNavigation(false);
            m_maininterface->setBuildBtnText(false);

        }
    });

    connect(buttonGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton* button) {
        m_stackedWidget->setCurrentWidget(m_maininterface);
        m_ishistory = false;
        m_titlebar->setGoHomeBtn(true);
        m_maininterface->isHistory(false);
        if (button == m_globalbtn) {
            DataCollect::sendMainWindowKdkDataAsync(DataCollect::m_GlobalTheme);
            m_maininterface->setGlobalTheme();
            m_fileProcess.FileCreate("globalTheme");
            m_bridge->createFileManager(FileProcess::g_date);
            m_bridge->updateIconCache(m_maininterface->getAppIconsMap(),"appicon");
            m_bridge->updateIconCache(m_maininterface->getSystemIconsMap(),"systemicon");
            m_bridge->updateIconCache(m_maininterface->getPlaceIconsMap(),"placeicon");
            m_bridge->updateIconCache(m_maininterface->getDeviceIconsMap(),"deviceicon");
            m_bridge->updateIconCache(m_maininterface->getCurosrMap(),"cursor");
            m_bridge->updateIconCache(m_maininterface->getTimeCurosrMap(),"timecursor");

            m_bridge->createConfig();

            m_bridge->createGrub();
            m_bridge->createPlymouth();
            m_bridge->creatDesktop();
            m_bridge->createSetupConf();
            m_maininterface->refresh("global");
            m_maininterface->refresh("icon");
            m_maininterface->refresh("cursor");
            m_maininterface->refresh("plymouth");
            m_maininterface->refresh("grub");
            m_maininterface->hideNavigation(true);
            m_maininterface->setBuildBtnText(true);

        } else if (button == m_iconbtn) {
            DataCollect::sendMainWindowKdkDataAsync(DataCollect::m_IconTheme);
            m_maininterface->setIconTheme();
            m_fileProcess.FileCreate("iconTheme");
            m_bridge->createFileManager(FileProcess::g_date);
            m_bridge->updateIconCache(m_maininterface->getAppIconsMap(),"appicon");
            m_bridge->updateIconCache(m_maininterface->getSystemIconsMap(),"systemicon");
            m_bridge->updateIconCache(m_maininterface->getPlaceIconsMap(),"placeicon");
            m_bridge->updateIconCache(m_maininterface->getDeviceIconsMap(),"deviceicon");
            m_maininterface->refresh("icon");
            m_maininterface->hideNavigation(false);
            m_maininterface->setBuildBtnText(false);
            m_bridge->creatDesktop();
            m_bridge->createSetupConf();
        } else if (button == m_cursorbtn) {
            DataCollect::sendMainWindowKdkDataAsync(DataCollect::m_CursorTheme);
            m_maininterface->setCursorTheme();
            m_fileProcess.FileCreate("cursorTheme");
            m_bridge->createFileManager(FileProcess::g_date);
            m_bridge->updateIconCache(m_maininterface->getCurosrMap(),"cursor");
            m_bridge->updateIconCache(m_maininterface->getTimeCurosrMap(),"timecursor");
            m_maininterface->refresh("cursor");
            m_maininterface->hideNavigation(false);
            m_maininterface->setBuildBtnText(false);
            m_bridge->creatDesktop();
            m_bridge->createSetupConf();
        }else if (button == m_plymouthbtn) {
            DataCollect::sendMainWindowKdkDataAsync(DataCollect::m_PlymouthTheme);
            m_maininterface->setPlymouthTheme();
            m_fileProcess.FileCreate("plymouthTheme");
            m_bridge->createFileManager(FileProcess::g_date);
            m_bridge->createPlymouth();
            startShowPlymouth();
            m_maininterface->refresh("plymouth");
            m_maininterface->hideNavigation(false);
            m_maininterface->setBuildBtnText(false);
            m_bridge->creatDesktop();
            m_bridge->createSetupConf();
        }else if (button == m_grubbtn) {
            DataCollect::sendMainWindowKdkDataAsync(DataCollect::m_GrubTheme);
            m_maininterface->setGrubTheme();
            m_fileProcess.FileCreate("grubTheme");
            m_bridge->createFileManager(FileProcess::g_date);
            m_bridge->createGrub();
            m_maininterface->refresh("grub");
            m_maininterface->hideNavigation(false);
            m_maininterface->setBuildBtnText(false);
            m_bridge->creatDesktop();
            m_bridge->createSetupConf();
        }

        if (false == m_historywidget->isVisible()) {
            m_historywidget->setVisible(true);
            alllayout->removeWidget(m_guideWidget);
            alllayout->addWidget(m_guideWidget);
            alllayout->addWidget(m_historywidget);
        }
    });

    QFileSystemWatcher* watcher = new QFileSystemWatcher(this);
    watcher->addPath(builderDir.absolutePath());
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, [=]() {
        m_historywidget->updateHistoryDir();

        QFileInfoList fileInfoList = builderDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);

        if(fileInfoList.isEmpty() || (fileInfoList.count() == 1 && fileInfoList.first().isFile() && fileInfoList.first().fileName() == "savepath.conf")) {
            alllayout->removeWidget(m_guideWidget);
            alllayout->removeItem(alllayout->itemAt(alllayout->count()-1));
            m_historywidget->setVisible(false);
            alllayout->addStretch(1);
            alllayout->addWidget(m_guideWidget);
            alllayout->addStretch(1);
        }
    });
}

/**
 * @brief 初始化入口
 *
 * 初始化各个按钮的入口，并设置相应的图标和提示信息。
 */
void MainWindow::initEntry()
{
    m_globalbtn = new EntryButton();
    m_iconbtn = new EntryButton();
    m_cursorbtn = new EntryButton();
    m_plymouthbtn = new EntryButton();
    m_grubbtn = new EntryButton();

    QIcon icon = QIcon(QPixmap(":/resource/background/homepage-global.png"));
    m_globalbtn->setEntry(icon,tr("Global Theme"));
    m_globalbtn->setTip(tr("Topics include basic styles for windows and controls, icons, cursors, and more!"));
    QIcon icon1 = QIcon(QPixmap(":/resource/background/homepage-icons.png"));
    m_iconbtn->setEntry(icon1,tr("Icon Theme"));
    m_iconbtn->setTip(tr("The icon theme includes multiple application icons and system settings homepage icons."));
    QIcon icon2 = QIcon(QPixmap(":/resource/background/homepage-cursor.png"));
    m_cursorbtn->setEntry(icon2,tr("Cursor Theme"));
    m_cursorbtn->setTip(tr("Cursor theme for pointer cursor icon"));
    QIcon icon3 = QIcon(QPixmap(":/resource/background/homepage-plymouth.png"));
    m_plymouthbtn->setEntry(icon3,tr("Plymouth Theme"));
    m_plymouthbtn->setTip(tr("Boot animation can be customized to boot screen effect"));
    QIcon icon4 = QIcon(QPixmap(":/resource/background/homepage-grub.png"));
    m_grubbtn->setEntry(icon4,tr("GRUB Theme"));
    m_grubbtn->setTip(tr("Modify GRUB background wallpaper"));
}

void MainWindow::startShowPlymouth()
{
    m_maininterface->startShowPlymouth();
}

void MainWindow::useMaximumScale()
{
    m_maininterface->useMaximumScale();
}

void MainWindow::useRestoreScale()
{
    m_maininterface->useRestoreScale();
}

void MainWindow::refresh()
{
//    m_maininterface->refresh();
}

/**
 * @brief 点击"GoHome"按钮后执行的函数
 *
 * 当用户点击"GoHome"按钮后，执行该函数。
 */
void MainWindow::onGoHomeClicked()
{
    //返回主界面判断，是否在主界面点击返回按钮
    if (FileProcess::g_date != nullptr){
        //是否存在改动
        if(CursorThemeWidget::g_themeChange || IconThemeWidget::g_themeChange || PlymouthThemeWidget::g_themeChange ||
                GrubThemeWidget::g_themeChange || GlobalThemeWidget::g_themeChange){
            //创建询问是否保存界面
            cacheConfirmedWidget *m_cacheConfirmedWidget = new cacheConfirmedWidget();
            m_cacheConfirmedWidget->show();
//            qDebug()<<"grub"<<GrubThemeWidget::g_themeChange<<"ply"<<PlymouthThemeWidget::g_themeChange<<"icon"<<IconThemeWidget::g_themeChange
//                   <<"cursor"<<CursorThemeWidget::g_themeChange<<"global"<<GlobalThemeWidget::g_themeChange;
            //根据询问界面返回信号处理历史记录
            connect(m_cacheConfirmedWidget, &cacheConfirmedWidget::cacheClean, this, [=](){
                m_historywidget->updateHistoryDir();
                m_stackedWidget->setCurrentIndex(0);
                //返回主界面前 清除全局变量
                FileProcess::g_createThemeType = nullptr;
                FileProcess::g_date = nullptr;
                FileProcess::g_themeCNName = nullptr;
                FileProcess::g_themeENName = nullptr;
                FileProcess::g_builderName = nullptr;
                FileProcess::g_builderMailName = nullptr;
                CursorThemeWidget::g_themeChange = false;
                IconThemeWidget::g_themeChange = false;
                PlymouthThemeWidget::g_themeChange = false;
                GrubThemeWidget::g_themeChange = false;
                GlobalThemeWidget::g_themeChange = false;
                m_ishistory = false;
                m_titlebar->setGoHomeBtn(false);
            });
            connect(m_cacheConfirmedWidget, &cacheConfirmedWidget::cacheSave, this, [=](){
                m_historywidget->updateHistoryDir();
                m_stackedWidget->setCurrentIndex(0);
                FileProcess::g_createThemeType = nullptr;
                FileProcess::g_date = nullptr;
                FileProcess::g_themeCNName = nullptr;
                FileProcess::g_themeENName = nullptr;
                FileProcess::g_builderName = nullptr;
                FileProcess::g_builderMailName = nullptr;
                CursorThemeWidget::g_themeChange = false;
                IconThemeWidget::g_themeChange = false;
                PlymouthThemeWidget::g_themeChange = false;
                GrubThemeWidget::g_themeChange = false;
                GlobalThemeWidget::g_themeChange = false;
                m_ishistory = false;
                m_titlebar->setGoHomeBtn(false);
            });
            connect(m_cacheConfirmedWidget, &cacheConfirmedWidget::cacheCancel, this, [=](){
                m_titlebar->setGoHomeBtn(true);
            });
            connect(m_cacheConfirmedWidget, &cacheConfirmedWidget::visibilityChanged, this, [=](bool visible) {
                if (!visible) {
                    if(m_stackedWidget->currentIndex() == 0){
                        m_titlebar->setGoHomeBtn(false);
                    } else {
                        m_titlebar->setGoHomeBtn(true);
                    }
                } else {
                    qDebug() << "m_cacheConfirmedWidget is visible";
                }
            });
        } else {
            if(!m_ishistory){
            //无修改时，删除时间戳目录
                FileProcess m_fileClean;
                m_fileClean.FileClean(FileProcess::g_date);
            }
            FileProcess::g_createThemeType = nullptr;
            FileProcess::g_date = nullptr;
            FileProcess::g_themeCNName = nullptr;
            FileProcess::g_themeENName = nullptr;
            FileProcess::g_builderName = nullptr;
            FileProcess::g_builderMailName = nullptr;
            CursorThemeWidget::g_themeChange = false;
            IconThemeWidget::g_themeChange = false;
            PlymouthThemeWidget::g_themeChange = false;
            GrubThemeWidget::g_themeChange = false;
            GlobalThemeWidget::g_themeChange = false;
            m_ishistory = false;
            m_stackedWidget->setCurrentIndex(0);
        }
        m_titlebar->setGoHomeBtn(false);
    } else {
        m_stackedWidget->setCurrentIndex(0);
    }

}

/**
 * @brief 设置入口按钮的图标和文本
 *
 * 设置入口按钮的图标和文本。
 *
 * @param icon QIcon 对象，表示要设置的图标
 * @param text QString 对象，表示要设置的文本
 */
void EntryButton::setEntry(const QIcon &icon, const QString &text)
{
    QFont font;
    font.setBold(true);
    textLabel->setFont(font);

    iconLabel->setPixmap(icon.pixmap(320, 320));
    textLabel->setText(text);
    textLabel->setAlignment(Qt::AlignCenter);
    font.setPointSize(18);
    textLabel->setFont(font);
}

/**
 * @brief 设置提示文字
 *
 * 设置 EntryButton 控件的提示文字。
 *
 * @param text 提示文字
 */
void EntryButton::setTip(const QString &text)
{
    tipLabel->setAlignment(Qt::AlignHCenter);
    tipLabel->setContentsMargins(QMargins(17,0,17,4));
    tipLabel->setText(text);
    QFont font;
    font.setPointSize(11);
    tipLabel->setFont(font);
    tipLabel->setText(text);
    tipLabel->setWordWrap(true);
}

/**
 * @brief 构造函数
 *
 * 创建一个 EntryButton 对象，继承自 QPushButton。
 *
 * @param parent 父对象指针
 */
EntryButton::EntryButton(QWidget *parent): QPushButton(parent)
{
    this->setFixedSize(240,280);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    iconLabel = new QLabel;
    textLabel = new QLabel;
    tipLabel = new QLabel;

    layout->addWidget(iconLabel);
    layout->addWidget(textLabel);
    layout->addWidget(tipLabel);
}

