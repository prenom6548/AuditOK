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


#ifndef PHOTO_PRINTER_H
#define PHOTO_PRINTER_H

#include <QObject>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QPrintDialog>
#include <QPrinter>
#include <QGSettings>
#include "kylinimagecodec.h"

class PhotoPrinter : public QObject
{
    Q_OBJECT
public Q_SLOTS:
    void acceptPrint(QPrinter *printer);//打印开始
public:
    PhotoPrinter(const QStringList &picPaths);
    ~PhotoPrinter();
    void importPrintSettings(QPrinter *printer);
    void exportPrintSettings(QPrinter *printer);

private:
    const QByteArray schemasId = "org.kylin-photo-printer.settings";
    const QString gsettingsFlag = "print";
    const QString gsettingsKey = "print";
    QStringList m_picPaths;
    QPrintDialog *m_printDialog;
    QPrinter *m_printer;
    QList<Mat> m_mats;

    void finishPrint(int result);//返回值
};

#endif // PHOTO_PRINTER_H