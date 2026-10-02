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

QString ImageBackend::statusText() const
{
    return m_statusText;
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
        setStatusText(tr("Open failed"));
        return;
    }

    if (!updateImageSource(selectedFiles.first()))
        setStatusText(tr("Open failed"));
}

void ImageBackend::closeImage()
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

    if (!m_statusText.isEmpty()) {
        m_statusText.clear();
        emit statusTextChanged();
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

    if (!m_statusText.isEmpty()) {
        m_statusText.clear();
        emit statusTextChanged();
    }

    return true;
}

void ImageBackend::setStatusText(const QString &text)
{
    const bool failed = !text.isEmpty();
    if (m_openFailed != failed) {
        m_openFailed = failed;
        emit openFailedChanged();
    }

    if (failed && m_hasImage) {
        m_hasImage = false;
        emit hasImageChanged();
    }

    if (m_statusText == text)
        return;
    m_statusText = text;
    emit statusTextChanged();
}

void ImageBackend::reportImageLoadFailed()
{
    if (m_imageSource.isEmpty())
        return;

    m_imageSource = QUrl();
    emit imageSourceChanged();
    setStatusText(tr("Open failed"));
}

