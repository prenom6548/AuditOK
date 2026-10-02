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
#ifndef CHANGEFEATURENAME_H
#define CHANGEFEATURENAME_H

#include <kdialog.h>
#include <QPainter>
#include <QPainterPath>
#include <QKeyEvent>
using namespace kdk;

namespace Ui {
class ChangeFeatureName;
}
class ChangeFeatureName : public KDialog
{
    Q_OBJECT

public:
    explicit ChangeFeatureName(QStringList names, QWidget *parent = nullptr);
    ~ChangeFeatureName();
    void setTitle(QString titleName);

private:
    QStringList names;
    QString ElideText(QFont font,int width,QString strInfo);
    void initInfoFormat();
private:
    Ui::ChangeFeatureName *ui;

protected:
    void keyPressEvent(QKeyEvent *);

Q_SIGNALS:
    void sendNewName(QString name);
};

#endif // CHANGEFEATURENAME_H
