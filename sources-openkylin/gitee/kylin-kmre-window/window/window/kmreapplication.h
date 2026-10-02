/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QApplication>
#include <QTranslator>

class KmreWindowManager;

class KmreApplication : public QApplication
{
public:
    KmreApplication(int &argc, char **argv, int flag = ApplicationFlags);
    ~KmreApplication();

    bool init();
    static QString getTranslationPath();

protected:
    //bool notify(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private:
    KmreWindowManager *mWindowManager = nullptr;
    QTranslator mTranslator;
    QTranslator mDefTranslator;

    void initTranslator();
    void initGlobalSettings();
};


