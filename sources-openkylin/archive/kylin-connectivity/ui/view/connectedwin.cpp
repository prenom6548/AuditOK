#include "connectedwin.h"

#include <QVariant>
#include <QRgb>

const int HEIGHT = 164;
const int DEVICEICON_W = 96;
const int DEVICEICON_H = 96;
const int STATEICON_W = 8;
const int STATEICON_H = 8;
const int SCREENBTN_W = 104;
const int SCREENBTN_H = 48;

ConnectedWin::ConnectedWin(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::ClickFocus);

    setAttribute(Qt::WA_StyledBackground);
    setObjectName("wid");
    setStyleSheet("QWidget#wid{background-color: rgba(98, 142, 255, 0.1);}");

    m_deviceIcon = new QLabel(this);
    m_deviceIcon->setFixedSize(DEVICEICON_W, DEVICEICON_H);
    m_deviceIcon->setPixmap(QIcon(":/devicetype/pc.svg").pixmap(QSize(DEVICEICON_W, DEVICEICON_H)));

    m_deviceName = new QLabel(this);
    QFont font;
    font.setPointSizeF(24);
    font.setBold(true);
    m_deviceName->setFont(font);
    m_deviceName->setText("Kylin");

    m_stateIcon = new QLabel(this);
    m_stateIcon->setFixedSize(STATEICON_W, STATEICON_H);
    m_stateIcon->setPixmap(QIcon(":/view/ellipse.svg").pixmap(QSize(STATEICON_W, STATEICON_H)));

    m_connectState = new QLabel(this);
    m_connectState->setText(tr("CONNECTED"));

    m_screenBtn = new QPushButton(this);
    connect(m_screenBtn, &QPushButton::clicked, this, &ConnectedWin::onClicked);
    m_screenBtn->setMinimumSize(SCREENBTN_W, SCREENBTN_H);
    m_screenBtn->setText("SCREEN");
    m_screenBtn->setProperty("isImportant", true);

    m_disconnectBtn = new DisconnectButton(this);
    connect(m_disconnectBtn, &DisconnectButton::clicked, this, &ConnectedWin::onClicked);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->setMargin(0);
    hLayout->setSpacing(0);
    hLayout->addWidget(m_deviceName);
    hLayout->addStretch();

    QHBoxLayout *hLayout1 = new QHBoxLayout;
    hLayout1->setMargin(0);
    hLayout1->setSpacing(0);
    hLayout1->addWidget(m_stateIcon);
    hLayout1->addSpacing(4);
    hLayout1->addWidget(m_connectState);
    hLayout1->addStretch();

    QVBoxLayout *vLayout = new QVBoxLayout;
    vLayout->setMargin(0);
    vLayout->setSpacing(0);
    vLayout->addStretch();
    vLayout->addLayout(hLayout);
    vLayout->addSpacing(5);
    vLayout->addLayout(hLayout1);
    vLayout->addStretch();

    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addSpacing(72);
    layout->addWidget(m_deviceIcon);
    layout->addSpacing(29);
    layout->addLayout(vLayout);
    layout->addStretch();
    layout->addWidget(m_screenBtn);
    layout->addSpacing(16);
    layout->addWidget(m_disconnectBtn);
    layout->addSpacing(72);
    QWidget *titleWin = new QWidget(this);
    titleWin->setFixedHeight(HEIGHT);
    titleWin->setLayout(layout);

    m_backgroundWin = new BackgroundWin(this);
    m_backgroundWin->setBackground(true);

    m_layout = new QHBoxLayout;
    m_layout->setSpacing(0);
    m_layout->setContentsMargins(24, 10, 24, 10);
    m_backgroundWin->setLayout(m_layout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(titleWin);
    mainLayout->addWidget(m_backgroundWin);
    setLayout(mainLayout);
}

void ConnectedWin::setTheme(PublicAttributes::Theme theme)
{
    m_backgroundWin->setTheme(theme);
    m_disconnectBtn->setTheme(theme);
}

void ConnectedWin::setDeviceName(QString deviceName)
{
    m_deviceName->setText(deviceName);
}

void ConnectedWin::setDeviceType(PublicAttributes::DeviceType type, bool isInitiator)
{
    switch (type) {
        case PublicAttributes::DeviceType::Pc: {
            m_deviceIcon->setPixmap(QIcon(":/devicetype/pc.svg").pixmap(QSize(DEVICEICON_W, DEVICEICON_H)));
            m_screenBtn->setText(tr("ComputerScreen"));
        } break;

        case PublicAttributes::DeviceType::Android: {
            m_deviceIcon->setPixmap(QIcon(":/devicetype/phone.svg").pixmap(QSize(DEVICEICON_W, DEVICEICON_H)));
            if (isInitiator) {
                m_screenBtn->setText(tr("MobileScreen"));
            } else {
                m_screenBtn->setText(tr("ComputerScreen"));
            }
        } break;
    }
    m_ScreenText = m_screenBtn->text();
}

void ConnectedWin::addWidget(QWidget *win)
{
    m_layout->addWidget(win);
}

void ConnectedWin::restartScreen()
{
    m_isScreenBtnClicked = true;
    Q_EMIT sigBtnClicked(ConnectedWin::BtnType::Screen);
    m_screenBtn->setText(tr("ExitScreen"));
}

void ConnectedWin::restoreScreenButton()
{
    if (m_isScreenBtnClicked) {
        m_isScreenBtnClicked = false;
        m_screenBtn->setText(m_ScreenText);
    }
}

void ConnectedWin::changeScreenButton(bool isShow)
{
    if (isShow) {
        m_screenBtn->show();
    } else {
        m_screenBtn->hide();
    }
}

void ConnectedWin::changeFontSize(double fontSize)
{
    QFont font;
    font.setPointSizeF(fontSize + 14);
    font.setBold(true);
    m_deviceName->setFont(font);

    font.setPointSizeF(fontSize);
    font.setBold(false);
    m_connectState->setFont(font);
    m_screenBtn->setFont(font);
}

void ConnectedWin::onClicked()
{
    QPushButton *btn = qobject_cast<QPushButton *>(sender());
    if (btn == m_screenBtn) {
        if (m_isScreenBtnClicked) {
            m_isScreenBtnClicked = false;
            Q_EMIT sigBtnClicked(ConnectedWin::BtnType::ExitScreen);
            m_screenBtn->setText(m_ScreenText);
        } else {
            m_isScreenBtnClicked = true;
            Q_EMIT sigBtnClicked(ConnectedWin::BtnType::Screen);
            m_screenBtn->setText(tr("ExitScreen"));
        }
    } else if (btn == m_disconnectBtn) {
        Q_EMIT sigBtnClicked(ConnectedWin::BtnType::Disconnect);
    }
}