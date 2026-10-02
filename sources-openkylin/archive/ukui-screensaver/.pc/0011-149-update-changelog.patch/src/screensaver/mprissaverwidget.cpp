#include "mprissaverwidget.h"
#include <QTimer>
#include <QIcon>
#include "commonfunc.h"
#include "../lock-dialog/lockdialogmodel.h"
#include "playcontroller.h"

MPRISSaverWidget::MPRISSaverWidget(QWidget *parent, bool saverMedia)
    : QFrame(parent)
    , m_bSaverMedia(saverMedia)
{
    initUI();
    initConnections();
    if (m_bSaverMedia) {
//        m_adapter = new DbusAdapter(this);
    }
}

MPRISSaverWidget::~MPRISSaverWidget()
{
    if (m_adapter) {
        delete m_adapter;
        m_adapter = nullptr;
    }
}

void MPRISSaverWidget::setSaverMediaType(bool saverMedia)
{
    if (!m_bSaverMedia && saverMedia && m_mprisMonitor) {
        disconnect(m_mprisMonitor, nullptr, this, nullptr);
        if (m_mprisDbusClient) {
            disconnect(m_mprisDbusClient, nullptr, this, nullptr);
        }
        connect(&playController::getInstance(), &playController::playerStateChange, this, &MPRISSaverWidget::onPlayerStateChanged);
        connect(&playController::getInstance(), &playController::mateDataChanged, this, &MPRISSaverWidget::onMetadataChanged);
    } else if (m_bSaverMedia && !saverMedia) {
        disconnect(&playController::getInstance());
    }
    m_bSaverMedia = saverMedia;
}

void MPRISSaverWidget::updateMediaSize(float scale)
{
    if (m_allLabels.isEmpty()) {
        QList<QLabel*> labels = this->findChildren<QLabel*>();
        foreach (QLabel* label, labels) {
            m_allLabels[label] = label->rect();
            const QPixmap* pixmap = label->pixmap();
            if (pixmap && !pixmap->isNull()) {
                if (!m_originalPixmaps.contains(label)) {
                    m_originalPixmaps[label] = *pixmap;
                }
            }
            const QFont font = label->font();
            if (!m_originalFonts.contains(label)) {
                m_originalFonts[label] = font;
            }
        }
        QList<QPushButton*> buttons = this->findChildren<QPushButton*>();
        foreach (QPushButton* button, buttons) {
            m_allButtons[button] = button->rect();
        }
    }

    QList<QLabel*> labels = this->findChildren<QLabel*>();
    foreach (QLabel* label, labels) {
        const QPixmap* pixmap = label->pixmap();
        QSize newSize = m_allLabels[label].size() * scale;
        if (pixmap && !pixmap->isNull()) {
            QPixmap scaledPixmap = m_originalPixmaps[label].scaled(
                newSize,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
            );
            label->setPixmap(scaledPixmap);
        }
        label->setFixedSize(newSize);
        QFont font = label->font();
        font.setPointSizeF(m_originalFonts[label].pointSize() * scale);
        label->setFont(font);
    }

    QList<QPushButton*> buttons = this->findChildren<QPushButton*>();
    foreach (QPushButton* button, buttons) {
        if (!button->icon().isNull()) {
            button->setIconSize(m_allButtons[button].size() * scale);
            button->setFixedSize(m_allButtons[button].size() * scale);
        }
    }

    m_labelName->setFixedWidth(this->width());
    m_labelArtist->setFixedWidth(this->width());

    m_buttonLayout->setSpacing(40 * scale);
    m_albumLayout->setContentsMargins(146 * scale, 20 * scale, 45 * scale, 80 * scale);
}

