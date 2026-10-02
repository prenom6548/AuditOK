#include "mainwindow.h"
#include "loadanimation.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPalette>
#include <QVariant>
#include <QPainter>
#include <QStyleOption>
#include <QColor>
#include <QBrush>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QDBusReply>
#include <QVariantList>
#include <gsettingmonitor.h>
#include <ukuistylehelper/ukuistylehelper.h>
#include "windowmanager/windowmanager.h"
#include "startuptools.h"
#include "tools.h"

const char *UKUI_STYLE_GSETTING_PATH = "org.ukui.style";
const char *UKUI_PANEL_NAME = "org.ukui.panel";
const char *UKUI_PANEL_PATH = "/panel/position";
const char *UKUI_PANEL_INTERFACE = "org.ukui.panel";
const char *KYLIN_STATUS_MANAGER_NAME = "com.kylin.statusmanager.interface";
const char *KYLIN_STATUS_MANAGER_PATH = "/";
const char *KYLIN_STATUS_MANAGER_INTERFACE = "com.kylin.statusmanager.interface";

const int WIN_SIZE_W = 430;
const int WIN_SIZE_H = 300;
const int DISPLAY_W = 400;
const int ICON_SIZE = 16;
const int BTN_SIZE = 30;
const int TIME_LIMIT = 10000;
const int TIME_LIMIT_IN = 5000;
const int MARGIN = 8;
const int PANEL_TOP = 1;
const int PANEL_LEFT = 2;
const int PANEL_RIGHT = 3;

MainWindow *MainWindow::getInstance()
{
    static MainWindow instance;
    return &instance;
}

void MainWindow::showView()
{
    if (isVisible()) {
        return;
    }
    m_animation->start();
    m_title->setText(tr("Hi ,You can try to say:"));
    m_labList->show();
    m_resultDisplay->hide();
    m_pintOutLab->hide();
    kdk::UkuiStyleHelper::self()->removeHeader(this);
    show();
    resetWindowPosition();
    raise();
    activateWindow();
    starServer();
}

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    initService();
    initUI();
    initTrayIcon();
    initGsettings();
}

void MainWindow::initService()
{
    // 加载配置文件
    StartUpTools::getInstance()->loadConfig();

    m_client = new SpeechRecognitionClient(this);
    connect(m_client, &SpeechRecognitionClient::sigServerStatus, this, &MainWindow::onServerStatus);
    connect(m_client, &SpeechRecognitionClient::sigNewMsg, this, &MainWindow::onRecvMsg);

    m_audioInput = new SpeechInput;
    connect(m_audioInput, &SpeechInput::sigSpeechInput, this, &MainWindow::onAudioInfo);

    m_audioThread = new QThread(this);
    m_audioInput->moveToThread(m_audioThread);
    connect(m_audioThread, &QThread::started, m_audioInput, &SpeechInput::start);
    m_audioThread->start();

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::timeout);

    m_networkManager = new QNetworkConfigurationManager(this);
    connect(m_networkManager, &QNetworkConfigurationManager::onlineStateChanged, this,
            &MainWindow::onOnlineStateChanged);
}

