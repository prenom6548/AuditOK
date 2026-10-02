#include "deviceitem.h"
#include <QVersionNumber>
#include <QMovie>
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
      m_deviceName(new K::TextLabel()),
      m_driverVersion(new K::TextLabel()),
      m_updateButton(new QPushButton()),
      m_uninstallButton(new QPushButton()),
      m_movieLabel(new QLabel()),
      m_movieLayout(new QHBoxLayout),
      m_movieWidget(new QWidget),
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

    QMovie *movieLoading = new QMovie(":/loading.gif");
    movieLoading->setScaledSize(QSize(18, 18));
    m_movieLabel->setFixedWidth(20);
    m_movieLabel->setMovie(movieLoading);
    movieLoading->start();
    m_movieLayout->addWidget(m_movieLabel, 0, Qt::AlignCenter);
    m_movieWidget->setLayout(m_movieLayout);

    switch (m_hardwareinfo.type) {
    case DeviceType::SCANNER:
        m_deviceType->setText(tr("Scanner"));
        m_Icon->setIcon(getIcon("ukui-scanner-symbolic", DEVICEICONBUTTONSIZE));
        break;
    case DeviceType::GraphicCard:
        m_deviceType->setText(tr("GraphicCard"));
        m_Icon->setIcon(getIcon("ukui-graphics-card-symbolic", DEVICEICONBUTTONSIZE));
        break;
    case DeviceType::SoundCard:
        m_deviceType->setText(tr("SoundCard"));
        m_Icon->setIcon(getIcon("audio-volume-high-symbolic", DEVICEICONBUTTONSIZE));
        break;
    case DeviceType::NetworkCard:
        m_deviceType->setText(tr("NetworkCard"));
        m_Icon->setIcon(getIcon("ukui-zs-network-card-symbolic", DEVICEICONBUTTONSIZE));
        break;
    case DeviceType::Bluetooth:
        m_deviceType->setText(tr("Bluetooth"));
        m_Icon->setIcon(getIcon("bluetooth-symbolic", DEVICEICONBUTTONSIZE));
        break;
    case DeviceType::PRINTER:
        m_deviceType->setText(tr("Printer"));
        m_Icon->setIcon(getIcon("printer-symbolic", DEVICEICONBUTTONSIZE));
        break;
    case DeviceType::InputDev:
        m_deviceType->setText(tr("Input Device"));
        m_Icon->setIcon(getIcon("ukui-zs-sidebar-yjjc-symbolic", DEVICEICONBUTTONSIZE));
        break;
    case DeviceType::VideoOrAudio:
        m_deviceType->setText(tr("Video Device"));
        m_Icon->setIcon(getIcon("camera-video-symbolic", DEVICEICONBUTTONSIZE));
        break;
    default:
        m_deviceType->setText(tr("Other Device"));
        m_Icon->setIcon(getIcon("ukui-bf-operation-log-symbolic", DEVICEICONBUTTONSIZE));
        break;
    }
    m_Icon->setIconSize(DEVICEICONSIZE);

    m_deviceName->setFixedWidth(200);
    m_deviceName->SetText(m_hardwareinfo.model);
    m_driverVersion->setMaximumWidth(230);

    // 判断是否有驱动可以更新
    if (m_hardwareinfo.upgradeable) {
        QList<DriverInfo> driverinfos = m_hardwareinfo.driverNameList;
        QString name = "";
        QStringList namelist;
        QStringList versionlist;
        QStringList sizelist;
        for (int i = 0; i < driverinfos.length(); ++i) {
            namelist.append(driverinfos[i].driverName);
            versionlist.append(driverinfos[i].version);

            double size = driverinfos[i].size.toDouble();  // 包大小
            if (driverinfos[i].size.toDouble() > 1048576) {
                size = size / 1048576 ;
                sizelist.append(QString(QString::number(size, 'f', 2) + " MB"));
            } else {
                size = size / 1024 ;
                sizelist.append(QString(QString::number(size, 'f', 2) + " KB"));
            }
        }
        name = tr("Version:");
        if(namelist.length() == versionlist.length()){
            for (int i = 0; i < namelist.length(); ++i) {
                if(i < sizelist.length()){
                    name += namelist[i] + " " + versionlist[i] + " " + sizelist[i] + " ";
                }else{
                    name += namelist[i] + " "+ versionlist[i] + " ";
                }
            }
        }else {
            for (int i = 0; i < namelist.length(); ++i) {
                name += namelist[i] + " ";
            }
        }
        m_driverVersion->SetText(name);

        m_updateText->hide();
    } else if(m_hardwareinfo.driver_now != "") {
        QString name;
        QStringList drivers;
        QStringList driver_versions;
        if (m_hardwareinfo.driver_version_now != ""){
            if(m_hardwareinfo.driver_version_now.contains(";")){
                drivers = m_hardwareinfo.driver_now.split(";");
                driver_versions = m_hardwareinfo.driver_version_now.split(";");
            }
            name = tr("Version:");
            for (int i = 0; i < drivers.length(); ++i) {
                name += drivers[i] + " " + driver_versions[i] + " ";
            }
        }else if (m_hardwareinfo.driver_now != ""){
            name = tr("Version:") + m_hardwareinfo.driver_now;
        }
        m_driverVersion->SetText(name);
        if (m_hardwareinfo.type == DeviceType::SCANNER || m_hardwareinfo.type == DeviceType::PRINTER){
            m_uninstallButton->setText(tr("Uninstall"));
            m_btnlist.append(m_uninstallButton);
            m_expandButton = new CustomExpandButton(m_btnlist);
            connect(m_uninstallButton, &QPushButton::clicked, this, &DeviceItem::uninstallBtnClicked);
        }else{
            m_btnStackWidget->hide();
            m_updateText->setText(tr("Installed"));
            m_btnlist.append(m_uninstallButton);
            m_expandButton = new CustomExpandButton(m_btnlist);
        }
        m_check->setEnabled(false);
    }else{
        m_driverVersion->setText(tr("No available drivers were detected"));
        m_updateButton->setText(tr("Feedback"));
        m_btnlist.append(m_updateButton);
        m_expandButton = new CustomExpandButton(m_btnlist);
        m_btnStackWidget->hide();   // 未实现反馈联动，暂时隐藏反馈按钮
        connect(m_updateButton, &QPushButton::clicked, this, &DeviceItem::feedBackInfoSlot);
        m_check->setEnabled(false);
    }
    m_updateButton->setFixedSize(UPDATEBTNSIZE);
    if (m_hardwareinfo.upgradeable && m_hardwareinfo.driver_now == "") {
        m_updateButton->setText(tr("Install"));
        m_btnlist.append(m_updateButton);
        m_expandButton = new CustomExpandButton(m_btnlist);
        connect(m_updateButton, &QPushButton::clicked, this, &DeviceItem::installBtnClicked);
    } else if (m_hardwareinfo.upgradeable && m_hardwareinfo.driver_now != ""){
        m_updateButton->setText(tr("Update"));
        connect(m_updateButton, &QPushButton::clicked, this, &DeviceItem::installBtnClicked);
        m_uninstallButton->setText(tr("Uninstall"));
        connect(m_uninstallButton, &QPushButton::clicked, this, &DeviceItem::uninstallBtnClicked);
        m_btnlist.append(m_updateButton);
        m_btnlist.append(m_uninstallButton);
        m_expandButton = new CustomExpandButton(m_btnlist);
    }

    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setTextVisible(false);
    m_progressBar->setFixedSize(PROGRESSBARSIZE);

    m_typeNameLayout->addWidget(m_deviceType);
    m_typeNameLayout->addWidget(m_deviceName);
    m_typeNameLayout->setContentsMargins(0, 12, 0, 12);
    m_typeNameLayout->setSpacing(0);
    m_typeNameWidget->setLayout(m_typeNameLayout);

    m_installingLayout->setContentsMargins(0, 0, 0, 0);
    m_installingLayout->setSpacing(0);
    m_installingLayout->addWidget(m_progressBar);
    m_installingWidget->setLayout(m_installingLayout);

    m_btnStackWidget->setContentsMargins(0, 0, 0, 0);
    m_btnStackWidget->addWidget(m_expandButton);
    m_btnStackWidget->addWidget(m_installingWidget);
    m_btnStackWidget->addWidget(m_movieWidget);
    m_btnStackWidget->setCurrentWidget(m_expandButton);

    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->addWidget(m_check);
    m_mainLayout->addSpacing(14);
    m_mainLayout->addWidget(m_Icon);
    m_mainLayout->addSpacing(12);
    m_mainLayout->addWidget(m_typeNameWidget);
    m_mainLayout->addStretch();
    m_mainLayout->addSpacing(24);
    m_mainLayout->addWidget(m_driverVersion, 2, Qt::AlignLeft);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_updateText);
    m_mainLayout->addSpacing(20);
    m_mainLayout->addWidget(m_btnStackWidget);

    this->setLayout(m_mainLayout);
}

