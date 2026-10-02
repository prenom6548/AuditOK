#include "musicthumbnailwidget.h"
#include <QPainter>
#include <QPainterPath>

MusicThumbnailWidget::MusicThumbnailWidget(QWidget *parent, bool isControlSaver)
    : QWidget{parent},
      m_bControlSaver(isControlSaver)
{
    initUI();
    installEventFilter(this);
}

void MusicThumbnailWidget::setBackground(QPixmap background)
{
    m_background = background;
    repaint();
}

void MusicThumbnailWidget::initUI()
{
    m_mainLayout = new QHBoxLayout(this);

    if (m_bControlSaver ) {
        setFixedSize(201, 113);
    } else {
        setFixedSize(292, 164);
    }

    m_timeThumbnailWidget = new TimeThumbnailWidget(this, m_bControlSaver);
    m_mainLayout->addWidget(m_timeThumbnailWidget);

    m_mprisSaverWidget = new MPRISSaverWidget(this);
    if (!m_bControlSaver ) {
        m_mprisSaverWidget->setFixedSize(521 * 0.15, 620 * 0.15);
        m_mprisSaverWidget->updateMediaSize(0.15);
    } else {
        m_mprisSaverWidget->setFixedSize(521 * 0.10, 620 * 0.10);
        m_mprisSaverWidget->updateMediaSize(0.10);
    }
    m_mainLayout->addWidget(m_mprisSaverWidget);

    m_maskLabel = new MaskLable(this, m_bControlSaver);
    m_maskLabel->setGeometry(0, 0, this->width(), this->height());
    m_maskLabel->hide();

    if (!m_bControlSaver) {
        m_animation = new QVariantAnimation(this);
        m_animation->setDuration(300);
        m_animation->setEasingCurve(QEasingCurve::OutQuad);

        connect(m_animation, &QVariantAnimation::valueChanged, [this](const QVariant &value) {
            QSize size = value.toSize();
            setFixedSize(size);
            m_maskLabel->setFixedSize(size);
        });
    }
}

void MusicThumbnailWidget::animateSize(const QSize &targetSize)
{
    m_animation->stop();
    m_animation->setStartValue(size());
    m_animation->setEndValue(targetSize);
    m_animation->start();
}

void MusicThumbnailWidget::setSelectStatus(bool status)
{
    if (!status && m_bSelectStatus) {
        m_maskLabel->hide();
        m_bSelectStatus = false;
    } else if (status && !m_bSelectStatus) {
        m_maskLabel->show();
        m_bSelectStatus = true;
    }
}

bool MusicThumbnailWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Enter) {
        if (!m_bControlSaver) {
            animateSize(QSize(320, 180));
            m_bHoverStatus = true;
        }
        if (m_bControlSaver ||  m_bSelectStatus) {
            m_maskLabel->show();
        }
    } else if (event->type() == QEvent::Leave) {
        if (!m_bControlSaver) {
            animateSize(QSize(292, 164));
            m_bHoverStatus = false;
        }
        if (!m_bSelectStatus) {
            m_maskLabel->hide();
        }
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        if (!m_bControlSaver) {
            m_maskLabel->show();
            m_bSelectStatus = true;
        }
        m_maskLabel->show();
        Q_EMIT musicModeClicked();
    }
    return QWidget::eventFilter(watched, event);
}

void MusicThumbnailWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    if (!m_bControlSaver) {
        QPainterPath backgroundPath;
        backgroundPath.addRoundedRect(rect(), 8, 8);
        painter.setClipPath(backgroundPath);
        painter.drawPixmap(this->rect(), m_background, m_background.rect());
        painter.setClipping(false);
        if (m_bHoverStatus) {
            QPen pen(QColor(255, 255, 255, 153));
            pen.setWidth(2);
            painter.setPen(pen);
            painter.setBrush(QColor(0, 0, 0, 78)); // 178
            painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 8, 8);
        } else {
            painter.setPen(Qt::transparent);
            painter.setBrush(QColor(0, 0, 0, 78)); // 178
            painter.drawRoundedRect(rect().adjusted(0, 0, 0, 0), 8, 8);
        }
    } else {
        painter.fillRect(rect(), palette().color(QPalette::Base));
        const QRect bgRect(4, 4, 193, 105);
        QPainterPath path;
        path.addRoundedRect(bgRect, 8, 8);
        painter.setClipPath(path);
        painter.drawPixmap(bgRect, m_background, m_background.rect());
        painter.setPen(Qt::transparent);
        painter.setBrush(QColor(0, 0, 0, 78));
        painter.drawRoundedRect(rect().adjusted(0, 0, 0, 0), 8, 8);
    }
}
