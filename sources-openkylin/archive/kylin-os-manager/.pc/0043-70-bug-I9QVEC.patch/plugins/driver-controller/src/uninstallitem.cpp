#include "uninstallitem.h"

#include <QDebug>
#include <QVersionNumber>
#include <gsettingmonitor.h>

#include "include/common.h"
#include "include/globalsignal.h"

UninstallItem::UninstallItem(HardWareInfo hardwareinfo, QWidget *parent)
        : QWidget(parent),
          m_hardwareinfo(hardwareinfo),
          m_Icon(new kdk::KPressButton()),
          m_deviceType(new QLabel()),
          m_deviceName(new CustomLabel()),
          m_driverVersion(new QLabel()),
          m_uninstallButton(new QPushButton()),
          m_progressBar(new QProgressBar()),
          m_btnStackWidget(new QStackedWidget),
          m_uninstallingLayout(new QHBoxLayout()),
          m_uninstallingWidget(new QWidget()),
          m_mainLayout(new QHBoxLayout()),
          m_typeNameLayout(new QVBoxLayout()),
          m_typeNameWidget(new QWidget()) {
    initGui();
    initConnect();
}

void UninstallItem::initGui() {
    this->setFixedHeight(ITEMHEIGET);

    m_Icon->setButtonType(kdk::KPressButton::CircleType);
    m_Icon->setFixedSize(DEVICEICONBUTTONSIZE);
    if (!m_Icon->isChecked()) {
        m_Icon->click();
    }
    switch (m_hardwareinfo.type) {
        case DeviceType::SCANNER:
            m_deviceType->setText(tr("Scanner"));
            m_Icon->setIcon(QIcon::fromTheme("ukui-scanner-symbolic"));
            break;
        case DeviceType::GraphicCard:
            m_deviceType->setText(tr("GraphicCard"));
            m_Icon->setIcon(QIcon::fromTheme("ukui-graphics-card-symbolic"));
            break;
        case DeviceType::SoundCard:
            m_deviceType->setText(tr("SoundCard"));
            m_Icon->setIcon(QIcon::fromTheme("audio-volume-high-symbolic"));
            break;
        case DeviceType::NetworkCard:
            m_deviceType->setText(tr("NetworkCard"));
            m_Icon->setIcon(QIcon::fromTheme("ukui-zs-network-card-symbolic"));
            break;
        case DeviceType::Bluetooth:
            m_deviceType->setText(tr("Bluetooth"));
            m_Icon->setIcon(QIcon::fromTheme("bluetooth-symbolic"));
            break;
        case DeviceType::PRINTER:
            m_deviceType->setText(tr("Printer"));
            m_Icon->setIcon(QIcon::fromTheme("printer-symbolic"));
            break;
        case DeviceType::InputDev:
            m_deviceType->setText(tr("Input Device"));
            m_Icon->setIcon(QIcon::fromTheme("ukui-zs-sidebar-yjjc-symbolic"));
            break;
        case DeviceType::VideoOrAudio:
            m_deviceType->setText(tr("Video Device"));
            m_Icon->setIcon(QIcon::fromTheme("camera-video-symbolic"));
            break;
        default:
            m_deviceType->setText(tr("Other Device"));
            m_Icon->setIcon(QIcon::fromTheme("ukui-bf-operation-log-symbolic"));
            break;
    }
    m_Icon->setIconSize(DEVICEICONSIZE);

    m_deviceName->setFixedWidth(200);
    m_deviceName->setText(m_hardwareinfo.model);

    // 判断是否有驱动可以卸载
    if (!m_hardwareinfo.upgradeable){
        QString name = m_hardwareinfo.driver_now;
        QStringList namelist;
        if (m_hardwareinfo.driver_now.contains(";")) {
            namelist = name.split(";");
            namelist.removeLast();
            name = " ";
            for (int i = 0; i < namelist.length(); ++i) {
                name = namelist[i] + " ";
            }
        }

        m_driverVersion->setText(name);
    }
    m_uninstallButton->setFixedSize(UPDATEBTNSIZE);
    m_uninstallButton->setText(tr("Uninstall"));


    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedSize(PROGRESSBARSIZE);

    m_typeNameLayout->addWidget(m_deviceType);
    m_typeNameLayout->addWidget(m_deviceName);
    m_typeNameLayout->setContentsMargins(0, 0, 0, 0);
    m_typeNameLayout->setSpacing(0);
    m_typeNameWidget->setLayout(m_typeNameLayout);

    m_uninstallingLayout->setContentsMargins(0, 0, 0, 0);
    m_uninstallingLayout->setSpacing(0);
    m_uninstallingLayout->addWidget(m_progressBar);
    m_uninstallingWidget->setLayout(m_uninstallingLayout);

    m_btnStackWidget->setContentsMargins(0, 0, 0, 0);
    m_btnStackWidget->addWidget(m_uninstallButton);
    m_btnStackWidget->addWidget(m_uninstallingWidget);
    m_btnStackWidget->setCurrentWidget(m_uninstallButton);

    m_mainLayout->setContentsMargins(0, 16, 0, 16);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_Icon);
    m_mainLayout->addSpacing(12);
    m_mainLayout->addWidget(m_typeNameWidget);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_driverVersion, 2, Qt::AlignCenter);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_btnStackWidget);

    this->setLayout(m_mainLayout);
}

void UninstallItem::initConnect() {
    connect(m_uninstallButton, &QAbstractButton::clicked, this, &UninstallItem::uninstallBtnClicked);

    connect(g_signal, &GlobalSignal::themeChangeSignal, this, &UninstallItem::themeChangeSlot);
}

void UninstallItem::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    int labelWidth = 200 + this->width() - 678;
    if (labelWidth <= 400) {
        m_deviceName->setFixedWidth(labelWidth);
    } else {
        m_deviceName->setFixedWidth(400);
    }
}

DriverInfo UninstallItem::getGpuDriver() {
    // 多个驱动信息
    QList<DriverInfo> driverNameList = m_hardwareinfo.driverNameList;
    DriverInfo driver = driverNameList.at(0);
    // 挑选最新驱动包
    for (int i = 1; i < driverNameList.length(); ++i) {
        QVersionNumber v1 = QVersionNumber::fromString(driver.version);
        QVersionNumber v2 = QVersionNumber::fromString(driverNameList.at(i).version);
        if (v2 > v1) {
            driver = driverNameList.at(i);
        }
    }
    return driver;
}

void UninstallItem::uninstallBtnClicked() {
    m_btnStackWidget->setCurrentWidget(m_uninstallingWidget);
    QString driver = m_hardwareinfo.driver_now;
    if(driver.contains(";")){
        driver.remove(";");
    }
    emit g_signal->uninstallDriver(this, driver);
}

void UninstallItem::themeChangeSlot() {
    float fontSize = kdk::GsettingMonitor::getSystemFontSize().toFloat();
    QFont font;
    font.setPointSize(fontSize);
    m_deviceType->setFont(font);
    m_deviceName->setFont(font);
    m_driverVersion->setFont(font);
    m_uninstallButton->setFont(font);
}

QString UninstallItem::getDevName() {
    return m_hardwareinfo.model;
}


DeviceType UninstallItem::getDevType() {
    return m_hardwareinfo.type;
}

void UninstallItem::changeProgress(int percent) {
    m_progressBar->setValue(percent);
}

