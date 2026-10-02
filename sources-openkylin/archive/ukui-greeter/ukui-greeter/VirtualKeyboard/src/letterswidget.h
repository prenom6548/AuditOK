/*
 * Copyright (C) 2018 KylinSoftCo., Ltd.
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
#ifndef LETTERSWIDGET_H
#define LETTERSWIDGET_H

#include <QWidget>
#include <QGSettings/qgsettings.h>
#include "kbbutton.h"
#include <QMap>

class LettersWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LettersWidget(QWidget *parent = nullptr);
    virtual ~LettersWidget();

    void adjustGeometry(double lfWidthScale, double lfHeightScale, bool isVertical = false, bool floatStatus = false);
    void changeFuncKeyStyle(QString obj, bool isLock);

public Q_SLOTS:
    void onBtnClicked(QChar charId);
    void onCapsChanged();

Q_SIGNALS:
    void clicked(int nKeyId);
    void specialBtnClicked(QString keyName);
    void normalBtnClicked(QChar c);


private slots:
    void onGlobalKeyRelease(const QString &key);

private:
    void initUI();
    void toggleCase();

private:
    QMap<KBButton*, QRect> m_mapBtnGeometrys;

    bool isShift = false;

    QGSettings *settings;
    bool capsState = false;
};

#endif // LETTERSWIDGET_H
