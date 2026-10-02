#include "computepowerlist.h"

#include <kysdk/applications/klabel.h>

#include <QMessageBox>

ComputePowerList::ComputePowerList(QWidget *parent) : QWidget(parent) {
    initMainLayout();
    initIpLayout();
}

void ComputePowerList::initMainLayout() {
    m_mainVLayout = new QVBoxLayout();
    m_mainVLayout->setContentsMargins(0, 0, 0, 0);
    m_mainVLayout->setSpacing(1);
    this->setLayout(m_mainVLayout);
}

void ComputePowerList::initIpLayout() {
    m_ipVLayout = new QVBoxLayout();
    m_ipVLayout->setContentsMargins(0, 0, 0, 0);
    m_ipVLayout->setSpacing(1);
}

void ComputePowerList::clearIpLayout() {
    while (QLayoutItem *item = m_ipVLayout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            delete widget;
        }
        delete item;
    }
}

void ComputePowerList::showErrorMessageBox(const QString &errorStr) {
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(errorStr);

    QPushButton *sureButton = msgBox.addButton(QObject::tr("Ok"), QMessageBox::AcceptRole);
    msgBox.setDefaultButton(sureButton);
    msgBox.exec();
}

void ComputePowerList::showErrorMessageBoxByErrorCode(ComputePowerShare::ErrorCode errorCode) {
    if (errorCode == ComputePowerShare::ErrorCode::NotSupportComputingPowerSharing) {
        showErrorMessageBox(
            tr("The current device has no available computing power and cannot share computing power."));
    } else if (errorCode == ComputePowerShare::ErrorCode::TimeOutError) {
        showErrorMessageBox(
            tr("The network connection is unavailable. Please check your network settings and try again"));
    } else {
        showErrorMessageBox(tr("System encountered an exception, please try again later"));
    }
}

void ComputePowerList::initNoDeviceAccessFrame() {
    if (m_noDeviceAccessFrame) {
        return;
    }
    m_noDeviceAccessFrame = createFrame();
    m_noDeviceAccessFrame->setAccessibleName("kylin-ai-subsystem-modelconfig_QFrame_noDeviceAccessFrame");

    QHBoxLayout *frameMainHLayout = new QHBoxLayout();
    frameMainHLayout->setAlignment(Qt::AlignHCenter);
    frameMainHLayout->setSpacing(8);
    m_noDeviceAccessFrame->setLayout(frameMainHLayout);

    QLabel *icon = new QLabel();
    QIcon selectIcon = QIcon::fromTheme("dialog-warning");
    icon->setPixmap(selectIcon.pixmap(selectIcon.actualSize(QSize(16, 16))));
    icon->setAccessibleName("kylin-ai-subsystem-modelconfig_QLabel_icon");

    kdk::KLabel *tipLabel = new kdk::KLabel();
    tipLabel->setText(tr("Currently, no devices are connected in the environment"));
    tipLabel->setMaximumWidth(470);
    tipLabel->setAccessibleName("kylin-ai-subsystem-modelconfig_KLabel_tipLabel");

    frameMainHLayout->addWidget(icon);
    frameMainHLayout->addWidget(tipLabel);
    m_mainVLayout->addWidget(m_noDeviceAccessFrame);
}

void ComputePowerList::showNoDeviceAccessFrame() {
    initNoDeviceAccessFrame();
    m_noDeviceAccessFrame->show();
}

void ComputePowerList::hideNoDeviceAccessFrame() {
    if (m_noDeviceAccessFrame) {
        m_noDeviceAccessFrame->hide();
    }
}

UkccFrame *ComputePowerList::createFrame() {
    UkccFrame *frame = new UkccFrame();
    frame->setFixedSize(QSize(496, 60));
    return frame;
}

QString ComputePowerList::getLocalIp() {
    m_localIp = m_computePowerInterface.getLocalIp();
    return m_localIp;
}
