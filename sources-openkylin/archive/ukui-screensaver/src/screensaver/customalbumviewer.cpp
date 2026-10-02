// customalbumviewer.cpp
#include "customalbumviewer.h"
#include "../common/commonfunc.h"
#include <QFileInfo>
#include <QDir>
#include <QPixmap>
#include <QDebug>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QCollator>

CustomAlbumViewer::CustomAlbumViewer(QWidget *parent) : QWidget(parent)
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(32);

    m_mainLayout->addStretch();
    m_tipLabel = new CustomLabel(this);
    m_tipLabel->setText(tr("No photos detected, please re-select them"));
    m_tipLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_tipLabel, Qt::AlignHCenter);

    m_topLabel = new AdaptiveLabel(this);
    m_topLabel->setAlignment(Qt::AlignCenter);
    m_topLabel->setFixedHeight(372);
    m_mainLayout->addWidget(m_topLabel, Qt::AlignHCenter);

    m_bottomLayout = new QHBoxLayout();
    m_bottomLayout->setContentsMargins(0, 0, 0, 0);
    m_bottomLayout->setSpacing(32);
    m_mainLayout->addLayout(m_bottomLayout);
    //    m_mainLayout->addStretch();

    m_leftBottomLabel = new AdaptiveLabel(this);
    m_rightBottomLabel = new AdaptiveLabel(this);
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
    if (scale == 0) {
        scale = m_scale;
    } else if (scale < 0.1) {
        scale = 1;
    }
    m_scale = scale;
    if (m_imagePaths.isEmpty()) {
        setFixedSize(847 * scale, 600 * scale);
        m_topLabel->setFixedSize(804 * scale, 512 * scale);
        m_tipLabel->setFixedWidth(m_topLabel->width());
        m_tipLabel->setFontSize(m_scale * 22);
    } else if (m_imagePaths.size() < 3) {
        setFixedSize(847 * scale, 790 * scale);
        m_topLabel->setFixedSize(804 * scale, 788 * scale);
    } else {
        setFixedSize(847 * scale, 790 * scale);
        m_topLabel->setFixedSize(847 * scale, 372 * scale);
        m_leftBottomLabel->setFixedSize(406 * scale, 372 * scale);
        m_rightBottomLabel->setFixedSize(406 * scale, 372 * scale);
    }
    m_topLabel->setScale(scale);
    m_leftBottomLabel->setScale(scale);
    m_rightBottomLabel->setScale(scale);
    m_mainLayout->setSpacing(32 * scale);
    m_bottomLayout->setSpacing(32 * scale);
}

void CustomAlbumViewer::setDefaultPixmap(QPixmap pixmap)
{
    m_defaultImage = pixmap;
    loadImages(m_sSelectPhotoPath);
    if (m_imagePaths.isEmpty()) {
        QPixmap defaultPix(m_defaultImage);
        m_topLabel->setPixmap(defaultPix);
        m_topLabel->show();
        m_tipLabel->show();
    }
}

QPixmap CustomAlbumViewer::scalePixmap(const QPixmap &original, const QSize &size)
{
    return original.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
}

void CustomAlbumViewer::loadImages(const QString &directoryPath)
{
    m_imagePaths.clear();

    QDir directory(directoryPath);
    if (!directory.exists()) {
        qWarning() << "Directory does not exist:" << directoryPath;
        return;
    }

    QStringList filters;
    filters << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp" << "*.gif";
    QFileInfoList fileList = directory.entryInfoList(filters, QDir::Files);

    QRegularExpression prefixPattern(R"(^\d+-usr-share-backgrounds-)");

    QList<QPair<QString, QString>> sortedList;

    foreach (const QFileInfo &fileInfo, fileList) {
        QString originalPath = fileInfo.absoluteFilePath();
        QString fileName = fileInfo.fileName();
        QString cleanedName = fileName;
        cleanedName.remove(prefixPattern);
        sortedList.append(qMakePair(cleanedName, originalPath));
    }

    QCollator collator;
    collator.setNumericMode(true);
    collator.setCaseSensitivity(Qt::CaseInsensitive);

    std::sort(sortedList.begin(), sortedList.end(),
              [&collator](const QPair<QString, QString> &a, const QPair<QString, QString> &b) {
                  return collator.compare(a.first, b.first) < 0;
              });

    foreach (const auto &item, sortedList) {
        m_imagePaths.append(item.second);
    }

    m_sSelectPhotoPath = directoryPath;
}

void CustomAlbumViewer::setUpdateInterval(int interval)
{
    if (m_timer->isActive()) {
        m_timer->stop();
    } else {
        nextSlide();
    }
    m_timer->start(interval);
}

void CustomAlbumViewer::setImagesPath(const QString &strPath)
{
    m_currentIndex = 0; // 重置索引
    loadImages(strPath);
    int nInterval = -1;
    if (m_timer->isActive()) {
        nInterval = m_timer->interval();
        m_timer->stop();
    }
    if (nInterval <= 0) {
        setUpdateInterval();
    } else {
        setUpdateInterval(nInterval);
    }
}

void CustomAlbumViewer::nextSlide()
{
    loadImages(m_sSelectPhotoPath);
    if (m_imagePaths.isEmpty()) {
        QPixmap defaultPix(m_defaultImage);
        m_topLabel->setPixmap(defaultPix);
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
        int index = m_isSequential ? m_currentIndex % m_imagePaths.size()
                                   : QRandomGenerator::global()->bounded(m_imagePaths.size());

        QPixmap pixmap(m_imagePaths[index]);
        m_topLabel->setPixmap(pixmap);
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
        m_topLabel->setPixmap(topPixmap);

        QPixmap leftBottompixmap(m_imagePaths[indices[1]]);
        m_leftBottomLabel->setPixmap(leftBottompixmap);

        QPixmap rightBottompixmap(m_imagePaths[indices[2]]);
        m_rightBottomLabel->setPixmap(rightBottompixmap);
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
    int nInterval = -1;
    if (m_timer->isActive()) {
        nInterval = m_timer->interval();
        m_timer->stop();
    }
    if (nInterval <= 0) {
        setUpdateInterval();
    } else {
        setUpdateInterval(nInterval);
    }
}