void MainWindow::initUI()
{
    // 设置毛玻璃
    setProperty("useSystemStyleBlur", true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setWindowFlags(Qt::Tool);
    // 禁止拖动
    setProperty("useStyleWindowManager", false);

    setFocus(Qt::ActiveWindowFocusReason);
    setFixedSize(WIN_SIZE_W, WIN_SIZE_H);

    // m_closeBtn = new QPushButton(this);
    // connect(m_closeBtn, &QPushButton::clicked, this, &MainWindow::close);
    // m_closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    // m_closeBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
    // m_closeBtn->setIconSize(QSize(ICON_SIZE, ICON_SIZE));
    // m_closeBtn->setProperty("isWindowButton", 0x2);
    // m_closeBtn->setProperty("useIconHighlightEffect", 0x8);
    // m_closeBtn->setFlat(true);
    // m_closeBtn->setContentsMargins(5, 0, 5, 0);
    // m_closeBtn->setFocusPolicy(Qt::NoFocus);
    // m_closeBtn->setToolTip(tr("Close"));

    m_title = new QLabel(this);
    m_title->setText(tr("Hi ,You can try to say:"));
    m_title->setStyleSheet("background: transparent;");

    m_labList = new QListWidget(this);
    m_labList->setFocusPolicy(Qt::NoFocus);
    m_labList->setFixedSize(DISPLAY_W, 96);
    m_labList->setSpacing(8);
    m_labList->setResizeMode(QListWidget::Adjust);
    m_labList->setViewMode(QListWidget::IconMode);
    // 列表中的图标不可拖动
    m_labList->setMovement(QListWidget::Static);
    // 去边框
    m_labList->setFrameShape(QListWidget::NoFrame);
    // 不可编辑
    m_labList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_labList->addItem(m_lab1);
    m_labList->addItem(m_lab2);
    m_labList->addItem(m_lab3);

    m_resultDisplay = new QTextEdit(this);
    m_resultDisplay->setFixedSize(DISPLAY_W, 60);
    m_resultDisplay->setAlignment(Qt::AlignCenter);
    m_resultDisplay->setFrameStyle(QFrame::NoFrame);
    m_resultDisplay->setReadOnly(true);
    m_resultDisplay->setFocusPolicy(Qt::NoFocus);
    m_resultDisplay->setTextInteractionFlags(Qt::NoTextInteraction);
    m_resultDisplay->viewport()->setCursor(Qt::ArrowCursor);
    QPalette palette;
    palette.setBrush(QPalette::Base, QBrush(QColor(255, 0, 0, 0)));
    m_resultDisplay->setPalette(palette);
    m_resultDisplay->hide();

    m_animation = new LoadAnimation(this);
    m_animation->setFocusPolicy(Qt::NoFocus);
    connect(m_audioInput, &SpeechInput::updateLevel, m_animation, &LoadAnimation::onUpdateLevel);
    connect(m_animation, &LoadAnimation::cliecked, this, &MainWindow::onAnimationClicked);

    m_pintOutLab = new QLabel(this);
    m_pintOutLab->setFocusPolicy(Qt::NoFocus);
    m_pintOutLab->setStyleSheet("background: transparent;");
    m_pintOutLab->setText(tr("click to start recognition"));
    m_pintOutLab->hide();

    // QHBoxLayout *titleLayout = new QHBoxLayout;
    // titleLayout->setSpacing(0);
    // titleLayout->addStretch();
    // titleLayout->addWidget(m_closeBtn);

    QHBoxLayout *titleLayout1 = new QHBoxLayout;
    titleLayout1->setSpacing(0);
    titleLayout1->setMargin(0);
    titleLayout1->addWidget(m_title);
    titleLayout1->addStretch();

    QHBoxLayout *m_displayLayout = new QHBoxLayout;
    m_displayLayout->setSpacing(0);
    m_displayLayout->setMargin(0);
    m_displayLayout->addStretch();
    m_displayLayout->addWidget(m_labList);
    m_displayLayout->addWidget(m_resultDisplay);
    m_displayLayout->addStretch();

    QHBoxLayout *animationLayout = new QHBoxLayout;
    animationLayout->addStretch();
    animationLayout->addWidget(m_animation);
    animationLayout->addStretch();

    QHBoxLayout *PintOutLayout = new QHBoxLayout;
    PintOutLayout->addStretch();
    PintOutLayout->addWidget(m_pintOutLab);
    PintOutLayout->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout;
    // vLayout->addLayout(titleLayout);
    vLayout->addLayout(titleLayout1);
    vLayout->addLayout(m_displayLayout);
    vLayout->addStretch();
    vLayout->addLayout(animationLayout);
    vLayout->addLayout(PintOutLayout);
    vLayout->setContentsMargins(24, BTN_SIZE, 4, 16);

    setLayout(vLayout);
}

void MainWindow::initTrayIcon()
{
    // m_shotcutAction = new QAction(tr("Keyboard Shortcuts"), this);
    // m_aboutAction = new QAction(tr("About"), this);
    // m_exitAction = new QAction(tr("Exit"), this);
    // connect(m_shotcutAction, &QAction::triggered, this, &MainWindow::onShotCutActionTriggled);
    // connect(m_aboutAction, &QAction::triggered, this, &MainWindow::onAboutActionTriggled);
    // connect(m_exitAction, &QAction::triggered, this, &MainWindow::onExitActionTriggled);
    // 右键菜单
    // m_trayIconMenu = new QMenu(this);
    // m_trayIconMenu->addAction(m_shotcutAction);
    // m_trayIconMenu->addAction(m_aboutAction);
    // m_trayIconMenu->addAction(m_exitAction);
    // 系统托盘
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setToolTip(QString(tr("Asr assistant")));
    // m_trayIcon->setContextMenu(m_trayIconMenu);
    m_trayIcon->setIcon(QIcon::fromTheme("ukui-kylin-asrassistant-symbolic"));
    // onTrayIconActivated(QSystemTrayIcon::ActivationReason::Trigger);
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::onTrayIconActivated);
    m_trayIcon->show();
}