void MPRISSaverWidget::initUI()
{
    setAttribute(Qt::WA_TranslucentBackground);

    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    m_labelName = new QLabel(this);
//    m_labelName->setMarqueueMode(true);
    QFont labelFont = m_labelName->font();
    labelFont.setPointSizeF(24);
    m_labelName->setFont(labelFont);
    m_labelName->setFixedSize(this->width(), 42);
    m_labelName->setStyleSheet("QLabel{ color: white;}");
    m_labelName->setAlignment(Qt::AlignHCenter);
    m_mainLayout->addWidget(m_labelName, 0, Qt::AlignCenter);
    m_labelArtist = new QLabel(this);
//    m_labelArtist->setMarqueueMode(true);
    labelFont = m_labelArtist->font();
    labelFont.setPointSizeF(16);
    m_labelArtist->setFont(labelFont);
    m_labelArtist->setFixedSize(this->width(), 42);
    m_labelArtist->setAlignment(Qt::AlignHCenter);
    m_labelArtist->setStyleSheet("QLabel{ color: white;}");
    m_mainLayout->addSpacing(6);
    m_mainLayout->addWidget(m_labelArtist, 0, Qt::AlignCenter);

    m_pLabelDitu = new QLabel(this);
    m_pixmapAlbumDefault
        = QPixmap(":/assets/ditu.png").scaled(521, 521, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_pLabelDitu->setPixmap(m_pixmapAlbumDefault);
    m_pLabelDitu->setFixedSize(521, 521);
    m_mainLayout->addWidget(m_pLabelDitu);
    m_albumLayout = new QHBoxLayout(m_pLabelDitu);
    m_albumLayout->setSpacing(0);
    m_albumLayout->setContentsMargins(146, 20, 45, 80);

    m_labelAlbum = new RotatingImageLabel(this);
    m_labelAlbum->setFixedSize(230, 230);
    m_albumLayout->addWidget(m_labelAlbum, 0, Qt::AlignCenter);

    m_labelBopian = new CornerRotatingLabel(this);
    m_labelBopian->setFixedSize(115, 186);
    QPixmap pixmapBopian = QPixmap(":/assets/bogan.png").scaled(115, 186, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    m_labelBopian->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_labelBopian->setPixmap(pixmapBopian);
//    m_labelBopian->setImage(pixmapBopian);
    m_albumLayout->addWidget(m_labelBopian, 0, Qt::AlignRight | Qt::AlignTop);

    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->setContentsMargins(0, 0, 0, 0);
    m_buttonLayout->setSpacing(40);
    m_buttonLayout->addStretch();
    m_btnPrev = new QPushButton(this);
    m_btnPrev->setFlat(true);
    m_btnPrev->setFixedSize(64, 64);
    QPixmap prevIcon = drawSymbolicColoredPixmap(
        QIcon::fromTheme("media-skip-backward-symbolic").pixmap(QSize(48, 48)), "white");
    m_btnPrev->setIcon(prevIcon);
    m_btnPrev->setIconSize(QSize(48, 48));
    m_btnPrev->setStyleSheet(
        QString("QPushButton{border-radius: 6px;  border:0px solid white; background-color: rgba(0,0,0,0);}"
                "QPushButton::hover{background-color: rgba(0,0,0,15%);}"
                "QPushButton::pressed {background-color: rgba(0,0,0,21%);}"));
    m_buttonLayout->addWidget(m_btnPrev);
    m_btnPlayPause = new QPushButton(this);
    m_btnPlayPause->setFlat(true);
    m_btnPlayPause->setFixedSize(64, 64);
    QPixmap playPauseIcon = drawSymbolicColoredPixmap(
        QIcon::fromTheme("ukui-play-full-symbolic").pixmap(QSize(64, 64)), "white");
    m_btnPlayPause->setIcon(playPauseIcon);
    m_btnPlayPause->setIconSize(QSize(64, 64));
    m_btnPlayPause->setStyleSheet(
        QString("QPushButton{border-radius: 6px;  border:0px solid white; background-color: rgba(0,0,0,0);}"
                "QPushButton::hover{background-color: rgba(0,0,0,15%);}"
                "QPushButton::pressed {background-color: rgba(0,0,0,21%);}"));
    m_buttonLayout->addWidget(m_btnPlayPause);
    m_btnNext = new QPushButton(this);
    m_btnNext->setFlat(true);
    m_btnNext->setFixedSize(64, 64);
    QPixmap nextIcon = drawSymbolicColoredPixmap(
        QIcon::fromTheme("media-skip-forward-symbolic").pixmap(QSize(48, 48)), "white");
    m_btnNext->setIcon(nextIcon);
    m_btnNext->setIconSize(QSize(48, 48));
    m_btnNext->setStyleSheet(
        QString("QPushButton{border-radius: 6px;  border:0px solid white; background-color: rgba(0,0,0,0);}"
                "QPushButton::hover{background-color: rgba(0,0,0,15%);}"
                "QPushButton::pressed {background-color: rgba(0,0,0,21%);}"));
    m_buttonLayout->addWidget(m_btnNext);
    m_buttonLayout->addStretch();
    m_mainLayout->addLayout(m_buttonLayout);

    setLayout(m_mainLayout);
}

void MPRISSaverWidget::initConnections()
{
    if (m_btnNext) {
        connect(m_btnNext, &QPushButton::clicked, this, &MPRISSaverWidget::onBtnNext);
    }
    if (m_btnPlayPause) {
        connect(m_btnPlayPause, &QPushButton::clicked, this, &MPRISSaverWidget::onBtnPlayPause);
    }
    if (m_btnPrev) {
        connect(m_btnPrev, &QPushButton::clicked, this, &MPRISSaverWidget::onBtnPreview);
    }
    if (m_bSaverMedia) {
        connect(&playController::getInstance(), &playController::playerStateChange, this, &MPRISSaverWidget::onPlayerStateChanged);
        connect(&playController::getInstance(), &playController::mateDataChanged, this, &MPRISSaverWidget::onMetadataChanged);
    }
}

void MPRISSaverWidget::init(QString strMediaPlayerPath)
{
    if (strMediaPlayerPath.isEmpty()) {
        m_labelName->setText(tr("No music detected"));
    }
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

void MPRISSaverWidget::onServiceStatusChanged(QString strName, bool isActive)
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

void MPRISSaverWidget::onLoadMediaPath(const QString &strPath)
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

void MPRISSaverWidget::onLostMediaPath(const QString &strPath)
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

void MPRISSaverWidget::onBtnPreview()
{
    m_labelBopian->startRotationAnimation();
    if (m_bSaverMedia) {
        playController::getInstance().previousSong();
        return ;
    }
    if (!m_mprisDbusClient) {
        return;
    }
    m_mprisDbusClient->Previous();
}

void MPRISSaverWidget::onBtnNext()
{
    m_labelBopian->startRotationAnimation();
    if (m_bSaverMedia) {
        playController::getInstance().nextSong();
        return ;
    }
    if (!m_mprisDbusClient) {
        return;
    }
    m_mprisDbusClient->Next();
}

void MPRISSaverWidget::onBtnPlayPause()
{
    if (m_isPlayBtnClicked) {
        return;
    }
    m_isPlayBtnClicked = true;
    QTimer::singleShot(300, this, [=]() {
        m_isPlayBtnClicked = false;
        if (m_bSaverMedia) {
            if (m_nPlayState != 1) {
                playController::getInstance().play();
                return ;
            } else {
                playController::getInstance().pause();
                return ;
            }
        }
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

void MPRISSaverWidget::onMetadataChanged()
{
    qDebug() << "m_bSaverMedia = " << m_bSaverMedia;
    if (m_bSaverMedia) {
        delayMetadataUpdate();
    }
    if (!m_mprisDbusClient)
        return;
    delayMetadataUpdate();
}

void MPRISSaverWidget::delayMetadataUpdate(int nTimeout /* = 1000*/)
{
    if (!m_timerDelayUpdate) {
        m_timerDelayUpdate = new QTimer(this);
        m_timerDelayUpdate->setSingleShot(true);
        connect(m_timerDelayUpdate, &QTimer::timeout, this, &MPRISSaverWidget::onDelayMetadataUpdate);
    }
    m_timerDelayUpdate->setInterval(nTimeout);
    m_timerDelayUpdate->start();
}

void MPRISSaverWidget::onDelayMetadataUpdate()
{
    if (m_nPlayState == -1 && !m_bSaverMedia) { // 停止播放状态
        m_labelName->setText(tr("Unknown"));
        m_labelArtist->setText(tr("Unknown"));
        m_labelAlbum->hide();
    } else {
        QVariantMap meta;
        if (m_bSaverMedia) {
            meta = playController::getInstance().getMetadata();
            qDebug() << "data = " << meta;
        } else {
            meta = m_mprisDbusClient->metadata();
        }
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
                m_labelAlbum->setPixmap(PixmapToRound(picture, 115 * m_scale, 115 * m_scale, 115 * m_scale, 115 * m_scale));
                m_labelAlbum->setImage(PixmapToRound(picture, 115 * m_scale, 115 * m_scale, 115 * m_scale, 115 * m_scale));
                m_labelAlbum->show();

                const QString status = m_mprisDbusClient->playbackStatus();
                if (status == "Playing") {
                    m_labelAlbum->startRotation(2000);
                }
            } else {
                m_labelAlbum->hide();
            }
        } else {
            m_labelAlbum->hide();
        }
    }
}

void MPRISSaverWidget::onPlayerStateChanged(int status)
{
    if (status == 1) {
        m_nPlayState = 1;
        m_btnPlayPause->setIcon(drawSymbolicColoredPixmap(QIcon::fromTheme("media-playback-pause-symbolic").pixmap(QSize(64, 64)), "white"));
        if (m_labelAlbum->isVisible()) {
            m_labelAlbum->startRotation(2000);
        }
    } else {
        if (status == 2) {
            m_nPlayState = 0;
        } else {
            m_nPlayState = -1;
        }
        m_btnPlayPause->setIcon(drawSymbolicColoredPixmap(QIcon::fromTheme("ukui-play-full-symbolic").pixmap(QSize(64, 64)), "white"));
        if (m_labelAlbum->isVisible()) {
            m_labelAlbum->pauseRotation();
        }
    }
}

void MPRISSaverWidget::onPlaybackStatusChanged()
{
    if (!m_mprisDbusClient) {
        return;
    }

    const QString status = m_mprisDbusClient->playbackStatus();
    qDebug() << "PlayStatus:" << status;
    if (status == "Playing") {
        m_nPlayState = 1;
        m_btnPlayPause->setIcon(drawSymbolicColoredPixmap(QIcon::fromTheme("media-playback-pause-symbolic").pixmap(QSize(64, 64)), "white"));
        if (m_labelAlbum->isVisible()) {
            m_labelAlbum->startRotation(2000);
        }
    } else {
        if (status == "Paused") {
            m_nPlayState = 0;
        } else {
            m_nPlayState = -1;
        }
        m_btnPlayPause->setIcon(drawSymbolicColoredPixmap(QIcon::fromTheme("ukui-play-full-symbolic").pixmap(QSize(64, 64)), "white"));
        if (m_labelAlbum->isVisible()) {
            m_labelAlbum->pauseRotation();
        }
    }
}

void MPRISSaverWidget::onCanControlChanged()
{
    if (!m_mprisDbusClient) {
        return;
    }
    // m_wdgControl->setVisible(m_mprisDbusClient->canControl());
}

void MPRISSaverWidget::resizeEvent(QResizeEvent *event)
{
    if (m_labelLine) {
        m_labelLine->setFixedSize(width(), 1);
    }
}

void MPRISSaverWidget::updateFont(QString strFont, double fontSizeF)
{
//    if (m_labelName) {
//        QFont font = m_labelName->font();
//        if (!strFont.isEmpty()) {
//            font.setFamily(strFont);
//        }
//        font.setPointSizeF((14 + fontSizeF) * LockDialogModel::getPtToPx());
//        m_labelName->setFont(font);
//    }
//    if (m_labelArtist) {
//        QFont font = m_labelArtist->font();
//        if (!strFont.isEmpty()) {
//            font.setFamily(strFont);
//        }
//        font.setPointSizeF((12 + fontSizeF) * LockDialogModel::getPtToPx());
//        m_labelArtist->setFont(font);
//    }
}
