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
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include "ocrresultwidget.h"
#include "controller/interaction.h"

OCRResultWidget::OCRResultWidget(QWidget *parent) : QTextEdit(parent)
{
    this->setObjectName("Kylin-photo-viewer_QTextEdit_ocrResult");
    this->setAccessibleName("Kylin-photo-viewer_QTextEdit_ocrResult");
    this->setAccessibleDescription("ocr result widget");
    this->viewport()->setAccessibleName("Kylin-photo-viewer_QWidget_ocrResultViewport");

    this->setReadOnly(true);
    this->setFrameShape(QFrame::NoFrame);
    this->setFontPointSize(10.5);
    this->setFormatedText(tr("OCR recognition..."));
    if (QGSettings::isSchemaInstalled(Variable::FITTHEMEWINDOW.toLocal8Bit())) {
        m_pGsettingThemeData = new QGSettings(Variable::FITTHEMEWINDOW.toLocal8Bit());
        connect(m_pGsettingThemeData,&QGSettings::changed,this,[=](const QString &key) {
            if (key == "styleName") {
                this->setThemeStyle();
            }
        });
    }
    this->setThemeStyle();
}

void OCRResultWidget::setFormatedText(QString plainText)
{

    this->clear();
    plainText = plainText.isEmpty() ? tr("No text dected") : plainText;
    QTextCursor textCursor = this->textCursor();
    QTextBlockFormat textFormat;
    if (plainText == tr("OCR recognition...") || plainText == tr("No text dected")) {
        textFormat.setAlignment(Qt::AlignCenter);
        textFormat.setTopMargin(this->height() / 2 - 40 - 14);
    } else {
        QString locale = QLocale::system().name();
        if (locale == "kk_KZ" || locale =="ug_CN" || locale == "ky_KG") {
            textFormat.setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        } else {
            textFormat.setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        }
        textFormat.setLeftMargin(35);
        textFormat.setRightMargin(35);
        textFormat.setLineHeight(21, QTextBlockFormat::FixedHeight);
    }
    textCursor.insertBlock(textFormat);
    textCursor.insertText(plainText);
}

void OCRResultWidget::setFormatedText(QVector<QString> textContain)
{
    QString result;
    for (QString line : textContain) {
        if (line.isEmpty()) {
            continue;
        }
        result += line + '\n';
    }
    setFormatedText(result);
}

void OCRResultWidget::setThemeStyle()
{
    QString nowThemeStyle = m_pGsettingThemeData->get("styleName").toString();
    QPalette pal = this->palette();
    QColor backgroundColor;
    QColor textColor;
    if (nowThemeStyle == "ukui-dark" || nowThemeStyle == "ukui-black") {
        backgroundColor = QColor(0, 0, 0, 165);
        textColor = QColor(217, 217, 217, 255);
    } else {
        backgroundColor = QColor(255, 255, 255, 165);
        textColor = QColor(38, 38, 38, 255);
    }
    pal.setBrush(QPalette::Base, backgroundColor);
    pal.setBrush(QPalette::Text, textColor);
    this->setPalette(pal);
}