void MainWindow::initGsettings()
{
    changeTheme();
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this,
            &MainWindow::changeTheme);
    changeFontSize();
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this,
            &MainWindow::changeFontSize);
    changeTransparency();
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemTransparencyChange, this,
            &MainWindow::changeTransparency);
    if (QGSettings::isSchemaInstalled(UKUI_STYLE_GSETTING_PATH)) {
        m_fontData = new QGSettings(UKUI_STYLE_GSETTING_PATH);
        connect(m_fontData, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "systemFont") {
                changeFontSize();
            }
        });
    }
    m_modeDbusInterface = new QDBusInterface(KYLIN_STATUS_MANAGER_NAME, KYLIN_STATUS_MANAGER_PATH,
                                             KYLIN_STATUS_MANAGER_INTERFACE, QDBusConnection::sessionBus());
    QObject::connect(m_modeDbusInterface, SIGNAL(mode_change_signal(bool)), this, SLOT(onModeChanged(bool)));
    if (m_modeDbusInterface->isValid()) {
        QDBusPendingReply<bool> reply_pcORpad = m_modeDbusInterface->call("get_current_tabletmode");
        if (reply_pcORpad.isValid()) {
            onModeChanged(reply_pcORpad.value());
        } else {
            onModeChanged(false);
        }
    } else {
        onModeChanged(false);
    }
}

void MainWindow::resetWindowPosition()
{
    if (m_model == PublicAttributes::Model::Table) {
        QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
        QRect rect((availableGeometry.width() - width()) / 2, (availableGeometry.height() - height()) / 2, width(),
                   height());
        kdk::WindowManager::setGeometry(windowHandle(), rect);
        return;
    }

    if (!m_positionInterface) {
        m_positionInterface =
            new QDBusInterface(UKUI_PANEL_NAME, UKUI_PANEL_PATH, UKUI_PANEL_INTERFACE, QDBusConnection::sessionBus());
    }
    QRect rect;
    QDBusReply<QVariantList> reply = m_positionInterface->call("GetPrimaryScreenGeometry");
    // reply获取的参数共5个，分别是
    // 主屏可用区域的起点x坐标，主屏可用区域的起点y坐标，主屏可用区域的宽度，主屏可用区域高度，任务栏位置
    if (!m_positionInterface->isValid() || !reply.isValid() || reply.value().size() < 5) {
        qCritical() << QDBusConnection::sessionBus().lastError().message();
        kdk::WindowManager::setGeometry(windowHandle(), QRect(0, 0, width(), height()));
        return;
    }
    QVariantList position_list = reply.value();
    int position = position_list.at(4).toInt();
    switch (position) {
        case PANEL_TOP:
            // 任务栏位于上方
            rect = QRect(position_list.at(0).toInt() + position_list.at(2).toInt() - width() - MARGIN,
                         position_list.at(1).toInt() + MARGIN, width(), height());
            break;
            // 任务栏位于左边
        case PANEL_LEFT:
            rect =
                QRect(position_list.at(0).toInt() + MARGIN,
                      position_list.at(1).toInt() + reply.value().at(3).toInt() - height() - MARGIN, width(), height());
            break;
            // 任务栏位于右边
        case PANEL_RIGHT:
            rect =
                QRect(position_list.at(0).toInt() + position_list.at(2).toInt() - width() - MARGIN,
                      position_list.at(1).toInt() + reply.value().at(3).toInt() - height() - MARGIN, width(), height());
            break;
            // 任务栏位于下方
        default:
            rect =
                QRect(position_list.at(0).toInt() + position_list.at(2).toInt() - width() - MARGIN,
                      position_list.at(1).toInt() + reply.value().at(3).toInt() - height() - MARGIN, width(), height());
            break;
    }
    kdk::WindowManager::setGeometry(windowHandle(), rect);
}

void MainWindow::hideView()
{
    hide();
    if (m_isStartServer) {
        stopServer();
    }
}

void MainWindow::starServer()
{
    if (!Tools::checkNet()) {
        onOnlineStateChanged(false);
        return;
    }
    m_isStartServer = true;
    m_isInit = true;
    m_client->start();
}

void MainWindow::stopServer()
{
    m_isStartServer = false;
    m_timer->stop();
    Q_EMIT m_audioInput->stopAudio();
    m_client->stop();
}

