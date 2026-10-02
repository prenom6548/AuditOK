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
#include "find_and_match_ppds.h"
#include "common.h"
#include "find_ppd_worker.h"

const QMap<std::pair<QString, QString>, QString> myMap{
    {{"Pantum", "M9705DN"}, "pantum/Pantum_M9705DN_Series_PCL.ppd"},
    {{"HP", "Color LaserJet MFP M277c6"}, "hp-color_laserjet_pro_mfp_m277-ps.ppd.gz"},
    {{"Pantum", "M7100DN"}, "pantum/Pantum_M7105DN_Series_PCL.ppd"},
    {{"Pantum", "P3300DN"}, "pantum/Pantum_P3305DN_Series_PCL.ppd"},
    {{"HP", "LaserJet Pro M501dn"}, "hp-laserjet_pro_m501n-ps.ppd.gz"},
    {{"HP", "LaserJet Pro M701n"}, "hp-laserjet_pro_m701-ps.ppd.gz"},
    {{"HP", "LaserJet Pro M706n"}, "hp-laserjet_pro_m706-ps.ppd.gz"},
    {{"HP", "LaserJet MFP M72655-M72630"}, "uld-hp/HP_LaserJet_MFP_M72625_72630.ppd"},
    {{"RICOH", "P 501"}, "Ricoh/Ricoh-P_501_PS.ppd"},
};

FindPpdWorker::FindPpdWorker()
{

}

FindPpdWorker::~FindPpdWorker()
{

}

void FindPpdWorker::setStatus(bool needUpdate)
{
    m_needUpdate = needUpdate;
}

void FindPpdWorker::setInput(const QString &vendor,
                             const QString &model,
                             const QString &suggestPpdName)
{
    m_vendor = vendor;
    m_model = model;
    m_suggestPpdName = suggestPpdName;
}


void FindPpdWorker::specialDeviceCheck()
{
    if (m_vendor == "Deli" && m_model.contains("(NEW)")) {
        m_model.remove("(NEW)");
    }

    if (!m_suggestPpdName.isEmpty()) {
        return;
    }

    QString value = myMap.value(std::make_pair(m_vendor, m_model));
    qDebug() << "value is " << value;
    m_suggestPpdName = value;
}

void FindPpdWorker::dowork()
{
    specialDeviceCheck();

    while (g_findAndMatchWorkThread->isBusy()) {
        delay_msec(1000);
    }

    if (!g_findAndMatchWorkThread->hasPPDsMap() || m_needUpdate) {
        while (g_findAndMatchWorkThread->updatePPDsMap() == static_cast<int>(FindAndMatchStatus::GETTING_PPD_THREAD_IS_RUNNING)) {
            delay_msec(1000);
        }

        QEventLoop loop;
        connect(g_findAndMatchWorkThread, &FindAndMatchPPDs::updateFinished, &loop, &QEventLoop::quit);
        connect(g_findAndMatchWorkThread, &FindAndMatchPPDs::updateFailed,   &loop, &QEventLoop::quit);
        loop.exec();
    }

    PPDsAndAttr ppd;
    QList<PPDsAndAttr> ppdList;

    int ret = g_findAndMatchWorkThread->matchPPDToPrinter(m_vendor, m_model, m_suggestPpdName, ppd, ppdList);

    QString strppd;
    QStringList strppdList;

    strppd = ppd.ppdname;

    for (auto ppd : ppdList) {
        strppdList.append(ppd.ppdname);
    }

    emit result(ret, strppd, strppdList);
    emit resultPPDs(ret, ppd, ppdList);
}
