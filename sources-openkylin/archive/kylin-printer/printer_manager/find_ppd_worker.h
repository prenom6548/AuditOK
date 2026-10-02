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
#ifndef __FIND_PPD_WORKER__
#define __FIND_PPD_WORKER__

#include <QString>
#include <QStringList>
#include "single_thread_worker.h"
#include "findppdsthread.h"

class FindPpdWorker : public  SingleThreadWorker
{
    Q_OBJECT
public:
    FindPpdWorker();

    void setStatus(bool needUpdate);

    void setInput(const QString &vendor,
                  const QString &model,
                  const QString &suggestPpdName);

    void specialDeviceCheck();

    ~FindPpdWorker();
Q_SIGNALS:
    void result(int res, QString exactMatchPPD, QStringList matchResultList);
    void resultPPDs(int res, PPDsAndAttr exactMatchPPD, QList<PPDsAndAttr> matchResultList);
private:
    void dowork() override;

    bool        m_needUpdate{false};
    QString     m_vendor{QString()};
    QString     m_model{QString()};
    QString     m_suggestPpdName{QString()};
};

#endif // __FIND_PPD_WORKER__