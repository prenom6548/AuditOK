// customalbumviewer.cpp
#include "customalbumviewer.h"
#include "../common/commonfunc.h"
#include <QFileInfo>
#include <QDir>
#include <QPixmap>
#include <QDebug>
#include <QRandomGenerator>

CustomAlbumViewer::CustomAlbumViewer(QWidget *parent)
    : QWidget(parent)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(32);

    m_mainLayout->addStretch();
    m_tipLabel= new QLabel(this);
    m_tipLabel->setStyleSheet("QLabel{ color: white;}");
    m_tipLabel->setText(tr("No photos detected, please re-select them"));
    m_mainLayout->addWidget(m_tipLabel, Qt::AlignHCenter);

    m_topLabel = new QLabel(this);
    m_topLabel->setAlignment(Qt::AlignCenter);
    m_topLabel->setFixedHeight(372);
    m_mainLayout->addWidget(m_topLabel, Qt::AlignHCenter);

    m_bottomLayout = new QHBoxLayout();
    m_bottomLayout->setContentsMargins(0, 0, 0, 0);
    m_bottomLayout->setSpacing(32);
    m_mainLayout->addLayout(m_bottomLayout);
//    m_mainLayout->addStretch();

    m_leftBottomLabel = new QLabel(this);
    m_rightBottomLabel = new QLabel(this);
    m_bottomLayout->addStretch();
    m_bottomLayout->addWidget(m_leftBottomLabel);
    m_bottomLayout->addWidget(m_rightBottomLabel);
    m_bottomLayout->addStretch();

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &CustomAlbumViewer::nextSlide);
}

CustomAlbumViewer::~CustomAlbumViewer()
{
    m_timer->stop();
}

void CustomAlbumViewer::updateViewerSize(float scale)
{
    if (scale < 0.1) {
        scale = 1;
    }
    m_scale = scale;
    if (m_imagePaths.isEmpty()) {
        m_topLabel->setFixedSize(804 * scale, 512 * scale);
    } else if (m_imagePaths.size() < 3) {
        m_topLabel->setFixedSize(804 * scale, 788 * scale);
    } else {
        m_topLabel->setFixedSize(847 * scale, 372 * scale);
        m_leftBottomLabel->setFixedSize(406 * scale, 372 * scale);
        m_rightBottomLabel->setFixedSize(406 * scale, 372 * scale);
    }
    m_mainLayout->setSpacing(32 * scale);
    m_bottomLayout->setSpacing(32 * scale);
}

void CustomAlbumViewer::setDefaultPixmap(QPixmap pixmap)
{
    m_defaultImage = pixmap;
}

QPixmap CustomAlbumViewer::scalePixmap(const QPixmap &original, const QSize &size)
{
    return original.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void CustomAlbumViewer::loadImages(const QStringList &directoryPaths)
{
    m_imagePaths.clear();
    foreach (const QString &directoryPath, directoryPaths) {
        QDir directory(directoryPath);
        if (!directory.exists()) {
            qWarning() << "Directory does not exist:" << directoryPath;
            continue;
        }
        QStringList filters;
        filters << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp" << "*.gif";
        QFileInfoList fileList = directory.entryInfoList(filters, QDir::Files);
        foreach (const QFileInfo &fileInfo, fileList) {
            m_imagePaths.append(fileInfo.absoluteFilePath());
        }
    }
    qDebug() << "Loaded" << m_imagePaths.size() << "images";
}

void CustomAlbumViewer::startSlideshow(int interval)
{
    if (m_timer->isActive()) {
        m_timer->stop();
    }
    m_timer->start(interval);
    nextSlide();
}

void CustomAlbumViewer::nextSlide()
{
    if (m_imagePaths.isEmpty()) {
        QPixmap defaultPix(m_defaultImage);
        m_topLabel->setPixmap(PixmapToRound(scalePixmap(defaultPix, m_topLabel->size()), 26 * m_scale, 26 * m_scale, 26 * m_scale, 26 * m_scale));
        m_topLabel->show();
        m_tipLabel->show();
        m_leftBottomLabel->hide();
        m_rightBottomLabel->hide();
        return;
    }
    m_tipLabel->hide();
    displayImages();
}

void CustomAlbumViewer::displayImages()
{
    if (m_imagePaths.size() < 3) {
        m_leftBottomLabel->hide();
        m_rightBottomLabel->hide();
        int index = m_isSequential ? m_currentIndex % m_imagePaths.size() :
            QRandomGenerator::global()->bounded(m_imagePaths.size());

        QPixmap pixmap(m_imagePaths[index]);
        m_topLabel->setPixmap(PixmapToRound(scalePixmap(pixmap, m_topLabel->size()), 26 * m_scale, 26 * m_scale, 26 * m_scale, 26 * m_scale));
        m_topLabel->show();

        if (m_isSequential) {
            m_currentIndex = (m_currentIndex + 1) % m_imagePaths.size();
        }
    } else {
        m_topLabel->show();
        m_leftBottomLabel->show();
        m_rightBottomLabel->show();

        QList<int> indices;
        if (m_isSequential) {
            // 顺序模式
            for (int i = 0; i < 3; ++i) {
                indices.append((m_currentIndex + i) % m_imagePaths.size());
            }
            m_currentIndex = (m_currentIndex + 3) % m_imagePaths.size();
        } else {
            // 随机模式
            while (indices.size() < 3) {
                int randomIndex = QRandomGenerator::global()->bounded(m_imagePaths.size());
                if (!indices.contains(randomIndex)) {
                    indices.append(randomIndex);
                }
            }
        }
        QPixmap topPixmap(m_imagePaths[indices[0]]);
        m_topLabel->setPixmap(PixmapToRound(scalePixmap(topPixmap, m_topLabel->size()), 26 * m_scale, 26 * m_scale, 26 * m_scale, 26 * m_scale));

        QPixmap leftBottompixmap(m_imagePaths[indices[1]]);
        m_leftBottomLabel->setPixmap(PixmapToRound(scalePixmap(leftBottompixmap, m_leftBottomLabel->size()),  26 * m_scale, 26 * m_scale, 26 * m_scale, 26 * m_scale));

        QPixmap rightBottompixmap(m_imagePaths[indices[2]]);
        m_rightBottomLabel->setPixmap(PixmapToRound(scalePixmap(rightBottompixmap, m_rightBottomLabel->size()),  26 * m_scale, 26 * m_scale, 26 * m_scale, 26 * m_scale));
    }
    Q_EMIT pixmapChanged();
}

void CustomAlbumViewer::stopDisplayImages()
{
    if (m_timer->isActive()) {
        m_timer->stop();
    }
}

void CustomAlbumViewer::setSequentialMode(bool sequential)
{
    m_isSequential = sequential;
    m_currentIndex = 0; // 重置索引
}

void CustomAlbumViewer::setRandomMode(bool random)
{
    m_isSequential = !random;
}
