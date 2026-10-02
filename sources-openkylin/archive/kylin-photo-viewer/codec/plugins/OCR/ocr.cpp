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
#include <QtConcurrent>
#include <QMutex>
#include <QDebug>
#include <QFile>
#include "ocr.h"
#include "kylinocr.h"

#define LOG_FLAG "OCR_MODULE#"

OCR::OCR() {}
OcrFunction::OcrFunction()
{
    m_thread = new QThread;
}
void OcrFunction::setEngineType(EngineType type)
{
    m_type = type;
}

void OcrFunction::getText(QString imgPath)
{
    if (m_ocr) {
        disconnect(m_ocr, &OCR::recResult, this, &OcrFunction::recResult);
        disconnect(this, &OcrFunction::realGetText, m_ocr, &OCR::getText);
        m_ocr->deleteLater();
    }
    // TODO:目前因为子类中无私有属性，目前不会造成内存泄漏，所以先作为待办项
    switch (m_type) {
    case EngineType::KySDK:
        m_ocr = dynamic_cast<OCR *>(new KylinOCR());
        m_ocr->moveToThread(m_thread);
        m_thread->start();
        break;
    default:
        qWarning() << LOG_FLAG << "Invaild OCR engine type";
    }
    if (!QFile::exists(imgPath) || !m_ocr) {
        qWarning() << LOG_FLAG << "getText Failed!Check image path or set engine type first?";
        return;
    }
    connect(m_ocr, &OCR::recResult, this, &OcrFunction::recResult);
    connect(this, &OcrFunction::realGetText, m_ocr, &OCR::getText);
    Q_EMIT realGetText(imgPath);
}
