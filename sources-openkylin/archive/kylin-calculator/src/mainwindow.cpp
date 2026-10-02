#define DEBUG_MODE false

#include <QKeyEvent>
#include <QDebug>
#include <QList>
#include <QDesktopWidget>
#include <QThread>
#include <KWindowSystem>
#include <QPainterPath>
#include <QTimer>
#include <iostream>
#include <unistd.h>
#include <gsettingmonitor.h>
#include <usermanual.h>
#include "buriedpoint.hpp"

#include "mainwindow.h"
#include "data_warehouse.h"

#define NORMAL_START 0

MainWindow *MainWindow::getInstance()
{
    static MainWindow *instance = nullptr;
    if (nullptr == instance) {
        instance = new MainWindow();
        instance->setObjectName("kylin-calculator_MainWindow_instance");
    }
    return instance;
}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    this->setWindowSize();
  QTimer::singleShot(50, this, [this](){
    InputProcess::inputFromButton(STANDARD);

    /* 初始化GSetting */
    this->initGsetting();

    /* 初始化列表项组件 */
    setWidgetUi();

    /* 设置组件样式 */
    setWidgetStyle();

    //监听平板模式切换
    m_statusSessionDbus = new QDBusInterface("com.kylin.statusmanager.interface",
                                             "/",
                                             "com.kylin.statusmanager.interface",
                                             QDBusConnection::sessionBus(),
                                             this);
    if (m_statusSessionDbus->isValid()) {
        QDBusReply<bool> isTabletMode = m_statusSessionDbus->call("get_current_tabletmode");
        this->setIsTabletMode(isTabletMode);
        tabletModeChange(isTabletMode);
        connect(m_statusSessionDbus, SIGNAL(mode_change_signal(bool)), this, SLOT(tabletModeChange(bool)));
    } else {
        tabletModeChange(false);
    }

    /* 初始化横竖屏的初值start */
    hLayoutFlag = PCMode;
    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        hOrVMode = new HorizontalOrVerticalMode;
        hOrVMode->setObjectName("kylin-calculator_HorizontalOrVerticalMode_hOrVMode");
        connect(hOrVMode, &HorizontalOrVerticalMode::RotationSig, this, &MainWindow::slotIntelModeChanged);
        hLayoutFlag = hOrVMode->defaultModeCapture();
        setIntelModeChange(hLayoutFlag);
        if (hLayoutFlag != deviceMode::PCMode) {
            m_isPcModeStarted = false;
        }
    }
    /* 初始化横竖屏的初值end */

    /* intel 菜单打开默认模式为 科学模式 , 发送信号 , 改变模式 */
    if (DataWarehouse::getInstance()->platform == QString("intel")
        && DataWarehouse::getInstance()->intelPlug == DataWarehouse::StartMode::MODE_MAIN) {
        emit pTitleBar->sigModeChange(QString("scientific"));
        pTitleBar->m_modeText->setText(tr("scientific"));
        pTitleBar->m_modeList->slotScienceClick();
    }

    m_windowHelper = new UkuiWindowHelper(this);
    kdk_dia_upload_program_startup();
  });
}

MainWindow::~MainWindow()
{
    delete hOrVMode;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    /* 只有 intel 需要重画 , 用于适配 毛玻璃 */
    if (DataWarehouse::getInstance()->platform != QString("intel")) {
        return;
    }

    /* 反锯齿 */
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    /* 小插件画圆角 */
    QPainterPath rectPath;
    if (DataWarehouse::getInstance()->intelPlug != DataWarehouse::StartMode::MODE_MAIN) {
        rectPath.addRoundedRect(this->rect(), 32, 32);
    } else {
        rectPath.addRoundedRect(this->rect(), 0, 0);
    }

    QStyleOption opt;
    opt.init(this);

    QColor mainColor;

    /* intel 不支持透明度变动 ， 直接写死 */
    if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
        mainColor = QColor(245, 245, 245, 0.85 * 255);
    } else {
        mainColor = QColor(20, 20, 20, 0.85 * 255);
    }

    if (DataWarehouse::getInstance()->platform == QString("intel")
        && DataWarehouse::getInstance()->transparency == 255) {
        if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
            mainColor = QColor(245, 245, 245, 1 * 255);
        } else {
            mainColor = QColor(20, 20, 20, 1 * 255);
        }
    }

#if 0
    if (QColor(255 , 255 , 255) == opt.palette.color(QPalette::Base) || QColor(248 , 248 , 248) == opt.palette.color(QPalette::Base)) {
        mainColor = QColor(245 , 245 , 245 , 0.85 * 255);
    } else {
        mainColor = QColor(20 , 20 , 20 , 0.85 * 255);
    }
#endif

    p.fillPath(rectPath, QBrush(mainColor));

    return;
}

/* 通过 gsetting 和 dbus 感知的后续可单独分离 , 目前暂时放在此处 */
void MainWindow::initGsetting(void)
{
    /* 透明度 */
    if (DataWarehouse::getInstance()->platform == QString("intel")) {

        DataWarehouse::getInstance()->transparency = kdk::GsettingMonitor::getSystemTransparency().toDouble() * 255;
        connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemTransparencyChange, this, [=]() {
            DataWarehouse::getInstance()->transparency = kdk::GsettingMonitor::getSystemTransparency().toDouble() * 255;
            emit this->sigTranparencyChange();
        });
    }

    /* 主题 */
    QString themeStr = kdk::GsettingMonitor::getSystemTheme().toString();
    if (themeStr == QString("ukui-dark") || themeStr == QString("ukui-black")) {
        WidgetStyle::themeColor = 1;
    } else {
        WidgetStyle::themeColor = 0;
    }
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [=]() {
        QString themeStr = kdk::GsettingMonitor::getSystemTheme().toString();
        if (themeStr == QString("ukui-dark") || themeStr == QString("ukui-black")) {
            WidgetStyle::themeColor = 1;
            changeDarkTheme();
        } else {
            WidgetStyle::themeColor = 0;
            changeLightTheme();
        }
    });

    return;
}
void MainWindow::slotIntelModeChanged(bool sig)
{
    hLayoutFlag = hOrVMode->defaultModeCapture();
    setIntelModeChange(hLayoutFlag);

    return;
}

