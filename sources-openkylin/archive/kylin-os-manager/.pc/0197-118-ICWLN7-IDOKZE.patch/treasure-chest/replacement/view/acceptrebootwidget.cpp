#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QDateTime>
#include "../globaldata.h"

#include "acceptrebootwidget.h"

AcceptRebootWidget::AcceptRebootWidget(QWidget *parent)
{
    initUI();
}

AcceptRebootWidget::~AcceptRebootWidget()
{

}

void AcceptRebootWidget::initUI()
{
    this->setFixedSize(600, 56);

    m_lableSize = new K::TextLabel(this);
    m_lableTime = new K::TextLabel(this);
    m_labeIconSize = new QLabel(this);
    m_labeIconTime = new QLabel(this);
    
    m_layoutHLabel = new QHBoxLayout(this);

    m_layoutHLabel->addSpacing(25);
    m_layoutHLabel->addWidget(m_labeIconSize);
    m_layoutHLabel->addWidget(m_lableSize);
    m_layoutHLabel->addStretch();
    m_layoutHLabel->addWidget(m_labeIconTime);
    m_layoutHLabel->addWidget(m_lableTime);
    m_layoutHLabel->addStretch();

    m_lableSize->SetText(GlobalData::g_textRectMigrateSize);
    m_lableTime->SetText(GlobalData::g_textRectMigrateTime);

    QIcon timeIcon = QIcon(":/res/time_icon.svg");
    m_labeIconTime->setPixmap(timeIcon.pixmap(timeIcon.actualSize(QSize(20, 20))));
    
    QIcon sizeIcon = QIcon(":/res/size_icon.svg");
    m_labeIconSize->setPixmap(sizeIcon.pixmap(sizeIcon.actualSize(QSize(20, 20))));

}

void AcceptRebootWidget::setLabelsText()
{
    m_lableSize->SetText(GlobalData::g_textRectMigrateSize + " " + GlobalData::getInstance()->g_migrateFinishDataStr);
    QStringList list = GlobalData::getInstance()->g_time.split(":");
    m_lableTime->SetText(GlobalData::g_textRectMigrateTime + " " + GlobalData::getInstance()->g_time );
}

void AcceptRebootWidget::paintEvent(QPaintEvent *event)
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
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);    
}
