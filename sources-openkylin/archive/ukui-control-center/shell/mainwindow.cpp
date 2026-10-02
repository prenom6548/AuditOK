/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "subplugininterface.h"

#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QPluginLoader>
#include <QPainter>
#include <QPainterPath>
#include <QProcess>
#include <QDebug>
#include <QMessageBox>
#include <QGSettings>
#include <QMenu>
#include <QShortcut>
#include <QMouseEvent>
#include <QScrollBar>
#include <KWindowSystem>
#include <QFuture>
#include <QScreen>
#include <QWindow>
#include <QApplication>

#include <kysdk/kysdk-system/libkypackages.h>
#include <kysdk/applications/accessinfohelper.h>

#include "lightlabel.h"
#include "../../shell/customstyle.h"
#include "utils/modulefactory.h"
#include "utils/functionselect.h"
#include "interface/ukcccommon.h"
#include <QFuture>
#include <QScreen>
#include "libkypackages.h"
#include "prescene.h"
#include <QWindow>
using namespace ukcc;

#define THEME_QT_SCHEMA "org.ukui.style"

#ifdef WITHKYSEC
#include <kysec/libkysec.h>
#include <kysec/status.h>
#endif

const QByteArray kVinoSchemas    = "org.gnome.Vino";
const int        kPreSceneTime   = 10;
const int        kMIN_HIGHT_REDUCE = 50;

#define KYLIN_USER_GUIDE_PATH              "/"
#define KYLIN_USER_GUIDE_SERVICE           "com.kylinUserGuide.hotel"
#define KYLIN_USER_GUIDE_INTERFACE         "com.guide.hotel"
#define UKCC_SCHEMA        "org.ukui.control-center"
#define FONT_SELECT_FILE     "fontSelectFile"

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
}

const int dbWitdth = 50;
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    // 值初始化：Ui 无构造函数，避免 setupUi 前 stackedWidget 等为野指针
    ui(new Ui::MainWindow()),
    m_searchWidget(nullptr)
{
    preInit();
    initWindowSize();

    isLoad = false;
    loadWidget = false;
    m_presceneShown = false;
}

MainWindow::~MainWindow()
{
    //执行插件的析构函数
    ModulesFactory::pluginDelete();
    qInfo()<<"~exit ukui-control-center";
    delete ui;
    ui = nullptr;
}

void MainWindow::initPlugins()
{
    qApp->setStyle(new InternalStyle("ukui"));
    pluginPathList = FunctionSelect::listExistsCustomNoticePath(PLUGINS_PATH);
    connect(WindowManager::self(),&WindowManager::windowAdded,this,[=](const WindowId& windowId){
        if (getpid() == WindowManager::getPid(windowId)) {
            m_listWinIds.append(windowId);
        }
    });
    connect(WindowManager::self(),&WindowManager::windowRemoved,this,[=](const WindowId& windowId){
        if (m_listWinIds.contains(windowId)) {
            m_listWinIds.removeOne(windowId);
        }
    });


    m_prescene = new PreScene(m_windowSize);
    double msecs = abs(QTime::currentTime().msecsTo(startTime)); // 获取毫秒差
    qDebug() << "Difference in seconds:" << msecs << "ms" << QTime::currentTime();

    this->setCentralWidget(m_prescene);

    // 先显示主窗口，否则子控件无法显示
    this->show();

    m_prescene->raise();

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [=]() {
        closePrescene();
        timer->stop();
    });
    timer->start(kPreSceneTime);
    // 初始化防抖定时器
}

void MainWindow::closePrescene()
{
    // setupUi 会 setCentralWidget 并 delete 旧中央部件；须先摘掉预览页，
    // 否则 m_prescene 悬空，后续 hide/二次打开会写坏堆，changeEvent 里 currentIndex 偶现崩。
    if (m_prescene) {
        if (centralWidget() == m_prescene) {
            takeCentralWidget();
        }
        delete m_prescene;
        m_prescene = nullptr;
        m_presceneShown = false;
    }

    ui->setupUi(this);
    this->resize(m_windowSize);
    m_isSetUI = true;

    qDebug() << Q_FUNC_INFO << "Closing prescene and loading modules";
    loadAllModules();

    initPageWidget(false);
}

bool MainWindow::isFontSelectFile(const QString &msg) {
    if (!msg.isEmpty() && isFontFile(msg)) {
        QFileInfo f(msg);
        if (f.isFile() && m_ukccSettings->keys().contains(FONT_SELECT_FILE)) {
            m_ukccSettings->set(FONT_SELECT_FILE, msg);
            qDebug() << Q_FUNC_INFO << "Click font file" << msg;
            return true;
        }
    }
    if (m_ukccSettings->keys().contains(FONT_SELECT_FILE)) {
        QString filePaths = m_ukccSettings->get(FONT_SELECT_FILE).toString();
        qDebug() << Q_FUNC_INFO << "isFontSelectFile" << filePaths;
        if (!filePaths.isEmpty()) {
            return true;
        } else {
            return false;
        }
    }
    return false;
}

void MainWindow::preInit()
{
    // Qt 要求 setAttribute需要放到最前面
    setAttribute(Qt::WA_AlwaysShowToolTips);
    setAttribute(Qt::WA_TranslucentBackground);

    qApp->setStyle(new InternalStyle("ukui"));
    const QByteArray id(THEME_QT_SCHEMA);
    QGSettings *mQtSettings = new QGSettings(id, QByteArray(), this);
    connect(mQtSettings, &QGSettings::changed, this, [=](QString key) {
        if (key == "styleName" || key == "themeColor" || key == "widgetThemeName") {
            qApp->setStyle(new InternalStyle("ukui"));
            refreshSidebarAfterThemeChange();
        }
    });

    qApp->installEventFilter(this);
    m_searchWidget = new SearchWidget(this);
    m_searchWidget->setHidden(true);
    connect(m_searchWidget, &SearchWidget::notifyModuleSearch, this, &MainWindow::switchPage);
    m_searchWidget->setFixedWidth(240);
    m_ModuleMap = UkccCommon::getModuleHideStatus();
    const QByteArray ukccID(UKCC_SCHEMA);
    m_ukccSettings = new QGSettings(ukccID, QByteArray(), this);

    m_pUkuiWindowHelper = new UkuiWindowHelper(this);
    // 设置毛玻璃效果 将毛玻璃效果提前，确保调用
    m_pUkuiWindowHelper->setBlurEffect(QRegion());

    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(300); // 300ms防抖时间

    // 初始化插件刷新防抖定时器
    m_pluginRefreshTimer = new QTimer(this);
    m_pluginRefreshTimer->setSingleShot(true);
    m_pluginRefreshTimer->setInterval(100);
    connect(m_pluginRefreshTimer, &QTimer::timeout, this, [=]() {
        if (m_pendingPlugin != nullptr && modulepageWidget != nullptr) {
            qDebug() << Q_FUNC_INFO << m_pendingPlugin->name();
            modulepageWidget->refreshPluginWidget(m_pendingPlugin);
            // 埋点点击左侧导航插件
            UkccCommon::buriedSettings(m_pendingPlugin->name(), nullptr, QString("left clicked"));
            m_pendingPlugin = nullptr;
        }
    });
}

