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
//


#include "device_check.h"
#include "find_ppd_worker.h"

// "HP" "LaserJet M407"
// "HP" "LaserJet Pro M501dn"  "hp/hp-laserjet_pro_m501dn-ps.ppd.gz"
// "HP" "LaserJet Pro M701n"

device_check::device_check(char *argv[])
{
    FindPpdWorker *worker = new FindPpdWorker();
    worker->setInput(argv[1], argv[2], argv[3]);

    int ret;
    PPDsAndAttr ppd;
    QList<PPDsAndAttr> ppdList;

    QEventLoop loop2;
    connect(worker, &FindPpdWorker::finished, &loop2, &QEventLoop::quit);
    connect(worker, &FindPpdWorker::resultPPDs, [=, &ret, &ppd, &ppdList](int res, PPDsAndAttr exactMatchPPD, QList<PPDsAndAttr> matchResultList) {
        ret = res;
        ppd = exactMatchPPD;
        ppdList = matchResultList;
        worker->finished();
    });

    worker->startWorker();
    loop2.exec();

    if (ret != 0) {
        qDebug() << ret;
    }

    qDebug() << ppd.ppdname;
    qDebug() << "-----------------";
    for (auto ppd : ppdList) {
        qDebug() << ppd.ppdname;
    }
}

device_check::~device_check()
{
}