void DeviceItem::initConnect() {
    connect(m_check, &QCheckBox::clicked, this, &DeviceItem::setChecked);
    connect(g_signal, &GlobalSignal::themeChangeSignal, this, &DeviceItem::themeChangeSlot);
    connect(m_Icon, &kdk::KPressButton::clicked, [=](){m_Icon->setChecked(true);});
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
    if (labelWidth <= 200) {
        m_deviceName->setFixedWidth(labelWidth);
    } else {
        m_deviceName->setFixedWidth(200);
    }
}

QIcon DeviceItem::getIcon(QString iconName, QSize iconSize)
{
    QString path = "/usr/share/kylin-os-manager/icons/";

    QIcon icon;
    if (iconName.indexOf("/") != -1) {
        icon.addFile(iconName, iconSize);
        return icon;
    }

    icon = QIcon::fromTheme(iconName);
    if (!icon.isNull())
        return icon;

    QString selfIconPath = path + iconName + ".svg";
    icon.addFile(selfIconPath, iconSize);
    return icon;
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
    if(g_signal->onUninstalling == true){
        QString result = tr("Uninstalling drivers. Please wait for the uninstallation to complete before clicking again.");
        QMessageBox::information(this, tr("Uninstall Failed"), result);
        return;
    }else if(g_signal->onInstalling == true){
        QString result = tr("Installing drivers. Please wait for the installation to complete before clicking again.");
        QMessageBox::information(this, tr("Install Failed"), result);
        return;
    }

    m_btnStackWidget->setCurrentWidget(m_installingWidget);
    QList<DriverInfo> drivers = m_hardwareinfo.driverNameList;
    QStringList driver;
    for (int i = 0; i < drivers.length(); ++i) {
        QString driver1 = drivers[i].driverName.trimmed() + "=" + drivers[i].version.trimmed();
        driver.append(driver1);
    }
    emit g_signal->installDriver(this, driver);
}

