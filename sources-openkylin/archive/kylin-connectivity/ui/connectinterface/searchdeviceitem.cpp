#include "searchdeviceitem.h"

#include <QEvent>
#include <QDebug>

SearchDeviceItem::SearchDeviceItem(QPushButton *parent) : QPushButton(parent)
{
    initUI();
}

SearchDeviceItem::~SearchDeviceItem() {}

void SearchDeviceItem::initUI()
{
    this->installEventFilter(this);
    QHBoxLayout *mainHLayout = new QHBoxLayout();

    QVBoxLayout *labelVLayout = new QVBoxLayout();
    //项按钮
    m_itemIconLabel = new QLabel(this);
    m_itemIconLabel->setStyleSheet("background:transparent");
    m_itemIconLabel->setFixedSize(48, 48);
    m_itemIconLabel->setMargin(0);
    //用户名
    m_itemNameLabel = new QLabel(this);
    m_itemNameLabel->setFixedWidth(142);
    m_itemNameLabel->setStyleSheet("background:transparent");
    m_itemNameLabel->setMargin(0);
    //手机型号
    //    m_itemModelLabel = new QLabel(this);
    //    m_itemModelLabel->setStyleSheet("color:#8F9399;font-size:12px");

    labelVLayout->addStretch();
    labelVLayout->addWidget(m_itemNameLabel);
    //    labelVLayout->addWidget(m_itemModelLabel);
    labelVLayout->addStretch();

    mainHLayout->addSpacing(16);
    mainHLayout->addWidget(m_itemIconLabel);
    mainHLayout->addSpacing(8);
    mainHLayout->addLayout(labelVLayout);
    mainHLayout->addStretch();
    mainHLayout->setSpacing(0);
    mainHLayout->setMargin(0);
    this->setLayout(mainHLayout);

    this->setProperty("useButtonPalette", true);
}

void SearchDeviceItem::setItemIcon(QString path)
{
    m_itemIconLabel->setPixmap(QIcon(path).pixmap(48, 48));
}

void SearchDeviceItem::setItemName(QString name)
{
    m_name = name;
    m_itemNameLabel->setText(name);
}

void SearchDeviceItem::setItemDeviceInfo(KCommon::DiscoveryDeviceInfo deviceInfo)
{
    m_deviceInfo = deviceInfo;
    // m_itemModelLabel->setText(deviceInfo.deviceMoudle);
}

void SearchDeviceItem::setItemIP(QString ip)
{
    m_itemIP = ip;
}

void SearchDeviceItem::setTheme(PublicAttributes::Theme theme)
{
    switch (theme) {
    case PublicAttributes::Theme::Light: {
        setStyleSheet("QPushButton "
                      "{border:0px;border-radius:8px;background-color:#FFFFFF;}"
                      "QPushButton:Hover"
                      "{border:0px;border-radius:8px;background-color:#E6F1FE;}"
                      "QPushButton:Pressed "
                      "{border:0px;border-radius:8px;background-color:#C4DEFD;}");
        break;
    }
    case PublicAttributes::Theme::Dark: {
        setStyleSheet("QPushButton "
                      "{border:0px;border-radius:8px;background-color:#5B636E;}"
                      "QPushButton:Hover"
                      "{border:0px;border-radius:8px;background-color:#373737;}"
                      "QPushButton:Pressed "
                      "{border:0px;border-radius:8px;background-color:#404953;}");
        break;
    }
    }
}

void SearchDeviceItem::changeFontSize(double fontSize)
{
    QFont font;
    font.setPointSizeF(fontSize);

    m_itemNameLabel->setFont(font);
    QFontMetrics fontmts = QFontMetrics(font);
    int dif = fontmts.width(m_name) - m_itemNameLabel->width();
    if (dif > 0) {
        // 文本比Label宽，省略处理
        m_itemNameLabel->setToolTip(m_name);       // 设置悬浮提示
        m_itemNameLabel->setToolTipDuration(2000); // 提示时间
        m_itemNameLabel->setText(fontmts.elidedText(m_name, Qt::ElideRight, m_itemNameLabel->width()));
    } else {
        m_itemNameLabel->setToolTip("");
        m_itemNameLabel->setText(m_name);
    }
}

QString SearchDeviceItem::getItemName()
{
    return m_name;
}

KCommon::DiscoveryDeviceInfo SearchDeviceItem::getItemDeviceInfo()
{
    return m_deviceInfo;
}

QString SearchDeviceItem::getItemIp()
{
    return m_itemIP;
}
