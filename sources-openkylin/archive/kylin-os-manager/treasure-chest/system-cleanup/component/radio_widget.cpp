#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QPen>
#include <QApplication>
#include <QMouseEvent>

#include "radio_widget.h"

RadioWidget::RadioWidget(QWidget *parent) : QWidget(parent)
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
    m_labelIcon->setPixmap(icon.pixmap(icon.actualSize(m_iconSize)));
}

void RadioWidget::setLabelText(QString text)
{
    m_labelInfo->SetText(text);
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
    m_labelInfo = new TextLabel(this);
    return;
}

void RadioWidget::initStyle()
{
    this->setFixedSize(m_widgetSize);
    m_labelIcon->setFixedSize(m_iconSize);
    m_labelInfo->SetPixelSize(18, true);
}

void RadioWidget::initLayout()
{
    QVBoxLayout *vlayout = new QVBoxLayout();
    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addStretch();
    hlayout->addWidget(m_btn);
    hlayout->setMargin(0);
    hlayout->setSpacing(0);
    
    vlayout->addLayout(hlayout);
    vlayout->addWidget(m_labelIcon, 0, Qt::AlignHCenter);
    vlayout->addSpacing(8);
    vlayout->addWidget(m_labelInfo, 0, Qt::AlignHCenter);
    vlayout->addSpacing(20);
    vlayout->addStretch();
    vlayout->setMargin(0);
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(4, 4, 4, 4);
    this->setLayout(vlayout);
}

void RadioWidget::initConn()
{
    connect(m_btn, &QRadioButton::clicked, this, &RadioWidget::clickWidget);
}

void RadioWidget::clickWidget()
{
    if (m_isClicked) {
        Q_EMIT sigCannel();
        m_btn->setChecked(false);
        m_isClicked = m_btn->isChecked();
    } else {
        Q_EMIT sigClicked();
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
    // QColor color = QColor("F6F6F6");
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
