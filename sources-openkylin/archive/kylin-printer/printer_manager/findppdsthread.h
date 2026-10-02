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
#ifndef FINDPPDSTHREAD_H
#define FINDPPDSTHREAD_H

#include <QObject>
#include <QApplication>
#include <QMetaType>
#include <QMutexLocker>
#include <QTimer>
#include "cupsconnection4ppds.h"


class PPDsAndAttr
{
public:
    QString ppdname{QString()};
    QString ppd_make{QString()};
    QString ppd_product{QString()};
    QString ppd_make_and_model{QString()};
    QString origin_ppd_make_and_model{QString()};
    QString ppd_device_id{QString()};
    QString match_str{QString()};

    void clear() {
        ppdname.clear();
        ppd_make.clear();
        ppd_product.clear();
        ppd_make_and_model.clear();
        origin_ppd_make_and_model.clear();
        ppd_device_id.clear();
        match_str.clear();
    }

    bool isEmpty() {
        return ppdname.isEmpty();
    }
};
Q_DECLARE_METATYPE(PPDsAndAttr)

enum class RequestError:int
{
    PPDS_GOT_SUCCESS = 0,
    CUPS_REQUEST_ERROR,
    IPP_REQUEST_ERROR,
    GOT_NO_PPDS,
};

using ppdPrinterMap = QMap<QString,QMap<QString,PPDsAndAttr> > ;

class FindPPDsThread : public QObject
{
    Q_OBJECT
public:
    static FindPPDsThread* getInstance(http_t* httpConnection);

protected:


signals:
    void gotAllHandledPPDs(const ppdPrinterMap &origin,const ppdPrinterMap &extra);
    void ippRequestError(int);
    void getPPDsFromCUPSErr();
    void ppdRequestSuccessed();

    void start();

public slots:
    void initPPDMapConstruct();

    void detectStart();
private:
    ~FindPPDsThread();
    explicit FindPPDsThread(http_t* httpConnection, QObject *parent = nullptr);
    FindPPDsThread(FindPPDsThread const&);
    FindPPDsThread& operator =(FindPPDsThread const&);

    static QMutex m_mutex;
    http_t* m_newHttp = nullptr;
    ipp_t* m_ppdRequest = nullptr;
    ipp_t* m_cupsAnswer = nullptr;
    ipp_attribute_t *m_attr = nullptr;

    QMap<QString,QMap<QString,PPDsAndAttr> > m_originPPDs;
    QMap<QString,QMap<QString,PPDsAndAttr> > m_deviceIDPPDs;
    int getPPDsFromCUPS();
};

#endif // FINDPPDSTHREAD_H