void MainWindow::bootOptionsFilter(QString opt, bool firstIn)
{
    qDebug() << Q_FUNC_INFO << "bootOptionsFilter" << opt << firstIn;
    PluginInfo* pluginInfo = ModulesFactory::getPluginInfoByName(opt);
    if (pluginInfo == nullptr) {
        //避免无限循环的风险
        if (UkccCommon::isTablet()) {
            if (firstIn) {
                bootOptionsFilter("userinfointel", false);
            }
            backBtn->hide();
        }
        return ;
    }
    QString pluginName = pluginInfo->nameString;
    QString moduleName = ModulesFactory::getModuleNamebyName(pluginName);
    QObject* pluginObj = ModulesFactory::getPluginObjectByName(pluginInfo->namei18nString);
    if (pluginObj != nullptr) {
        if (m_ModuleMap.isEmpty()) {
            //开始跳转
            functionBtnClicked(pluginObj);
            return ;
        } else {
            qDebug() << "module:" << pluginName.toLower() << ";superordinate:" << moduleName;
            // 若跳转节点或跳转节点父节点不存在直接跳转，不受管控文件控制，若存在，则根据管控文件判断是否被禁用
            if (!m_ModuleMap.contains(pluginName.toLower()) || !m_ModuleMap.contains(moduleName)
                 || (m_ModuleMap[pluginName.toLower()].toBool() && m_ModuleMap[moduleName].toBool())) {
                //开始跳转
                functionBtnClicked(pluginObj);
                return ;
            }
        }

        QMessageBox::information(this, tr("Warnning"), tr("This function has been controlled"));
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (titleLabel != nullptr && m_pTextSetLabel != nullptr) {
        if (event->type() == QEvent::WindowActivate) {
            QPalette palette = this->palette();;
            palette.setBrush(QPalette::Active, QPalette::Text, ThemeController::getCustomColorFromDT("kfont-primary"));
            titleLabel->setPalette(palette);
            m_pTextSetLabel->setPalette(palette);
        } else if (event->type() == QEvent::WindowDeactivate) {
            QColor color;
            if (ThemeController::themeMode() == LightTheme) {
                color = ThemeController::getCustomColorFromDT("kfont-placeholdertext");
            } else {
                color = ThemeController::getCustomColorFromDT("kfont-secondary-disable");
            }
            QPalette palette = this->palette();;
            palette.setBrush(QPalette::Inactive, QPalette::Text, color);
            titleLabel->setPalette(palette);
            m_pTextSetLabel->setPalette(palette);
        }
    }

    if (watched == maxBtn) {
        if (event->type() == QEvent::ToolTip) {
            event->setAccepted(true);
            event->accept();
            QRect rect = maxBtn->geometry();
            auto pos = maxBtn->mapTo(this, QPoint(rect.width(), 0));
            if (isMainUiReady() && ui->stackedWidget->currentIndex() == 1) {
                rect.translate(pos.x() - rect.x() - closeBtn->width(), 0);
            }
            qDebug() << "maxBtn rect = " << rect << "maxBtn pox mapTo this = " << pos;
            m_pUkuiWindowHelper->showTileFlyout(rect);
            return true;
        }
    }

    if (watched == scrollArea) {
        if (!UkccCommon::isTablet()) {
            if (event->type() == QEvent::Enter) {
                scrollArea->verticalScrollBar()->setVisible(true);
            } else if (event->type() == QEvent::Leave) {
                scrollArea->verticalScrollBar()->setVisible(false);
            }
        }
    } else if (watched == this) {
        if (event->type() == QEvent::WindowStateChange && maxBtn != nullptr ) {
            if (this->windowState() == Qt::WindowMaximized) {
                maxBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
                maxBtn->setToolTip(tr("Restore"));
                maxBtn->setAccessibleName(tr("Restore"));
            } else {
                maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
                maxBtn->setToolTip(tr("Maximize"));
                maxBtn->setAccessibleName(tr("Maximize"));
            }
        } else if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton ){
                bool res = dblOnEdge(dynamic_cast<QMouseEvent*>(event));
                if (res) {
                    if (this->windowState() == Qt::WindowMaximized) {
                        this->showNormal();
                    } else {
                        this->showMaximized();
                    }
                }
            }
        }
    } else if (watched == homepageWidget) {
        if (event->type() == QEvent::Paint) {
            QTimer::singleShot(1, this, [=]() {
                if (!UkccCommon::isTablet()) {
                    QString language = QLocale::system().name();
                    // 维吾尔 哈萨克斯 柯尔克孜 语言控件翻转
                    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
                        m_searchWidget->setFixedWidth(240 > titleLabel->x() - mOptionBtn->x() - mOptionBtn->width() ? (titleLabel->x() - m_searchWidget->x() - 16) : 240);
                    } else {
                        m_searchWidget->setFixedWidth(240 > mOptionBtn->x() - titleLabel->x() - titleLabel->width() ? (mOptionBtn->x() - m_searchWidget->x() - 16) : 240);
                    }
                }
            });
        }
    }
    if (scrollArea && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            QWidget *w = qobject_cast<QWidget *>(watched);
            if (w && (w == scrollArea || scrollArea->isAncestorOf(w))) {
                m_sidebarSkipClick = false;
            }
        }
    }
    if (scrollArea && watched == scrollArea->listview() && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        const int key = keyEvent->key();
        switch (key) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Home:
        case Qt::Key_End:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            m_sidebarSkipClick = true;
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Space:
            // 与鼠标点击一致：进插件后焦点仍留在侧栏对应项，不抢到插件内容
            m_sidebarSkipClick = true;
            navigateSidebarPlugin(scrollArea->listview()->currentIndex());
            break;
        default:
            break;
        }
    }

    return QObject::eventFilter(watched, event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    mFirstKey = 0;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1) {
        if (!event->isAutoRepeat()) //避免重复触发该事件
            onF1ButtonClicked();
    }

    if (mFirstKey == 0 && event->key() == Qt::Key_Control) {
        mFirstKey = Qt::Key_Control;
    } else if (mFirstKey == Qt::Key_Control && event->key() == Qt::Key_E) {
#ifdef Nile
        m_searchWidget->setFocus(Qt::ShortcutFocusReason);
#endif
    } else {
        mFirstKey = 0;
    }
}

void MainWindow::loadAllModules() {
    ModulesFactory::loadConfig();
    //加载插件
    loadPlugins();
    loadSearchXml();
}

void MainWindow::initPageWidgetSlot() {
    if (loadWidget) {
        qDebug() << Q_FUNC_INFO << "Page widget already initialized, skipping...";
        return;
    }

    qDebug() << Q_FUNC_INFO << "Starting to initialize page widget...";
    loadWidget = true;

    try {
        initUI();
        hideComponent();

        //加载首页Widget
        homepageWidget = new HomePageWidget(this, vecGsettins);
        homepageWidget->installEventFilter(this);
        ui->stackedWidget->addWidget(homepageWidget);
        ui->stackedWidget->setFocusPolicy(Qt::ClickFocus);

        m_fontInterface = new QDBusInterface("org.ukui.ukcc.session",
                                             "/Font",
                                             "org.ukui.ukcc.session.Font",
                                             QDBusConnection::sessionBus(), this);
        if (m_fontInterface->isValid()) {
            QTime time;
            time.start();
            m_fontInterface->asyncCall("updateFontList", true);
            qDebug() << Q_FUNC_INFO << "async call updateFontList" << time.elapsed();
        }
        //加载左侧边栏一级菜单
        initLeftsideBar();

        //加载功能页Widget
        modulepageWidget = new ModulePageWidget(this);
        ui->stackedWidget->addWidget(modulepageWidget);
        connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [=] (int id){
            if (id == 0) {
                modulepageWidget->mCurrentPluName =  "";
            }
        });
        connect(modulepageWidget, &ModulePageWidget::hScrollBarHide, this, [=]() {
            m_searchWidget->setFixedWidth(240);
        });
        connect(modulepageWidget, &ModulePageWidget::hScrollBarShow, this, [=]() {
            QTimer::singleShot(1, this, [=]() {
                if (!UkccCommon::isTablet()) {
                    QString language = QLocale::system().name();
                    // 维吾尔 哈萨克斯 柯尔克孜 语言控件翻转
                    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
                        if (m_searchWidget->width() > mOptionBtn->x() - backBtn->x()) {
                            m_searchWidget->setFixedWidth(backBtn->x() - mOptionBtn->x() - backBtn->width() - 16);
                        }
                    } else {
                        if (m_searchWidget->width() > mOptionBtn->x() - backBtn->x()) {
                            m_searchWidget->setFixedWidth(mOptionBtn->x() - backBtn->x() - mOptionBtn->width() - 16);
                        }
                    }
                }
            });
        });
    } catch (const std::exception& e) {
        qDebug() << Q_FUNC_INFO << "Error during page widget initialization:" << e.what();
        loadWidget = false; // 重置状态以便重试
    } catch (...) {
        qDebug() << Q_FUNC_INFO << "Unknown error during page widget initialization";
        loadWidget = false; // 重置状态以便重试
    }

    // 快捷参数

    qDebug() << QApplication::arguments().length() << QApplication::arguments();
    if (QApplication::arguments().length() > 3 && QApplication::arguments().at(1) == "-m") {
        m_secondWidgetName = QString(QApplication::arguments().at(3));
    }

    if (QApplication::arguments().length() > 2) {
        if (QApplication::arguments().at(1) == "-p") {
            QString fileFullName = "";
            QString fileName = QApplication::arguments().at(2);
            if (!fileName.startsWith("/")) { //非绝对路径
                char *path = get_current_dir_name();
                QString currentPath = QString(path);
                if (!currentPath.endsWith("/")) {
                    currentPath = currentPath + "/";
                }
                fileFullName = currentPath + fileName;
                g_free(path);
            } else {
                fileFullName = fileName;
            }
            loadSpecifiedPlugin(fileFullName);
        } else if (QApplication::arguments().at(1) == "-m" || isFontSelectFile()) {
            ui->stackedWidget->setCurrentIndex(1);
            if (isFontSelectFile()) {
                bootOptionsFilter("fonts");
            } else {
                bootOptionsFilter(QApplication::arguments().at(2));
            }
        }
    } else if (isFontSelectFile()) {
        QTimer::singleShot(2, this, [=](){  //延时保证菜单栏选中插件居中，控件未画出来，无法准确居中
            bootOptionsFilter("fonts");
        });
    }

    // 普通打开首页：无历史焦点时落到搜索框
    if (m_searchWidget && isMainUiReady()
            && ui->stackedWidget->currentIndex() == 0) {
        restoreHomepageFocus();
    }
}

void MainWindow::initPageWidget(bool isDbus)
{
    pluginWatchSlot(isDbus);
}

void MainWindow::loadSearchXml()
{
    m_searchWidget->setLanguage(QLocale::system().name());
    qDebug() << Q_FUNC_INFO << "finished loadSearchXml" << m_searchWidget->getSearchItems().count();
}

