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
#ifndef LANGUAGESETTING_H
#define LANGUAGESETTING_H

#include <QObject>
#include <QTranslator>

class LanguageSetting : public QObject
{
    Q_OBJECT

public:
    static LanguageSetting *instance(QObject *parent = nullptr);

    void onLanguageChanged(QString languageCode);

Q_SIGNALS:
    void languageChanged(bool isCompleted);

private:
    LanguageSetting(QObject *parent = nullptr);
    virtual ~LanguageSetting();

    QString languageCode2Locale(QString strLangCode, QString strDefault);

private:
    static LanguageSetting *m_singleInstance;
    QTranslator *m_translator = nullptr;
    QString m_strLanguageCode;
};

#endif // LANGUAGESETTING_H
