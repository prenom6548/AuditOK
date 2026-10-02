#include "deviceitem.h"
#include <QVersionNumber>
#include <QDebug>
#include <gsettingmonitor.h>
#include "include/common.h"
#include "include/globalsignal.h"

DeviceItem::DeviceItem(HardWareInfo hardwareinfo, QWidget *parent)
        : QWidget(parent),
          m_hardwareinfo(hardwareinfo),
          m_check(new QCheckBox()),
          m_Icon(new kdk::KPressButton()),
          m_deviceType(new QLabel()),
          m_deviceName(new CustomLabel()),
          m_driverVersion(new QLabel()),
          m_updateButton(new QPushButton()),
          m_updateText(new QLabel()),
          m_progressBar(new QProgressBar()),
          m_btnStackWidget(new QStackedWidget),
          m_installingLayout(new QHBoxLayout()),
          m_installingWidget(new QWidget()),
          m_mainLayout(new QHBoxLayout()),
          m_typeNameLayout(new QVBoxLayout()),
          m_typeNameWidget(new QWidget()) {
    initGui();
    initConnect();
}

void DeviceItem::initGui() {
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

    // 判断是否有驱动可以更新
    if (m_hardwareinfo.upgradeable) {
        DriverInfo driver = getGpuDriver();
        QString name = driver.driverName;
        QString version = driver.version;
        QStringList namelist;
        QStringList versionlist;
        if (name.contains(";")) {
            namelist = name.split(";");
            versionlist = version.split(" ");
            namelist.removeLast();
            versionlist.removeLast();
            name = " ";
            version = " ";
            for (int i = 0; i < namelist.length(); ++i) {
                name = namelist[i] + " " + versionlist[i] + " ";
            }
        }
        name = tr("Version: ") + versionlist[0] + "  ";

        // 设置驱动大小
        if (driver.size != "") {
            double size = QString(driver.size).toDouble() / 1024;
            if (size > 1024) {
                size = size / 1024;
                name = name + QString(QString::number(size, 'f', 2) + " MB");
                m_driverVersion->setText(name);
            } else {
                name = name + QString(QString::number(size, 'f', 2) + " KB");
                m_driverVersion->setText(name);
            }
        }

        m_updateText->hide();
    } else if(m_hardwareinfo.driver_now != "") {
        QString name;
        if (m_hardwareinfo.driver_version_now != ""){
            name = tr("Version: ") + m_hardwareinfo.driver_now + " " + m_hardwareinfo.driver_version_now;
        }else if (m_hardwareinfo.driver_now != ""){
            name = tr("Version: ") + m_hardwareinfo.driver_now;
        }
        QString version = m_hardwareinfo.driver_version_now;
        QStringList namelist;
        QStringList versionlist;
        if (m_hardwareinfo.driver_now.contains(";")) {
            name = m_hardwareinfo.driver_now;
            namelist = name.split(";");
            versionlist = version.split(";");
            namelist.removeLast();
            versionlist.removeLast();
            name = " ";
            for (int i = 0; i < namelist.length(); ++i) {
                name = namelist[i] + " " + versionlist[i] + " ";
            }
        }

        m_driverVersion->setText(name);
        m_btnStackWidget->hide();
        m_updateText->setText(tr("Installed"));
        m_check->setEnabled(false);
    }else{
        m_driverVersion->setText(tr("No available drivers were detected"));
        m_updateButton->setText(tr("Feedback"));
        m_btnStackWidget->hide();   // 未实现反馈联动，暂时隐藏反馈按钮
        connect(m_updateButton, &QPushButton::clicked, this, &DeviceItem::feedBackInfoSlot);
        m_check->setEnabled(false);
    }
    m_updateButton->setFixedSize(UPDATEBTNSIZE);
    if (m_hardwareinfo.upgradeable && m_hardwareinfo.driver_now == "") {
        m_updateButton->setText(tr("Install"));
        connect(m_updateButton, &QPushButton::clicked, this, &DeviceItem::installBtnClicked);
    } else if (m_hardwareinfo.upgradeable && m_hardwareinfo.driver_now != ""){
        m_updateButton->setText(tr("Update"));
        connect(m_updateButton, &QPushButton::clicked, this, &DeviceItem::installBtnClicked);
    }

    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedSize(PROGRESSBARSIZE);

    m_typeNameLayout->addWidget(m_deviceType);
    m_typeNameLayout->addWidget(m_deviceName);
    m_typeNameLayout->setContentsMargins(0, 0, 0, 0);
    m_typeNameLayout->setSpacing(0);
    m_typeNameWidget->setLayout(m_typeNameLayout);

    m_installingLayout->setContentsMargins(0, 0, 0, 0);
    m_installingLayout->setSpacing(0);
    m_installingLayout->addWidget(m_progressBar);
    m_installingWidget->setLayout(m_installingLayout);

    m_btnStackWidget->setContentsMargins(0, 0, 0, 0);
    m_btnStackWidget->addWidget(m_updateButton);
    m_btnStackWidget->addWidget(m_installingWidget);
    m_btnStackWidget->setCurrentWidget(m_updateButton);

    m_mainLayout->setContentsMargins(0, 16, 0, 16);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_check);
    m_mainLayout->addSpacing(14);
    m_mainLayout->addWidget(m_Icon);
    m_mainLayout->addSpacing(12);
    m_mainLayout->addWidget(m_typeNameWidget);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_driverVersion, 2, Qt::AlignLeft);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_updateText);
    m_mainLayout->addWidget(m_btnStackWidget);

    this->setLayout(m_mainLayout);
}