/* 初始化列表项组件 */
void MainWindow::setWidgetUi()
{
    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        /* 设置毛玻璃 */
        this->setProperty("useSystemStyleBlur", true);
        this->setAttribute(Qt::WA_TranslucentBackground, true);
    } else {
        this->setAutoFillBackground(true);
//        this->setBackgroundRole(QPalette::Base);
    }

    mainWid = new QWidget(this);
    mainWid->setObjectName("kylin-calculator_QWidget_mainWid");

    outputWid = new QWidget(this);
    outputWid->setObjectName("kylin-calculator_QWidget_outputWid");
    outputWid->setObjectName("outputWid");
    mainOutputLayout = new QVBoxLayout();
    mainOutputLayout->setMargin(0);
    outputWid->setLayout(mainOutputLayout);

    buttonWid = new QWidget();
    buttonWid->setObjectName("kylin-calculator_QWidget_buttonWid");
    buttonWid->setObjectName("buttonWid");
    buttonWid->setMaximumHeight(800);
    mainButtonLayout = new QVBoxLayout();
    mainButtonLayout->setMargin(0);
    buttonWid->setLayout(mainButtonLayout);

    /* intel 小插件画圆角 */
    if (DataWarehouse::getInstance()->intelPlug != DataWarehouse::StartMode::MODE_MAIN) {
        buttonWid->setStyleSheet("border-radius:32px;");
    }

    /* 设置标题栏 */
    setCommonUi();
    /* 标准计算界面布局 */
    setStandardUi();

    mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    mainLayout->addWidget(pTitleBar);
    mainLayout->addWidget(outputWid);
    mainLayout->addWidget(buttonWid);
    mainLayout->setSpacing(0);
    this->mainWid->setLayout(mainLayout);

    return;
}

// 设置组件样式
void MainWindow::setWidgetStyle()
{
    this->setCentralWidget(mainWid);

    return;
}

/* 公有组件 ，即标题栏和功能列表 */
void MainWindow::setCommonUi()
{
    /* 设置图标 */
    this->setWindowTitle(tr("Calculator"));

    /* 标题栏 */
    pTitleBar = new TitleBar(this);
    pTitleBar->setObjectName("kylin-calculator_TitleBar_pTitleBar");
    installEventFilter(pTitleBar);

    /* menuBar里面的一些需要被触发的槽 */
    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        connect(pTitleBar, &TitleBar::sigModeChange, this, &MainWindow::changeModel);
        connect(pTitleBar, &TitleBar::sigFontUpdate, this, &MainWindow::fontUpdate);
    } else {
        connect(pTitleBar->menuBar, SIGNAL(menuModuleClose()), pTitleBar->window(), SLOT(close()));
        connect(pTitleBar->menuBar, SIGNAL(menuModuleChanged(QString)), this, SLOT(changeModel(QString)));
        connect(pTitleBar, &TitleBar::sigFontUpdate, this, &MainWindow::fontUpdate);
        connect(pTitleBar->m_pTopButton, SIGNAL(clicked(bool)), this, SLOT(stayTop()));

        pTitleBar->setFuncLabel(pTitleBar->STANDARD_LABEL);
    }

    /* 右键菜单 */
    labelMenu = new QMenu(this);
    labelMenu->setObjectName("kylin-calculator_QMenu_labelMenu");
    copyAction = new QAction(this);
    copyAction->setObjectName("kylin-calculator_QAction_copyAction");
    pasteAction = new QAction(this);
    pasteAction->setObjectName("kylin-calculator_QAction_pasteAction");

    labelMenu->addAction(copyAction);
    labelMenu->addAction(pasteAction);

    copyAction->setText(tr("Copy"));
    pasteAction->setText(tr("Paste"));

    connect(copyAction, &QAction::triggered, this, &MainWindow::copyCalResult);
    connect(pasteAction, &QAction::triggered, this, &MainWindow::pasteToLabNow);

    return;
}

/* 设置UI界面大小 */
void MainWindow::setWindowSize() {
    QString currentPlatform = DataWarehouse::getInstance()->platform;

    QString label = this->currentModel;

    // 根据当前模式获得显示UI的界面size值
    int currentWindowW = 0;
    int currentWindowH = 0;

    if (label == PROGRAMMER) {
        if (!m_prograModel->isBinWinShow()) {
            currentWindowW = PROGRAM_WIN_WIDTH;
            currentWindowH = PROGRAM_WIN_HEIGHT;
        } else {
            currentWindowW = PROGRAM_WIN_WIDTH;
            currentWindowH = PROGRAM_WIN_HEIGHT + PROGRAM_BIN_HEIGHT;
        }
    } else if (label == SCIENTIFIC) {
        if (currentPlatform == QString("intel")) {
            currentWindowW = INTEL_SCIENTIFIC_WINDOWW;
            currentWindowH = INTEL_SCIENTIFIC_WINDOWH;
        }
        else {
            currentWindowW = SCIENTIFIC_WINDOWW;
            currentWindowH = SCIENTIFIC_WINDOWH;
        }
    } else if (label == STANDARD) {
        if (currentPlatform == QString("intel")) {
            currentWindowW = INTEL_STANDARD_WINDOWW;
            currentWindowH = INTEL_STANDARD_WINDOWH;
        }
        else {
            currentWindowW = STANDARD_WINDOWW;
            currentWindowH = STANDARD_WINDOWH;
        }
    } else {
        currentWindowW = STANDARD_WINDOWW;
        currentWindowH = STANDARD_WINDOWH;
    }

    // 根据当前系统平台设置对应UI界面的size
    if (currentPlatform == QString("intel")) {
        if (!this->isTabletMode() && this->windowState() == Qt::WindowNoState) {
            this->setMinimumSize(INTEL_STANDARD_WINDOWW, INTEL_STANDARD_WINDOWH);
            this->resize(INTEL_STANDARD_WINDOWW, INTEL_STANDARD_WINDOWH);
       }
    } else if (currentPlatform == QString("xc-tablet")) {
        /* xc 平板平台 */
        if(this->isTabletMode()){     //平板模式下界面大小直接设置为最大值
            this->setMinimumSize(1, 1);
            this->setMaximumSize(99999, 99999);
            setWindowState(windowState() | Qt::WindowMaximized);
        } else {       //PC 模式下界面设置为对应模式固定值
            setWindowState(windowState() & ~Qt::WindowMaximized);
            this->setFixedSize(currentWindowW, currentWindowH);
//            this->resize(currentWindowW, currentWindowH);
        }
    } else {
        this->setFixedSize(currentWindowW, currentWindowH);
//        this->resize(currentWindowW, currentWindowH);
    }
}

