#include <QDebug>
#include <QApplication>

#include <QPainter>
#include <QStyleOption>
#include <QApplication>

#include "../globaldata.h"
#include "acceptdatastatus.h"

AcceptDataStatus::AcceptDataStatus(QWidget *parent)
{
    initUI();
}

AcceptDataStatus::~AcceptDataStatus()
{

}

void AcceptDataStatus::setStatusStyle(FlowPart part)
{
    switch (part)
    {
    case FlowPart::FirstPart:
        m_statusDevice->setHighColorStatus();
        m_statusData->setNomarlStatus();
        m_statusMigration->setNomarlStatus();
        m_statusReboot->setNomarlStatus();
        break;
    
    case FlowPart::SecondPart:
        m_statusDevice->setNomarlStatus();
        m_statusData->setHighColorStatus();
        m_statusMigration->setNomarlStatus();
        m_statusReboot->setNomarlStatus();
        break;
    
    case FlowPart::ThirdPart:
        m_statusDevice->setNomarlStatus();
        m_statusData->setNomarlStatus();
        m_statusMigration->setHighColorStatus();
        m_statusReboot->setNomarlStatus();
        break;
    
    case FlowPart::LastPart:
        m_statusDevice->setNomarlStatus();
        m_statusData->setNomarlStatus();
        m_statusMigration->setNomarlStatus();
        m_statusReboot->setHighColorStatus();
        break;
    
    default:
        break;
    }
    update();
}

void AcceptDataStatus::initUI()
{
    this->setFixedSize(600, 100);
    m_statusDevice = new StatusWidget(this);
    m_statusData = new StatusWidget(this);
    m_statusMigration = new StatusWidget(this);
    m_statusReboot = new StatusWidget(this);

    m_statusDevice->setStatusText(GlobalData::g_textReceTabPair, GlobalData::g_textReceTabTipPair);
    m_statusData->setStatusText(GlobalData::g_textReceTabRange, GlobalData::g_textReceTabTipRange);
    m_statusMigration->setStatusText(GlobalData::g_textReceTabMigrate, GlobalData::g_textReceTabTipMigrate);
    m_statusReboot->setStatusText(GlobalData::g_textReceTabComplete, GlobalData::g_textReceTabTipComplete);

    m_statusMigration->setHighColorStatus();
    
    m_hlayout = new QHBoxLayout(this);
    m_hlayout->addWidget(m_statusDevice, 0, Qt::AlignTop);
    m_hlayout->addSpacing(100);
    m_hlayout->addWidget(m_statusData, 0, Qt::AlignTop);
    m_hlayout->addSpacing(100);
    m_hlayout->addWidget(m_statusMigration, 0, Qt::AlignTop);
    m_hlayout->addSpacing(100);
    m_hlayout->addWidget(m_statusReboot, 0, Qt::AlignTop);
}

void AcceptDataStatus::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;

    //设置背景颜色
    QColor lineColor = QColor("#595959");
    QPen pen;
    pen.setWidth(3);
    pen.setColor(lineColor);
    pen.setStyle(Qt::DotLine);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);

    QPoint p1;
    QPoint p2;
    for (QWidget* obj : m_statusDevice->findChildren<QWidget *>()) {
        if ( "btn" == obj->objectName()){
            p1 = obj->mapTo(this, {2, 3});
        }
    }
    for (QWidget* obj : m_statusReboot->findChildren<QWidget *>()) {
        if ( "btn" == obj->objectName()){
            p2 = obj->mapTo(this, {0, 3});
        }
    }
    painter.drawLine(p1, p2);

    QWidget::paintEvent(event);    
}
