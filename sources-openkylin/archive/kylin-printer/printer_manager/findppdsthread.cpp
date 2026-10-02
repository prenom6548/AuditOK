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
#include <QThread>
#include "findppdsthread.h"
QMutex FindPPDsThread:: m_mutex;
QString modelUniform(QString handledStr);
QString deviceUniform(QString handledStr);
QString makeandmodelUniform(QString handledStr);

QSet<QString> keywords = {
"hpcups 3.22.10",
"Canon MB5100 series - CUPS+Gutenprint",
"Canon MB5400 series - CUPS+Gutenprint",
"Canon E470 series - CUPS+Gutenprint",
"Canon MG3600 series - CUPS+Gutenprint",
};

bool containsAnyKeyword(const QString& text, const QSet<QString>& keywords) {
    for (const QString& keyword : keywords) {
        if (text.contains(keyword, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

FindPPDsThread::FindPPDsThread(http_t* httpConnection, QObject *parent)
                    : QObject(parent),
                    m_newHttp(httpConnection),
                    m_originPPDs(),
                    m_deviceIDPPDs()
{
    qDebug() << "FindPPDsThread is begin!";
}

FindPPDsThread* FindPPDsThread::getInstance(http_t* httpConnection)
{
    static FindPPDsThread *singleInstance = nullptr;
    static QThread *pFindPPDsWorkPPDs = nullptr;
    if (singleInstance == nullptr)
    {
        m_mutex.lock();
        if (singleInstance == nullptr)
        {
            singleInstance = new FindPPDsThread(httpConnection);
            pFindPPDsWorkPPDs = new QThread();
            singleInstance->moveToThread(pFindPPDsWorkPPDs);

            connect(pFindPPDsWorkPPDs, &QThread::finished, singleInstance, &QObject::deleteLater);
            connect(singleInstance, SIGNAL(start()),
                    singleInstance, SLOT(detectStart()));
            pFindPPDsWorkPPDs->start();
        }
        m_mutex.unlock();
    }
    return singleInstance;

}

FindPPDsThread::~FindPPDsThread()
{
    if(!m_originPPDs.isEmpty())
    {
        m_originPPDs.clear();
    }
    if(!m_deviceIDPPDs.isEmpty())
    {
        m_deviceIDPPDs.clear();
    }
}

void FindPPDsThread::initPPDMapConstruct()
{
    QMutexLocker lockData( &m_mutex);

    if(!m_originPPDs.isEmpty())
    {
        m_originPPDs.clear();
    }
    if(!m_deviceIDPPDs.isEmpty())
    {
        m_deviceIDPPDs.clear();
    }

    int operateStatus = getPPDsFromCUPS();

    if(operateStatus == static_cast<int>(RequestError::PPDS_GOT_SUCCESS))
    {
        if(!m_originPPDs.isEmpty())
        {
            qDebug() << "PPDs got!";
            emit gotAllHandledPPDs(m_originPPDs, m_deviceIDPPDs);
        }
        else
        {
            emit ippRequestError(static_cast<int>(RequestError::GOT_NO_PPDS));
        }
    }
    else if(operateStatus == static_cast<int>(RequestError::CUPS_REQUEST_ERROR))
    {
        emit ippRequestError(static_cast<int>(RequestError::CUPS_REQUEST_ERROR));
    }
    else if(operateStatus == static_cast<int>(RequestError::IPP_REQUEST_ERROR))
    {
        emit ippRequestError(static_cast<int>(RequestError::IPP_REQUEST_ERROR));
    }
    return;
}

int FindPPDsThread::getPPDsFromCUPS()
{
    if(!m_originPPDs.isEmpty())
    {
        m_originPPDs.clear();
    }
    if(!m_deviceIDPPDs.isEmpty())
    {
        m_deviceIDPPDs.clear();
    }

    m_ppdRequest = ippNewRequest(CUPS_GET_PPDS);

    m_cupsAnswer = cupsDoRequest(m_newHttp, m_ppdRequest, "/admin/");

    if (!m_cupsAnswer || ippGetStatusCode(m_cupsAnswer) > IPP_OK_CONFLICT)
    {
        qDebug() << "cupsAnswer request failed!";
        if(!m_cupsAnswer)
        {

            return static_cast<int>(RequestError::CUPS_REQUEST_ERROR);
        }
        else
        {
            qDebug() << ippGetStatusCode(m_cupsAnswer);

            return static_cast<int>(RequestError::IPP_REQUEST_ERROR);
        }
    }
    else
    {
        if(ippGetStatusCode(m_cupsAnswer) == 0)
        {
            qDebug() << "ippGetStatusCode(cupsAnswer) result is OK";
        }
    }

    for (m_attr = ippFirstAttribute(m_cupsAnswer); m_attr; m_attr = ippNextAttribute(m_cupsAnswer))
    {
        const char *name = nullptr;

        name = ippGetName(m_attr);
        if(!name)
        {
            continue;
        }
        if (!strcmp(name, "ppd-name") && ippGetValueTag(m_attr) == IPP_TAG_NAME)
        {
            QString ppdname = nullptr;
            QString ppd_make_and_model = nullptr;
            QString ppd_device_id = nullptr;
            QString ppd_make = nullptr;
            QString ppd_product = nullptr;
            QString keyMFG = nullptr;
            QString deviceID = nullptr;
            QString deviceMakeAndModel = nullptr;

            ppdname = ippGetString(m_attr, 0, nullptr);
            PPDsAndAttr attr;
            attr.ppdname = ppdname;
            if(attr.ppdname.contains("driverless",Qt::CaseInsensitive))
            {
                continue;
            }


            for (; m_attr && ippGetGroupTag(m_attr) == IPP_TAG_PRINTER; m_attr = ippNextAttribute(m_cupsAnswer))
            {
                const char *AttrName = nullptr;

                AttrName = ippGetName(m_attr);
                if(!AttrName)
                {
                    break;
                }

                if(AttrName && !strcmp(AttrName, "ppd-make-and-model"))
                {
                    ppd_make_and_model = QString::fromLocal8Bit((char *)ippGetString(m_attr, 0, nullptr));
                    if(containsAnyKeyword(ppd_make_and_model, keywords))
                    {
                        continue;
                    }

                    attr.origin_ppd_make_and_model = ppd_make_and_model;

                    QStringList temp = {};
                    if(ppd_make_and_model.contains("fuji",Qt::CaseInsensitive)
                            &&ppd_make_and_model.contains("xerox",Qt::CaseInsensitive))
                    {
                        keyMFG = "Fuji-Xerox";
                        if(!ppd_make_and_model.compare("Fuji Xerox",Qt::CaseInsensitive))
                        {
                            continue;
                        }
                        attr.match_str = modelUniform(ppd_make_and_model.remove(0,11));
                        if(ppd_make_and_model.compare("") || ppd_make_and_model.compare(" "))
                        {
                            temp = ppd_make_and_model.split(' ');
                            for (int i = 1; i < temp.size(); ++i)
                            {
                                attr.ppd_make_and_model.append(temp[i]);
                                attr.ppd_make_and_model.append("-");
                            }
                            attr.ppd_make_and_model.remove(attr.ppd_make_and_model.length() - 1, 1);
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else if(ppd_make_and_model.contains("konica",Qt::CaseInsensitive)
                            &&ppd_make_and_model.contains("minolta",Qt::CaseInsensitive))
                    {
                        keyMFG = "Konica-Minolta";
                        if(!ppd_make_and_model.compare("konica minolta",Qt::CaseInsensitive))
                        {
                            continue;
                        }

                        attr.match_str = modelUniform(ppd_make_and_model.remove(0,15));
                        if(ppd_make_and_model.compare("") || ppd_make_and_model.compare(" "))
                        {
                            temp = ppd_make_and_model.split(' ');
                            if (temp.size() == 1)
                            {
                                if(temp[0].contains("PS(P)",Qt::CaseInsensitive))
                                {
                                    temp[0].remove(temp[0].length() - 5, temp[0].length());
                                }
                                attr.ppd_make_and_model.append(temp[0]);
                            }
                            else
                            {
                                if (temp.size() == 2 && temp[1].contains("PS(P)",Qt::CaseInsensitive))
                                {
                                    if(temp[0].contains("PS(P)",Qt::CaseInsensitive))
                                    {
                                        temp[0].remove(temp[0].length() - 5, temp[0].length());
                                    }
                                    attr.ppd_make_and_model.append(temp[0]);
                                }
                                else
                                {
                                    for (int i = 1; i < temp.size(); ++i)
                                    {
                                        attr.ppd_make_and_model.append(temp[i]);
                                        attr.ppd_make_and_model.append("-");
                                    }
                                    attr.ppd_make_and_model.remove(attr.ppd_make_and_model.length() - 1, 1);
                                }
                            }
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else if(ppd_make_and_model.contains("lexmark",Qt::CaseInsensitive)
                            &&ppd_make_and_model.contains("international",Qt::CaseInsensitive))
                    {
                        keyMFG = "Lexmark-International";
                        if(!ppd_make_and_model.compare("lexmark international",Qt::CaseInsensitive))
                        {
                            continue;
                        }
                        attr.match_str = modelUniform(ppd_make_and_model.remove(0,22));
                        if(ppd_make_and_model.compare("") || ppd_make_and_model.compare(" "))
                        {
                            temp = ppd_make_and_model.split(' ');
                            for (int i = 1; i < temp.size(); ++i)
                            {
                                attr.ppd_make_and_model.append(temp[i]);
                                attr.ppd_make_and_model.append("-");
                            }
                            attr.ppd_make_and_model.remove(attr.ppd_make_and_model.length() - 1, 1);
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else if(ppd_make_and_model.contains("kyocera",Qt::CaseInsensitive)
                            &&ppd_make_and_model.contains("mita",Qt::CaseInsensitive))
                    {
                        keyMFG = "Kyocera-Mita";
                        if(!ppd_make_and_model.compare("kyocera mita",Qt::CaseInsensitive))
                        {
                            continue;
                        }
                        attr.match_str = modelUniform(ppd_make_and_model.remove(0,13));
                        if(ppd_make_and_model.compare("") || ppd_make_and_model.compare(" "))
                        {
                            temp = ppd_make_and_model.split(' ');
                            if (temp.size() == 1)
                            {
                                attr.ppd_make_and_model.append(temp[0]);
                            }
                            else
                            {
                                for (int i = 1; i < temp.size(); ++i)
                                {
                                    attr.ppd_make_and_model.append(temp[i]);
                                    attr.ppd_make_and_model.append("-");
                                }
                                attr.ppd_make_and_model.remove(attr.ppd_make_and_model.length() - 1, 1);
                            }
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else if(ppd_make_and_model.contains("hewlett",Qt::CaseInsensitive)
                            &&ppd_make_and_model.contains("packard",Qt::CaseInsensitive))
                    {
                        keyMFG = "HP";
                        if(!ppd_make_and_model.compare("hewlett packard",Qt::CaseInsensitive))
                        {
                            continue;
                        }
                        attr.match_str = modelUniform(ppd_make_and_model.remove(0,16));
                        if(ppd_make_and_model.compare("") || ppd_make_and_model.compare(" "))
                        {
                            temp = ppd_make_and_model.split(' ');
                            for (int i = 1; i < temp.size(); ++i)
                            {
                                attr.ppd_make_and_model.append(temp[i]);
                                attr.ppd_make_and_model.append("-");
                            }
                            attr.ppd_make_and_model.remove(attr.ppd_make_and_model.length() - 1, 1);
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else if(ppd_make_and_model.contains("lenovo",Qt::CaseInsensitive)
                            &&ppd_make_and_model.contains("image",Qt::CaseInsensitive))
                    {
                        keyMFG = "Lenovo";
                        if(!ppd_make_and_model.compare("lenovo image",Qt::CaseInsensitive))
                        {
                            continue;
                        }
                        attr.match_str = modelUniform(ppd_make_and_model.remove(0,13));
                        if(ppd_make_and_model.compare("") || ppd_make_and_model.compare(" "))
                        {
                            attr.ppd_make_and_model.append(ppd_make_and_model);
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else
                    {
                        temp = ppd_make_and_model.split(' ');
                        keyMFG = temp[0];

                        ppd_make_and_model = ppd_make_and_model.remove(0,keyMFG.size()+1);

                        if(attr.ppd_make_and_model.contains("driverless",Qt::CaseInsensitive))
                        {
                            attr.ppd_make_and_model.clear();
                        }

                        if(ppd_make_and_model.compare("") || ppd_make_and_model.compare(" "))
                        {
                            for (int i = 1; i < temp.size(); ++i)
                            {
                                attr.ppd_make_and_model.append(temp[i]);
                                attr.ppd_make_and_model.append("-");
                            }
                            attr.ppd_make_and_model.remove(attr.ppd_make_and_model.length() - 1, 1);
                            attr.match_str = modelUniform(attr.ppd_make_and_model);
                        }
                        else
                        {
                            continue;
                        }
                    }

                }
                if(AttrName && !strcmp(AttrName, "ppd-device-id"))
                {
                    ppd_device_id = QString::fromLocal8Bit((char *)ippGetString(m_attr, 0, nullptr));
                    if(!ppd_device_id.isEmpty())
                    {
                        deviceID = deviceUniform(ppd_device_id);
                        deviceMakeAndModel = makeandmodelUniform(ppd_device_id);
                    }

                    attr.ppd_device_id = ppd_device_id;
                }
                if(AttrName && !strcmp(AttrName, "ppd-make"))
                {
                    ppd_make = QString::fromLocal8Bit((char *)ippGetString(m_attr, 0, nullptr));
                    attr.ppd_make = ppd_make;
                }
                if(AttrName && !strcmp(AttrName, "ppd-product"))
                {
                    ppd_product = QString::fromLocal8Bit((char *)ippGetString(m_attr, 0, nullptr));
                    attr.ppd_product = ppd_product;
                }
            }

            keyMFG = keyMFG.toLower();

            bool hasExist = false;
            auto it = m_originPPDs.begin();
            for (;it != m_originPPDs.end();++it)
            {
                if (it.key().contains(keyMFG, Qt::CaseInsensitive))
                {
                    hasExist = true;
                    keyMFG = it.key();
                }
            }


            if (true == hasExist)
            {
                auto &mfg = m_originPPDs[keyMFG];
                if(!attr.match_str.isEmpty())
                {
                    mfg.insert(attr.match_str,attr);
                }
            }
            else
            {
                QMap<QString,PPDsAndAttr> tempMap ;
                if(!attr.match_str.isEmpty())
                {
                    tempMap.insert(attr.match_str,attr);
                    m_originPPDs.insert(keyMFG,tempMap);
                }
            }

            if(keyMFG != deviceID && !deviceID.isEmpty() && deviceID != " " && !deviceID.contains(keyMFG))
            {
                bool hasExist = false;
                auto it = m_originPPDs.begin();
                for (;it != m_originPPDs.end();++it)
                {
                    if (it.key().contains(deviceID, Qt::CaseInsensitive))
                    {
                        hasExist = true;
                        deviceID = it.key();
                    }
                }

                if (true == hasExist)
                {
                    auto &mfg = m_deviceIDPPDs[deviceID];
                    attr.match_str = (deviceMakeAndModel.isEmpty())? attr.match_str:deviceMakeAndModel;
                    if(!attr.match_str.isEmpty())
                    {
                        mfg.insert(attr.match_str,attr);
                    }
                }
                else
                {
                    QMap<QString,PPDsAndAttr> tempMap ;
                    attr.match_str = (deviceMakeAndModel.isEmpty())? attr.match_str:deviceMakeAndModel;
                    if(!attr.match_str.isEmpty())
                    {
                        tempMap.insert(attr.match_str,attr);
                        m_deviceIDPPDs.insert(deviceID,tempMap);
                    }
                }
            }
        }
    }
    if (m_cupsAnswer!=nullptr)
    {
        ippDelete(m_cupsAnswer);
    }

    return static_cast<int>(RequestError::PPDS_GOT_SUCCESS);
}

void FindPPDsThread::detectStart()
{
    qDebug() << "FindPPDsThread Thread is:" << QThread::currentThreadId();
}

QString modelUniform(QString handledStr)
{
    QString outputStr;
    outputStr.clear();
    foreach (auto &letter, handledStr.toLower())
    {
        if (letter.isSpace())
        {
            continue;
        }
        else if(!letter.isLetterOrNumber())
        {
            continue;
        }
        else
        {
            outputStr.append(letter);
        }
    }
    return outputStr;
}

QString deviceUniform(QString handledStr)
{
    QString outputStr;
    outputStr.clear();

    QStringList deviceInfoList = (handledStr.trimmed()).split(";");
    for(QString str: deviceInfoList)
    {
        if(str.contains("mfg",Qt::CaseInsensitive))
        {
            handledStr = str;
            break;
        }
        if(str.contains("MANUFACTURER",Qt::CaseInsensitive))
        {
            handledStr = str;
            break;
        }
    }

    foreach (auto &letter, handledStr)
    {
        if (letter.isSpace())
        {
            outputStr.append("-");
        }
        else
        {
            outputStr.append(letter);
        }
    }
    return outputStr;
}

QString makeandmodelUniform(QString handledStr)
{
    QString outputStr;
    outputStr.clear();

    QStringList deviceInfoList = (handledStr.trimmed()).split(";");
    for(QString str: deviceInfoList)
    {
        if(str.contains("mdl",Qt::CaseInsensitive))
        {
            handledStr = str;
            break;
        }
    }

    foreach (auto &letter, handledStr)
    {
        if (letter.isSpace())
        {
            outputStr.append("-");
        }
        else
        {
            outputStr.append(letter);
        }
    }
    if(outputStr.startsWith("mdl:", Qt::CaseInsensitive))
    {
        outputStr.remove(0,4);
    }
    return outputStr;
}
