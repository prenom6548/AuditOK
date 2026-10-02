/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#include <QDebug>
#include <QPainter>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QStringList>
#include <QVariant>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QDateTime>
#include <QIcon>

#include "photo_printer.h"


PhotoPrinter::PhotoPrinter(const QStringList &picPaths)
    : m_picPaths(picPaths)
{
    m_mats.clear();
    for (auto picPath : m_picPaths) {
        qDebug() << picPath;
        MatResult matr = KylinImageCodec::loadImageToMat(picPath);
        if (matr.matList == nullptr) {
            m_mats.append(matr.mat);
        } else {
            for (int i = 0; i < matr.matList->size(); i++) {
                m_mats.append(matr.matList->at(i));
            }
        }
    }
    m_printer = new QPrinter();
    QString docName;
    // importPrintSettings(m_printer);
    if (m_picPaths.size() == 1) {
        QFileInfo info = QFileInfo(m_picPaths.at(0));
        docName = info.fileName();
    } else {
        docName = tr("Picture printing tasks: ") + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm ddd");
    }
    qDebug() << docName;
    m_printer->setDocName(docName);
    m_printDialog = new QPrintDialog(m_printer);
    m_printDialog->setProperty("ukui_surface_icon", "kylin-photo-viewer");
    m_printDialog->setMinMax(1, m_mats.size());
    connect(m_printDialog, SIGNAL(accepted(QPrinter *)), this, SLOT(acceptPrint(QPrinter *)));
    connect(m_printDialog, &QPrintDialog::finished, this, &PhotoPrinter::finishPrint);
    m_printDialog->setModal(true);
    m_printDialog->show();
}

PhotoPrinter::~PhotoPrinter() {}

void PhotoPrinter::acceptPrint(QPrinter *printer)
{
    int l;
    int r;
    switch (printer->printRange()) {
    case QPrinter::PageRange:
        l = max(printer->fromPage() - 1, 0);
        r = min(printer->toPage() - 1, m_mats.size() - 1);
        break;
    default:
        l = 0;
        r = m_mats.size() - 1;
        break;
    }

    int cnt = 0;
    QPainter p(printer);

    for (int i = l; i <= r; i++) {
        Mat mat = m_mats.at(i);
        if (cnt != 0) {
            printer->newPage();
        }
        QPixmap pix = KylinImageCodec::converFormat(mat);
        QImage img = pix.toImage();
        static QRect rect = p.viewport();
        QSize size = img.size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        p.setViewport(rect.x(), rect.y(), size.width(), size.height());
        p.setWindow(img.rect());
        p.drawImage(0, 0, img);
        cnt++;
    }
}

//打印结束
void PhotoPrinter::finishPrint(int result)
{
    //失败才打印
    if (1 != result) {
        qDebug() << "========" << result;
    }
}

// 打印设置存储，以后可能会提需要保存上一次打印的设置。
void PhotoPrinter::importPrintSettings(QPrinter *printer)
{
    if (!QGSettings::isSchemaInstalled(schemasId)) {
        return;
    }
    QGSettings printerSettings(schemasId);
    QVariant var = printerSettings.get(gsettingsKey);
    QByteArray s = var.toByteArray();

    QJsonParseError jsonParserError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(s, &jsonParserError);

    if (jsonDocument.isNull() || jsonParserError.error != QJsonParseError::NoError) {
        qDebug() << "json解析失败";
        return;
    }
    if (!jsonDocument.isObject()) {
        return;
    }

    QJsonObject jsonObject = jsonDocument.object();
    qDebug() << jsonObject;
    if (jsonObject.size() == 0) {
        qDebug() << "json is empty!";
        return;
    }
    QJsonObject::Iterator it = jsonObject.begin();
    for (it = jsonObject.begin(); it != jsonObject.end(); it++) {
        if (!it.value().isObject() || it.key().isEmpty())
            continue;
        QJsonObject subObj = it.value().toObject();
        if (subObj.contains("printerName") && subObj.value("printerName").isString()) {
            printer->setPrinterName(subObj.value("printerName").toString());
        } else {
            continue;
        }
    }
}

void PhotoPrinter::exportPrintSettings(QPrinter *printer)
{
    QJsonObject jsonObject;
    QJsonObject subObj;
    subObj.insert("printerName", printer->printerName());
    jsonObject.insert("aaa", subObj);

    QJsonDocument jsonDoc;
    jsonDoc.setObject(jsonObject);

    if (!QGSettings::isSchemaInstalled(schemasId)) {
        return;
    }
    QGSettings printerSettings(schemasId);
    printerSettings.set(gsettingsKey, jsonDoc.toJson());

    return;
}
