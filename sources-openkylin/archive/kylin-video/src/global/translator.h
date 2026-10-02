/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _TRANSLATOR_H_
#define _TRANSLATOR_H_

#include <QString>
#include <QTranslator>

class Translator
{

public:
	Translator();
	~Translator();

    void load(const QString &snap);


protected:
	static bool loadCatalog(QTranslator & t, QString name, QString locale, QString dir);

	QTranslator app_trans;
    QTranslator app_qt_trans; // lc add 20201015
	QTranslator qt_trans;
};

#endif
