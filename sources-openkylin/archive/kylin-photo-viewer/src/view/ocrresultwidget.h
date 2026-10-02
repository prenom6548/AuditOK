/* Copyright (C) 2020, KylinSoft Co., Ltd.
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
#ifndef OCRRESULTWIDGET_H
#define OCRRESULTWIDGET_H

#include <QObject>
#include <QTextEdit>
#include <QGSettings>
#include "global/variable.h"

class OCRResultWidget : public QTextEdit
{
    Q_OBJECT
public:
    OCRResultWidget(QWidget *parent = nullptr);
    //    void setAlignment(Qt::Alignment alignment)override;
    void setFormatedText(QString);
public Q_SLOTS:

    void setFormatedText(QVector<QString>);

private Q_SLOTS:

    void setThemeStyle();

private:
    QGSettings *m_pGsettingThemeData = nullptr;

};

#endif // OCRRESULTWIDGET_H
