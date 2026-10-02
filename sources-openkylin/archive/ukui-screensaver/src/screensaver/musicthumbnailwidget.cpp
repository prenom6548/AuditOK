#include "musicthumbnailwidget.h"
#include "commonfunc.h"
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QEvent>

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

    m_mprisSaverWidget = new QWidget(this);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    QLabel* m_pLabelDitu = new QLabel(this);
    if (!m_bControlSaver ) {
        QPixmap m_pixmapAlbumDefault
            = QPixmap(":/assets/ditu.png").scaled(78, 78, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_pLabelDitu->setPixmap(m_pixmapAlbumDefault);
        m_pLabelDitu->setFixedSize(78, 78);
    } else {
        QPixmap m_pixmapAlbumDefault
            = QPixmap(":/assets/ditu.png").scaled(52, 52, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        m_pLabelDitu->setPixmap(m_pixmapAlbumDefault);
        m_pLabelDitu->setFixedSize(104, 104);
    }
    mainLayout->addWidget(m_pLabelDitu);
    QHBoxLayout* m_albumLayout = new QHBoxLayout(m_pLabelDitu);
    m_albumLayout->setSpacing(0);
    if (!m_bControlSaver ) {
        m_albumLayout->setContentsMargins(22, 3, 7, 12);
    } else {
        m_albumLayout->setContentsMargins(10, 2, 5, 8);
    }

    QLabel* m_labelBopian = new QLabel(this);
    QPixmap pixmapBopian;
    if (!m_bControlSaver ) {
        m_labelBopian->setFixedSize(17, 29);
        pixmapBopian = QPixmap(":/assets/bogan.png").scaled(17, 29, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    } else {
        m_labelBopian->setFixedSize(11, 19);
        pixmapBopian = QPixmap(":/assets/bogan.png").scaled(11, 19, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    m_labelBopian->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_labelBopian->setPixmap(pixmapBopian);
    m_albumLayout->addWidget(m_labelBopian, 0, Qt::AlignRight | Qt::AlignTop);

    QHBoxLayout* m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonLayout->setSpacing(8);
    m_buttonLayout->addStretch();
    QPushButton* m_btnPrev = new QPushButton(this);
    m_btnPrev->setFlat(true);
    QPixmap prevIcon;
    m_btnPrev->setFixedSize(9, 9);
    prevIcon = drawSymbolicColoredPixmap(
        QIcon::fromTheme("media-skip-backward-symbolic").pixmap(QSize(7, 7)), "white");
    m_btnPrev->setIconSize(QSize(7, 7));
    m_btnPrev->setIcon(prevIcon);
    m_btnPrev->setStyleSheet(
        QString("QPushButton{border-radius: 6px;  border:0px solid white; background-color: rgba(0,0,0,0);}"
                "QPushButton::hover{background-color: rgba(0,0,0,15%);}"
                "QPushButton::pressed {background-color: rgba(0,0,0,21%);}"));
    m_buttonLayout->addWidget(m_btnPrev);
    QPushButton* m_btnPlayPause = new QPushButton(this);
    m_btnPlayPause->setFlat(true);
    m_btnPlayPause->setFixedSize(9, 9);
    QPixmap playPauseIcon = drawSymbolicColoredPixmap(
        QIcon::fromTheme("ukui-play-full-symbolic").pixmap(QSize(9, 9)), "white");
    m_btnPlayPause->setIcon(playPauseIcon);
    m_btnPlayPause->setIconSize(QSize(9, 9));
    m_btnPlayPause->setStyleSheet(
        QString("QPushButton{border-radius: 6px;  border:0px solid white; background-color: rgba(0,0,0,0);}"
                "QPushButton::hover{background-color: rgba(0,0,0,15%);}"
                "QPushButton::pressed {background-color: rgba(0,0,0,21%);}"));
    m_buttonLayout->addWidget(m_btnPlayPause);
    QPushButton* m_btnNext = new QPushButton(this);
    m_btnNext->setFlat(true);
    m_btnNext->setFixedSize(9, 9);
    QPixmap nextIcon = drawSymbolicColoredPixmap(
        QIcon::fromTheme("media-skip-forward-symbolic").pixmap(QSize(9, 9)), "white");
    m_btnNext->setIcon(nextIcon);
    m_btnNext->setIconSize(QSize(7, 7));
    m_btnNext->setStyleSheet(
        QString("QPushButton{border-radius: 6px;  border:0px solid white; background-color: rgba(0,0,0,0);}"
                "QPushButton::hover{background-color: rgba(0,0,0,15%);}"
                "QPushButton::pressed {background-color: rgba(0,0,0,21%);}"));
    m_buttonLayout->addWidget(m_btnNext);
    m_buttonLayout->addStretch();
    mainLayout->addLayout(m_buttonLayout);

    m_mprisSaverWidget->setLayout(mainLayout);

    if (!m_bControlSaver ) {
        m_mprisSaverWidget->setFixedSize(78, 93);
    } else {
        m_mprisSaverWidget->setFixedSize(52, 80);
    }

    m_mainLayout->addWidget(m_mprisSaverWidget, Qt::AlignVCenter);

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
    if (parentWidget() && !parentWidget()->isEnabled()) {
        event->ignore();
        return QWidget::eventFilter(watched, event);
    }
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
