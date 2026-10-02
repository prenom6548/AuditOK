#include "widgetconnect.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QThread>
#include <QLayout>
#include <QFormLayout>
#include <QDebug>
#include <gsettingmonitor.h>
#include <QApplication>
#include <QScreen>
#include "kom-configure.h"
#include "kom-buriedpoint.h"
WidgetConnect::WidgetConnect(QWidget *parent) : WidgetCommon(parent)
{
    createAppLogic();
    initPrompt();
    initUI();
    initConnect();
    initMigration();
    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    m_widgetMigration->move(availableGeometry.width() / 2 - m_widgetMigration->width() / 2,
                            availableGeometry.height() / 2 - m_widgetMigration->height() / 2);

#ifdef DEBUG_MODE
    m_lineEditIP->setText("172.17.126.135");
    m_lineEditWorkGroup->setText("zxx");
    m_lineEditUserName->setText("zxx");
    m_lineEditPassWord->setText("k123123k");
#endif
}

void WidgetConnect::connetcResult(int errCode, QVariant var)
{
    switch (errCode) {
    case SMB_CONNECT_SUCCESS: {
        m_connbtn->setText(Constant::g_connectSuccessText);
        hide();
        QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
        m_widgetMigration->move(availableGeometry.width() / 2 - m_widgetMigration->width() / 2,
                                availableGeometry.height() / 2 - m_widgetMigration->height() / 2);
        m_widgetMigration->show();
        emit startLoad();
        break;
    }
    case SMB_ERROR_1:
    case SMB_ERROR_2: {
        m_connbtn->setText(Constant::g_connectFailText);
        if (m_lineEditIP->text() != "" && m_lineEditWorkGroup->text() != "" && m_lineEditUserName->text() != "" && m_lineEditPassWord->text() != "") {
            m_connbtn->setEnabled(true);
        }else {
            m_connbtn->setEnabled(false);
        }
        break;
    }
    }
}

void WidgetConnect::onConnbtnClick()
{
    kom::Configure configure;
    int debugLevel = configure.value(CONFIG_CHEST_WIN_DATA_MIGRATION, CONFIG_CHEST_WIN_DATA_MIGRATION_DEBUG_LEVEL, false).toInt();
    if (debugLevel < 0 || debugLevel > 10) {
        debugLevel = 0;
    }
    qDebug() << "SMB日志级别：" << debugLevel;
    m_connbtn->setText(Constant::g_connectingText);
    m_connbtn->setEnabled(false);
    SmbConf sc;
    sc.ip = m_lineEditIP->text().toLocal8Bit();
    sc.workgroup = m_lineEditWorkGroup->text().toLocal8Bit();
    sc.username = m_lineEditUserName->text().toLocal8Bit();
    sc.password = m_lineEditPassWord->text().toLocal8Bit();
    sc.debugLevel = debugLevel;
    emit connectSmbServer(sc);

    kom::BuriedPoint::uploadMessage(kom::PluginsWinDataMigration, "EstablishConnection");
}

void WidgetConnect::textChanged(const QString &str)
{
    Q_UNUSED(str);
    if (m_lineEditIP->text().isEmpty() || m_lineEditWorkGroup->text().isEmpty() || m_lineEditUserName->text().isEmpty()
        || m_lineEditPassWord->text().isEmpty()) {
        m_connbtn->setEnabled(false);
    } else {
        m_connbtn->setEnabled(true);
    }
}

void WidgetConnect::createAppLogic()
{
    m_appLogic = new AppLogic();
    m_itemModel = m_appLogic->getStandardItemModel();
    m_cancelHandle = m_appLogic->getCancelHandle();
    QThread *thread = new QThread();
    m_appLogic->moveToThread(thread);
    thread->start();
}

void WidgetConnect::initConnect()
{
    qRegisterMetaType<SmbConf>("SmbConf");
    qRegisterMetaType<SmbUrlInfoList>("SmbUrlInfoList");
    connect(m_lineEditIP, &QLineEdit::textChanged, this, &WidgetConnect::textChanged);
    connect(m_lineEditWorkGroup, &QLineEdit::textChanged, this, &WidgetConnect::textChanged);
    connect(m_lineEditUserName, &QLineEdit::textChanged, this, &WidgetConnect::textChanged);
    connect(m_lineEditPassWord, &kdk::KPasswordEdit::textChanged, this, &WidgetConnect::textChanged);
    connect(m_connbtn, &QPushButton::clicked, this, &WidgetConnect::onConnbtnClick);
    connect(this, &WidgetConnect::connectSmbServer, m_appLogic, &AppLogic::connectSmbServer);
    connect(m_appLogic, &AppLogic::connetcResult, this, &WidgetConnect::connetcResult);
    connect(this, &WidgetConnect::startLoad, m_appLogic, &AppLogic::startLoad);
}

void WidgetConnect::initPrompt()
{
    kom::Configure configure;
    QString firstOpen;
    firstOpen = configure.value(CONFIG_CHEST_WIN_DATA_MIGRATION, CONFIG_CHEST_WIN_DATA_MIGRATION_FIRST_OPEN, "").toString();
    if (firstOpen != "false") {
        m_showPrompt = true;
        configure.setValue(CONFIG_CHEST_WIN_DATA_MIGRATION, CONFIG_CHEST_WIN_DATA_MIGRATION_FIRST_OPEN, "false");
    } else {
        m_showPrompt = false;
    }

    //    m_showPrompt = false;
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/kylin-os-manager/";
    QString configDir = configPath + m_projectName + "/";
    QString configFileName = m_projectName + ".conf";
    if (QFile::exists(configDir + configFileName)) {
        return;
    }
    if (!QDir().mkpath(configDir)) {
        qDebug() << "创建配置文件目录失败:" << configDir;
        return;
    }
    QFile file(configDir + configFileName);
    if (!file.open(QFile::WriteOnly | QFile::Text | QIODevice::Append)) {
        qDebug() << "创建配置文件失败:" << file.fileName();
        return;
    }
    file.close();
}

