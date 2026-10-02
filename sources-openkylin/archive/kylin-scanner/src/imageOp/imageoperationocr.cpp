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

#include "imageoperationocr.h"
#include "../saneobject.h"
#include "../include/common.h"
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <QDebug>

ImageOperationOCR::ImageOperationOCR(QImage *origin)
{
    m_origin = origin;
    isInterupt = false;
}

void ImageOperationOCR::ImageOP(){
    ocr();
    g_user_signal->toolbarOcrOperationFinished();
    emit finished();
}

void ImageOperationOCR::ocr()
{
    qDebug() << "begin to run ocr thread !\n";
    m_origin->save(ScanningPicture);
    tesseract::TessBaseAPI  *api = new tesseract::TessBaseAPI();

    if (api->Init(NULL, "chi_sim")) {
        qDebug() << "Could not initialize tesseract.\n";
        g_sane_object->ocrOutputText = tr("Unable to read text, please retrey");
        return;
    }
    if(isInterupt){
        if (api)
            api->End();
        return;
    }
    qDebug() << "before pixRead.";
    Pix *image = pixRead(ScanningPicture);
    if (! image) {
        qDebug() << "pixRead error!";
        g_sane_object->ocrOutputText = tr("Unable to read text, please retrey");
        if (api)
            api->End();
        return;
    }
    if(isInterupt){
        if (api)
            api->End();
        return;
    }

    if (image) {
        qDebug() << "before setImage.";
        api->SetImage(image);
        g_sane_object->ocrOutputText = api->GetUTF8Text();
    }
    qDebug() << "before destroy image.";
    if (api) {
        api->End();
    }
    if (image) {
        pixDestroy(&image);
    }
}