/* 标准计算界面布局 */
void MainWindow::setStandardUi()
{
    m_hisNumMax = PC_HIS_NUM;

    /* 设置当前模式 */
    this->currentModel = STANDARD;

    /* 设置当前界面大小*/
    this->setWindowSize();

    if (standardOutput == nullptr) {
        standardOutput = new StandardOutput(this);
        standardOutput->setObjectName("kylin-calculator_StandardOutput_standardOutput");
        standardModel = new StandardModel(this);
        standardModel->setObjectName("kylin-calculator_StandardModel_standardModel");

        standardOutput->staLabNow->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(standardOutput->staLabNow, &MainWindow::customContextMenuRequested, this,
                &MainWindow::myCustomContextMenuRequested);

        /* 绑定处理函数 */
        for (int i = 0; i < 10; i++) {
            QObject::connect(standardModel->btnNum[i], SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        }

        QObject::connect(standardModel->btnClear, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(standardModel->btnDiv, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(standardModel->btnMulti, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(standardModel->btnSub, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(standardModel->btnAdd, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(standardModel->btnEqual, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(standardModel->btnPer, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(standardModel->btnPoint, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(standardModel->btnDelete, SIGNAL(clicked(bool)), this, SLOT(delete_btn_handle(bool)));
    }

    installEventFilter(standardModel);

    /* 绑定输出组件 */
    this->lab_last = standardOutput->staLabLast;
    this->lab_prepare = standardOutput->staLabPre;
    this->lab_now = standardOutput->staLabNow;

    if (DataWarehouse::getInstance()->intelPlug != DataWarehouse::StartMode::MODE_MAIN) {
        /* 小插件模式 */
        this->pTitleBar->m_min->hide();
        this->pTitleBar->m_max->hide();
        this->pTitleBar->m_close->hide();
    }

    /* 不同平台 按钮 风格处理 */
    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        this->pTitleBar->m_max->hide();
        standardModel->createIntelStyle();
    }

    /* 不同平台 采用相同的显示屏 */
    standardOutput->setWidgetStyle();

    mainOutputLayout->addWidget(standardOutput);
    mainButtonLayout->addWidget(standardModel);

    standardOutput->show();
    standardModel->show();

    return;
}

// 科学计算界面布局
void MainWindow::setScientificUi()
{
    m_hisNumMax = PC_HIS_NUM;

    /* 设置当前模式 */
    this->currentModel = SCIENTIFIC;

    /* 设置当前界面大小*/
    this->setWindowSize();

    /* 初始化界面布局 */
    if (scientificOutput == nullptr) {
        scientificOutput = new ScientificOutput(this);
        scientificOutput->setObjectName("kylin-calculator_ScientificOutput_scientificOutput");
        scientificModel = new ScientificModel(this);
        scientificModel->setObjectName("kylin-calculator_ScientificModel_scientificModel");

        scientificOutput->staLabNow->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(scientificOutput->staLabNow, &MainWindow::customContextMenuRequested, this,
                &MainWindow::myCustomContextMenuRequested);

        InputProcess::inputFromButton(RAD_SYMBOL);

        for (int i = 0; i < 10; i++) {
            QObject::connect(scientificModel->btnNum[i], SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        }

        QObject::connect(scientificModel->btnClear, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(scientificModel->btnDiv, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(scientificModel->btnMulti, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(scientificModel->btnSub, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(scientificModel->btnAdd, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(scientificModel->btnEqual, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(scientificModel->btnPer, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(scientificModel->btnPoint, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(scientificModel->btnDelete, SIGNAL(clicked(bool)), this, SLOT(delete_btn_handle(bool)));

        QObject::connect(scientificModel->btnUndo, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));
        QObject::connect(scientificModel->btnBracketLeft, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(scientificModel->btnBracketRight, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));

        QObject::connect(scientificModel->btnReci, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));
        QObject::connect(scientificModel->btnXPower2, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));
        QObject::connect(scientificModel->btnXPower3, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));
        QObject::connect(scientificModel->btnYPowerX, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));

        QObject::connect(scientificModel->btnFac, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));
        QObject::connect(scientificModel->btnXSquare2, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));
        QObject::connect(scientificModel->btnXSquare3, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));
        QObject::connect(scientificModel->btnYSquareX, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));

        QObject::connect(scientificModel->btnSin, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));
        QObject::connect(scientificModel->btnCos, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));
        QObject::connect(scientificModel->btnTan, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));
        QObject::connect(scientificModel->btnLog, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));

        QObject::connect(scientificModel->btnRad, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));
        QObject::connect(scientificModel->btnPi, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));
        QObject::connect(scientificModel->btnExp, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));
        QObject::connect(scientificModel->btnLn, SIGNAL(clicked(bool)), this, SLOT(sciBtnHandler(bool)));
    }

    installEventFilter(scientificModel);

    /* 绑定屏幕 */
    this->lab_last = scientificOutput->staLabLast;
    this->lab_prepare = scientificOutput->staLabPre;
    this->lab_now = scientificOutput->staLabNow;

    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        scientificModel->createIntelStyle();
        if (hLayoutFlag == deviceMode::PCMode) {
            // 窗口模式
            this->pTitleBar->m_max->show();
        }
    }

    scientificModel->updateBtnSinDisplay();
    scientificModel->updateBtnRadDisplay();

    scientificOutput->setWidgetStyle();

    mainOutputLayout->addWidget(scientificOutput);
    mainButtonLayout->addWidget(scientificModel);

    scientificOutput->show();
    scientificModel->show();

    return;
}