void MainWindow::deleteServer()
{
    if (m_timer != nullptr) {
        m_timer->stop();
        m_timer->deleteLater();
    }
    if (m_audioThread != nullptr) {
        m_audioThread->quit();
        m_audioThread->wait();
        m_audioThread->deleteLater();
    }
    if (m_client != nullptr) {
        m_client->deleteLater();
    }
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    QColor color = palette().color(QPalette::Base);
    color.setAlpha(m_opacity * 255);
    QPalette pal(palette());
    pal.setColor(QPalette::Window, QColor(color));
    setPalette(pal);
    QBrush brush = QBrush(color);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect, 0, 0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void MainWindow::focusOutEvent(QFocusEvent *event)
{
    hideView();
}

void MainWindow::onServerStatus(SpeechRecognitionClient::ServerState state)
{
    switch (state) {
        case SpeechRecognitionClient::ServerState::CONNECTED: {
            m_timer->start(TIME_LIMIT);
            Q_EMIT m_audioInput->startAudio();
        } break;
        case SpeechRecognitionClient::ServerState::UNCONNECT: {
            onOnlineStateChanged(false);
        } break;
        default:
            break;
    }
}

void MainWindow::onRecvMsg(const QString &msg)
{
    if (msg.size() == 0) {
        Q_EMIT m_audioInput->read();
        return;
    }
    QString displayText = msg;
    Tools::processStr(displayText);
    if (m_isInit) {
        m_isInit = false;
        m_labList->hide();
        m_resultDisplay->show();
    }
    if (m_oldResults == msg) {
        if (!m_isStartTimer) {
            m_timer->start(TIME_LIMIT_IN);
            m_isStartTimer = true;
        }
        Q_EMIT m_audioInput->read();
        return;
    }

    m_isStartTimer = false;
    m_timer->stop();
    m_oldResults = msg;
    m_resultDisplay->setText(displayText);
    m_resultDisplay->setAlignment(Qt::AlignCenter);

    QString key = StartUpTools::getInstance()->findKey(displayText);

    if (!key.isEmpty()) {
        QTime dieTime = QTime::currentTime().addMSecs(800);
        while (QTime::currentTime() < dieTime) {
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }
        qInfo() << "The wake word was matched successfully!";
        hideView();
        StartUpTools::getInstance()->startUp(key);
        return;
    }
    Q_EMIT m_audioInput->read();
}

void MainWindow::onAudioInfo(const QByteArray &data)
{
    m_client->senMsg(data);
}

void MainWindow::onOnlineStateChanged(bool isOnline)
{
    if (!isOnline) {
        qInfo() << "The network connection is disconnected!";
        stopServer();
        m_labList->hide();
        m_title->setText(tr("Identify the results:"));
        m_resultDisplay->setText(
            tr("Sorry, the network is disconnected, please check the network connection and try again"));
        m_resultDisplay->setAlignment(Qt::AlignCenter);
        m_resultDisplay->show();
        m_pintOutLab->show();
        m_animation->stop();
    }
}

void MainWindow::timeout()
{
    stopServer();
    m_labList->hide();
    m_title->setText(tr("Identify the results:"));
    if (!m_isInit) {
        m_isStartTimer = false;
        m_resultDisplay->setText(tr("Sorry, the app was not successfully detected"));
    } else {
        m_resultDisplay->setText(tr("Sorry, I didn't hear it clearly, please say it again"));
    }
    m_resultDisplay->setAlignment(Qt::AlignCenter);
    m_resultDisplay->show();
    m_pintOutLab->show();
    m_animation->stop();
}

void MainWindow::onModeChanged(bool isModel)
{
    qInfo() << "mode changed ---- is table:" << isModel;
    if (isModel) {
        m_model = PublicAttributes::Model::Table;
    } else {
        m_model = PublicAttributes::Model::PcModel;
    }
    StartUpTools::getInstance()->setModel(m_model);
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::ActivationReason::Context) {
        // m_trayIconMenu->popup(QCursor::pos());
    } else {
        if (isVisible()) {
            hideView();
        } else {
            showView();
        }
    }
}

