/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "languagesetting.h"
#include <QApplication>
#include "definetypes.h"
#include <QDebug>
#include <QFileInfo>
#include <QProcess>
#include <QString>

LanguageSetting *LanguageSetting::m_singleInstance = nullptr;

LanguageSetting::LanguageSetting(QObject *parent) : QObject(parent) {}

LanguageSetting::~LanguageSetting() {}

LanguageSetting *LanguageSetting::instance(QObject *parent)
{
    if (m_singleInstance == nullptr)
        m_singleInstance = new LanguageSetting(parent);
    return m_singleInstance;
}

void LanguageSetting::onLanguageChanged(QString languageCode)
{
    if (languageCode == "")
        return;

    if (m_strLanguageCode == languageCode) {
        return;
    }

    qDebug() << "onLanguageChanged " << languageCode;
    Q_EMIT languageChanged(false);
    if (m_translator) {
        qApp->removeTranslator(m_translator);
        delete m_translator;
        m_translator = nullptr;
    }
    m_translator = new QTranslator(this);
    QString qmFile;

    if (languageCode.startsWith("zh_CN")) { /*这里写的有问题，忘记之前为什么写了，会导致繁体也会加载zh_CN*/
        qmFile = QString(WORKING_DIRECTORY "/i18n_qm/%1.qm").arg("zh_CN");
        setenv("LANGUAGE", "zh_CN", 1);
        setenv("LANG", languageCode2Locale("zh_CN", "zh_CN").toLatin1().data(), 1);
        setlocale(LC_ALL, "zh_CN.utf8");
        QLocale lang("zh_CN");
        /*这里设置一个QLocale的默认值，用来影响插件的语言。插件加载翻译文件时，不使用QLocale::system().name()来获取语言，
         * 应该使用QLocale local; local.name() 来获取语言,这样可以识别到登录界面设置的的语言环境*/
        QLocale::setDefault(lang);
    } else {
        qmFile = QString(WORKING_DIRECTORY "/i18n_qm/%1.qm").arg(languageCode);
        setenv("LANGUAGE", languageCode.toLatin1().data(), 1);
        setenv("LANG", languageCode2Locale(languageCode, languageCode).toLatin1().data(), 1);
        setlocale(LC_ALL, "");
        QLocale lang(languageCode);
        /*这里设置一个QLocale的默认值，用来影响插件的语言。插件加载翻译文件时，不使用QLocale::system().name()来获取语言，
         * 应该使用QLocale local; local.name() 来获取语言,这样可以识别到登录界面设置的的语言环境*/
        QLocale::setDefault(languageCode);
    }

    m_translator->load(qmFile);
    qApp->installTranslator(m_translator);
    qDebug() << "load translation file " << qmFile;

    m_strLanguageCode = languageCode;
    Q_EMIT languageChanged(true);
}

QString LanguageSetting::languageCode2Locale(QString strLangCode, QString strDefault)
{
    QString strLocale = strDefault;
    QFileInfo cmdInfo("/usr/share/language-tools/language2locale");
    if (cmdInfo.exists() && cmdInfo.isExecutable()) {
        QProcess process;
        QString strCmd = cmdInfo.filePath() + " " + strLangCode;
        process.start(strCmd);
        process.waitForFinished(3000);
        QString result = process.readAll();
        QStringList langLocales = result.split("\n");
        if (langLocales.count() > 0) {
            QString strFirstLocale = langLocales[0].trimmed();
            if (!strFirstLocale.isEmpty()) {
                strLocale = strFirstLocale;
            }
        }
    } else {
        qInfo() << cmdInfo.path() << "is invalid!";
    }
    qDebug() << strLangCode << " to " << strLocale;
    return strLocale;
}
