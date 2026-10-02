#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QPen>
#include <QApplication>
#include <QMouseEvent>

#include "radio_widget.h"

RadioWidget::RadioWidget(bool isVLayout, QWidget *parent) : QWidget(parent), m_isVLayout(isVLayout)
{
    initUI();
    initStyle();
    initLayout();
    initConn();
}

RadioWidget::~RadioWidget()
{

}

void RadioWidget::setLabelIcon(QIcon icon)
{
    if (m_isVLayout) {
        m_labelIcon->setPixmap(icon.pixmap(icon.actualSize(GlobalData::g_sizeDeviceIconBig)));
    } else {
        m_labelIcon->setPixmap(icon.pixmap(icon.actualSize(GlobalData::g_sizeDeviceIcon)));
    }
    
}

void RadioWidget::setLabelText(SysType type)
{
    switch (type)
    {
    case SysType::NewSys:
        m_labelName->SetText(GlobalData::g_textNewSystem);
        m_labelInfo->SetText(GlobalData::g_textNewSystemNote);
        break;
    case SysType::OldSys:
        m_labelName->SetText(GlobalData::g_textOldSystem);
        m_labelInfo->SetText(GlobalData::g_textOldSystemNote);
        break;
    case SysType::LinuxSys:
        m_labelName->SetText(GlobalData::g_textLinuxSystem);
        m_labelInfo->SetText(GlobalData::g_textLinuxSystemNote);
        break;
    case SysType::WinSys:
        m_labelName->SetText(GlobalData::g_textWinSystem);
        m_labelInfo->SetText(GlobalData::g_textWinSystemNote);
        break;
    default:
        break;
    }
}

bool RadioWidget::getCilckStyle()
{
    return m_isClicked;
}

void RadioWidget::setClickStyle()
{
    clickWidget();
}

void RadioWidget::initUI()
{
    m_btn = new QRadioButton(this);
    m_labelIcon = new QLabel(this);
    m_labelName = new TextLabel(this);
    m_labelInfo = new TextLabel(this);
    return;
}

void RadioWidget::initStyle()
{
    if (m_isVLayout) {
        this->setFixedSize(254, 264);
        m_labelIcon->setFixedSize(144, 144);
        m_labelName->SetPixelSize(18, true);
        m_labelInfo->SetFontColor(QColor(166, 166, 166));
    } else {
        this->setFixedSize(530, 98);
        // this->setMinimumSize(524, 98);
        // this->setMaximumSize(800, 98);
        m_labelIcon->setFixedSize(66, 66);
        m_labelInfo->SetFontColor(QColor(166, 166, 166));
        m_labelInfo->SetPixelSize(12);
    }
}

void RadioWidget::initLayout()
{
    if (m_isVLayout) {
        QVBoxLayout *vlayout = new QVBoxLayout();
        QHBoxLayout *hlayout = new QHBoxLayout();
        hlayout->addStretch();
        hlayout->addWidget(m_btn);
        hlayout->setMargin(0);
        hlayout->setSpacing(0);
        
        vlayout->addLayout(hlayout);
        vlayout->addWidget(m_labelIcon, 0, Qt::AlignHCenter);
        vlayout->addSpacing(8);
        vlayout->addWidget(m_labelName, 0, Qt::AlignHCenter);
        vlayout->addSpacing(4);
        vlayout->addWidget(m_labelInfo, 0, Qt::AlignHCenter);
        vlayout->addSpacing(30);
        vlayout->addStretch();
        vlayout->setMargin(0);
        vlayout->setSpacing(0);
        vlayout->setContentsMargins(4, 4, 4, 4);
        this->setLayout(vlayout);
    } else {
        QHBoxLayout *hlayout = new QHBoxLayout();
        QVBoxLayout *vlayout = new QVBoxLayout();
        
        vlayout->addStretch();
        vlayout->addWidget(m_labelName);
        vlayout->addSpacing(4);
        vlayout->addWidget(m_labelInfo);
        vlayout->addStretch();
        vlayout->setMargin(0);
        vlayout->setSpacing(0);

        hlayout->addSpacing(32);
        hlayout->addWidget(m_btn, 0, Qt::AlignVCenter);
        hlayout->addSpacing(24);
        hlayout->addWidget(m_labelIcon, 0, Qt::AlignVCenter);
        hlayout->addSpacing(24);
        hlayout->addLayout(vlayout);
        hlayout->addSpacing(10);
        hlayout->addStretch();
        hlayout->setMargin(0);
        hlayout->setSpacing(0);
        this->setLayout(hlayout);
    }
}

void RadioWidget::initConn()
{
    connect(m_btn, &QRadioButton::clicked, this, &RadioWidget::clickWidget);
}

void RadioWidget::clickWidget()
{
    if (m_isClicked) {
        emit sigCannel();
        m_btn->setChecked(false);
        m_isClicked = m_btn->isChecked();
    } else {
        emit sigClicked();
        m_btn->setChecked(true);
        m_isClicked = m_btn->isChecked();
    }
    this->update();
}

void RadioWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton) {
        clickWidget();
    }
}

void RadioWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    //设置背景颜色
    QColor color = QApplication::palette().window().color();
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    painter.setBrush(brush);
    painter.setPen(Qt::transparent);
    QStyleOption opt;
    QColor highLightColor = opt.palette.highlight().color();
    opt.init(this);
    painter.drawRoundedRect(opt.rect, 16, 16); // 绘制圆角
    // 点击后设置高亮
    if(m_isClicked) {
        QPen pen(highLightColor, 2);
        painter.setPen(pen);
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 12, 12); // 绘制圆角
    } else {
        painter.setPen(Qt::transparent);
        painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 12, 12); // 绘制圆角
    }
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);
}