void MainWindow::initWindowSize()
{
    QRect screenSize = UkccCommon::sizeOnCursor();
    int minWidth = qMin(screenSize.width(), 978);
    int minHeight = qMin(screenSize.height(), 630);
    QScreen* screen = QGuiApplication::primaryScreen();
    qreal scaleFactor = 1.0;
    if (screen) {
        scaleFactor = screen->devicePixelRatio();
    }

    m_windowSize.setWidth(minWidth);
    if (scaleFactor > 1.0) {
        m_windowSize.setHeight(minHeight - kMIN_HIGHT_REDUCE);
        this->setMinimumSize(minWidth, minHeight - kMIN_HIGHT_REDUCE);
    } else {
        m_windowSize.setHeight(minHeight);
        this->setMinimumSize(minWidth, minHeight);
    }

    // 先设置窗口尺寸，确保 prescene 获取正确的尺寸
    if (screenSize.width() > 1440) {
        m_windowSize.setWidth(1160);
        m_windowSize.setHeight(720);
        this->resize(1160, 720);
    }
}

void MainWindow::pluginWatchSlot(bool isDbus) {
    qDebug() << Q_FUNC_INFO << "dbus request" << isDbus;
    if (!isDbus) {
        initPageWidgetSlot();
        UkccCommon::buriedSettings("", "", "Open");
    }
}

void MainWindow::pluginSettingChangedSlot(QString key, QString plugini18nName, QString pluginName, bool show) {
    Q_UNUSED(key);
    m_searchWidget->hiddenSearchItem(QLocale::system().name() == "zh_CN" ? plugini18nName : pluginName, show);
}

void MainWindow::initUI()
{
    if (!m_isSetUI) {
        // 与 closePrescene 相同：setupUi 会销毁当前中央部件，先释放预览页避免悬空
        if (m_prescene) {
            if (centralWidget() == m_prescene) {
                takeCentralWidget();
            }
            delete m_prescene;
            m_prescene = nullptr;
            m_presceneShown = false;
        }
        ui->setupUi(this);
        this->resize(m_windowSize);
        // 二次拉起等路径可能只走 initUI、不经 closePrescene，须同步就绪标记
        m_isSetUI = true;
    }

    QDBusInterface ifc("com.control.center.qt.systemdbus",
                       "/",
                       "com.control.center.interface",
                       QDBusConnection::systemBus());
    ifc.asyncCall("getDiskInfo");
    qDebug() << Q_FUNC_INFO << "async call getDiskInfo";

    const QByteArray id("org.ukui.style");
    m_fontSetting = new QGSettings(id, QByteArray(), this);
    connect(m_fontSetting, &QGSettings::changed, this, [=](QString key) {
        if ("systemFont" == key || "systemFontSize" == key) {
            QFont font = this->font();
            int width = font.pointSize();
            for (auto widget : qApp->allWidgets()) {
                QString className(widget->metaObject()->className());
                if (widget->objectName() == "timeClockLable") {
                    QFont fontTime;
                    fontTime.setWeight(QFont::Medium);
                    fontTime.setBold(true);
                    if (QLocale::system().amText() == "སྔ་དྲོ་") {
                        fontTime.setPixelSize(m_fontSetting->get("systemFontSize").toInt() * 20 / 11);
                    } else {
                        fontTime.setPixelSize(font.pointSize() * 28 / 11);
                    }
                    widget->setFont(fontTime);
                } else if(widget->objectName() == "DateTime_Info") {
                    QFont fontTitle;
                    fontTitle.setPixelSize(font.pointSize() * 18 / 11);
                    fontTitle.setWeight(QFont::Medium);
                    widget->setFont(fontTitle);
                }
            }
        }
    });

    initTileBar();
    initStyleSheet();

    connect(minBtn, SIGNAL(clicked()), this, SLOT(showMinimized()));
    connect(maxBtn, &QPushButton::clicked, this, [=] {
        if (isMaximized()) {
            showNormal();
            maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
            maxBtn->setToolTip(tr("Maximize"));
            maxBtn->setAccessibleName(tr("Maximize"));
        } else {
            showMaximized();
            maxBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
            maxBtn->setToolTip(tr("Restore"));
            maxBtn->setAccessibleName(tr("Restore"));
        }
    });
    connect(closeBtn, &QPushButton::clicked, this, [=] {
        qApp->quit();
    });

    connect(ui->stackedWidget, &QStackedWidget::currentChanged, this, [=](int index){
        ui->centralWidget->setVisible(false);    //避免出现明显的卡顿现象，在选择进入屏保界面之后这个问题比较明显，这种做法只是优化
        if (index){ //次页部分组件样式
            titleLabel->setHidden(true);
            mTitleIcon->setHidden(true);
              ui->leftsidebarWidget->setVisible(true);
            //左上角显示字符/返回按钮
            if (!m_tabletmode) {
                backBtn->setVisible(true);
            }
            ui->stackedWidget->status = false;
            ui->titleWidget->status = false;
        } else { //首页部分组件样式
            //左侧边栏显示/不显示
            ui->leftsidebarWidget->setHidden(true);
            titleLabel->setVisible(true);
            mTitleIcon->setVisible(true);
            //左上角显示字符/返回按钮
            backBtn->setHidden(true);
            if (modulepageWidget) {
                modulepageWidget->pluginLeave();
            }
            ui->stackedWidget->status = true;
            ui->titleWidget->status = true;
            restoreHomepageFocus();
        }
        ui->centralWidget->setVisible(true);
    });

    //top left return button
    connect(backBtn, &QPushButton::clicked, this, [=]{
        ui->stackedWidget->setCurrentIndex(0);
        UkccCommon::buriedSettings(backBtn->objectName(), nullptr, QString("clicked"));
    });

    //监听平板模式切换
    m_statusSessionDbus = new QDBusInterface("com.kylin.statusmanager.interface",
                                              "/",
                                              "com.kylin.statusmanager.interface",
                                              QDBusConnection::sessionBus(),this);
    if (m_statusSessionDbus->isValid()) {
        QDBusReply<bool> tabletmode = m_statusSessionDbus->call("get_current_tabletmode");
        m_tabletmode = tabletmode;
        connect(m_statusSessionDbus, SIGNAL(mode_change_signal(bool)), this, SLOT(mainWindow_statusDbusSlot(bool)));

    } else {
        qDebug() << "Create statusmanager Interface Failed When : " << QDBusConnection::systemBus().lastError();
    }

    mainWindow_statusDbusSlot(m_tabletmode);
}