void DeviceItem::uninstallBtnClicked() {
    if(g_signal->onUninstalling == true){
        QString result = tr("Uninstalling drivers. Please wait for the uninstallation to complete before clicking again.");
        QMessageBox::information(this, tr("Uninstall Failed"), result);
        return;
    }else if(g_signal->onInstalling == true){
        QString result = tr("Installing drivers. Please wait for the installation to complete before clicking again.");
        QMessageBox::information(this, tr("Install Failed"), result);
        return;
    }

    m_btnStackWidget->setCurrentWidget(m_installingWidget);
    QString driver = m_hardwareinfo.driver_now;
    QStringList drivers;
    if(driver.contains(";")){
        drivers = driver.split(";");
    }
    drivers.removeLast();
    emit g_signal->uninstallUpdateDriver(this, drivers);
}

void DeviceItem::changeProgress(int percent) {
    m_progressBar->setValue(percent);
}

void DeviceItem::installFailed() {
    m_progressBar->setValue(0);
    m_btnStackWidget->setCurrentWidget(m_expandButton);
}

void DeviceItem::installSucceed()
{
    m_btnStackWidget->setCurrentWidget(m_expandButton);
    m_btnStackWidget->hide();
    m_updateText->show();
    m_updateText->setText(tr("Installed"));
    m_check->setCheckState(Qt::Unchecked);
    m_check->setEnabled(false);
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

QString DeviceItem::getBtnText() {
    // 用于获取m_updateBtton的文案内容以确定item在哪一类中
    return m_updateButton->text();
}

void DeviceItem::toLoadingPage() {
    m_btnStackWidget->setCurrentWidget(m_movieWidget);
}

HardWareInfo DeviceItem::getHardwareInfo()
{
    return m_hardwareinfo;
}

QString DeviceItem::getDriverName() {
    return m_hardwareinfo.driver_now.split("=").first();
}

void DeviceItem::setToInstallPage() {
    m_btnStackWidget->setCurrentWidget(m_expandButton);
}



