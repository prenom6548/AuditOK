/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "savefilebase.h"
#include <QFile>
#include "../saneobject.h"
#include <QPixmap>
#include <QPainter>
#include <QDebug>
#include <QFile>
#include <QThread>
#include <QApplication>
#include "createofd.h"

constexpr inline int U(const char *str)
{
    return str[0] + (str[1] ? U(str + 1) : 0);
}
SaveFileBase::SaveFileBase(QObject *parent) : QObject(parent)
{

}

void SaveFileBase::saveFileOP(QString filename,QImage *imgOP){
    m_fileName = filename;
    bool res = imageSave(imgOP);
    if (!res) {
        saveToPdf(imgOP);
    }
}
bool SaveFileBase::imageSave(QImage *imgOP)
{
    qDebug() << "save image: " << m_fileName;
    if (g_sane_object->ocrFlag == 0) {
        if (m_fileName.endsWith(".pdf"))
            return false;
        /*****************mark***************************/
        QImage tmp = imgOP->copy();
        /*****************mark***************************/
        if (m_fileName.endsWith(".jpg") || m_fileName.endsWith(".bmp"))
            tmp.save(m_fileName);
        if (m_fileName.endsWith(".tiff")){
            saveAsTiff(tmp, m_fileName);
        }
        if(m_fileName.endsWith(".png") || m_fileName.endsWith(".ofd")){
            if(m_fileName.endsWith(".ofd")){
                m_ofdName.append(m_fileName);
                m_fileName.replace(".ofd", ".png");
                m_pngName.append(m_fileName);
                m_ofdCount++;
            }
            QImage img = *imgOP;
            img.save(m_fileName);

            if(m_ofdCount == g_sane_object->scanPageNumber){
                if(g_sane_object->scanPageNumber==1){
                    bool result = OFD::createOFD(m_ofdName.at(0), m_pngName);
                    if(result){
                        QFile::remove(m_pngName[0]);
                    }
                }else{
                    QString ofdname = QString(m_ofdName[0]).remove("[1]");
                    bool result = OFD::createOFD(ofdname, m_pngName);
                    if(result){
                        for (int var = 0; var < m_pngName.length(); ++var) {
                            QFile::remove(m_pngName[var]);
                        }
                    }
                }
                m_ofdCount = 0;
                m_ofdName.clear();
                m_pngName.clear();
            }
        }
    } else {
        if (!m_fileName.endsWith(".txt"))
            m_fileName += ".txt";
        QFile file(m_fileName);
        file.open(QIODevice::ReadWrite | QIODevice::Text);
        QByteArray str = g_sane_object->ocrOutputText.toUtf8();
        file.write(str);
        file.close();
    }
    g_user_signal->exitWindowWithSaveFlag = false;
    return true;
}

void SaveFileBase::saveToPdf(QImage *imgOP)
{
    QImage tmp = imgOP->copy();
    QFile pdfFile(m_fileName);
    pdfFile.open(QIODevice::WriteOnly);
    QPdfWriter *pdfWriter = new QPdfWriter(&pdfFile);
    setPdfSize(&tmp, pdfWriter, g_sane_object->userInfo.size);

    int resolution = g_sane_object->resolutionValue;
    pdfWriter->setResolution(resolution);

    int pageMargin = 0;
    pdfWriter->setPageMargins(QMarginsF(pageMargin, pageMargin, pageMargin, pageMargin));
    if(tmp.width() < tmp.height()){
        pdfWriter->setPageOrientation(QPageLayout::Portrait);
    }else{
        pdfWriter->setPageOrientation(QPageLayout::Landscape);
    }
    QPainter *pdfPainter = new QPainter(pdfWriter);

    int yCurrentP = 0;
    int xCurrentP = 0;

    QPixmap pixmap = QPixmap::fromImage(tmp);

    pdfPainter->drawPixmap(xCurrentP, yCurrentP, pixmap.width(), pixmap.height(), pixmap);
    pdfPainter->end();
    delete pdfPainter;
    delete pdfWriter;
    pdfFile.close();
    g_user_signal->exitWindowWithSaveFlag = false;
    m_pdfCount++;
    m_pdfName.append(m_fileName);

    qDebug() << "----------------------------";
    qDebug() << g_sane_object->scanPageNumber;
    qDebug() << g_sane_object->isOnScanning;
    qDebug() << "----------------------------";

    if(m_pdfCount == g_sane_object->scanPageNumber && g_sane_object->scanPageNumber != 1 && g_sane_object->isOnScanning == false){
        QFileInfo a = QFileInfo(m_fileName);
        QString outputName = a.absolutePath() + "/" + a.baseName().split("[")[0] + ".pdf";
        QString str = QString("pdfunite ");
        for(int i = 0; i < m_pdfName.length(); i++){
            str += m_pdfName[i];
            str += " ";
        }
        str += outputName;
        qDebug() << str;
        QProcess *process = new QProcess();
        process->start(str);
        m_pdfCount = 0;
        if(process->waitForFinished()){
            for(int i = 0; i < m_pdfName.length(); i++){
                QFile a(m_pdfName[i]);
                if(a.exists()){
                    a.remove();
                }
            }
            m_pdfName.clear();
            process->close();
        }
    }
    if(g_sane_object->scanPageNumber == 1){
        m_pdfName.clear();
        m_pdfCount = 0;
    }
}