void MainWindow::initTileBar() {

    mLogoSetLabel  = new QLabel(this);
    m_pTextSetLabel  = new QLabel(this);
    ui->leftTopWidget->layout()->addWidget(mLogoSetLabel);
    ui->leftTopWidget->layout()->addWidget(m_pTextSetLabel);
    mLogoSetLabel->setFixedSize(24,24);
    mLogoSetLabel->setPixmap(QPixmap::fromImage(QIcon::fromTheme("ukui-control-center").pixmap(24,24).toImage()));
    const QByteArray id(THEME_QT_SCHEMA);
    QGSettings *mQtSettings = new QGSettings(id, QByteArray(), this);

    m_pTextSetLabel->setText(tr("Settings"));
    m_pTextSetLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    titleLayout = new QHBoxLayout(ui->titleWidget);
    ui->titleWidget->setLayout(titleLayout);
    ui->titleWidget->setObjectName("titleWidget");
    titleLayout->setContentsMargins(8, 2, 4, 2);
    titleLayout->setSpacing(0);
    titleLayout->addWidget(m_searchWidget,Qt::AlignCenter);

    m_searchWidget->setHidden(false);

    backBtn     = new QPushButton(this);
    backBtn->setObjectName("go-home");
    backBtn->setToolTip(tr("Back Home"));
    mOptionBtn  = new KMenuButton(this);
    minBtn      = new QPushButton(this);
    maxBtn      = new QPushButton(this);
    closeBtn    = new QPushButton(this);
    mTitleIcon  = new QLabel(this);
    titleLabel  = new QLabel(this);
    titleLabel->setText(tr("Settings"));
    titleLabel->installEventFilter(this);

    backBtn->setFixedSize(36, 36);
    mOptionBtn->setFixedSize(30, 30);
    if (UkccCommon::isTablet()) {
        minBtn->setFixedSize(48, 48);
        maxBtn->setFixedSize(48, 48);
        closeBtn->setFixedSize(48, 48);
        ui->titleWidget->setFixedHeight(48);
        titleLayout->setContentsMargins(8, 0, 0, 0);
        mOptionBtn->hide();
    } else {
        minBtn->setFixedSize(30, 30);
        maxBtn->setFixedSize(30, 30);
        closeBtn->setFixedSize(30, 30);
        mOptionBtn->show();
    }

    mTitleIcon->setFixedSize(24, 24);

    mOptionBtn->setToolTip(tr("Option"));
    minBtn->setToolTip(tr("Minimize"));
    maxBtn->setToolTip(tr("Maximize"));
    closeBtn->setToolTip(tr("Close"));
    m_titleIcon = QIcon::fromTheme("ukui-control-center");
    mTitleIcon->setPixmap(m_titleIcon.pixmap(m_titleIcon.actualSize(QSize(24, 24))));

    connect(mQtSettings, &QGSettings::changed, this, [=](QString key) {
        if (key == "iconThemeName") {
            mLogoSetLabel->setPixmap(QPixmap::fromImage(QIcon::fromTheme("ukui-control-center").pixmap(24,24).toImage()));
            mTitleIcon->setPixmap(QPixmap::fromImage(QIcon::fromTheme("ukui-control-center").pixmap(24,24).toImage()));
        }
        if (key == "styleName" || key == "themeColor" || key == "widgetThemeName") {
            if (isActiveWindow()) {
                QPalette palette = titleLabel->palette();
                palette.setBrush(QPalette::Active, QPalette::Text, ThemeController::getCustomColorFromDT("kfont-primary"));
                titleLabel->setPalette(palette);
                m_pTextSetLabel->setPalette(palette);
            } else {
                QColor color;
                if (ThemeController::themeMode() == LightTheme) {
                    color = ThemeController::getCustomColorFromDT("kfont-placeholdertext");
                } else {
                    color = ThemeController::getCustomColorFromDT("kfont-secondary-disable");
                }
                QPalette palette = titleLabel->palette();
                palette.setBrush(QPalette::Inactive, QPalette::Text, color);
                titleLabel->setPalette(palette);
                m_pTextSetLabel->setPalette(palette);
            }
            qDebug() << Q_FUNC_INFO << "ukui style key changed" << key << "set style";
        }
    });

    titleLayout->addWidget(mTitleIcon);
    titleLayout->addSpacing(8);
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(backBtn);
    titleLayout->addStretch();
    titleLayout->addWidget(m_searchWidget);
    titleLayout->addStretch();
    titleLayout->addWidget(mOptionBtn);
    titleLayout->addSpacing(4);
    titleLayout->addWidget(minBtn);
    titleLayout->addSpacing(4);
    titleLayout->addWidget(maxBtn);
    titleLayout->addSpacing(4);
    titleLayout->addWidget(closeBtn);

    KDK_ALL_INFO_FORMAT(mLogoSetLabel, "");
    KDK_ALL_INFO_FORMAT(m_pTextSetLabel, "");
    KDK_ALL_INFO_FORMAT(mTitleIcon, "");
    KDK_ALL_INFO_FORMAT(titleLabel, "");

    // 标题栏图标按钮与搜索框不用 KDK_ALL_INFO_FORMAT：该宏会把 accessibleName 写成
    // ukui-control-center_QPushButton_xxx 英文机器名。SDK 无「仅设 accessibleName」宏，
    // 直接用 Qt setAccessibleName + tr()（zh_CN.ts 已有翻译）。搜索框在 SearchWidget 中设置。
    backBtn->setAccessibleName(tr("Back Home"));
    mOptionBtn->setAccessibleName(tr("Option"));
    minBtn->setAccessibleName(tr("Minimize"));
    maxBtn->setAccessibleName(isMaximized() ? tr("Restore") : tr("Maximize"));
    closeBtn->setAccessibleName(tr("Close"));

    // 搜索框在 preInit 中较早创建，默认 Tab 链在返回按钮之前，需与视觉顺序对齐
    // 有内容时清空按钮可见且可 Tab：搜索 → 清空 → 选项（隐藏时 Qt 会跳过）
    QWidget::setTabOrder(backBtn, m_searchWidget);
    if (QWidget *clearBtn = m_searchWidget->clearButton()) {
        QWidget::setTabOrder(m_searchWidget, clearBtn);
        QWidget::setTabOrder(clearBtn, mOptionBtn);
    } else {
        QWidget::setTabOrder(m_searchWidget, mOptionBtn);
    }
    QWidget::setTabOrder(mOptionBtn, minBtn);
    QWidget::setTabOrder(minBtn, maxBtn);
    QWidget::setTabOrder(maxBtn, closeBtn);

    initUkccAbout();
}

void MainWindow::mainWindow_statusDbusSlot(bool tablet_mode)
{
    m_tabletmode =  tablet_mode;
    if (tablet_mode) {

        ui->leftTopWidget->layout()->setContentsMargins(16, 0, 0, 0);
        ui->leftTopWidget->setFixedHeight(64);
        ui->titleWidget->setFixedHeight(64);
        mLogoSetLabel->setFixedSize(32,32);
        mLogoSetLabel->setPixmap(QPixmap::fromImage(QIcon::fromTheme("ukui-control-center").pixmap(32,32).toImage()));

        mTitleIcon->setFixedSize(32, 32);
        mTitleIcon->setPixmap(m_titleIcon.pixmap(m_titleIcon.actualSize(QSize(32, 32))));

        titleLayout->setContentsMargins(16, 0, 8, 0);

        if (ui->stackedWidget->currentIndex() == -1 || ui->stackedWidget->currentIndex() == 0) {
            this->bootOptionsFilter("userinfo", true);
        }
        backBtn->hide();

        mOptionBtn->setFixedSize(48, 48);
        minBtn->setFixedSize(48, 48);
        maxBtn->hide();
        closeBtn->setFixedSize(48, 48);

        m_searchWidget->setFixedHeight(40);
    } else {
        ui->leftTopWidget->layout()->setContentsMargins(8, 0, 0, 0);
        ui->leftTopWidget->setFixedHeight(40);
        ui->titleWidget->setFixedHeight(40);
        mLogoSetLabel->setFixedSize(24,24);
        mLogoSetLabel->setPixmap(QPixmap::fromImage(QIcon::fromTheme("ukui-control-center").pixmap(24,24).toImage()));

        mTitleIcon->setFixedSize(24, 24);
        mTitleIcon->setPixmap(m_titleIcon.pixmap(m_titleIcon.actualSize(QSize(24, 24))));

        titleLayout->setContentsMargins(8, 2, 5, 2);
        if (ui->stackedWidget->currentIndex() == 1) {
            backBtn->show();
        }

        mOptionBtn->setFixedSize(30, 30);
        minBtn->setFixedSize(30, 30);
        maxBtn->show();
        closeBtn->setFixedSize(30, 30);

        m_searchWidget->setFixedHeight(36);
    }
    emit tabletModeChanged(tablet_mode);
    qApp->setStyle(new InternalStyle("ukui"));
}

void MainWindow::onF1ButtonClicked() {
    QString pluName = "ukui-control-center";
    QString curPluName;
    if (modulepageWidget != nullptr) {
        curPluName = modulepageWidget->mCurrentPluName;
    }

    if (!curPluName.isEmpty()) {
        if (curPluName == "Autostart") {
            curPluName = "Autoboot";
        }
        pluName = QString("%1%2%3").arg(pluName).arg("/").arg(curPluName);
    }
    qDebug()<<pluName;
    showGuide(pluName);
}

void MainWindow::initUkccAbout()
{
    mOptionBtn->setPopupMode(QToolButton::InstantPopup);

    QAction* ukccHelp  = mOptionBtn->assistAction();
    QAction* ukccAbout = mOptionBtn->aboutAction();
    QAction* ukccExit  = mOptionBtn->quitAction();

    mOptionBtn->themeAction()->setVisible(false);
    mOptionBtn->settingAction()->setVisible(false);

    connect(ukccExit, SIGNAL(triggered()), this, SLOT(close()));

    connect(ukccAbout, &QAction::triggered, this, [=] {
        KAboutDialog *ukcc = new KAboutDialog(this);
        ukcc->setAppIcon(QIcon::fromTheme("ukui-control-center"));
        ukcc->setAppName(tr("Settings"));
        ukcc->setAppVersion(tr("Version: ") + UkccCommon::getUkccVersion());
        ukcc->setAppSupport(UkccCommon::isOpenkylin() ? "" : ukcc->appSupport());
        ukcc->exec();
    });

     connect(ukccHelp, &QAction::triggered, this, &MainWindow::onF1ButtonClicked);
}
void MainWindow::setBtnLayout(QPushButton * &pBtn) {
    QLabel * imgLabel = new QLabel(pBtn);
    QSizePolicy imgLabelPolicy = imgLabel->sizePolicy();
    imgLabelPolicy.setHorizontalPolicy(QSizePolicy::Fixed);
    imgLabelPolicy.setVerticalPolicy(QSizePolicy::Fixed);
    imgLabel->setSizePolicy(imgLabelPolicy);
    imgLabel->setScaledContents(true);

    QVBoxLayout * baseVerLayout = new QVBoxLayout(pBtn);

    QHBoxLayout * contentHorLayout = new QHBoxLayout();
    contentHorLayout->addStretch();
    contentHorLayout->addWidget(imgLabel);
    contentHorLayout->addStretch();

    baseVerLayout->addStretch();
    baseVerLayout->addLayout(contentHorLayout);
    baseVerLayout->addStretch();

    pBtn->setLayout(baseVerLayout);
}