// void MainWindow::onShotCutActionTriggled()
// {
//     m_shortCutWin = new ShortCutWin;
//     m_shortCutWin->setAttribute(Qt::WA_DeleteOnClose);
//     m_shortCutWin->setWindowModality(Qt::WindowModal);
//     m_shortCutWin->setWindowModality(Qt::ApplicationModal);
//     m_shortCutWin->changeTheme(m_theme);
//     QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
//     QRect rect((availableGeometry.width() - m_shortCutWin->width()) / 2,
//                (availableGeometry.height() - m_shortCutWin->height()) / 2, m_shortCutWin->width(),
//                m_shortCutWin->height());
//     m_shortCutWin->show();
//     kdk::WindowManager::setGeometry(m_shortCutWin->windowHandle(), rect);
// }

// void MainWindow::onAboutActionTriggled()
// {
//     m_aboutWindow = new kdk::KAboutDialog(this);
//     m_aboutWindow->setAppIcon(QIcon::fromTheme("ukui-kylin-asrassistant-symbolic"));
//     m_aboutWindow->setAppName(QString(tr("Asr assistant")));
//     m_aboutWindow->setAppVersion(tr("Version:") + qApp->applicationVersion());
//     m_aboutWindow->setAttribute(Qt::WA_DeleteOnClose);
//     m_aboutWindow->setWindowModality(Qt::WindowModal);
//     m_aboutWindow->setWindowModality(Qt::ApplicationModal);
//     QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
//     QRect rect((availableGeometry.width() - m_aboutWindow->width()) / 2,
//                (availableGeometry.height() - m_aboutWindow->height()) / 2, m_aboutWindow->width(),
//                m_aboutWindow->height());
//     m_aboutWindow->show();
//     kdk::WindowManager::setGeometry(m_aboutWindow->windowHandle(), rect);
// }

// void MainWindow::onExitActionTriggled()
// {
//     deleteServer();
//     QCoreApplication::quit();
// }

void MainWindow::changeTheme()
{
    QString themeStr = kdk::GsettingMonitor::getSystemTheme().toString();
    if (themeStr == QString("ukui-dark") || themeStr == QString("ukui-black")) {
        m_theme = PublicAttributes::Theme::Dark;
        if (m_labList != nullptr) {
            m_labList->setStyleSheet("QListWidget{background: transparent;}"
                                     "QListWidget::Item{height: 36px;border-radius: 6px;background-color: "
                                     "rgba(216,216,216,0.15);color: rgba(217,217,217,1);}"
                                     "QListWidget::Item:hover{height: 36px;border-radius: 6px;background-color: "
                                     "rgba(216,216,216,0.15);color: rgba(217,217,217,1);}"
                                     "QListWidget::item:selected{height: 36px;border-radius: "
                                     "6px;background-color:rgba(216,216,216,0.15);color: rgba(217,217,217,1);}");
        }
    } else {
        m_theme = PublicAttributes::Theme::Light;
        if (m_labList != nullptr) {
            m_labList->setStyleSheet("QListWidget{background: transparent;}"
                                     "QListWidget::Item{height: 36px;border-radius: 6px;background-color: "
                                     "rgba(0,0,0,0.05);color: rgba(38,38,38,1);}"
                                     "QListWidget::Item:hover{height: 36px;border-radius: 6px;background-color: "
                                     "rgba(0,0,0,0.05);color: rgba(38,38,38,1);}"
                                     "QListWidget::item:selected{height: 36px;border-radius: "
                                     "6px;background-color:rgba(0,0,0,0.05);color: rgba(38,38,38,1);}");
        }
    }
    // if (m_shortCutWin != nullptr) {
    //     m_shortCutWin->changeTheme(m_theme);
    // }
}

void MainWindow::changeFontSize()
{
    double fontSize = kdk::GsettingMonitor::getSystemFontSize().toDouble();
    QFont font;
    font.setPointSizeF(fontSize);
    if (m_title != nullptr) {
        m_title->setFont(font);
    }
    if (m_labList != nullptr) {
        m_labList->setFont(font);
    }
    if (m_pintOutLab != nullptr) {
        m_pintOutLab->setFont(font);
    }
    // if (m_shortCutWin != nullptr) {
    //     m_shortCutWin->changeFont(fontSize);
    // }
    font.setPointSizeF(fontSize + 2);
    font.setBold(true);
    if (m_resultDisplay != nullptr) {
        m_resultDisplay->setFont(font);
    }
    update();
}

void MainWindow::changeTransparency()
{
    m_opacity = kdk::GsettingMonitor::getSystemTransparency().toDouble();
    update();
}

void MainWindow::onAnimationClicked()
{
    qInfo() << "restart Audio ...";
    m_title->setText(tr("Listening:"));
    m_resultDisplay->clear();
    m_pintOutLab->hide();
    starServer();
}