void SaveFileBase::setPdfSize(QImage *tmp, QPdfWriter *pdfWriter, QString size)
{
    switch (toUnicode(size)) {
    case U("A0"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A0));
        break;
    case U("A1"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A1));
        break;
    case U("A2"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A2));
        break;
    case U("A3"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A3));
        break;
    case U("A4"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A4));
        break;
    case U("A5"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A5));
        break;
    case U("A6"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A6));
        break;
    case U("A7"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A7));
        break;
    case U("A8"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A8));
        break;
    case U("A9"):
        pdfWriter->setPageSize(QPageSize(QPageSize::A9));
        break;
    default:
        pdfWriter->setPageSize(QPageSize(QPageSize::A4));
        break;
    }

}

void SaveFileBase::saveAsTiff(QImage image, QString file_name)
{
    m_tiffImageList.append(image);
    QString fileTrueName = QFileInfo(file_name).absolutePath() + "/" + QFileInfo(file_name).baseName() + ".tiff";
    m_fileImages.append(fileTrueName);
    image.save(fileTrueName);

    QString save_name = g_sane_object->saveFullScanFileName + ".tiff";
    if(!QFile::exists(save_name)){
        m_tiffImageList[0].save(save_name);
    }

    FREE_IMAGE_FORMAT fif = FIF_TIFF;
    FIBITMAP *page;

    int i = 1;
    int count = g_sane_object->loadFullScanFileNames.length();
    FIMULTIBITMAP *newfile;
    if(m_tiffImageList.length() == count){
        FIMULTIBITMAP *src = FreeImage_OpenMultiBitmap(fif, save_name.toLocal8Bit().data(), false, false, true, 0);
        for(int i = 1; i < m_tiffImageList.length(); i++){
            QString newFileName = m_fileImages[i];
            newfile = FreeImage_OpenMultiBitmap(fif, newFileName.toLocal8Bit().data(), false, true);
            page = FreeImage_LockPage(newfile, 0);
            m_filist.append(FreeImage_Clone(page));
            FreeImage_UnlockPage(newfile, page, 0);
        }
        FreeImage_CloseMultiBitmap(src);
    }
    bool result = false;
    if(m_filist.length() == (count - 1)){
        FIMULTIBITMAP *out = FreeImage_OpenMultiBitmap(fif, save_name.toLocal8Bit().data(), false, false);
        for(int i = 0; i < m_filist.length(); i++){
            FreeImage_AppendPage(out, m_filist.at(i));
        }
        result = FreeImage_CloseMultiBitmap(out);
    }

    if(result || ((g_sane_object->userInfo.pageNumber.compare(QApplication::tr("Single"), Qt::CaseInsensitive) == 0 || g_sane_object->userInfo.pageNumber.compare("Single", Qt::CaseInsensitive) == 0) && (g_sane_object->userInfo.type.compare(QApplication::tr("ADF Duplex"), Qt::CaseInsensitive) != 0 || g_sane_object->userInfo.type.compare("ADF Duplex", Qt::CaseInsensitive) != 0))){
        for(int j = 0; j < m_fileImages.length(); j++){
            QString filename = m_fileImages.at(j);
            if(filename != save_name){
                QFile *a = new QFile(filename);
                if(a->exists()){
                    a->remove();
                }
                a->deleteLater();
            }
        }
        m_tiffImageList.clear();
        m_fileImages.clear();
        m_filist.clear();
    }
}

int SaveFileBase::toUnicode(QString str)
{
    char  *ch;
    QByteArray ba = str.toLatin1();
    ch = ba.data();
    return ch[0] + (ch[1] ? toUnicode(ch + 1) : 0);

}