void MainWindow::loadPlugins()
{
    if (isLoad) {
        qDebug() << Q_FUNC_INFO << "Plugins already loaded, skipping";
        return;
    }

    qDebug() << Q_FUNC_INFO << "Starting to load plugins";
    isLoad = true;

    bool installed = (QCoreApplication::applicationDirPath() == QDir(("/usr/bin")).canonicalPath());
    if (installed) {
        pluginsDir = QDir(PLUGIN_INSTALL_DIRS);
    } else {
        pluginsDir = QDir(qApp->applicationDirPath() + "/plugins");
    }
#ifdef WITHKYSEC
    if (!kysec_is_disabled() && kysec_get_3adm_status() && (getuid() || geteuid())) {
        m_isOpen3adm = true;
        qDebug() << Q_FUNC_INFO << "open 3adm kysec_get_3adm_status" << kysec_get_3adm_status() << "getuid" << getuid() << "geteuid" << geteuid();
    }
#endif

    m_ukccPlugins.clear();
    char** pluginPaths = kdk_package_get_standard_path("INSTALL_LIB_PATH");
    qDebug() << Q_FUNC_INFO << "loadPlugins installed" << installed;
    if (installed && pluginPaths != nullptr) {
        for (int idx = 0; pluginPaths[idx] != nullptr; idx++) {
            qDebug() << "kdk_package_get_standard_path path index" << idx << "plugin path" << pluginPaths[idx];
            QString ukccPluginPath = QString(pluginPaths[idx]) + "/ukui-control-center";
            QDir ukccPluginDir = QDir(ukccPluginPath);
            if (!ukccPluginDir.exists()) {
                qDebug() << Q_FUNC_INFO << "ukcc plugin path not exist, skip" << ukccPluginPath;
                continue;
            }
            foreach (QString fileName, ukccPluginDir.entryList(QDir::Files)) {
                if (m_ukccPlugins.contains(fileName)) {
                    qDebug() << Q_FUNC_INFO << "loadPlugins load plugin" << fileName << "already, skip";
                    continue;
                }
                m_ukccPlugins.append(fileName);
                qDebug() << Q_FUNC_INFO << "loadPlugins fileName" << fileName << "directory" << ukccPluginPath;
                determinePlugin(fileName, ukccPluginDir);
            }
        }
    }

    if (m_ukccPlugins.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "loadPlugins pluginsDir" << pluginsDir;
        if (!UkccCommon::isV11()) {
            loadUpdatePlugins();
        }

        foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
            if (!m_updatePlugins.contains(fileName)) {
                determinePlugin(fileName, pluginsDir);
            }
        }
    }

    qDebug() << Q_FUNC_INFO << "Plugin loading completed. Total plugins loaded:" << m_pluginList.size();
}

void MainWindow::loadUpdatePlugins()
{
    updatePluginDir = QDir(PLUGIN_INSTALL_UPDATE);
    foreach (QString fileName, updatePluginDir.entryList(QDir::Files)) {
        m_updatePlugins.append(fileName);
        determinePlugin(fileName, updatePluginDir);
    }
}

void MainWindow::determinePlugin(const QString &fileName, const QDir &dir)
{
    QString pluginPath = dir.absoluteFilePath(fileName);
    //三权分立开启
#ifdef WITHKYSEC
    if (m_isOpen3adm){
        //时间和日期 | 用户帐户 | 电源管理 |网络连接 |网络代理
        if (fileName.contains("datetime") || fileName.contains("userinfo") || fileName.contains("power") || \
                fileName.contains("netconnect") || fileName.contains("proxy") || fileName.contains("update") || \
                fileName.contains("upgrade") || fileName.contains("backup") || fileName.contains("vino") ||
                fileName.contains("printer") || fileName.contains("bluetooth", Qt::CaseInsensitive) || fileName.contains("mobilehotspot", Qt::CaseInsensitive) ||
                fileName.contains("vpn") || fileName.contains("autoboot") || fileName.contains("wlanconnect") ||
                fileName.contains("projection") || fileName.contains("area")) {
            qDebug() << Q_FUNC_INFO << "open 3adm skip load plugin" << fileName;
            return;
        }
    }
#endif

    if (!fileName.endsWith(".so")) {
        qDebug() << Q_FUNC_INFO << "file is not plugin" << fileName << pluginPath;
        return;
    }

    QTime stTime = QTime::currentTime();

    QPluginLoader loader(pluginPath);
    QObject * plugin = loader.instance();
    double msecs = abs(QTime::currentTime().msecsTo(stTime)); // 获取毫秒差
    if (plugin) {
        CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
        // 插件是否启用
        if (!pluginInstance || !fileName.endsWith("so")) {
            return;
        }
        qDebug() << "Load Plugin : " << pluginInstance->name() << "isEnable:" << pluginInstance->isEnable() << msecs;

        m_pluginList.append(pluginInstance);

        QGSettings *pluginSettings = setGsettingsPath(pluginPathList, pluginInstance->name(), pluginInstance->isEnable());
        vecGsettins.insert(pluginInstance->name(), pluginSettings);
        qDebug() << Q_FUNC_INFO << "pluginWatchSlot add vecGsettins" << pluginInstance->name() << vecGsettins.size();
        QString plugini18nName = pluginInstance->plugini18nName();
        QString pluginName = pluginInstance->name();
        bool show = pluginSettings->get(SHOW_KEY).toBool();
        connect(pluginSettings, &QGSettings::changed, [=, plugini18nName, pluginName, show](QString key) {
            qDebug() << Q_FUNC_INFO << "pluginName" << pluginName << "gsetting show change" << show;
            pluginSettingChangedSlot(key, plugini18nName, pluginName, show);
        });
        ModulesFactory::loadPluginInfo(plugin);
        if (pluginInstance->isEnable() && pluginSettings != nullptr && pluginSettings->get(SHOW_KEY).toBool() &&
                (!m_ModuleMap.contains(pluginInstance->name().toLower()) ||
                 m_ModuleMap[pluginInstance->name().toLower()].toBool())) {
            m_searchWidget->addModulesName(pluginInstance->name(), pluginInstance->plugini18nName(), pluginInstance->translationPath());
            registerSubPluginSearchPaths(pluginInstance);
        }
    } else {
        //如果加载错误且文件后缀为so，输出错误
        if (fileName.endsWith(".so"))
            qDebug() << fileName << "Load Failed: " << loader.errorString() << "\n";
    }
}

QString MainWindow::subPluginDirFor(const QString &l1Name) const
{
    return QDir(QStringLiteral(SUB_PLUGIN_INSTALL_DIRS))
        .filePath(l1Name.toLower() + QStringLiteral("-plugin"));
}

void MainWindow::registerSubPluginSearchPaths(CommonInterface *l1)
{
    if (!l1 || !m_searchWidget) {
        return;
    }

    const QString subDir = subPluginDirFor(l1->name());
    QDir dir(subDir);
    if (!dir.exists()) {
        qDebug() << Q_FUNC_INFO << "sub-plugin directory not found:" << subDir;
        return;
    }

    const QFileInfoList plugins = dir.entryInfoList({QStringLiteral("*.so")}, QDir::Files);
    for (const QFileInfo &fi : plugins) {
        auto *loader = new QPluginLoader(fi.absoluteFilePath(), this);
        QObject *obj = loader->instance();
        if (!obj) {
            qWarning() << Q_FUNC_INFO << "Failed to load sub-plugin for search:"
                       << fi.absoluteFilePath() << loader->errorString();
            loader->deleteLater();
            continue;
        }

        auto *sub = qobject_cast<SubPluginInterface *>(obj);
        if (!sub || !sub->isEnable()) {
            loader->unload();
            loader->deleteLater();
            continue;
        }

        const QString tsPath = QStringLiteral("/usr/share/%1-plugin/i18n/%2/%3.ts")
                                   .arg(l1->name().toLower(), sub->name(), QStringLiteral("%1"));

        m_searchWidget->addTranslationPath(tsPath);
        m_subPluginLoaders.append(loader);
        qDebug() << Q_FUNC_INFO << l1->name() << sub->name() << "search ts" << tsPath;
    }
}

void MainWindow::loadSpecifiedPlugin(QString pluginFullName)
{
    QPluginLoader loader(pluginFullName);
    QObject *plugin = loader.instance();
    if (plugin) {
        CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
        // 插件是否启用
        if (!pluginInstance || !pluginInstance->isEnable() || !pluginFullName.endsWith("so")) {
            qDebug()<<"plugin " <<pluginFullName<<" unavailable.";
            plugin->deleteLater();
            return;
        }
        Q_EMIT specifiedPluginLoaded(pluginInstance);
    } else {
        //如果加载错误且文件后缀为so，输出错误
        if (pluginFullName.endsWith(".so"))
            qDebug() << pluginFullName << "Load Failed: " << loader.errorString();
    }
}

