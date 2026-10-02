#include <QPainter>
#include <QStyleOption>
#include "radiobutton.h"
#include "../globaldata.h"

RadioButton::RadioButton(bool isRect, QWidget *parent)
{
    m_isRect = isRect;
    initUI();
    initStyle();
    initLayout();
    initConn();
}

RadioButton::~RadioButton()
{
}
void RadioButton::initUI()
{
    m_radioBtn = new QRadioButton(this);
    m_labelDevic = new K::TextLabel(this);
    m_labelDeviceNote = new K::TextLabel(this);
    m_labelIcon = new QLabel(this);

    m_layoutHRadio = new QHBoxLayout();
    m_layoutVMain = new QVBoxLayout();
    m_layoutVRadio = new QVBoxLayout();
    m_layoutHMain = new QHBoxLayout();
    return;
}

void RadioButton::initStyle()
{
    if (m_isRect == false) {
        this->setFixedSize(254, 264);
        m_labelIcon->setFixedSize(GlobalData::g_sizeDeviceIconBig);
        m_labelDevic->SetPixelSize(18, true);
        m_labelDeviceNote->SetPixelSize(14);
    } else {
        this->setFixedSize(GlobalData::g_sizeRadioRectWidget);
        m_labelIcon->setFixedSize(GlobalData::g_sizeDeviceIcon);
        m_labelDevic->SetPixelSize(14);
        m_labelDeviceNote->SetPixelSize(12);
    }
    m_radioBtn->setFixedSize(GlobalData::g_sizeRadioBtn);
    adjustBackground();

}

void RadioButton::initLayout()
{
    if (m_isRect == false) {
        m_layoutHRadio->addStretch();
        m_layoutHRadio->addWidget(m_radioBtn);

        m_layoutVMain->addLayout(m_layoutHRadio);
        m_layoutVMain->addWidget(m_labelIcon, 0, Qt::AlignHCenter);
        m_layoutVMain->addSpacing(8);
        m_layoutVMain->addWidget(m_labelDevic, 0, Qt::AlignHCenter);
        m_layoutVMain->addSpacing(4);
        m_layoutVMain->addWidget(m_labelDeviceNote, 0, Qt::AlignHCenter); 
        m_layoutVMain->addStretch();
        m_layoutVMain->setMargin(0);
        m_layoutVMain->setSpacing(0);

        this->setLayout(m_layoutVMain);
    } else {
        m_layoutVRadio->addWidget(m_labelDevic);
        m_layoutVRadio->addWidget(m_labelDeviceNote);

        m_layoutHMain->addSpacing(32);
        m_layoutHMain->addWidget(m_radioBtn);
        m_layoutHMain->addSpacing(12);
        m_layoutHMain->addWidget(m_labelIcon);
        m_layoutHMain->addSpacing(24);
        m_layoutHMain->addLayout(m_layoutVRadio);
        m_layoutHMain->addStretch();
        m_layoutHMain->setMargin(0);
        m_layoutHMain->setSpacing(0);

        this->setLayout(m_layoutHMain);
    }
}

void RadioButton::initConn()
{
}

void RadioButton::adjustBackground()
{

}

void RadioButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    //设置背景颜色
    QColor color = this->palette().color(QPalette::Window);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    painter.setBrush(brush);
    painter.setPen(Qt::transparent);
    QStyleOption opt;
    opt.init(this);
    painter.drawRoundedRect(opt.rect, 12, 12); // 绘制圆角
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);
}

void RadioButton::setLabelIcon(QIcon icon)
{
    if (m_isRect == false) {
        m_labelIcon->setPixmap(icon.pixmap(icon.actualSize(GlobalData::g_sizeDeviceIconBig)));
    } else {
        m_labelIcon->setPixmap(icon.pixmap(icon.actualSize(GlobalData::g_sizeDeviceIcon)));
    }
}

void RadioButton::setLabelText(SysType type)
{
    switch (type)
    {
    case SysType::NewSys:
        m_labelDevic->SetText(GlobalData::g_textNewSystem);
        m_labelDeviceNote->SetText(GlobalData::g_textNewSystemNote);
        break;  
    case SysType::OldSys:
        m_labelDevic->SetText(GlobalData::g_textOldSystem);
        m_labelDeviceNote->SetText(GlobalData::g_textOldSystemNote);
        break;  
    case SysType::WinSys:
        m_labelDevic->SetText(GlobalData::g_textWinSystem);
        m_labelDeviceNote->SetText(GlobalData::g_textWinSystemNote);
        break;  
    default:
        m_labelDevic->SetText(GlobalData::g_textLinuxSystem);
        m_labelDeviceNote->SetText(GlobalData::g_textLinuxSystemNote);
        break;
    }
}
