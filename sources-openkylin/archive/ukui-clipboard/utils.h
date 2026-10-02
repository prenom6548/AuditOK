#pragma once

#include <QImage>

QImage convertPlainTextToImage(const QString& plainText);

QImage convertRichTextToImage(const QString& richText);

QString convertRichTextToPlainText(const QString& richText);

QString convertImageToPlainText(const QImage &image);

QString convertImageToRichText(const QImage &image);

void copyImagesToClipboard(const QList<QImage> &images);

bool grabKeyboard();

void ungrabKeyboard();

static QByteArray packMultiImages(const QList<QImage> &images);
