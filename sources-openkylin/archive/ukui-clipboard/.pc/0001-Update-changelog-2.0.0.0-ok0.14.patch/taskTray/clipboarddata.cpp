#include "clipboarddata.h"
#include <QDebug>
#include <QBuffer>
#include <QMimeData>

clipboardData::clipboardData(QObject *parent)
    : QObject(parent), m_sourceText("")
{

}

QString clipboardData::id()
{
    return m_id;
}

void clipboardData::setId(QString str)
{
    m_id = str;
}

bool clipboardData::hasImage()
{
    return m_hasImage;
}

void clipboardData::setHasImage(bool value)
{
    m_hasImage = value;
    emit HasImageChanged();
}

QUrl clipboardData::imageUrl()
{
    return m_imageUrl;
}

void clipboardData::setImage(QImage ima)
{
    QByteArray bytes;
    QBuffer buffer(&bytes);
    buffer.open(QIODevice::WriteOnly);
    ima.save(&buffer, "PNG"); // saves the image into bytes in PNG format
    m_imageUrl = QString("data:image/png;base64,") + bytes.toBase64();
    emit imageUrlChanged();
}

QString clipboardData::sourceText()
{
    return m_sourceText;
}

void clipboardData::setSourceText(QString str)
{
    m_sourceText = str;
    emit sourceTextChanged();
}

QDateTime clipboardData::sourceTime()
{
    return m_sourceTime;
}

void clipboardData::setSourceTime(QDateTime dataTime)
{
    m_sourceTime = dataTime;
    emit sourceTimeChanged();
}

bool clipboardData::isCollected()
{
    return m_isCollected;
}

void clipboardData::setIsCollected(bool flag)
{
    qDebug()<<"flag="<<flag;
    if (m_isCollected != flag) {
        m_isCollected = flag;
        emit isCollectedChanged();
    }
}

void clipboardData::copyToClipboard()
{
    if (m_mineData == nullptr || m_dataClipboard == nullptr) {
        qWarning()<<"数据为空";
    } else {
        // 设置新的QMimeData到剪贴板
        m_dataClipboard->setMimeData(m_mineData);
        emit dataCopyed();
    }
}