/* 换算器界面布局 */
void MainWindow::setToolUi()
{
    qInfo() << "Switch to exchange rate mode.";

    // 设置当前模式
    this->currentModel = EXCHANGE_RATE;

    /* 设置当前界面大小*/
    this->setWindowSize();

    // 初始化数据输出界面
    if (toolModelOutput == nullptr) {
        toolModelOutput = new ToolModelOutput(this);
        toolModelOutput->setObjectName("kylin-calculator_ToolModelOutput_toolModelOutput");
        toolModelButton = new ToolModelButton(this);
        toolModelButton->setObjectName("kylin-calculator_ToolModelButton_toolModelButton");

        toolModelOutput->toolLabBef->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(toolModelOutput->toolLabBef, &MainWindow::customContextMenuRequested, this,
                &MainWindow::myCustomContextMenuRequested);

        // 绑定处理函数
        for (int i = 0; i < 10; i++) {
            QObject::connect(toolModelButton->btnNum[i], SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        }

        QObject::connect(toolModelButton->btnClear, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(toolModelButton->btnDiv, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(toolModelButton->btnMulti, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(toolModelButton->btnSub, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(toolModelButton->btnAdd, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(toolModelButton->btnEqual, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(toolModelButton->btnPer, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(toolModelButton->btnPoint, SIGNAL(clicked(bool)), this, SLOT(btn_handler(bool)));
        QObject::connect(toolModelButton->btnDelete, SIGNAL(clicked(bool)), this, SLOT(delete_btn_handle(bool)));
    }

    installEventFilter(toolModelButton);

    // 绑定输出组件
    this->lab_last = toolModelOutput->toolLabHis;
    this->lab_prepare = toolModelOutput->toolLabAft;
    this->lab_now = toolModelOutput->toolLabBef;

    toolModelOutput->setWidgetStyle();

    mainOutputLayout->addWidget(toolModelOutput);
    mainButtonLayout->addWidget(toolModelButton);

    toolModelOutput->show();
    toolModelButton->show();

    return;
}

/* 程序员模式界面布局 */
void MainWindow::setProgrammerUi()
{
    qInfo() << "Switch to programmer mode.";
    this->currentModel = PROGRAMMER;

    outputWid->hide();
    buttonWid->hide();

    if (m_prograModel == nullptr) {
        m_prograModel = new ProgramModel(this);
        m_prograModel->setObjectName("kylin-calculator_ProgramModel_m_prograModel");
        m_prograModel->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(m_prograModel, &MainWindow::customContextMenuRequested, this,
                &MainWindow::myCustomContextMenuRequested);
    }

    /* 设置当前界面大小*/
    this->setWindowSize();

    if (WidgetStyle::ThemeColor::LIGHT == WidgetStyle::themeColor) {
        m_prograModel->setLightUI();
    } else if (WidgetStyle::ThemeColor::DARK == WidgetStyle::themeColor) {
        m_prograModel->setDarkUI();
    }

    m_prograModel->show();

    mainLayout->addWidget(m_prograModel);
}

// 计算器界面切换布局
void MainWindow::changeCalculatorUi()
{
    mainLayout->addWidget(outputWid);
    mainLayout->addWidget(buttonWid);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    this->mainWid->setLayout(mainLayout);
    this->mainWid->setObjectName("mainWid");
    this->setCentralWidget(mainWid);
}

// 换算器界面切换布局
void MainWindow::changeToolUi()
{
    mainLayout->addWidget(outputWid);
    mainLayout->addWidget(buttonWid);

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    this->mainWid->setLayout(mainLayout);
    this->mainWid->setObjectName("mainWid");
    this->setCentralWidget(mainWid);

    return;
}

/* 实现键盘输入 */
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1) {
        /* 用户手册 */
        kdk::UserManual userManual;
        if (!userManual.callUserManual("kylin-calculator")) {
            qCritical() << "user manual call fail!";
        }

    } else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_C) {
        /* 复制 */
        copyCalResult();
    } else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_V) {
		/* 粘贴 */
		QString clipText = clipboard->text();
		clipText.remove(QRegularExpression("\\s+"));
		if (clipText.isEmpty()) {
			return;
		}
		pasteToLabNow();
    }

    QString label = this->currentModel;

    if (label == STANDARD) {
        standardModel->keyPressEvent(event);
    } else if (label == SCIENTIFIC) {
        scientificModel->keyPressEvent(event);
    } else if (label == EXCHANGE_RATE) {
        toolModelButton->keyPressEvent(event);
    } else if (label == PROGRAMMER) {
        m_prograModel->keyPressEvent(event);
    }

    return;
}

// 实现千分位加入
QString MainWindow::addComma(QString s)
{
    if (s.isEmpty())
        return "";

    int pointFlag = 0;
    int pointIndex;

    for (int i = 0; i < s.size(); i++) {

        if (s[i] == '.') {
            pointFlag = 1;
            pointIndex = i;
        }
    }

    if (pointFlag == 1) {
        return addComma(s.mid(0, pointIndex)) + s.mid(pointIndex, s.size() - pointIndex);
    } else {
        QString ans;
        for (int i = s.size() - 3; i > 0; i -= 3)
            s.insert(i, ",");
        return s;
    }
}

// 换算器输入响应事件
void MainWindow::unitCalc()
{
    double rate = 0.15;
    QString textAft = this->lab_prepare->text().remove(QRegExp(","));

    double moneyAft = textAft.toDouble() * rate;

    this->lab_prepare->setText(QString::number(moneyAft));
}

// 将用于显示的表达式格式化为用于运算的表达式
QString MainWindow::formatDisToCal(QString text)
{
    text.replace(SUB, "-");
    text.replace("-", "ｰ");
    text.replace("×", "*");
    text.replace("÷", "/");
    text.replace(",", "");

    return text;
}

// 切换深色主题
void MainWindow::changeDarkTheme()
{
    WidgetStyle::themeColor = WidgetStyle::ThemeColor::DARK;

    QString label = this->currentModel;

    this->setWidgetStyle();

    /* 根据平台加载不同的样式 */
    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        pTitleBar->createInterStyle();

        if (label.contains(STANDARD)) {
            standardModel->createIntelStyle();
            standardOutput->setWidgetStyle(false);
        } else if (label.contains(SCIENTIFIC)) {
            scientificModel->createIntelStyle();
            scientificOutput->setWidgetStyle(false);
        }
        this->update();
    } else {
        pTitleBar->setWidgetStyle();

        if (label.contains(STANDARD)) {
            standardOutput->setWidgetStyle(false);
        } else if (label.contains(SCIENTIFIC)) {
            scientificOutput->setWidgetStyle(false);
        } else if (label.contains(EXCHANGE_RATE)) {
            toolModelOutput->setWidgetStyle(false);
        } else if (label.contains(PROGRAMMER)) {
            m_prograModel->setDarkUI();
        }
    }
}

// 切换浅色主题
void MainWindow::changeLightTheme()
{
    qInfo() << "Switch to light mode.";
    WidgetStyle::themeColor = WidgetStyle::ThemeColor::LIGHT;

    QString label = this->currentModel;

    this->setWidgetStyle();

    /* 根据不同的平台加载不同的样式 */

    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        pTitleBar->createInterStyle();

        if (label.contains(STANDARD)) {
            standardModel->createIntelStyle();
            standardOutput->setWidgetStyle(false);
        } else if (label.contains(SCIENTIFIC)) {
            scientificModel->createIntelStyle();
            scientificOutput->setWidgetStyle(false);
        }
        this->update();
    } else {
        pTitleBar->setWidgetStyle();

        if (label.contains(STANDARD)) {
            standardOutput->setWidgetStyle(false);
        } else if (label.contains(SCIENTIFIC)) {
            scientificOutput->setWidgetStyle(false);
        } else if (label.contains(EXCHANGE_RATE)) {
            toolModelOutput->setWidgetStyle(false);
        } else if (label.contains(PROGRAMMER)) {
            m_prograModel->setLightUI();
        }
    }
}

// 最小化状态下拉起主界面
void MainWindow::pullUpWindow()
{
    qDebug() << "show MainWindow";
    KWindowSystem::forceActiveWindow(this->winId());
    this->show();
}

/* 重置输入界面字号 */
void MainWindow::resetFontSize(QString calModel, QString fontSizeStr)
{
    if (calModel == STANDARD) {
        lab_now->setStyleSheet("font-size:" + fontSizeStr + "px;font-weight:15px;margin:0 0 5px 7px;");
    } else if (calModel == SCIENTIFIC) {
        lab_now->setStyleSheet("font-size:" + fontSizeStr + "px;font-weight:15px;margin:0 0 0 7px;");
    } else if (calModel == EXCHANGE_RATE) {
        lab_now->setStyleSheet("font-size:" + fontSizeStr + "px;font-weight:15px;");
    }

    return;
}

/* 更新输出界面 */
void MainWindow::updateOutput(QVector<QString> outVector)
{
    for (int i = 0; i < outVector.size(); i++) {
        if (outVector[i] == INF_SYMBOL) {
            outVector[i] = tr("Error!");
        }

        if (outVector[i] == NAN_SYMBOL) {
            outVector[i] = tr("Error!");
        }

        if (outVector[i] == "input Error!") {
            outVector[i] = tr("Input error!");
        }
    }

    /* 获取数据 */
    this->disData = outVector[DISPLAY_ON_LABEL_NOW];
    this->calData = outVector[LABEL_NOW_CAL_QSTR];

    /* 更新界面显示 */
    QString tmp = outVector[DISPLAY_ON_LABEL_NOW];
    if (tmp.count() > 1) {
        if (tmp.at(0) == QString("0") && tmp.at(1) == QString("ｰ")) {
            tmp.remove(0, 1);
        }
        OutputBase *output = getOutputParent(this->lab_now);
        if (output) {
            output->setNowText(tmp);
        } else {
            this->lab_now->setText(tmp);
        }
    } else {
        OutputBase *output = getOutputParent(this->lab_now);
        if (output) {
            output->setNowText(outVector[DISPLAY_ON_LABEL_NOW]);
        } else {
            this->lab_now->setText(outVector[DISPLAY_ON_LABEL_NOW]);
        }
    }

    this->lab_prepare->setText(outVector[DISPLAY_ON_LABEL_PREPARE]);

    qInfo() << "\n"
            << "DISPLAY_ON_LABEL_NOW" << outVector[DISPLAY_ON_LABEL_NOW] << "\n"
            << "LABEL_NOW_CAL_QSTR" << outVector[LABEL_NOW_CAL_QSTR] << "\n"
            << "CAL_ANS" << outVector[CAL_ANS] << "\n"
            << "LATEST_HISTORY" << outVector[LATEST_HISTORY] << "\n"
            << "DISPLAY_ON_LABEL_PREPARE" << outVector[DISPLAY_ON_LABEL_PREPARE] << "\n"
            << "LABEL_PREPARE_CAL_QSTR" << outVector[LABEL_PREPARE_CAL_QSTR] << "\n";

    /* 输出为"0"时设置为最大字号 */
    if (lab_now->text().size() == 1) {
        this->resetFontSize(this->currentModel, "48");
    } else if (lab_now->fontInfo().pixelSize() > MIN_FONT_SIZE) {
        /* 数字过长字号缩小 */
        QFont labFont = lab_now->font();
        QFontMetrics fontMts(labFont);
        int dif = fontMts.width(lab_now->text()) - lab_now->width();
        QString fontSizeStr = QString::number(lab_now->fontInfo().pixelSize() - 8);

        /* 不同模式下需要判断的距离阈值不一样 */
        while ((this->currentModel == STANDARD && dif > -12) || (this->currentModel == SCIENTIFIC && dif > -12)
               || (this->currentModel == EXCHANGE_RATE && dif > -20)) {
            this->resetFontSize(this->currentModel, fontSizeStr);
            labFont.setPixelSize(labFont.pixelSize() - 8);
            QFontMetrics fontMts(labFont);
            dif = fontMts.width(lab_now->text()) - lab_now->width();
            fontSizeStr = QString::number(labFont.pixelSize() - 8);
        }
    }

    return;
}

void MainWindow::fontUpdate()
{
    resetFontSize(this->currentModel, "48");
    int width = lab_now->width();
    if (this->windowState() == Qt::WindowMaximized) {
        //获取主屏幕分辨率
        QRect screenRect = QApplication::desktop()->availableGeometry();
        width = screenRect.width();
    }
    if (lab_now->fontInfo().pixelSize() > 16) {
        QFont labFont = lab_now->font();
        QFontMetrics fontMts(labFont);
        int dif = fontMts.width(lab_now->text()) - width;
        QString fontSizeStr = QString::number(lab_now->fontInfo().pixelSize() - 8);
        // 不同模式下需要判断的距离阈值不一样
        while ((this->currentModel == STANDARD && dif > -12) || (this->currentModel == SCIENTIFIC && dif > -12)
               || (this->currentModel == EXCHANGE_RATE && dif > -20)) {
            this->resetFontSize(this->currentModel, fontSizeStr);
            labFont.setPixelSize(labFont.pixelSize() - 8);
            QFontMetrics fontMts(labFont);
            dif = fontMts.width(lab_now->text()) - width;
            fontSizeStr = QString::number(labFont.pixelSize() - 8);
        }
    }
}

void MainWindow::btn_merge(const QString &disText)
{
    this->inputLongSym = false;

    if (this->currentModel == EXCHANGE_RATE && lab_prepare->fontInfo().pixelSize() < 40) {
        lab_prepare->setStyleSheet("font-size:40px;font-weight:15px;line-height:50px;");
    }
    /* 处理屏幕显示字号大小 */
    if (disText != BACKSPACE && disText != CLEAN && disText != EQUAL) {
        /* 判断是否输入过长 */
        QString str = lab_now->text().remove(",");
        int numberLength = 0;
        if (!str.isEmpty()) {
            while (str.at(str.length() - 1 - numberLength).isNumber()) {
                numberLength++;
                if (str.length() == numberLength) {
                    break;
                }
            }
        }
        if ((this->currentModel == STANDARD && numberLength >= DataWarehouse::getInstance()->maxInputNum)
            || (this->currentModel == SCIENTIFIC && numberLength >= (DataWarehouse::getInstance()->maxInputNum + 44))
            || (this->currentModel == EXCHANGE_RATE
                && numberLength >= (DataWarehouse::getInstance()->maxInputNum) - 12)) {

            this->lab_prepare->setText(tr("input too long"));
            QFont labFont = lab_prepare->font();
            QFontMetrics fontMts(labFont);
            int dif = fontMts.width(lab_prepare->text()) - lab_prepare->width();
            QString fontSizeStr = QString::number(lab_prepare->fontInfo().pixelSize() - 8);

            /* 不同模式下需要判断的距离阈值不一样 */
            while (this->currentModel == EXCHANGE_RATE && dif > -20) {
                lab_prepare->setStyleSheet("font-size:" + fontSizeStr + "px;font-weight:15px;");
                labFont.setPixelSize(labFont.pixelSize() - 8);
                QFontMetrics fontMts(labFont);
                dif = fontMts.width(lab_prepare->text()) - lab_prepare->width();
                fontSizeStr = QString::number(labFont.pixelSize() - 8);
            }
            this->inputLongSym = true;
            return;
        }
    } else if (disText == BACKSPACE && lab_now->fontInfo().pixelSize() < 48) {
        /* 删除时放大字号 */
        QFont labFont = lab_now->font();
        labFont.setPixelSize(lab_now->fontInfo().pixelSize() + 8);
        QFontMetrics fm(labFont);
        int dif = fm.width(lab_now->text()) - (lab_now->width() - 10);

        /* 放大字号时的距离阈值 */
        if (dif < 50) {
            QString fontSizeStr = QString::number(lab_now->fontInfo().pixelSize() + 8);

            this->resetFontSize(this->currentModel, fontSizeStr);
        }
    } else if (disText == CLEAN || disText == EQUAL) {
        /* 清除时设置为最大字号 */
        this->resetFontSize(this->currentModel, "48");
    }

    /* 格式化为用于运算的表达式 */
    QString calText = disText;
    calText = formatDisToCal(calText);

    disData.push_back(disText);
    calData.push_back(calText);

    /* 获取显示字符串 运算字符串 */
    QVector<QString> resVector = InputProcess::inputFromButton(disText);

    /* 将显示结果和运算结果进行显示 */
    updateOutput(resVector);


    /* 等于号的运算逻辑 更新界面显示 */
    if (resVector[LATEST_HISTORY].size()) {
        disHistory.push_back(resVector[LATEST_HISTORY] + '\n');

        disData.clear();
        calData.clear();

        /* 获取历史记录 */
        int size = disHistory.size();
        QString historyText = "";
        int hisIndex = (size - m_hisNumMax >= 0) ? (size - m_hisNumMax) : 0;
        for (int i = hisIndex; i < size; i++) {
            historyText = historyText + disHistory.at(i);
        }

        /* 去除末尾换行符 */
        historyText.chop(1);

        /* 在汇率模式下，根据历史记录修改为对应的记录 */
        QString label = this->currentModel;
        if (label != STANDARD && label != SCIENTIFIC) {
            historyText = toolModelOutput->unitConvHistory(disHistory[disHistory.size() - 1]);
        }

        historyText.replace(SUB, "-");

        OutputBase *output = getOutputParent(this->lab_last);
        if (output) {
            output->setHistoryText(historyText);
        } else {
            this->lab_last->setText(historyText);
        }
    }
}

OutputBase *MainWindow::getOutputParent(QObject *obj)
{
    OutputBase *output = nullptr;
    QObject *tmpParent = obj->parent();
    bool hasOutputParent = false;
    while (tmpParent) {
        if (tmpParent->objectName() == OUTPUTBASE_OBJECT_NAME) {
            hasOutputParent = true;
            break;
        }
        tmpParent = tmpParent->parent();
    }
    if (hasOutputParent) {
        output = qobject_cast<OutputBase *>(tmpParent);
    }
    return output;
}

/* 按键处理 */
void MainWindow::btn_handler(bool)
{
    /* 获取按键信息 */
    BasicButton *btn = dynamic_cast<BasicButton *>(sender());
    QString disText = btn->text();

    /* 用于 保存按键 , 计算 , 显示 , 添加历史记录 */
    btn_merge(disText);

    /* 当模式为汇率模式时 , 在正常的计算结果基础上 , 在乘以汇率 , 在进行显示 */
    QString label = this->currentModel;
    if (label != STANDARD && label != SCIENTIFIC && inputLongSym == false) {
        toolModelOutput->unitConversion();
    }

    return;
}

/* 科学模式下专属按钮点击事件 */
void MainWindow::sciBtnHandler(bool)
{
    /* 获取当前输入 */
    BasicButton *btn = dynamic_cast<BasicButton *>(sender());
    QString text = btn->text();
    btn_merge(text);

    return;
}

void MainWindow::delete_btn_handle(bool)
{
    // "B" 可能要改
    btn_merge(BACKSPACE);

    QString label = this->currentModel;
    if (label != STANDARD && label != SCIENTIFIC) {
        toolModelOutput->unitConversion();
    }

    return;
}

void MainWindow::funcListHandle(bool)
{
    if (funcListWid->isHidden()) {
        this->funcListWid->show();
    } else {
        this->funcListWid->hide();
    }
}

void MainWindow::changeModel(QString label)
{
    /* 适配 intel 小插件模式切换 */
    if (label == SCIENTIFIC && DataWarehouse::getInstance()->platform == QString("intel")
        && DataWarehouse::getInstance()->intelPlug != DataWarehouse::StartMode::MODE_MAIN) {
        pid_t pid = fork();
        if (pid < 0) {
            qDebug() << "Error : fork process fail";
        } else if (pid == 0) {
            /* 子进程 */
            pid_t pid1 = fork();
            if (pid1 < 0) {
                qDebug() << "Error : fork process fail";
            } else if (pid1 == 0) {
                /* 子进程 孤儿进程*/
                execl("/usr/bin/kylin-calculator", "kylin-calculator", NULL);
                exit(-1);
            } else {
                /* 父进程 */
                exit(0);
            }
        } else {
            /* 父进程 */
        }

        this->pTitleBar->m_modeText->setText(tr("standard"));
        this->pTitleBar->m_modeList->slotStandardClick();

        return;
    }

    // 埋点接口
    kabase::BuriedPoint buriedPoint;

    if (label != this->currentModel) {
        if (label == PROGRAMMER) {
            if (DataWarehouse::getInstance()->platform != QString("intel")) {
                // 埋点
                KTrackData* d = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
                kdk_dia_upload_default(d, "Calculator_Mode_Programmer", "ModeSwitcher");
                kdk_dia_data_free(d);
                this->pTitleBar->setFuncLabel(pTitleBar->PROGRAMMER_LABEL);
            }
            setProgrammerUi();
            return;
        }
        if (m_prograModel != nullptr) {
            m_prograModel->handleAC();
            m_prograModel->hide();
        }

        outputWid->show();
        buttonWid->show();

        mainLayout->addWidget(outputWid);
        mainLayout->addWidget(buttonWid);

        QLayoutItem *outputChild = mainOutputLayout->takeAt(0);
        QLayoutItem *buttonChild = mainButtonLayout->takeAt(0);

        mainOutputLayout->removeItem(outputChild);
        mainButtonLayout->removeItem(buttonChild);

        outputChild->widget()->hide();
        buttonChild->widget()->hide();

        this->lab_now->clear();
        this->dis_data.clear();

        if (label == STANDARD) {
            if (DataWarehouse::getInstance()->platform != QString("intel")) {
                // 埋点
                KTrackData* d = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
                kdk_dia_upload_default(d, "Calculator_Mode_Standard", "ModeSwitcher");
                kdk_dia_data_free(d);
                this->pTitleBar->setFuncLabel(pTitleBar->STANDARD_LABEL);
            }

            calData += STANDARD;
            InputProcess::inputFromButton(STANDARD);

            setStandardUi();
            standardOutput->setNowText("0");
            if (this->windowState() == Qt::WindowMaximized && !this->isTabletMode()) {
                m_isSwitchMode = true;
            }
        } else if (label == SCIENTIFIC) {
            if (DataWarehouse::getInstance()->platform != QString("intel")) {
                // 埋点
                KTrackData* d = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
                kdk_dia_upload_default(d, "Calculator_Mode_Scientific", "ModeSwitcher");
                kdk_dia_data_free(d);
                this->pTitleBar->setFuncLabel(pTitleBar->SCIENTIFIC_LABEL);
            }

            calData += SCIENTIFIC;
            InputProcess::inputFromButton(SCIENTIFIC);

            setScientificUi();
	    scientificOutput->setNowText("0");
            if (this->windowState() == Qt::WindowMaximized && !this->isTabletMode()) {
                m_isSwitchMode = true;
            }
        } else if (label == EXCHANGE_RATE) {
            if (DataWarehouse::getInstance()->platform != QString("intel")) {
                // 埋点
                KTrackData* d = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
                kdk_dia_upload_default(d, "Calculator_Mode_Exchange", "ModeSwitcher");
                kdk_dia_data_free(d);
                this->pTitleBar->setFuncLabel(pTitleBar->EXCHANGE_RATE_LABEL);
            }

            calData += EXCHANGE_RATE;
            InputProcess::inputFromButton(EXCHANGE_RATE);

            setToolUi();
        }
	this->lab_now->setText("0");
        this->lab_prepare->setText("");
        /* 显示历史记录 , 最多显示最后 5 条*/
        QString historyText = "";

        int size = disHistory.size();
        int hisIndex = (size - m_hisNumMax >= 0) ? (size - m_hisNumMax) : 0;

        for (int i = hisIndex; i < size; i++) {
            historyText = historyText + disHistory.at(i);
        }

        historyText.replace(SUB, "-");
        /* 去除末尾换行符 */
        historyText.chop(1);
        OutputBase *output = getOutputParent(this->lab_last);
        if (output) {
            output->setHistoryText(historyText);
        } else {
            this->lab_last->setText(historyText);
        }
    }

    return;
}

/* 判断字符串是否为纯数字 */
bool MainWindow::isDigitStr(QString str)
{
    str.replace(SUB, "-");
    QByteArray byteArray = str.toLatin1();
    const char *s = byteArray.data();
    if (*s == '-') {
        s++;
    }
    /* 判断是否为纯数字 */
    while (*s && *s >= '0' && *s <= '9') {
        s++;
    }

    if (*s) {
        return false;
    } else {
        return true;
    }

    return false;
}

/* 响应右键菜单 */
void MainWindow::myCustomContextMenuRequested(const QPoint &pos)
{

	QString clipText = clipboard->text();
	clipText.remove(QRegularExpression("\\s+"));

	/*
	 * 可以粘贴的条件
	 * 1. 剪贴板内容是正确的四则运算表达式
	 * 2. 最后一位不是“%!”
	 */
	if (clipText.isEmpty()) {
		pasteAction->setEnabled(false);
	} else {
		pasteAction->setEnabled(true);
	}

	labelMenu->exec(QCursor::pos());
}

/* 复制计算结果到剪切板 */
void MainWindow::copyCalResult()
{
    if (PROGRAMMER == this->currentModel) {
        clipboard->setText(m_prograModel->getFormulaNow());
        return;
    }
    clipboard->setText(this->lab_now->text().remove(","));

    return;
}

/* 将剪切板内容粘贴到运算式中 */
void MainWindow::pasteToLabNow()
{
    QString clipText = clipboard->text();
    // clipText.remove(QRegularExpression("\\s+"));
    clipText.replace(QRegularExpression("\\s+"), "");
    clipText.replace("×", "*" );
    clipText.replace("÷", "/" );
    clipText.replace("ｰ", "-");
    clipText.remove(",");
    QString labNowStr = lab_now->text().remove(",");
    if (currentModel == PROGRAMMER) {
        labNowStr = m_prograModel->getFormulaNow();
    }

    /*
    * 可以粘贴的条件
    * 1. 剪贴板内容是正确的四则运算表达式
    * 2. 最后一位不是“%!”
    */
    if (!isVaildExpression(clipText) || (QString("%!").contains(labNowStr.right(1)))) {
        pasteError();
        return;
    }

    if (PROGRAMMER == this->currentModel) {
        clipText.replace("-", "s");
        m_prograModel->setFormulaNow(clipText);
        return;
    } else {
        clipText.replace("*", "×");
        clipText.replace("/", "÷");
        clipText.replace("-", "ｰ");
    }

    for (int i = 0; i < clipText.size(); i++) {
        this->btn_merge(QString(clipText[i]));
    }

    /* 当模式为汇率模式时 , 在正常的计算结果基础上 , 在乘以汇率 , 在进行显示 */
    QString label = this->currentModel;
    if (label != STANDARD && label != SCIENTIFIC && inputLongSym == false) {
        toolModelOutput->unitConversion();
    }

    return;
}

void MainWindow::funcListItemClicked(QListWidgetItem *item)
{
    QString label = item->text().trimmed();
    this->funcListWid->hide();

    if (label != this->currentModel) {
        this->pTitleBar->setFuncLabel(label + tr("calculator"));

        if (label == tr("standard") || label == tr("scientific")) {

            mainLayout->removeWidget(outputWid);
            mainLayout->removeWidget(buttonWid);

            this->outputWid->close();
            this->buttonWid->close();

            this->lab_now->clear();
            this->dis_data.clear();

            // 换算器选项失去焦点
            for (int i = 0; i < funcList->funcToolWid->count(); i++) {
                funcList->funcToolWid->item(i)->setSelected(false);
            }

            if (label == tr("standard")) {
                calData += STANDARD;
                InputProcess::inputFromButton(STANDARD);
                setStandardUi();
            } else if (label == tr("scientific")) {
                calData += SCIENTIFIC;
                InputProcess::inputFromButton(SCIENTIFIC);
                InputProcess::inputFromButton(RAD_SYMBOL);
                setScientificUi();
            }
        } else {

            // 计算器选项失去焦点
            for (int i = 0; i < funcList->funcModelWid->count(); i++) {
                funcList->funcModelWid->item(i)->setSelected(false);
            }

            if (label == tr("exchange rate")) {
                mainLayout->removeWidget(outputWid);
                mainLayout->removeWidget(buttonWid);

                this->outputWid->close();
                this->buttonWid->close();

                setToolUi();
            }

            // changeToolUi();
        }
    }
}

void MainWindow::stayTop()
{
    auto states = m_windowHelper->defaultState();
    if (winFlags == NULL) {
        winFlags = windowFlags();
        states |= UkuiWindowHelper::State::KeepAbove;
        pTitleBar->m_pTopButton->setIcon(QIcon::fromTheme("ukui-fixed-symbolic"));
        pTitleBar->m_pTopButton->setFlat(false);
        pTitleBar->m_pTopButton->setToolTip(tr("Cancel StayTop"));
        // pTitleBar->m_pTopButton->setAutoFillBackground(true);
        // pTitleBar->m_pTopButton->setBackgroundRole(QPalette::Window);
    } else {
        winFlags = NULL;
        states &= ~UkuiWindowHelper::State::KeepAbove;
        pTitleBar->m_pTopButton->setIcon(QIcon::fromTheme("ukui-unfixed-symbolic"));
        pTitleBar->m_pTopButton->setFlat(true);
        pTitleBar->m_pTopButton->setToolTip(tr("StayTop"));
        // pTitleBar->m_pTopButton->setBackgroundRole(QPalette::Base);
    }
    m_windowHelper ->setWindowState(states);
    return;
}

/* 汇率换算事件 */
void MainWindow::unitConversion()
{
    toolModelOutput->unitConversion();
    this->lab_prepare->setText(addComma(lab_prepare->text()));
}

/* 设置是否平板模式*/
void MainWindow::setIsTabletMode(bool is) {
    m_isTabletMode = is;
}

/* 是否平板模式 */
bool MainWindow::isTabletMode() {
    return m_isTabletMode;
}

/* 平板模式切换 */
void MainWindow::tabletModeChange(bool isTabletMode)
{
    this->setIsTabletMode(isTabletMode);
    this->setWindowSize();
}

/* 鼠标移动事件 , 用于点击主界面时来隐藏模式切换列表 */
void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        QPoint pos = event->pos();
        QPoint modeListPos = this->pTitleBar->m_modeList->pos();

        /* 当鼠标点击不在模式切换列表中时 , 隐藏模式切STANDARD换列表 */
        if (!(pos.x() > modeListPos.x() && pos.x() < modeListPos.x() + this->pTitleBar->m_modeList->width()
              && pos.y() > modeListPos.y() && pos.y() < modeListPos.y() + this->pTitleBar->m_modeList->height())) {
            this->pTitleBar->m_modeList->hide();
            this->pTitleBar->changeModeIcon();
        }
    }

    return;
}

