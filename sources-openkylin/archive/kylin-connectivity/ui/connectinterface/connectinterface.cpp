#include "connectinterface.h"

#include "generatetools.h"

const int BUTTON_W = 80;
const int BUTTON_H = 30;
const int TABBAR_W = 314;
const int TABBAR_H = 48;

ConnectInterface::ConnectInterface(QWidget *parent) : QWidget(parent)
{
    initUI();
}

void ConnectInterface::initUI()
{
    m_backBtn = new QPushButton(this);
    // m_backBtn->setFixedSize(BUTTON_W, BUTTON_H);
    m_backBtn->setText(tr("Back"));  // Back
    m_backBtn->setIcon(QIcon(":/connectinterface/back.svg"));
    m_backBtn->setFlat(true);
    m_backBtn->setFocusPolicy(Qt::NoFocus);
    m_backBtn->setProperty("useButtonPalette", true);
    connect(m_backBtn, &QPushButton::clicked, this, [=]() {
        m_searchWin->setSearchBtnState(false);
        Q_EMIT sigBackBtnClicked();
    });

    m_tabBar = new KTabBar(KTabBarStyle::SegmentDark, this);
    connect(m_tabBar, &KTabBar::tabBarClicked, this, &ConnectInterface::slotTabBarClicked);
    m_tabBar->setMinimumWidth(TABBAR_W);
    m_tabBar->setFixedHeight(TABBAR_H);
    m_tabBar->addTab(tr("Nearby device"));
    m_tabBar->addTab(tr("Connect Phone"));
    m_tabBar->addTab(tr("Device Code"));

    m_searchWin = new SearchDeviceWin(this);
    connect(m_searchWin, &SearchDeviceWin::sigContinueSearch, this, &ConnectInterface::sigContinueSearch);
    connect(m_searchWin, &SearchDeviceWin::sigItemInfo, this, &ConnectInterface::sigConnectAddress);
    m_mobileConnectWin = new MobileConnectWin(this);
    connect(m_mobileConnectWin, &MobileConnectWin::sigUSBconnectBtnClicked, this,
            &ConnectInterface::sigUSBconnectBtnClicked);
    m_deviceCodeWidget = new DeviceCodeWidget(this);
    connect(m_deviceCodeWidget, &DeviceCodeWidget::sigDeviceCode, this,
            [=](QString code) { Q_EMIT sigConnectAddress(GenerateTools::getCodetoIp(code)); });

    m_stackedWin = new QStackedWidget(this);
    m_stackedWin->addWidget(m_searchWin);
    m_stackedWin->addWidget(m_mobileConnectWin);
    m_stackedWin->addWidget(m_deviceCodeWidget);

    m_backgroundWin = new BackgroundWin(this);
    m_backgroundWin->setBackground(false);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_stackedWin);
    m_backgroundWin->setLayout(layout);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setMargin(0);
    hLayout->addWidget(m_backBtn);
    hLayout->addStretch();
    hLayout->addWidget(m_tabBar);
    hLayout->addStretch();
    hLayout->addSpacing(BUTTON_W);

    QVBoxLayout *mainVLayout = new QVBoxLayout;
    mainVLayout->setSpacing(0);
    mainVLayout->setContentsMargins(8, 8, 8, 8);
    mainVLayout->addLayout(hLayout);
    mainVLayout->addSpacing(12);
    mainVLayout->addWidget(m_backgroundWin);

    setLayout(mainVLayout);
}

void ConnectInterface::setTheme(PublicAttributes::Theme theme)
{
    m_backgroundWin->setTheme(theme);
    m_mobileConnectWin->setTheme(theme);
    m_searchWin->setTheme(theme);
}

void ConnectInterface::setInterFaceCodeInfo(const QString wifiInfo)
{
    if (wifiInfo.isEmpty()) {
        m_searchWin->showNoNetWork();
    } else {
        m_searchWin->showLoad();
    }
    m_mobileConnectWin->setInterFaceCodeInfo(wifiInfo);
}

void ConnectInterface::slotTabBarClicked(int index)
{
    m_stackedWin->setCurrentIndex(index);
    switch (index) {
        case TabBarIndex::NearbyDevice:
            // 埋点
            m_searchWin->setSearchBtnState(true);
            break;
        case TabBarIndex::ConnectPhone:
            // 埋点
            m_searchWin->setSearchBtnState(false);
            break;
        case TabBarIndex::DeviceCode:
            // 埋点
            m_searchWin->setSearchBtnState(false);
            m_deviceCodeWidget->setInput();
            break;
    }
}

void ConnectInterface::showLoad()
{
    m_searchWin->showLoad();
}

void ConnectInterface::setItemInfo(const QMap<QString, KCommon::DiscoveryDeviceInfo> &map)
{
    m_searchWin->showLoad();
    m_searchWin->setItemInfo(map);
}

void ConnectInterface::setSearchBtnState(bool state)
{
    if (state && m_stackedWin->currentIndex() == TabBarIndex::NearbyDevice) {
        m_searchWin->setSearchBtnState(true);
    } else {
        m_searchWin->setSearchBtnState(false);
    }
}

void ConnectInterface::changeFontSize(double fontSize)
{
    m_searchWin->changeFontSize(fontSize);
    m_mobileConnectWin->changeFontSize(fontSize);
}