#include "utils.h"
#include "interface.h"
#include <QBuffer>
#include <QClipboard>
#include <QCursor>
#include <QDebug>
#include <QGuiApplication>
#include <QMimeData>
#include <QPainter>
#include <QTextDocument>
#include <QX11Info>
#include <QPalette>

#include <X11/Xlib.h>

QImage convertPlainTextToImage(const QString& plainText)
{
    // 创建 QTextDocument 对象，并设置富文本数据
    QTextDocument textDocument;
    textDocument.setPlainText(plainText);
    // 创建图片对象，并设置大小
    QImage image(textDocument.idealWidth(), textDocument.size().height(), QImage::Format_ARGB32);
    QColor c = qApp->palette().color(QPalette::Base);
    image.fill(c);
    // 创建绘图对象
    QPainter painter(&image);
    // 渲染富文本数据到图片中
    textDocument.drawContents(&painter);
    return image;
}

QImage convertRichTextToImage(const QString& richText)
{
    // 创建 QTextDocument 对象，并设置富文本数据
    QTextDocument textDocument;
    textDocument.setHtml(richText);
    // 创建图片对象，并设置大小
    QImage image(textDocument.idealWidth(), textDocument.size().height(), QImage::Format_ARGB32);
    QColor c = qApp->palette().color(QPalette::Base);
    image.fill(c);
    // 创建绘图对象
    QPainter painter(&image);
    // 渲染富文本数据到图片中
    textDocument.drawContents(&painter);
    return image;
}

QString convertRichTextToPlainText(const QString& richText)
{
    // 创建 QTextDocument 对象并加载富文本
    QTextDocument textDocument;
    textDocument.setHtml(richText);

    QString plainText = textDocument.toPlainText();

    return plainText;
}

QString convertImageToPlainText(const QImage &image)
{
    // OcrSession session;
    // ocr_create_session(&session);

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG"); // 使用PNG格式，您可以选择其他格式
    QByteArray imageData = buffer.data();

    // 将QByteArray转换为uchar*并存储到std::string
    std::string apiKey_(imageData.constData(), imageData.size());

    const char* textFromImageData = getTextFromImageData(imageData.data(), imageData.size());
    QString resultStr(textFromImageData);
    delete textFromImageData;
    return resultStr;
}

QString convertImageToRichText(const QImage &image)
{
    return QString();
}

void copyImagesToClipboard(const QList<QImage> &images)
{
    QByteArray packedData = packMultiImages(images);
    // 创建MIME数据
    QMimeData *mimeData = new QMimeData();
    mimeData->setData("application/x-multiple-images", packedData); // 自定义MIME类型
    QGuiApplication::clipboard()->setMimeData(mimeData);
}

bool grabKeyboard()
{
    int grabResult = XGrabKeyboard(QX11Info::display(), QX11Info::appRootWindow(), True, GrabModeAsync, GrabModeAsync, CurrentTime);
    return grabResult == GrabSuccess;
}

void ungrabKeyboard()
{
    XUngrabKeyboard(QX11Info::display(), CurrentTime);
    XFlush(QX11Info::display());
}

QByteArray packMultiImages(const QList<QImage> &images)
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    for (const auto& image : images) {
        // 将每张QImage保存为字节流（如PNG格式）
        QByteArray imageData;
        QBuffer buffer(&imageData);
        buffer.open(QIODevice::WriteOnly);
        image.save(&buffer, "PNG");
        buffer.close();

        // 将字节流长度写入数据流，便于接收方解析
        quint32 length = imageData.size();
        stream << length;
        stream.writeRawData(imageData.constData(), length);
    }

    return data;
}
