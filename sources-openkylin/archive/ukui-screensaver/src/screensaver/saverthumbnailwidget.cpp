#include "saverthumbnailwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>

SaverThumbnailWidget::SaverThumbnailWidget(QWidget *parent, bool isControlSaver)
    : QWidget{ parent }, m_bControlSaver(isControlSaver)
{
    initUI();
    installEventFilter(this);
}

void SaverThumbnailWidget::setBackground(QPixmap background)
{
    m_background = background;
    repaint();
}

void SaverThumbnailWidget::initUI()
{
    if (m_bControlSaver) {
        setFixedSize(201, 113);
    } else {
        setFixedSize(292, 164);
    }

    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 8, 0, 8);
    m_timeThumbnailWidget = new TimeThumbnailWidget(this, m_bControlSaver);

    QHBoxLayout *timeLayout = new QHBoxLayout();
    timeLayout->addWidget(m_timeThumbnailWidget, 0, Qt::AlignHCenter);
    m_mainLayout->addLayout(timeLayout, 0);
    m_dictumWidget = new QWidget(this);
    m_dictumWidget->setFixedHeight(15);

    QFont dictumFont = font();
    dictumFont.setPointSize(4);

    QVBoxLayout *dictumLayout = new QVBoxLayout(m_dictumWidget);
    dictumLayout->setContentsMargins(0, 0, 0, 0);
    dictumLayout->setSpacing(2);
    m_topLabel = new QLabel(m_dictumWidget);
    m_topLabel->setAlignment(Qt::AlignCenter);
    m_topLabel->setStyleSheet("color: white");
    m_topLabel->setFont(dictumFont);
    dictumLayout->addWidget(m_topLabel);
    m_bottomLabel = new QLabel(m_dictumWidget);
    m_bottomLabel->setAlignment(Qt::AlignCenter);
    m_bottomLabel->setStyleSheet("color: white");
    m_bottomLabel->setFont(dictumFont);
    dictumLayout->addWidget(m_bottomLabel);
    m_authorLabel = new QLabel(m_dictumWidget);
    m_authorLabel->setAlignment(Qt::AlignCenter);
    m_authorLabel->setStyleSheet("color: white");
    m_authorLabel->setFont(dictumFont);
    dictumLayout->addWidget(m_authorLabel);

    m_mainLayout->addWidget(m_dictumWidget, 1, Qt::AlignCenter);
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

void SaverThumbnailWidget::animateSize(const QSize &targetSize)
{
    m_animation->stop();
    m_animation->setStartValue(size());
    m_animation->setEndValue(targetSize);
    m_animation->start();
}

void SaverThumbnailWidget::setSelectStatus(bool status)
{
    if (!status && m_bSelectStatus) {
        m_maskLabel->hide();
        m_bSelectStatus = false;
    } else if (status && !m_bSelectStatus) {
        m_maskLabel->show();
        m_bSelectStatus = true;
    }
}

void SaverThumbnailWidget::setDictumText(QString topText, QString bottomText, QString authorText)
{
    m_topLabel->setText(topText);
    m_bottomLabel->setText(bottomText);
    m_authorLabel->setText(authorText);
}

bool SaverThumbnailWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (parentWidget() && !parentWidget()->isEnabled()) {
        event->ignore();
        return QWidget::eventFilter(watched, event);
    }
    if (event->type() == QEvent::Enter) {
        if (!m_bControlSaver) {
            animateSize(QSize(320, 180));
            m_bHoverStatus = true;
        }
        if (m_bControlSaver || m_bSelectStatus) {
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
        Q_EMIT defaultSaverModeClicked();
    }
    return QWidget::eventFilter(watched, event);
}

void SaverThumbnailWidget::paintEvent(QPaintEvent *event)
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
