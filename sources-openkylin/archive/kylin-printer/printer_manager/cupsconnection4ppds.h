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
#ifndef CUPSCONNECTION4PPDS_H
#define CUPSCONNECTION4PPDS_H

#include <sys/utsname.h>
#include <cups/cups.h>
#include <cups/http.h>
#include <QDebug>
#include <map>
#include <string>
#include <qstring.h>
#include <stdexcept>
#include <QMutex>
#include <QMutexLocker>

class CupsConnection4PPDs
{
public:
//    static CupsConnection4PPDs* getInstance();
//    static QMutex m_mutex;
    http_t* getCupsConnection() const;
    ~CupsConnection4PPDs();
    explicit CupsConnection4PPDs();
protected:

private:
    http_t *m_ppdRequestConnection;
    const char *m_server;
    int m_ippPortNum;
    http_encryption_t m_encryptionType;

//    void CupsPpdsRequest();
};

#endif // CUPSCONNECTION4PPDS_H
