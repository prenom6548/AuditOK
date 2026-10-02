#include "photocorver.h"
#include <QPainter>
#include <QPainterPath>
#include <QIcon>
#include <QEvent>

PhotoCorver::PhotoCorver(QWidget *parent)
    : QLabel{parent}
{
    setFixedSize(201, 113);
    installEventFilter(this);
    m_maskLabel = new MaskLable(this, true);
    m_maskLabel->setGeometry(0, 0, this->width(), this->height());
    m_maskLabel->hide();
}

void PhotoCorver::setBackground(QPixmap background)
{

    m_background = background;
    repaint();
}

bool PhotoCorver::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        m_maskLabel->show();
    } else if (event->type() == QEvent::Leave) {
        if (!m_bSelectStatus) {
            m_maskLabel->hide();
        }
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        m_bSelectStatus = true;
        m_maskLabel->show();
        Q_EMIT photoCorverClicked();
    }
    return QWidget::eventFilter(watched, event);
}

void PhotoCorver::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    painter.fillRect(rect(), palette().color(QPalette::Base));
    const QRect bgRect(4, 4, 193, 105);
    QPainterPath path;
    path.addRoundedRect(bgRect, 8, 8);
    painter.setClipPath(path);
    painter.drawPixmap(bgRect, m_background, m_background.rect());
}
