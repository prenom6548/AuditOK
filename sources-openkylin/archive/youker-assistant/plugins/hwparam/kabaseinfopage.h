/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KABASEINFOPAGE_H
#define KABASEINFOPAGE_H

#define BLANKNUM 10
#define BLANKNUM_EN 20

#include <QWidget>
#include <QProcess>
#include "kalabel.h"

class KABaseInfoPage : public QWidget
{
    Q_OBJECT
public:
    explicit KABaseInfoPage(QWidget *parent = nullptr);
    int getBlankNum();

public slots:
    virtual void onRefreshInfo();
    virtual void onUpdateInfo(QString strInfoJson);
    virtual void onUpdateInfo(unsigned uStatus, QString strInfoJson);
    void onCopyContent(QString strPageInfo);
    void onSelectAllLabel(KALabel* selectedLabel);
    void disableMouse(QString strProduct);
    void enableMouse(QString strProduct);
    QString queryMouseId(QString strProduct);
    QString mouseStatus(QString strProduct);
    QString bluetoothStatus();
    bool bluetoothExist();
    void disableTooltip();
};

#endif // KABASEINFOPAGE_H