void MainWindow::initLeftsideBar()
{
    scrollArea = new KNavigationBar(ui->leftBotWidget);

    connect(this,&MainWindow::specifiedPluginLoaded,this,[=](CommonInterface * pluginInstance){
        int type = ModulesFactory::size();
        QString typeName = tr("Specified");

        QStandardItem *specifiedPlugin = new QStandardItem(pluginInstance->icon(), pluginInstance->plugini18nName());
        QList<QStandardItem *> secondList;
        secondList << specifiedPlugin;
        specifiedPlugin->setData(type, Qt::UserRole+1);
        specifiedPlugin->setData(pluginInstance->plugini18nName(), Qt::UserRole+2);
        scrollArea->addGroupItems(secondList, typeName);

        pluginInstance->pluginBtn = specifiedPlugin;
    });

    int totalModule = ModulesFactory::size();
    for(int index = 0; index < totalModule; index++) {
        //循环构建左侧边栏一级菜单按钮
        ModuleInfo *curModuleInfo = ModulesFactory::getModuleInfoByIndex(index);
        if (curModuleInfo == nullptr) {
            continue;
        }
        int type = curModuleInfo->moduleType;
        if (ModulesFactory::checkModuleType(type)){   // 一级菜单不会有多个
            QString mnameString = curModuleInfo->moduleName;
            const QString locale = QLocale::system().name();
            QString mnamei18nString = curModuleInfo->getModuleNameLocale(locale);
            if (m_ModuleMap.keys().contains(mnameString)) {
                if (!m_ModuleMap[mnameString].toBool()) {
                    continue;
                }
            }

            QList<QStandardItem *> secondList;
            QList<PluginInfo> functionStructList = curModuleInfo->pluginInfoList;
            if (functionStructList.size() <= 0) { // 下属无插件，不显示
                continue;
            }
            int showModuleCount = 0;
            for (int funcIndex = 0; funcIndex < functionStructList.size(); funcIndex++) {
                PluginInfo single = functionStructList.at(funcIndex);

                //跳过插件不存在的功能项
                if (!ModulesFactory::checkPluginExist(single.namei18nString)) {
                    qDebug() << Q_FUNC_INFO << "initLeftsideBar checkPluginExist failed, skip" << single.nameString.toLower();
                    continue;
                }

                bool plgIsVisiable = false;
                if (m_ModuleMap.keys().contains(single.nameString.toLower())) {
                    if (!m_ModuleMap[single.nameString.toLower()].toBool()) {
                        m_searchWidget->hiddenSearchItem(single.namei18nString, false);
                        qDebug() << Q_FUNC_INFO << "initLeftsideBar ModuleMap hide plugin, skip" << single.nameString.toLower();
                        continue;
                    }
                    else plgIsVisiable = true;
                } else {
                    plgIsVisiable = true;
                }

                // intel与sp1做区分
                if ((UkccCommon::isTablet() && single.nameString == "Userinfo")
                    || (!UkccCommon::isTablet() && single.nameString == "Userinfointel")) {
                    continue;
                }

                //填充左侧菜单
                QObject* pluginObj = ModulesFactory::getPluginObjectByName(single.namei18nString);
                if (pluginObj == nullptr) {
                    continue;
                }
                CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(pluginObj);
                QStandardItem *pluginItem = new QStandardItem(pluginInstance->icon(), single.namei18nString);
                secondList << pluginItem;
                pluginItem->setData(type, Qt::UserRole+1);
                pluginItem->setData(single.namei18nString, Qt::UserRole+2);
                pluginInstance->pluginBtn = pluginItem;
                // 初始化插件状态
                QGSettings *msettings = nullptr;
                int row = scrollArea->model()->rowCount() + secondList.count();
                qDebug() << Q_FUNC_INFO << "initLeftsideBar set plugin" << single.oriNameString << "row" << row << "vecGsettins contains" << vecGsettins.contains(single.oriNameString) << "plgIsVisiable" << plgIsVisiable << vecGsettins.size();
                if (vecGsettins.contains(single.oriNameString)) {
                    msettings = vecGsettins[single.oriNameString];
                    if (msettings && plgIsVisiable) {
                        qDebug() << Q_FUNC_INFO << "initLeftsideBar plugin" << single.oriNameString << "connect QGSettings::changed";
                        connect(msettings, &QGSettings::changed,[=](QString key){
                            bool visible = msettings->get(SHOW_KEY).toBool();
                            qDebug() << msettings->get(SHOW_KEY).toBool() << single.oriNameString;
                            if (key == SHOW_KEY) {
                                if (!msettings->get(SHOW_KEY).toBool() && scrollArea->listview()->currentIndex().row() == row) {
                                    ui->stackedWidget->setCurrentIndex(0);
                                }
                                qDebug() << Q_FUNC_INFO << "plugin" << single.oriNameString  << "row" << row << "visible" << visible;
                                scrollArea->listview()->setRowHidden(row, !visible);
                                m_searchWidget->hiddenSearchItem(QLocale::system().name() == "zh_CN" ? single.namei18nString : single.oriNameString, visible);
                            }
                        });
                        if (!msettings->get(SHOW_KEY).toBool()) {
                            continue;
                        }
                    } else {
                        qDebug() << Q_FUNC_INFO << "initLeftsideBar plugin" << single.oriNameString << "not connect QGSettings::changed" << single.oriNameString;
                    }
                }
                ++showModuleCount;
            }
            // 下属无插件，不显示
            if (curModuleInfo->pluginInfoList.size() == 0 || showModuleCount == 0 || secondList.size() == 0) {
                continue;
            }
            scrollArea->addGroupItems(secondList, mnamei18nString);
            for (int funcIndex = 0; funcIndex < functionStructList.size(); funcIndex++) {
                PluginInfo single = functionStructList.at(funcIndex);
                bool gsettingShowValue = single.isEnable;
                if (vecGsettins.contains(single.oriNameString)) {
                    QGSettings *msettings = vecGsettins[single.oriNameString];
                    if (msettings) {
                        gsettingShowValue = msettings->get(SHOW_KEY).toBool();
                    }
                }
                if (!gsettingShowValue) {
                    QList<QStandardItem *> itemList = scrollArea->model()->findItems(single.namei18nString);
                    if (!itemList.isEmpty()) {
                        qDebug() << "row = " << scrollArea->model()->indexFromItem(itemList.at(0)).row() << ";" <<single.namei18nString;
                        scrollArea->listview()->setRowHidden(scrollArea->model()->indexFromItem(itemList.at(0)).row(), true);
                        m_searchWidget->hiddenSearchItem(QLocale::system().name() == "zh_CN" ? single.namei18nString : single.oriNameString, true);
                    }
                }
            }
        }
    }

    // 鼠标点击走 clicked；回车/空格键走 eventFilter；上下键、Tab 键仅切换选中项
    connect(scrollArea->listview(), &QListView::clicked, this, [=](const QModelIndex &index) {
        if (m_sidebarSkipClick) {
            m_sidebarSkipClick = false;
            return;
        }
        navigateSidebarPlugin(index);
    });
    scrollArea->listview()->installEventFilter(this);

    // KNavigationBar 继承 QScrollArea，默认 StrongFocus，会多占一次 Tab；
    // 侧栏只焦到 listview（插件项），关闭按钮后一次 Tab 即到侧栏。
    scrollArea->setFocusPolicy(Qt::NoFocus);
    if (scrollArea->viewport()) {
        scrollArea->viewport()->setFocusPolicy(Qt::NoFocus);
    }
    QListView *sidebarView = scrollArea->listview();
    if (sidebarView) {
        sidebarView->setFocusPolicy(Qt::StrongFocus);
        QWidget::setTabOrder(closeBtn, sidebarView);
    }

    ui->leftBotLayout->addWidget(scrollArea);
}

void MainWindow::navigateSidebarPlugin(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }
    qDebug() << "index = " << index.data(Qt::UserRole+2).toString();
    QString pluginil8Name = index.data(Qt::UserRole+2).toString();
    QObject* pluginObj = ModulesFactory::getPluginObjectByName(pluginil8Name);
    if (pluginObj != nullptr) {
        CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(pluginObj);
        if (pluginInstance) {
            // 使用防抖机制：停止之前的定时器，设置新的插件，重新启动定时器
            if (m_pluginRefreshTimer != nullptr) {
                m_pluginRefreshTimer->stop();
            }
            m_pendingPlugin = pluginInstance;
            if (m_pluginRefreshTimer != nullptr) {
                m_pluginRefreshTimer->start();
            }
        }
    }
}

QGSettings *MainWindow::setGsettingsPath(const QList<char *> &list, const QString &name, bool isEnable)
{
    const QByteArray id(PLUGINS_SCHEMA);
    if (!QGSettings::isSchemaInstalled(id)) {
        qWarning() << "Schema not installed:" << id;
        return nullptr;
    }

    // 构造路径，确保 QByteArray 生命周期
    const QByteArray pathBytes = (name + "/").toUtf8();
    const char *path = pathBytes.constData();

    const QByteArray pluginBytes = QString("%1%2%3").arg(PLUGINS_PATH).arg(name).arg("/").toUtf8();
    QGSettings *settings = new QGSettings(id, pluginBytes.constData(), this);

    // 检查 schema 中是否包含必需的 key
    const QStringList availableKeys = settings->keys();
    if (!availableKeys.contains("pluginName")) {
        qWarning() << "Schema missing key:" << PLUGIN_NAME << "available keys:" << availableKeys;
        delete settings;
        return nullptr;
    }

    if (!availableKeys.contains(SHOW_KEY)) {
        qWarning() << "Schema missing key:" << SHOW_KEY << "available keys:" << availableKeys;
        delete settings;
        return nullptr;
    }

    // 判断是否已存在该路径
    for (int j = 0; j < list.count(); j++) {
        if (qstrcmp(path, list.at(j)) == 0) {
            settings->set(SHOW_KEY, isEnable);  // 已检查，安全
            return settings;
        }
    }

    // 初始化新插件设置
    settings->set(PLUGIN_NAME, name);      // 已检查，安全
    settings->set(SHOW_KEY, isEnable);     // 已检查，安全
    return settings;
}

