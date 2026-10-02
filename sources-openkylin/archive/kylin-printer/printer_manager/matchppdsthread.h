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
#ifndef MATCHPPDSTHREAD_H
#define MATCHPPDSTHREAD_H

#include <QObject>
#include<QMutexLocker>
#include "findppdsthread.h"

enum class matchRes:int
{
    EXACT_MATCH = 0,
    MATCHED_NORMAL,
    MATCHED_GENERIC,
    INPUT_ERR,
};

class MatchPPDsThread : public QObject
{
    Q_OBJECT
public:
    static MatchPPDsThread* getInstance();

protected:

signals:
    void matchFailed();

    void start();
public slots:
    int initPPDMatch(QString bandName,
                      QString printerName,
                      QString suggestPpdName,
                      ppdPrinterMap data,
                      ppdPrinterMap extra,
                      PPDsAndAttr &exactMatchPPD,
                      QList<PPDsAndAttr> &matchResultList);
    void detectStart();
private:
    explicit MatchPPDsThread(QObject *parent = nullptr);
    ~MatchPPDsThread();
    MatchPPDsThread(MatchPPDsThread const&);
    MatchPPDsThread& operator =(MatchPPDsThread const&);

    static QMutex m_mutex;
    QMap<QString, QMap<QString, PPDsAndAttr> > m_originData;

    QString m_printerBandName = nullptr;
    QString m_printerModelName = nullptr;

    int exactMatch(QString printerModel
                   , QMap<QString, PPDsAndAttr> map
                   , PPDsAndAttr &exactMatchPPD
                   , QList<PPDsAndAttr> &matchResultList);
    int genericMatch(QString printerModel
                     , QMap<QString, PPDsAndAttr> map
                     , PPDsAndAttr &exactMatchPPD
                     , QList<PPDsAndAttr> &matchResultList);

    bool m_exactMatchFlag = false;

    QString originStringHandle(QString originString);

    void modelUniform(QString handledStr, QString& outputStr);
    bool hasThisBandName(ppdPrinterMap extra, QString inputName);
    QString getMfgName(ppdPrinterMap extra, QString inputName);
};

#endif // MATCHPPDSTHREAD_H
