#include <QBoxLayout>
#include <QEvent>
#include <QDebug>
#include "kom-scorewidget.h"

namespace kom
{

ScoreWidget::ScoreWidget(QWidget *parent) : QWidget(parent)
{
    setFixedSize(112,16);
    QHBoxLayout *hl = new QHBoxLayout(this);
    hl->setSpacing(0);
    hl->setMargin(0);
    m_l1 = new QLabel(this);
    hl->addWidget(m_l1);
    m_l2 = new QLabel(this);
    hl->addWidget(m_l2);
    m_l3 = new QLabel(this);
    hl->addWidget(m_l3);
    m_l4 = new QLabel(this);
    hl->addWidget(m_l4);
    m_l5 = new QLabel(this);
    hl->addWidget(m_l5);
    setLayout(hl);
    m_l1->installEventFilter(this);
    m_l2->installEventFilter(this);
    m_l3->installEventFilter(this);
    m_l4->installEventFilter(this);
    m_l5->installEventFilter(this);
}

void ScoreWidget::setPix(int i)
{
    m_score = i;
    switch (i) {
    case 1:
        m_l1->setPixmap(QPixmap::fromImage(m_lightPix.scaled(16,16)));
        m_l2->setPixmap(QPixmap::fromImage(m_darkPix.scaled(16,16)));
        m_l3->setPixmap(QPixmap::fromImage(m_darkPix.scaled(16,16)));
        m_l4->setPixmap(QPixmap::fromImage(m_darkPix.scaled(16,16)));
        m_l5->setPixmap(QPixmap::fromImage(m_darkPix.scaled(16,16)));
        break;
    case 2:
        m_l1->setPixmap(QPixmap::fromImage(m_lightPix.scaled(16,16)));
        m_l2->setPixmap(QPixmap::fromImage(m_lightPix.scaled(16,16)));
        m_l3->setPixmap(QPixmap::fromImage(m_darkPix.scaled(16,16)));
        m_l4->setPixmap(QPixmap::fromImage(m_darkPix.scaled(16,16)));
        m_l5->setPixmap(QPixmap::fromImage(m_darkPix.scaled(16,16)));
        break;
    case 3:
        m_l1->setPixmap(QPixmap::fromImage(m_lightPix.scaled(16,16)));
        m_l2->setPixmap(QPixmap::fromImage(m_lightPix.scaled(16,16)));
        m_l3->setPixmap(QPixmap::fromImage(m_lightPix.scaled(16,16)));
        m_l4->setPixmap(QPixmap::fromImage(m_darkPix.scaled(16,16)));
        m_l5->setPixmap(QPixmap::fromImage(m_darkPix.scaled(16,16)));
        break;
    case 4:
        m_l1->setPixmap(QPixmap::fromImage(m_lightPix.scaled(16,16)));
        m_l2->setPixmap(QPixmap::fromImage(m_lightPix.scaled(16,16)));
        m_l3->setPixmap(QPixmap::fromImage(m_lightPix.scaled(16,16)));
        m_l4->setPixmap(QPixmap::fromImage(m_lightPix.scaled(16,16)));
        m_l5->setPixmap(QPixmap::fromImage(m_darkPix.scaled(16,16)));
        break;
    case 5:
        m_l1->setPixmap(QPixmap::fromImage(m_lightPix.scaled(16,16)));
        m_l2->setPixmap(QPixmap::fromImage(m_lightPix.scaled(16,16)));
        m_l3->setPixmap(QPixmap::fromImage(m_lightPix.scaled(16,16)));
        m_l4->setPixmap(QPixmap::fromImage(m_lightPix.scaled(16,16)));
        m_l5->setPixmap(QPixmap::fromImage(m_lightPix.scaled(16,16)));
        break;
    default:
        break;
    }
}

void ScoreWidget::setStarPix(QImage light, QImage dark)
{
    m_lightPix = light;
    m_darkPix = dark;
    setPix();
}

int ScoreWidget::getScore()
{
    return m_score;
}

bool ScoreWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (watched == m_l1) {
            setPix(1);
        } else if (watched == m_l2) {
            setPix(2);
        } else if (watched == m_l3) {
            setPix(3);
        } else if (watched == m_l4) {
            setPix(4);
        } else if (watched == m_l5) {
            setPix(5);
        }
    }
    return QWidget::eventFilter(watched,event);
}



} // namespace kom
