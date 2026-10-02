#include "imagebackend.h"

#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>

ImageBackend::ImageBackend(QObject *parent)
    : QObject(parent),
      m_hasImage(false),
      m_openFailed(false)
{
}

QUrl ImageBackend::imageSource() const
{
    return m_imageSource;
}

bool ImageBackend::hasImage() const
{
    return m_hasImage;
}

bool ImageBackend::openFailed() const
{
    return m_openFailed;
}

void ImageBackend::initializeImage(const QString &configuredLocalFile)
{
    if (configuredLocalFile.isEmpty()) {
        clearImageState();
        return;
    }

    if (m_hasImage && !m_openFailed
            && m_imageSource.toLocalFile() == configuredLocalFile)
        return;

    if (!updateImageSource(configuredLocalFile)) {
        clearImageState();
        setOpenFailed(true);
    }
}

void ImageBackend::openImage()
{
    QFileDialog fileDialog;
    fileDialog.setWindowTitle(tr("Open Image"));
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setNameFilter(tr("Images (*.png *.jpg *.jpeg *.bmp *.gif *.webp *.svg)"));

    if (fileDialog.exec() != QDialog::Accepted)
        return;

    const QStringList selectedFiles = fileDialog.selectedFiles();
    if (selectedFiles.isEmpty()) {
        setOpenFailed(true);
        return;
    }

    if (!updateImageSource(selectedFiles.first()))
        setOpenFailed(true);
}

void ImageBackend::closeImage()
{
    clearImageState();
    emit persistedImagePathChanged(QString());
}

void ImageBackend::clearImageState()
{
    if (!m_imageSource.isEmpty()) {
        m_imageSource = QUrl();
        emit imageSourceChanged();
    }

    if (m_hasImage) {
        m_hasImage = false;
        emit hasImageChanged();
    }

    if (m_openFailed) {
        m_openFailed = false;
        emit openFailedChanged();
    }

}

bool ImageBackend::updateImageSource(const QString &localFile)
{
    QFileInfo fileInfo(localFile);
    if (!fileInfo.exists() || !fileInfo.isFile())
        return false;

    QImageReader reader(localFile);
    if (!reader.canRead())
        return false;

    const QUrl newSource = QUrl::fromLocalFile(localFile);
    if (m_imageSource != newSource) {
        m_imageSource = newSource;
        emit imageSourceChanged();
    }

    if (!m_hasImage) {
        m_hasImage = true;
        emit hasImageChanged();
    }

    if (m_openFailed) {
        m_openFailed = false;
        emit openFailedChanged();
    }

    emit persistedImagePathChanged(localFile);
    return true;
}

void ImageBackend::setOpenFailed(bool failed)
{
    if (m_openFailed != failed) {
        m_openFailed = failed;
        emit openFailedChanged();
    }

    if (failed && m_hasImage) {
        m_hasImage = false;
        emit hasImageChanged();
    }

}

void ImageBackend::reportImageLoadFailed()
{
    if (m_imageSource.isEmpty())
        return;

    clearImageState();
    setOpenFailed(true);
}
