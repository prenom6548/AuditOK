/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
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
#ifndef PRINTER_CHECK_HELPER_H
#define PRINTER_CHECK_HELPER_H

#include <QObject>
#include <QThread>
#include <QString>


class PrinterCheckHelper : public QObject
{
    Q_OBJECT
public:
    static PrinterCheckHelper *getInstance();
    void dowork(const QString &printername);
private:
    QString m_printername{QString()};
    PrinterCheckHelper();
    ~PrinterCheckHelper();
    void sendMessage(bool repairSucceed, const QString &msg, const QString &printername);

    //安装状态，0：安装成功，1：安装失败，2：已经安装
    int m_installOk = -1;
    bool m_needInstall = false;
    bool m_pkgHelperFlag = false;

    bool m_needmessage = false;
signals:
    void installEnd();

};

#endif // PRINTER_CHECK_HELPER_H
