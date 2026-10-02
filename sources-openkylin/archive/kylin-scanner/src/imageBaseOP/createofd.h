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

#ifndef CREATOFD_H
#define CREATOFD_H

#include <QString>
#include <QList>

struct zip;
class OFD
{
public:
    OFD(){}
    static bool createOFD(QString fileName, QStringList imageList);
    static void setOFDTitle(QString value);
    static void setOFDCreator(QString value);
private:
    static void addFileFromData(QString fileName, QByteArray data);
    static void addFileFromFile(QString fileName, QString origFileName);
    static QString getMaxIDAndAdd();
    static void clearImageData();
    static QString m_OFD;
    static QString m_OFDDoc;
    static QString m_OFDContent;
    static QString m_OFDPublicRes;
    static QString m_docTitle;
    static QString m_docCreator;
    static zip *m_zipHandle;
    static int m_docID;
    static QList<char *> m_imageData;
};

#endif // CREATOFD_H