void WidgetConnect::initUI()
{
    setFixedWidth(312 * kdk::GsettingMonitor::getSystemFontSize().toDouble() / 10);
    setFixedHeight(340);
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this, [=]() {
        setFixedWidth(312 * kdk::GsettingMonitor::getSystemFontSize().toDouble() / 10);
    });

    QVBoxLayout *vl = new QVBoxLayout;
    vl->setMargin(0);
    vl->setSpacing(0);
    if (m_showPrompt) {
        setFixedHeight(368);
        QWidget *basePrompt = new QWidget(this);
        basePrompt->setObjectName("basePrompt");
        basePrompt->installEventFilter(this);
        basePrompt->setMinimumHeight(36);
        basePrompt->setBackgroundRole(QPalette::Window);
        basePrompt->setAutoFillBackground(true);
        QHBoxLayout *hl0 = new QHBoxLayout;
        hl0->addSpacing(16);
        QLabel *labelIconWarning = new QLabel(this);
        labelIconWarning->setPixmap(QPixmap(":/res/Frame.svg"));
        hl0->addWidget(labelIconWarning);
        QLabel *labelText = new QLabel(this);
        labelText->setText(Constant::g_howToUseImportantText);
        labelText->setAlignment(Qt::AlignVCenter);
        connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemFontSizeChange, this, [=]() {
            QFont ft = labelText->font();
            ft.setPointSizeF(kdk::GsettingMonitor::getSystemFontSize().toDouble() / 10 * 9);
            labelText->setFont(ft);
        });
        QFont ft = labelText->font();
        ft.setPointSizeF(kdk::GsettingMonitor::getSystemFontSize().toDouble() / 10 * 9);
        labelText->setFont(ft);
        hl0->addSpacing(8);
        hl0->addWidget(labelText);
        QPushButton *labelIconArrow = new QPushButton(this);
        labelIconArrow->setFixedSize(16, 16);
        labelIconArrow->setIcon(QIcon(":/res/right.svg"));
        labelIconArrow->setFlat(true);
        labelIconArrow->setAttribute(Qt::WA_TransparentForMouseEvents);
        hl0->addStretch(9);
        hl0->addWidget(labelIconArrow);
        hl0->setMargin(0);
        hl0->setSpacing(0);
        basePrompt->setLayout(hl0);
        vl->addWidget(basePrompt);
    } else {
        vl->addSpacing(16);
    }

    QFormLayout *fl = new QFormLayout;
    fl->setContentsMargins(24, 16, 24, 0);
    fl->setSpacing(8);
    QLabel *labelIP = new QLabel(this);
    labelIP->setText(Constant::g_winIPText);
    m_lineEditIP = new QLineEdit(this);
    fl->addRow(labelIP, m_lineEditIP);

    QLabel *labelUsergroup = new QLabel(this);
    labelUsergroup->setText(Constant::g_winWorkGroupText);
    m_lineEditWorkGroup = new QLineEdit(this);
    fl->addRow(labelUsergroup, m_lineEditWorkGroup);

    QLabel *labelUsername = new QLabel(this);
    labelUsername->setText(Constant::g_winUserNameText);
    m_lineEditUserName = new QLineEdit(this);
    fl->addRow(labelUsername, m_lineEditUserName);

    QLabel *labelPassword = new QLabel(this);
    labelPassword->setText(Constant::g_winPassWordText);
    m_lineEditPassWord = new kdk::KPasswordEdit(this);
    m_lineEditPassWord->setFocusPolicy(Qt::StrongFocus);
    fl->addRow(labelPassword, m_lineEditPassWord);
    vl->addLayout(fl);

    vl->addSpacing(16);

    QHBoxLayout *hl5 = new QHBoxLayout;
    m_connbtn = new QPushButton(this);
    m_connbtn->setEnabled(false);
    m_connbtn->setProperty("isImportant", true);
    m_connbtn->setText(Constant::g_creatConnectText);
    m_connbtn->setMinimumWidth(120);
    hl5->addStretch(9);
    hl5->addWidget(m_connbtn);
    hl5->addStretch(9);
    vl->addLayout(hl5);

    vl->addStretch(9);
    baseBar()->setLayout(vl);

    setTabOrder(m_lineEditIP, m_lineEditWorkGroup);
    setTabOrder(m_lineEditWorkGroup, m_lineEditUserName);
    setTabOrder(m_lineEditUserName, m_lineEditPassWord);
}

void WidgetConnect::initMigration()
{
    if (m_widgetMigration) {
        return;
    }
    m_widgetMigration = new WidgetMigration;
    m_widgetMigration->setModel(m_itemModel);
    m_widgetMigration->setAppLogic(m_appLogic);
    m_widgetMigration->setCancelHandle(m_cancelHandle);
}

bool WidgetConnect::eventFilter(QObject *target, QEvent *event)
{
    if (target->objectName() == "basePrompt" && event->type() == QEvent::MouseButtonPress) {
        DialogHowToUse::getInstance()->updateAndActive();
    }
    return WidgetCommon::eventFilter(target, event);
}
