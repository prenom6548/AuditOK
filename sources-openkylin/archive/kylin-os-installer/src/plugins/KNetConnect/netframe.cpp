#include "netframe.h"
#include "kylin-nm/kylin-nm-interface.h"
#include <kylin-nm/kylinnetworkmanager.h>
#include <QPluginLoader>
#include <QDebug>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QButtonGroup>
#include <QRadioButton>
#include <QStackedWidget>
#include <QProcess>
#define  TAB_WIDTH 300

NetFrame::NetFrame(QWidget *parent) : MiddleFrameManager(parent)
{
    initUI();//完成本插件主窗口m_Widget的绘画
    this->initNextBnt();//实例化下一步按钮控件
    nextBtnAvailable();//是否使能下一步按钮
    initAllConnect();//初始化信号与槽函数
    translateStr();//国际化语言显示
    this->setFocus();
}

void NetFrame::initUI()
{

    thread = new QThread;
    manager = new KyNetworkManager();
    manager->moveToThread(thread);
    connect(thread, &QThread::started, manager, &KyNetworkManager::kylinNetworkManagerInit);
    connect(thread, &QThread::finished, manager, &KyNetworkManager::deleteLater);
    connect(thread, &QThread::finished, [=](){
        qDebug() << "NetConnect thread quit";
    });
    thread->start();

    m_mainLayout = new QVBoxLayout();

    m_Widget->setLayout(m_mainLayout);
    // 连接网络
    m_titleLabel = new QLabel(tr("Connect network"), this);
    m_titleLabel->setStyleSheet("QLabel { font-size:30px; color:#FFFFFF; }");
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_titleLabel);
    // 你可以在进入系统前配置好网络环境，以获得更完整的体验。
    m_subtitleLabel = new QLabel(tr("You can configure your network environment before entering the system to get a more complete experience."), this);
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    m_subtitleLabel->setStyleSheet("QLabel {font-size: 20px; color:#FFFFFF}");
    m_mainLayout->addWidget(m_subtitleLabel);
    
    // 创建两个按钮
    // 有线网络
    wiredBtn = new QPushButton(tr("Wired network"));
    // 无线局域网
    wifiBtn = new QPushButton(tr("Wi-Fi"));

    wiredBtn->setStyleSheet("background: #3790FA; color: white;");
    wifiBtn->setStyleSheet("background: #E6E6E6; color: black;");
    wiredBtn->setFixedWidth(150);
    wifiBtn->setFixedWidth(150);

    QHBoxLayout *tabLayout = new QHBoxLayout();
    tabLayout->addWidget(wiredBtn);
    tabLayout->addWidget(wifiBtn);
    tabLayout->setContentsMargins(0, 0, 0, 0);
    tabLayout->setAlignment(Qt::AlignCenter);
    m_mainLayout->addLayout(tabLayout);

    // 创建 QStackedWidget
    QVBoxLayout *stackLayout = new QVBoxLayout();
    m_pluginStackedWidget = new QStackedWidget(this);
    m_pluginStackedWidget->setStyleSheet("background: transparent;");
    m_pluginStackedWidget->setFixedWidth(400);
    stackLayout->addWidget(m_pluginStackedWidget);
    stackLayout->setAlignment(Qt::AlignCenter);
    m_mainLayout->addLayout(stackLayout);

    // 创建有线和无线插件的小部件
    QWidget *wiredWidget = loadWiredPlugin();
    QWidget *wirelessWidget = loadWirelessPlugin();
    // 设置背景透明
    wiredWidget->setStyleSheet("color: white; height: 700px;");
    wirelessWidget->setStyleSheet("background: transparent; color: white; height: 700px;");
    // 将小部件添加到 QStackedWidget
    m_pluginStackedWidget->addWidget(wiredWidget);
    m_pluginStackedWidget->addWidget(wirelessWidget);
}

QWidget* NetFrame::loadWiredPlugin() {
    return loadPlugin("/usr/lib/kylin-nm/libnetconnect.so");
}

QWidget* NetFrame::loadWirelessPlugin() {
    return loadPlugin("/usr/lib/kylin-nm/libwlanconnect.so");
}

QWidget* NetFrame::loadPlugin(const QString &pluginPath) {
    QPluginLoader loader(pluginPath);
    if (loader.load()) {
        QObject *plugin = loader.instance();
        if (plugin) {
            Interface *pluginInstance = qobject_cast<Interface *>(plugin);
            if (!pluginInstance) {
                return nullptr; // 返回 nullptr
            }
            // OOBE
            pluginInstance->setPluginType(OOBE);
            pluginInstance->setParentWidget(this);
            widget = pluginInstance->pluginUi();
            widget->setFixedHeight(700);
            return widget; // 返回 widget
        } else {
            qDebug() << "Load Failed: " << loader.errorString() << "\n";
        }
    } else {
        qDebug() << "Load Failed: " << loader.errorString() << "\n";
    }
    return nullptr; // 返回 nullptr
}

void NetFrame::currentRowChangeSlot(int row)
{
    m_pluginStackedWidget->setCurrentIndex(row);
}

void NetFrame::initAllConnect()
{
    // 连接按钮以切换插件
    QObject::connect(wiredBtn, &QPushButton::clicked, [&]() {
        wiredBtn->setStyleSheet("background: #3790FA; color: white;");
        wifiBtn->setStyleSheet("background: #E6E6E6; color: black;");
        emit switchToPlugin(0); 
    });

    QObject::connect(wifiBtn, &QPushButton::clicked, [&]() {
        wiredBtn->setStyleSheet("background: #E6E6E6; color: black;");
        wifiBtn->setStyleSheet("background: #3790FA; color: white;");
        emit switchToPlugin(1); // 发射信号切换到无线插件
    });
    // 连接切换插件信号与页面切换
    connect(this, &NetFrame::switchToPlugin, m_pluginStackedWidget, &QStackedWidget::setCurrentIndex);
    // connect(manager, &KyNetworkManager::deviceStateChange, this, &NetFrame::onDeviceStatusChanged);
}

void NetFrame::onDeviceStatusChanged(QString deviceName, KyDeviceState newstate, KyDeviceState oldstate,
                                        NetworkManager::Device::StateChangeReason reason)
{
    QProcess process;
    process.start("nmcli -t -f STATE general");
    process.waitForFinished();
    QString output = process.readAllStandardOutput().trimmed();

    if (output == "connected (site only)" || output == "connected") {
        m_nextBtn->setEnabled(true);
    } else {
        m_nextBtn->setEnabled(false);
    }
}

void NetFrame::nextBtnAvailable()
{
    m_nextBtn->setEnabled(true);
}

void NetFrame::clickNextButton()
{
    emit signalStackPageChanged(1);//跳转显示下个插件界面
}

void NetFrame::translateStr()
{
    m_titleLabel->setText(tr("Connect network"));
    m_subtitleLabel->setText(tr("You can configure your network environment before entering the system to get a more complete experience."));
    wiredBtn->setText(tr("Wired network"));
    wifiBtn->setText(tr("Wi-Fi"));
    m_nextBtn->setText(tr("Next"));//下一步
}

void NetFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {//响应国际化语言切换
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}