void MainWindow::changeEvent(QEvent *event)
{
    if (QEvent::WindowStateChange == event->type()) {
        if (DataWarehouse::getInstance()->platform == QString("intel")) {
            Qt::WindowStates state = this->windowState();
            qInfo() << "Window state change: " << state;
            QString label = this->currentModel;
            if (state == Qt::WindowNoState) {
                // 窗口模式
                pTitleBar->setMaxBtnMode(true);
                if (!m_isPcModeStarted) {
                    if (label == STANDARD) {
                        this->setMinimumSize(INTEL_STANDARD_WINDOWW, INTEL_STANDARD_WINDOWH);
                        this->resize(INTEL_STANDARD_WINDOWW, INTEL_STANDARD_WINDOWH);
                    } else {
                        this->setMinimumSize(INTEL_SCIENTIFIC_WINDOWW, INTEL_SCIENTIFIC_WINDOWH);
                        this->resize(INTEL_SCIENTIFIC_WINDOWW, INTEL_SCIENTIFIC_WINDOWH);
                    }
                    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
                    this->move(availableGeometry.width() / 2 - MainWindow::getInstance()->width() / 2,
                               availableGeometry.height() / 2 - MainWindow::getInstance()->height() / 2);
                    m_isPcModeStarted = true;
                } else if (m_isSwitchMode) {
                    if (label == STANDARD) {
                        this->setMinimumSize(INTEL_STANDARD_WINDOWW, INTEL_STANDARD_WINDOWH);
                        this->resize(INTEL_STANDARD_WINDOWW, INTEL_STANDARD_WINDOWH);
                    } else {
                        this->setMinimumSize(INTEL_SCIENTIFIC_WINDOWW, INTEL_SCIENTIFIC_WINDOWH);
                        // this->resize(INTEL_SCIENTIFIC_WINDOWW, INTEL_SCIENTIFIC_WINDOWH);
                    }
                    m_isSwitchMode = false;
                }
            } else if (state == Qt::WindowMaximized) {
                // 最大化模式
                pTitleBar->setMaxBtnMode(false);
            }
        }
    }
}

