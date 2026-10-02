/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "languageinfo.h"
#include <QMetaType>

LanguageInfo::LanguageInfo(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<QMap<QString, QString>>("QMap<QString, QString>");
}

LanguageInfo::~LanguageInfo()
{
    if (m_backend) {
        delete m_backend;
        m_backend = nullptr;
    }
}

void LanguageInfo::init()
{
    threadRun = QtConcurrent::run([=]{
        availableLangMap.clear();
        localLangMap.clear();
        installedLangMap.clear();
        if (m_backend) {
            delete m_backend;
            m_backend = nullptr;
        }
        m_backend = new QApt::Backend;
        m_backend->init();
        readLangInfo();
        getAvailableLanguage();
        getInstalledLanguage();
        Q_EMIT initFinished(availableLangMap, installedLangMap);
    });

}

void LanguageInfo::readLangInfo(QString localFile)
{
    QFile file(localFile);
    if (!file.open(QFile::ReadOnly | QFile::Text)){
       qDebug() << localFile << " Error Open XML File: " << file.errorString();
       return;
    }

    QXmlStreamReader reader;
    reader.setDevice(&file);
    if (reader.hasError()){
       qDebug() << localFile << " Error Read XML File: "<<reader.errorString();
    }

    while (!reader.atEnd()) {
        QXmlStreamReader::TokenType nType = reader.readNext();
        if (nType == QXmlStreamReader::StartDocument) {
           continue;
        } else if (nType == QXmlStreamReader::StartElement) {
            if (reader.name().toString() == "iso_639_3_entry") {
                QXmlStreamAttributes attributes = reader.attributes();
                QString code = "id";
                QString lang = "lang";
                if (attributes.hasAttribute("common_name")) {
                    lang = attributes.value("common_name").toString();
                } else {
                    lang = attributes.value("name").toString();
                }
                if (attributes.hasAttribute("part1_code")) {
                    code = attributes.value("part1_code").toString();
                } else {
                    code = attributes.value("id").toString();
                }
                localLangMap.insert(code, lang);
            }
        }
    }

    localLangMap["zh-hans"] = QObject::tr("Chinese (simplified)");
    localLangMap["zh-hant"] = QObject::tr("Chinese (traditional)");

    file.close();
    return;
}

void LanguageInfo::getAvailableLanguage()
{
    QMap<QString, QString>::iterator iter = localLangMap.begin();
    while (iter != localLangMap.end())
    {
        if (isAvailable(iter.key())) {
            if (availableLangMap.find(iter.key()) == availableLangMap.end()) {
                availableLangMap.insert(iter.key(), iter.value());
            }
        }
        iter++;
    }
    return;
}

void LanguageInfo::getInstalledLanguage()
{
    QMap<QString, QString>::iterator iter = localLangMap.begin();
    while (iter != localLangMap.end())
    {
        if (isInstalled(iter.key())) {
            if (installedLangMap.find(iter.key()) == installedLangMap.end()) {
                installedLangMap.insert(iter.key(), iter.value());
            }
        }
        iter++;
    }
    return;
}

bool LanguageInfo::isAvailable(QString languageCode)
{
    QApt::Package *p1 = m_backend->package(QString("language-pack-") + languageCode);
    QApt::Package *p2 = m_backend->package(QString("language-pack-") + languageCode  + QString("-base"));
    if (p1 && p2) {
        return !p1->availableVersion().isEmpty() && !p2->availableVersion().isEmpty();
    }
    return false;
}
bool LanguageInfo::isInstalled(QString languageCode)
{
    QApt::Package *p1 = m_backend->package(QString("language-pack-") + languageCode);
    QApt::Package *p2 = m_backend->package(QString("language-pack-") + languageCode + QString("-base"));
    if (p1 && p2) {
        return p1->isInstalled() && p2->isInstalled();
    }

    return false;
}
