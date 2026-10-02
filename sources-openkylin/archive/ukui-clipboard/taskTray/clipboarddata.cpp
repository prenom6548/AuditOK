#include "clipboarddata.h"
#include <QDebug>
#include <QBuffer>
#include <QMimeData>
#include <QFileInfo>
#include <QFileIconProvider>
#include <QFontMetrics>

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

bool clipboardData::isLinkMode()
{
    return m_isLinkMode;
}

void clipboardData::setIsLinkMode(bool value)
{
    m_isLinkMode = value;
    emit isLinkModeChanged();
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

//QClipboard::setMimeData(QMineData *mimeData)函数在执行后
//mimeData指针会出现异常，用该指针调用QMineData::hasUrls()等函数qt会报错
//再次将该指针用为setMimeData（）函数的参数qt也会报莫名错误
QMimeData* clipboardData::getMimedata(QMimeData* mData)
{
    QMimeData *newMimeData = new QMimeData();

    QStringList mimeTypeList = mData->formats();
    foreach (const QString &mimeType, mimeTypeList) {
        QByteArray data = mData->data(mimeType);
        newMimeData->setData(mimeType,data);
    }
    if (mData->hasImage()) {
        newMimeData->setImageData(mData->imageData());
    }

    return newMimeData;
}

void clipboardData::sendNeedMoveToFirst() {
    if (m_mineData == nullptr || m_dataClipboard == nullptr) {
        qWarning()<<"数据为空";
    } else {
        // 设置新的QMimeData到剪贴板
        m_dataClipboard->setMimeData(getMimedata(m_mineData));
        emit needMoveToFirst();
    }
}

void clipboardData::updateImage()
{
    QString localFilePath = m_mineData->urls().at(0).toLocalFile();
    QFileInfo fileInfo(localFilePath);
    QFileIconProvider iconProvider;
    QIcon fileIcon = iconProvider.icon(fileInfo);
    int width = 48, height = 48;  // 你可以根据需要调整这个尺寸
    QPixmap pixmap = fileIcon.pixmap(width, height);
    QImage image = pixmap.toImage();
    setImage(image);
}

void clipboardData::setIconName(QString str)
{
    if (!str.isEmpty()) {
        m_iconName = str;
    }
}

QString clipboardData::getIconName()
{
    if (!m_iconName.isEmpty()) {
        qDebug()<<"iconname="<<m_iconName;
        return m_iconName;
    } else {
        qDebug()<<"iconname=";
        return "Unknow";
    }
}

void clipboardData::sendNeedToDelete()
{
    emit needToDelete();
}

void clipboardData::sendNeedToDeleteAll()
{
    emit needToDeleteAll();
}

clipboardData::Mode clipboardData::getMode()
{
    return mode;
}

QString clipboardData::getFilePath()
{
    if (mode == clipboardData::File || mode == clipboardData::Image) {
        QString str = m_mineData->urls().at(0).toLocalFile();
        return str;
    } else {
        qDebug()<<"不是图片或文件";
        return "";
    }
}

QString clipboardData::getToolTipString()
{
    if (mode == clipboardData::File) {
        QString str = tr("File name: ") + sourceText() + "\n" + tr("File path: ")+getFilePath();
        return str;
    } else if (mode == clipboardData::Text){
        QFont font;
        QString str = sourceText();
        QFontMetrics fm(font);
        return fm.elidedText(str, Qt::ElideRight, 10000);//超过长度裁剪
    }
    return "";
}

bool clipboardData::isSame(clipboardData *data) {
    if (data->mode == clipboardData::Text || data->mode == clipboardData::Link) {
        return m_sourceText == data->sourceText();
    } else if (data->mode == clipboardData::File) {
        return m_mineData->urls() == data->m_mineData->urls();
    } else if (data->mode == clipboardData::Image) {
        return m_mineData->urls() == data->m_mineData->urls() && m_imageUrl.toString() == data->imageUrl().toString();;
    }
    return false;
}