void MainWindow::setIntelModeChange(deviceMode mode)
{
    if (DataWarehouse::getInstance()->intelPlug == DataWarehouse::StartMode::MODE_MAIN) {
        QString label = this->currentModel;
        if (mode != deviceMode::PCMode) {
            qInfo() << "DeviceMode change...";
            this->pTitleBar->m_min->hide();
            this->pTitleBar->m_max->hide();
            this->pTitleBar->m_close->hide();
            this->setMinimumSize(0, 0);
        } else {
            qInfo() << "DeviceMode change PC";
            this->pTitleBar->m_min->show();
            this->pTitleBar->m_close->show();
            if (this->windowState() == Qt::WindowNoState) {
                // 窗口模式
                pTitleBar->setMaxBtnMode(true);
                QPoint oldPoint = this->pos();
                if (label == STANDARD) {
                    this->setMinimumSize(INTEL_STANDARD_WINDOWW, INTEL_STANDARD_WINDOWH);
                    this->resize(INTEL_STANDARD_WINDOWW, INTEL_STANDARD_WINDOWH);
                } else {
                    this->setMinimumSize(INTEL_SCIENTIFIC_WINDOWW, INTEL_SCIENTIFIC_WINDOWH);
                    this->resize(INTEL_SCIENTIFIC_WINDOWW, INTEL_SCIENTIFIC_WINDOWH);
                }
                this->move(oldPoint.x(), oldPoint.y());
            } else if (this->windowState() == Qt::WindowMaximized) {
                // 最大化模式
                pTitleBar->setMaxBtnMode(false);
            }
            if (label == SCIENTIFIC) {
                this->pTitleBar->m_max->show();
            }
        }
    }
}