void DeviceItem::initConnect() {
    connect(m_check, &QCheckBox::clicked, this, &DeviceItem::setChecked);

    connect(g_signal, &GlobalSignal::themeChangeSignal, this, &DeviceItem::themeChangeSlot);
}


DriverInfo DeviceItem::getGpuDriver() {
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

void DeviceItem::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event);
    int labelWidth = 200 + this->width() - 678;
    if (labelWidth <= 400) {
        m_deviceName->setFixedWidth(labelWidth);
    } else {
        m_deviceName->setFixedWidth(400);
    }
}

void DeviceItem::setChecked() {
    if (m_check->isChecked()) {
        m_isChecked = true;
        if (m_hardwareinfo.upgradeable && m_hardwareinfo.driver_now == "") {
            emit g_signal->checkAllChildenCheckboxState(0);
        } else if (m_hardwareinfo.upgradeable && m_hardwareinfo.driver_now != "") {
            emit g_signal->checkAllChildenCheckboxState(1);
        }
    } else {
        m_isChecked = false;
        if (m_hardwareinfo.upgradeable && m_hardwareinfo.driver_now == "") {
            emit g_signal->uncheckCheckBox(0);
        } else if (m_hardwareinfo.upgradeable && m_hardwareinfo.driver_now != "") {
            emit g_signal->uncheckCheckBox(1);
        }
    }
}

void DeviceItem::themeChangeSlot() {
    float fontSize = kdk::GsettingMonitor::getSystemFontSize().toFloat();
    QFont font;
    font.setPointSize(fontSize);
    m_deviceType->setFont(font);
    m_deviceName->setFont(font);
    m_driverVersion->setFont(font);
    m_updateText->setFont(font);
    m_updateButton->setFont(font);
}

void DeviceItem::checkTheBox() {
    m_check->setCheckState(Qt::Checked);
    m_isChecked = true;
}

void DeviceItem::unCheckTheBox() {
    m_check->setCheckState(Qt::Unchecked);
    m_isChecked = false;
}

void DeviceItem::installBtnClicked() {
    m_btnStackWidget->setCurrentWidget(m_installingWidget);
    QString driver = getGpuDriver().driverName;
    if(driver.contains(";")){
        driver.remove(";");
    }
    emit g_signal->installDriver(this, driver);
}

void DeviceItem::changeProgress(int percent) {
    m_progressBar->setValue(percent);
    if(percent == 100){
        m_btnStackWidget->setCurrentWidget(m_updateButton);
        m_btnStackWidget->hide();
        m_updateText->show();
        m_updateText->setText(tr("Installed"));
        m_check->setCheckState(Qt::Unchecked);
        m_check->setEnabled(false);
    }
}

QString DeviceItem::getDevName() {
    return m_hardwareinfo.model;
}

QString DeviceItem::getDevPID(){
    return m_hardwareinfo.PID;
}

QString DeviceItem::getDevVID(){
    return m_hardwareinfo.VID;
}

DeviceType DeviceItem::getDevType() {
    return m_hardwareinfo.type;
}

void DeviceItem::clickInstall() {
    m_updateButton->click();
}

void DeviceItem::feedBackInfoSlot() {
    //反馈，跳转到服务与支持
}