bool MainWindow::isExitsCloudAccount() {
    QProcess *wifiPro = new QProcess();
    QString shellOutput = "";
    wifiPro->start("dpkg -l  | grep kylin-sso-client");
    wifiPro->waitForFinished();
    QString output = wifiPro->readAll();
    shellOutput += output;
    QStringList slist = shellOutput.split("\n");

    for (QString res : slist) {
        if (res.contains("kylin-sso-client")) {
            return true;
        }
    }
    return false;
}

bool MainWindow::isExitWirelessDevice()
{
    QDBusInterface *interface = new QDBusInterface("com.kylin.network", "/com/kylin/network",
                                     "com.kylin.network",
                                     QDBusConnection::sessionBus());
    if (!interface->isValid()) {
        qDebug() << "/com/kylin/network is invalid";
        return false;
    }

    QDBusMessage result = interface->call(QStringLiteral("getDeviceListAndEnabled"),1);
    if(result.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "getWirelessDeviceList error:" << result.errorMessage();
        return false;
    }

    auto dbusArg =  result.arguments().at(0).value<QDBusArgument>();
    QMap<QString, bool> deviceListMap;
    dbusArg >> deviceListMap;


    if (deviceListMap.isEmpty()) {
        qDebug() << "no wireless device";
        return false;
    }
    return true;
}

bool MainWindow::dblOnEdge(QMouseEvent *event) {
    QPoint pos = event->globalPos();
    int globalMouseY = pos.y();

    int frameY = this->y();

    bool onTopEdges = (globalMouseY >= frameY &&
                       globalMouseY <= frameY + dbWitdth);
    return onTopEdges;
}

void MainWindow::initStyleSheet() {
    // 设置panel图标
    QIcon panelicon;
    if (QIcon::hasThemeIcon("ukui-control-center"))
        panelicon = QIcon::fromTheme("ukui-control-center");

    qApp->setWindowIcon(panelicon);
    this->setWindowTitle(tr("Settings"));
    ui->stackedWidget->setAutoFillBackground(true);

    // 左上角返回按钮
    backBtn->setProperty("useIconHighlightEffect", true);
    backBtn->setProperty("iconHighlightEffectMode", 1);
    backBtn->setFlat(true);

    mOptionBtn->setProperty("useIconHighlightEffect", 0x2);
    mOptionBtn->setProperty("isWindowButton", 0x01);
    mOptionBtn->setAutoRaise(true);

    minBtn->setProperty("useIconHighlightEffect", 0x2);
    minBtn->setProperty("isWindowButton", 0x01);
    minBtn->setFlat(true);

    maxBtn->setProperty("useIconHighlightEffect", 0x2);
    maxBtn->setProperty("isWindowButton", 0x1);
    maxBtn->setFlat(true);

    closeBtn->setProperty("isWindowButton", 0x02);
    closeBtn->setProperty("useIconHighlightEffect", 0x08);
    closeBtn->setFlat(true);

    ui->centralWidget->setAttribute(Qt::WA_TranslucentBackground);
    ui->leftsidebarWidget->setFixedWidth(200); //一级菜单宽度

    // 设置左上角按钮图标
    backBtn->setIcon(QIcon::fromTheme("go-home-symbolic"));
    backBtn->setIconSize(QSize(16, 16));

    // 设置右上角按钮图标
    mOptionBtn->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    minBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    maxBtn->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
}

void MainWindow::showGuide(QString pluName)
{
    QString service_name = "com.kylinUserGuide.hotel_" + QString::number(getuid());
    QDBusInterface *interface = new QDBusInterface(service_name,
                                                   KYLIN_USER_GUIDE_PATH,
                                                   KYLIN_USER_GUIDE_INTERFACE,
                                                   QDBusConnection::sessionBus(),
                                                   this);
    QDBusMessage msg = interface->call("showGuide" , pluName);
}

void MainWindow::hideComponent()
{
    QTimer::singleShot(100, this, [=]() {
        if(UkccCommon::isTablet()) {
            if (QApplication::arguments().length() < 2) {
                this->initPageWidgetSlot();
                bootOptionsFilter("userinfointel");
            }
            backBtn->hide();
        }
    });
}

void MainWindow::setModuleBtnHightLight(int id) {
    leftBtnGroup->button(id)->setChecked(true);
    leftMicBtnGroup->button(id)->setChecked(true);
}

bool MainWindow::isHomepageContextFocus(QWidget *widget) const
{
    if (!widget) {
        return false;
    }
    if (widget == m_searchWidget) {
        return true;
    }
    return homepageWidget && homepageWidget->isAncestorOf(widget);
}

bool MainWindow::isMainUiReady() const
{
    // 须等 setupUi 且首页/插件页已挂到 stackedWidget；仅判指针非空挡不住悬空/半初始化
    return m_isSetUI && loadWidget && ui && ui->stackedWidget;
}

void MainWindow::saveHomepageFocus()
{
    if (!isMainUiReady() || ui->stackedWidget->currentIndex() != 0) {
        return;
    }
    QWidget *focus = QApplication::focusWidget();
    if (!isHomepageContextFocus(focus)) {
        return;
    }
    m_homepageSavedFocus = focus;
}

void MainWindow::restoreHomepageFocus()
{
    if (!isMainUiReady() || ui->stackedWidget->currentIndex() != 0) {
        return;
    }
    // 略延后，避免与窗口激活清焦竞态
    QTimer::singleShot(50, this, [this]() {
        if (!isMainUiReady() || ui->stackedWidget->currentIndex() != 0) {
            return;
        }
        QWidget *target = m_homepageSavedFocus.data();
        if (target && isHomepageContextFocus(target) && target->isVisible()
                && target->isEnabled()
                && target->focusPolicy() != Qt::NoFocus) {
            target->setFocus(Qt::TabFocusReason);
            return;
        }
        if (m_searchWidget) {
            m_searchWidget->setFocus(Qt::TabFocusReason);
        }
    });
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ActivationChange) {
        // 任务栏/开始菜单打开：show(PreScene) 远早于页面加载；首页焦点由 initPageWidgetSlot 恢复，
        // 此处未就绪时绝不能碰 stackedWidget（含 setupUi 前后与 load 过程中的激活事件）
        if (isMainUiReady()) {
            if (isActiveWindow()) {
                if (ui->stackedWidget->currentIndex() == 0) {
                    restoreHomepageFocus();
                }
            } else if (ui->stackedWidget->currentIndex() == 0) {
                saveHomepageFocus();
            }
        }
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::pluginBtnClicked(QObject *plugin) {

    CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
    UkccCommon::buriedSettings(pluginInstance->name(), nullptr, "home clicked");

    functionBtnClicked(plugin);
}

void MainWindow::functionBtnClicked(QObject *plugin, QString jumpText)
{
    if (!scrollArea)
        return;

    saveHomepageFocus();
    ui->stackedWidget->setCurrentIndex(1);
    modulepageWidget->switchPage(plugin, true, jumpText);

    CommonInterface * pluginInstance = qobject_cast<CommonInterface *>(plugin);
    if (pluginInstance != nullptr && pluginInstance->pluginBtn != nullptr) {
        scrollArea->listview()->setCurrentIndex(pluginInstance->pluginBtn->index());
    }
    QRect size = scrollArea->listview()->visualRect(scrollArea->listview()->currentIndex());
    qDebug() << "size.y() = " <<size.y() << "size.height() = " << size.height() << "step = " << scrollArea->height() << "scrollArea->height()/2" << scrollArea->height()/2;
    int value = size.y() + size.height() - scrollArea->height();
    value = value + scrollArea->height()/2; //尽量让选中的显示在中间位置
    if (value <= 0) {
        scrollArea->verticalScrollBar()->setValue(0);
    } else if (value > scrollArea->verticalScrollBar()->maximum()){
        scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->maximum());
    } else {
        scrollArea->verticalScrollBar()->setValue(value);
    }

    // 首页/搜索进插件：焦点落在左侧栏，而不是插件内首个可 Tab 控件。
    // 部分插件在 showEvent 里会 QTimer::singleShot(0) 抢焦（如显示器预览框），
    // 同步 setFocus 会被下一拍覆盖，故先设一次再延后一拍抢回。
    // 须用 TabFocusReason：UKUI 样式下 OtherFocusReason 不画焦点框（仅有选中背景）。
    // 方向键切换当前项时焦点框随 current 移动（ListView 默认行为）。
    QListView *sidebarView = scrollArea->listview();
    sidebarView->setFocus(Qt::TabFocusReason);
    QTimer::singleShot(0, this, [this, sidebarView]() {
        if (!scrollArea || scrollArea->listview() != sidebarView) {
            return;
        }
        if (!isMainUiReady() || ui->stackedWidget->currentIndex() != 1) {
            return;
        }
        sidebarView->setFocus(Qt::TabFocusReason);
    });

    qDebug() << "m_secondWidgetName = " << m_secondWidgetName;
    if (!m_secondWidgetName.isEmpty()) {
        QTimer::singleShot(10, this, [=](){ //延时保证插件二级窗口居中，控件未画出来，无法准确居中
            UkccArgEvent *event = new UkccArgEvent(QString(m_secondWidgetName));
            QApplication::sendEvent(plugin, event);         //发送自定义事件
            m_secondWidgetName = "";
        });
    }
}