bool MainWindow::isVaildExpression(QString expression)
{
    QRegExp numberPattern("^-?ｰ?[0-9]+(\\.[0-9]+)?%?((-|ｰ|[\\+\\*/])[0-9]+(\\.[0-9]+)?%?)*$");
    QRegExp operatorPattern("[-+*/ｰ]+$"); // 匹配仅运算符
    QRegExp tailOpPattern("^-?ｰ?[0-9]+(\\.[0-9]+)?%?((-|ｰ|[\\+\\*/])[0-9]+(\\.[0-9]+)?%?)*(-|ｰ|[\\+\\*/])$");
    if(operatorPattern.exactMatch(expression)){
        return true;
    }
    return numberPattern.exactMatch(expression) || tailOpPattern.exactMatch(expression);
}

void MainWindow::pasteError(void)
{
	if (currentModel == PROGRAMMER) {
		m_prograModel->handlePasteError();
	} else if (currentModel == STANDARD) {
		calData += STANDARD;
		InputProcess::inputFromButton(STANDARD);
		standardOutput->setNowText("0");
	} else if (currentModel == SCIENTIFIC) {
		calData += SCIENTIFIC;
		InputProcess::inputFromButton(SCIENTIFIC);
		scientificOutput->setNowText("0");
	} else if (currentModel == EXCHANGE_RATE) {
		calData += EXCHANGE_RATE;
		InputProcess::inputFromButton(EXCHANGE_RATE);
	}

	lab_now->setText("0");
	lab_prepare->setText(tr("Input error!"));
}
