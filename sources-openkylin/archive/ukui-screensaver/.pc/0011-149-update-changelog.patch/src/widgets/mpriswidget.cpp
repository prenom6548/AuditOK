#include "mpriswidget.h"
#include <QTimer>
#include <QIcon>
#include "commonfunc.h"
#include "../lock-dialog/lockdialogmodel.h"

MPRISWidget::MPRISWidget(QWidget *parent) : QFrame(parent)
{
    initUI();
    initConnections();
}

void MPRISWidget::initUI()
{
    m_mainLayout = new QHBoxLayout();
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    m_labelAlbum = new QLabel();
    m_pixmapAlbumDefault
        = QPixmap(":/image/assets/fengmian.png").scaled(120, 120, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_labelAlbum->setPixmap(PixmapToRound(m_pixmapAlbumDefault, 12, 0, 12, 0));
    m_labelAlbum->setFixedSize(120, 120);
    m_mainLayout->addWidget(m_labelAlbum);

    m_rightLayout = new QVBoxLayout();
    m_rightLayout->setContentsMargins(0, 8, 0, 8);
    m_rightLayout->setSpacing(0);
    m_labelName = new CustomLabel();
    m_labelName->setMarqueueMode(true);
    QFont labelFont = m_labelName->font();
    labelFont.setPointSizeF(14);
    m_labelName->setFont(labelFont);
    m_labelName->setFixedSize(168, 32);
    m_labelName->setStyleSheet("QLabel{ color:rgba(26, 26, 26, 100%);}");
    m_rightLayout->addWidget(m_labelName, 0, Qt::AlignCenter);
    m_labelArtist = new CustomLabel();
    m_labelArtist->setMarqueueMode(true);
    labelFont = m_labelArtist->font();
    labelFont.setPointSizeF(12);
    m_labelArtist->setFont(labelFont);
    m_labelArtist->setFixedSize(168, 24);
    m_labelArtist->setStyleSheet("QLabel{ color:rgba(0, 0, 0, 35%);}");
    m_rightLayout->addWidget(m_labelArtist, 0, Qt::AlignCenter);
    m_rightLayout->addSpacing(4);
    m_labelLine = new QLabel();
    m_labelLine->setStyleSheet("QLabel{background-color: rgba(0, 0, 0, 8%); }");
    m_rightLayout->addWidget(m_labelLine);
    m_rightLayout->addSpacing(4);

    m_wdgControl = new QWidget();
    m_wdgControl->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *controlLayout = new QHBoxLayout(m_wdgControl);
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->setSpacing(8);
    controlLayout->addStretch();
    m_btnPrev = new QPushButton();
    m_btnPrev->setFlat(true);
    m_btnPrev->setFixedSize(40, 40);
    m_btnPrev->setIcon(QIcon::fromTheme("media-skip-backward-symbolic").pixmap(16, 16));
    m_btnPrev->setStyleSheet("");
    m_btnPrev->setStyleSheet(
        QString("QPushButton{border-radius: 6px;  border:0px solid white; background-color: rgba(0,0,0,0);}"
                "QPushButton::hover{background-color: rgba(0,0,0,15%);}"
                "QPushButton::pressed {background-color: rgba(0,0,0,21%);}"));
    controlLayout->addWidget(m_btnPrev);
    m_btnPlayPause = new QPushButton();
    m_btnPlayPause->setFlat(true);
    m_btnPlayPause->setFixedSize(40, 40);
    m_btnPlayPause->setIcon(QIcon::fromTheme("ukui-play-full-symbolic").pixmap(16, 16));
    m_btnPlayPause->setStyleSheet(
        QString("QPushButton{border-radius: 6px;  border:0px solid white; background-color: rgba(0,0,0,0);}"
                "QPushButton::hover{background-color: rgba(0,0,0,15%);}"
                "QPushButton::pressed {background-color: rgba(0,0,0,21%);}"));
    controlLayout->addWidget(m_btnPlayPause);
    m_btnNext = new QPushButton();
    m_btnNext->setFlat(true);
    m_btnNext->setFixedSize(40, 40);
    m_btnNext->setIcon(QIcon::fromTheme("media-skip-forward-symbolic").pixmap(16, 16));
    m_btnNext->setStyleSheet(
        QString("QPushButton{border-radius: 6px;  border:0px solid white; background-color: rgba(0,0,0,0);}"
                "QPushButton::hover{background-color: rgba(0,0,0,15%);}"
                "QPushButton::pressed {background-color: rgba(0,0,0,21%);}"));
    controlLayout->addWidget(m_btnNext);
    controlLayout->addStretch();
    m_rightLayout->addWidget(m_wdgControl);

    m_mainLayout->addLayout(m_rightLayout);

    setLayout(m_mainLayout);
}

void MPRISWidget::initConnections()
{
    if (m_btnNext) {
        connect(m_btnNext, &QPushButton::clicked, this, &MPRISWidget::onBtnNext);
    }
    if (m_btnPlayPause) {
        connect(m_btnPlayPause, &QPushButton::clicked, this, &MPRISWidget::onBtnPlayPause);
    }
    if (m_btnPrev) {
        connect(m_btnPrev, &QPushButton::clicked, this, &MPRISWidget::onBtnPreview);
    }
}

void MPRISWidget::init(QString strMediaPlayerPath)
{
    if (!m_mprisMonitor) {
        m_mprisMonitor = QDBusConnection::sessionBus().interface();
        connect(
            m_mprisMonitor,
            &QDBusConnectionInterface::serviceOwnerChanged,
            this,
            [=](const QString &name, const QString &oldOwner, const QString &newOwner) {
                Q_UNUSED(oldOwner)
                if (name.startsWith("org.mpris.MediaPlayer2.")) {
                    onServiceStatusChanged(name, !newOwner.isEmpty());
                }
            });
    }
    if (!strMediaPlayerPath.isEmpty()) {
        onLoadMediaPath(strMediaPlayerPath);
    }
}

void MPRISWidget::onServiceStatusChanged(QString strName, bool isActive)
{
    qDebug() << "-------------------" << strName << isActive;
    if (strName == m_strMediaPlayerPath) {
        if (isActive) {
            onLoadMediaPath(strName);
        } else {
            onLostMediaPath(strName);
        }
    }
}

void MPRISWidget::onLoadMediaPath(const QString &strPath)
{
    qDebug() << "onLoadMediaPath:" << strPath;
    MPRISDBusClient *newMprisClient
        = new MPRISDBusClient(strPath, "/org/mpris/MediaPlayer2", QDBusConnection::sessionBus(), this);

    if (m_mprisDbusClient)
        m_mprisDbusClient->deleteLater();

    m_mprisDbusClient = newMprisClient;

    connect(m_mprisDbusClient, SIGNAL(MetadataChanged(QVariantMap)), this, SLOT(onMetadataChanged()));
    connect(m_mprisDbusClient, SIGNAL(PlaybackStatusChanged(QString)), this, SLOT(onPlaybackStatusChanged()));
    connect(m_mprisDbusClient, SIGNAL(CanControlChanged(bool)), this, SLOT(onCanControlChanged()));

    onPlaybackStatusChanged();
    onMetadataChanged();
    onCanControlChanged();

    m_strMediaPlayerPath = strPath;
    Q_EMIT statusChanged(true);
}

void MPRISWidget::onLostMediaPath(const QString &strPath)
{
    if (!m_strMediaPlayerPath.isEmpty() && m_strMediaPlayerPath == strPath) {
        if (m_mprisDbusClient) {
            m_mprisDbusClient->deleteLater();
            m_mprisDbusClient = nullptr;
        }

        m_strMediaPlayerPath.clear();
        Q_EMIT statusChanged(false);
    }
}

void MPRISWidget::onBtnPreview()
{
    if (!m_mprisDbusClient) {
        return;
    }
    m_mprisDbusClient->Previous();
}

void MPRISWidget::onBtnNext()
{
    if (!m_mprisDbusClient) {
        return;
    }
    m_mprisDbusClient->Next();
}

void MPRISWidget::onBtnPlayPause()
{
    if (m_isPlayBtnClicked) {
        return;
    }
    m_isPlayBtnClicked = true;
    QTimer::singleShot(300, this, [=]() {
        m_isPlayBtnClicked = false;
        if (!m_mprisDbusClient) {
            return;
        }
        if (m_nPlayState != 1) {
            m_mprisDbusClient->Play();
        } else {
            m_mprisDbusClient->Pause();
        }
    });
}

void MPRISWidget::onMetadataChanged()
{
    if (!m_mprisDbusClient)
        return;
    delayMetadataUpdate();
}

void MPRISWidget::delayMetadataUpdate(int nTimeout /* = 1000*/)
{
    if (!m_timerDelayUpdate) {
        m_timerDelayUpdate = new QTimer(this);
        m_timerDelayUpdate->setSingleShot(true);
        connect(m_timerDelayUpdate, &QTimer::timeout, this, &MPRISWidget::onDelayMetadataUpdate);
    }
    m_timerDelayUpdate->setInterval(nTimeout);
    m_timerDelayUpdate->start();
}

void MPRISWidget::onDelayMetadataUpdate()
{
    if (m_nPlayState == -1) { // 停止播放状态
        m_labelName->setText(tr("Unknown"));
        m_labelArtist->setText(tr("Unknown"));
        m_labelAlbum->setPixmap(PixmapToRound(m_pixmapAlbumDefault, 12, 0, 12, 0));
    } else {
        const auto &meta = m_mprisDbusClient->metadata();
        qDebug() << "Metadata:" << meta;
        const QString &title = meta.value("xesam:title").toString();
        const QString &album = meta.value("xesam:album").toString();
        const QString &artist = meta.value("xesam:artist").toString();
        const QString &lyrics = meta.value("xesam:asText").toString();
        const QUrl &pictureUrl = meta.value("mpris:artUrl").toString();

        if (title.isEmpty()) {
            if (album.isEmpty()) {
                m_labelName->setText(tr("Unknown"));
            } else {
                m_labelName->setText(album);
            }
        } else {
            m_labelName->setText(title);
        }
        if (artist.isEmpty()) {
            if (lyrics.isEmpty()) {
                m_labelArtist->setText(tr("Unknown"));
            } else {
                m_labelArtist->setText(lyrics);
            }
        } else {
            if (lyrics.isEmpty()) {
                m_labelArtist->setText(artist);
            } else {
                m_labelArtist->setText(artist + "/" + lyrics);
            }
        }
        if (!pictureUrl.isEmpty()) {
            const QSize &pictureSize = m_labelAlbum->size();
            const QPixmap &picture = QPixmap(pictureUrl.toLocalFile()).scaled(pictureSize, Qt::IgnoreAspectRatio);
            if (!picture.isNull()) {
                m_labelAlbum->setPixmap(PixmapToRound(picture, 12, 0, 12, 0));
            } else {
                m_labelAlbum->setPixmap(PixmapToRound(m_pixmapAlbumDefault, 12, 0, 12, 0));
            }
        } else {
            m_labelAlbum->setPixmap(PixmapToRound(m_pixmapAlbumDefault, 12, 0, 12, 0));
        }
    }
}

void MPRISWidget::onPlaybackStatusChanged()
{
    if (!m_mprisDbusClient) {
        return;
    }

    const QString status = m_mprisDbusClient->playbackStatus();
    qDebug() << "PlayStatus:" << status;
    if (status == "Playing") {
        m_nPlayState = 1;
        m_btnPlayPause->setIcon(QIcon::fromTheme("media-playback-pause-symbolic").pixmap(16, 16));
    } else {
        if (status == "Paused") {
            m_nPlayState = 0;
        } else {
            m_nPlayState = -1;
        }
        m_btnPlayPause->setIcon(QIcon::fromTheme("ukui-play-full-symbolic").pixmap(16, 16));
    }
}

void MPRISWidget::onCanControlChanged()
{
    if (!m_mprisDbusClient) {
        return;
    }
    // m_wdgControl->setVisible(m_mprisDbusClient->canControl());
}

void MPRISWidget::resizeEvent(QResizeEvent *event)
{
    if (m_labelLine) {
        m_labelLine->setFixedSize(width(), 1);
    }
}

void MPRISWidget::updateFont(QString strFont, double fontSizeF)
{
    if (m_labelName) {
        QFont font = m_labelName->font();
        if (!strFont.isEmpty()) {
            font.setFamily(strFont);
        }
        font.setPointSizeF((14 + fontSizeF) * LockDialogModel::getPtToPx());
        m_labelName->setFont(font);
    }
    if (m_labelArtist) {
        QFont font = m_labelArtist->font();
        if (!strFont.isEmpty()) {
            font.setFamily(strFont);
        }
        font.setPointSizeF((12 + fontSizeF) * LockDialogModel::getPtToPx());
        m_labelArtist->setFont(font);
    }
}