bool MainWindow::isFontFile(QString path)
{
    QStringList list = path.split(".");
    QString fontTypeTTF = "ttf";
    QString fontTypeOTF = "otf";
    QString fontTypeTTC = "ttc";
    QString fontType = list.at(list.size()-1);

    if (fontType.compare(fontTypeTTF, Qt::CaseInsensitive) == 0) {
        return true;
    } else if (fontType.compare(fontTypeOTF, Qt::CaseInsensitive) == 0) {
        return true;
    } else if (fontType.compare(fontTypeTTC, Qt::CaseInsensitive) == 0) {
        return true;
    }
    return false;
}

void MainWindow::sltMessageReceived(const QString &msg)
{
    QString platform = QGuiApplication::platformName();
    qDebug() << Q_FUNC_INFO << "Message received" << msg << "platform" << platform;

    // 使用防抖机制避免快速双击问题
    if (m_debounceTimer != nullptr) {
        if (m_debounceTimer->isActive()) {
            qDebug() << Q_FUNC_INFO << "Debounce timer active, ignoring message";
            return;
        }
        // 启动防抖定时器
        m_debounceTimer->start();
    }

    // 确保插件已经加载
    if (!isLoad) {
        loadAllModules();
        loadSearchXml();
    }

    if(platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)) {
        if (!m_listWinIds.isEmpty()) {
            WindowManager::activateWindow(m_listWinIds.back());
        }
    } else {
        KWindowSystem::forceActiveWindow(this->winId());
    }

    // 显示预览界面，然后延迟显示主界面
    if (m_prescene && !loadWidget && !m_presceneShown) {
        qDebug() << Q_FUNC_INFO << "Showing prescene for quick double-click";

        // 停止原有的定时器，避免冲突
        if (timer && timer->isActive()) {
            timer->stop();
            qDebug() << Q_FUNC_INFO << "Stopped original timer";
        }

        // 确保预览界面可见并置顶
        m_prescene->show();
        m_prescene->raise();
        m_prescene->activateWindow();
        m_presceneShown = true;

        // 延迟200ms后关闭预览界面并显示主界面
        QTimer::singleShot(200, this, [=]() {
            qDebug() << Q_FUNC_INFO << "Closing prescene and showing main interface";

            // 确保页面组件已初始化
            if (!loadWidget) {
                this->initPageWidgetSlot();
            }

            // initUI/closePrescene 已释放预览页；仅当仍存活时再 hide
            if (m_prescene) {
                m_prescene->hide();
                m_presceneShown = false;
                qDebug() << Q_FUNC_INFO << "Prescene hidden in message handler";
            }

            this->show();
            this->raise();
            this->activateWindow();

            // 处理消息内容
            processMessageContent(msg);
        });
    } else {
        // 主界面已 setupUi 后绝不能再 setCentralWidget(预览页)，否则会毁掉 stackedWidget
        if (!m_prescene && !m_isSetUI && !loadWidget) {
            qDebug() << Q_FUNC_INFO << "prescene is null, restart";
            m_prescene = new PreScene(this->size());
            this->setCentralWidget(m_prescene);

            // 先显示主窗口，否则子控件无法显示
            this->show();
            m_prescene->raise();

            timer = new QTimer(this);
            connect(timer, &QTimer::timeout, this, [=]() {
                presceneTimeoutSlot();
                processMessageContent(msg);
            });
            timer->start(200);
        }

        // 如果预览界面不可用或已经加载，直接显示主界面
        qDebug() << Q_FUNC_INFO << "Prescene not available or already loaded, showing main interface directly";

        this->show();
        this->raise();
        this->activateWindow();
        processMessageContent(msg);
    }
}

void MainWindow::presceneTimeoutSlot() {
    if (!loadWidget) {
        this->initPageWidgetSlot();
    }

    // closePrescene/initUI 已负责释放；此处仅兜底隐藏仍存活的预览页
    if (m_prescene) {
        m_prescene->hide();
        m_presceneShown = false;
        qDebug() << Q_FUNC_INFO << "Prescene hidden";
    }
    timer->stop();
}

// 新增方法：处理消息内容
void MainWindow::processMessageContent(const QString &msg)
{
    // 获取所有顶级窗口
    QList<QWindow*> topLevelWindows = QApplication::topLevelWindows();
    // 遍历并打印窗口标题
    for (QWindow *window : topLevelWindows) {
        qDebug() << "topLevelWindows " << window->modality() << window->title() << window->isVisible();
        if ((window->modality() == Qt::WindowModal || window->modality() == Qt::ApplicationModal) && window->isVisible()) {
            qDebug() << Q_FUNC_INFO << "is modality window, return" << window->title();
            return;
        };
    }

    if (msg.contains("##")) {
        if (msg.split("##").count() > 1) {
            m_secondWidgetName = msg.split("##").at(1);
            if (isFontSelectFile(msg.split("##").at(0))) {
                bootOptionsFilter("fonts", false);
            } else {
                bootOptionsFilter(msg.split("##").at(0), false);
            }

        }
    } else {
        if (isFontSelectFile(msg)) {
            bootOptionsFilter("fonts", false);
        } else {
            bootOptionsFilter(msg, false);
        }
    }
}

QVariantMap MainWindow::getSearchItems()
{
    return m_searchWidget->getSearchItems();
}

void MainWindow::switchPage(QString moduleName, QString jumpMoudle, QString jumpText) {
    //开始跳转
    QObject* pluginObj = ModulesFactory::getPluginObjectByName(moduleName);
    if (pluginObj != nullptr) {
        if (m_ModuleMap.isEmpty()) {
            functionBtnClicked(pluginObj);
            return;
        } else {
            QString superordinate = ModulesFactory::getModuleNamebyName(jumpMoudle);
            // 若跳转节点或跳转节点父节点不存在直接跳转，不受管控文件控制，若存在，则根据管控文件判断是否被禁用
            if (!m_ModuleMap.contains(jumpMoudle.toLower()) || !m_ModuleMap.contains(superordinate.toLower())
                    || (m_ModuleMap[jumpMoudle.toLower()].toBool() && m_ModuleMap[superordinate.toLower()].toBool())) {
                functionBtnClicked(pluginObj);
                return;
            }
        }
    }
    QMessageBox::information(this, tr("Warning"), tr("This function has been controlled"));
    return;
}

void MainWindow::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event);
    Q_EMIT posChanged();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    Q_EMIT posChanged();
}

void MainWindow::refreshSidebarAfterThemeChange()
{
    QTimer::singleShot(0, this, [=]() {
        if (scrollArea && scrollArea->listview()) {
            scrollArea->listview()->viewport()->update();
            scrollArea->update();
        }
    });
}

QString MainWindow::pluginBtnHoverColor(QString styleName, bool hoverFlag)
{
    QColor color1 = palette().color(QPalette::Active, QPalette::Button);
    QColor color2 = palette().color(QPalette::Active, QPalette::BrightText);
    QColor color;
    qreal r,g,b,a;
    QString hoverColor;
    if (((styleName.contains("dark") || styleName.contains("black")) && hoverFlag) ||
        ((!styleName.contains("dark") && !styleName.contains("black")) && !hoverFlag)) {
        r = color1.redF() * 0.8 + color2.redF() * 0.2;
        g = color1.greenF() * 0.8 + color2.greenF() * 0.2;
        b = color1.blueF() * 0.8 + color2.blueF() * 0.2;
        a = color1.alphaF() * 0.8 + color2.alphaF() * 0.2;
    } else {
        r = color1.redF() * 0.95 + color2.redF() * 0.05;
        g = color1.greenF() * 0.95 + color2.greenF() * 0.05;
        b = color1.blueF() * 0.95 + color2.blueF() * 0.05;
        a = color1.alphaF() * 0.95 + color2.alphaF() * 0.05;
    }
    color = QColor::fromRgbF(r, g, b, a);
    hoverColor = QString("rgba(%1, %2, %3, %4)").arg(color.red())
                                                .arg(color.green())
                                                .arg(color.blue())
                                                .arg(color.alpha());
    return hoverColor;
}

void MainWindow::initDbusConfigTranslation()
{
    tr("Authorization required to modify user data.");
    tr("To set the grub settings, you need to authenticate.");
    tr("To set the hostname, you need to authenticate.");
    tr("To set the pid, you need to authenticate.");
    tr("Change auto login, you need to authenticate.");
    tr("To change the ntp server address, you need to authenticate.");
}
