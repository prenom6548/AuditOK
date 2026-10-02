#include <QThread>
#include "matchppdsthread.h"
QMutex MatchPPDsThread:: m_mutex;
MatchPPDsThread::MatchPPDsThread(QObject *parent) : QObject(parent)
{
    qDebug() << "FindPPDsThread is begin!";
}
MatchPPDsThread::~MatchPPDsThread()
{

}
MatchPPDsThread* MatchPPDsThread::getInstance()
{
    static MatchPPDsThread *singleInstance = nullptr;
    static QThread *pMatchPPDsThreadWorkPPDs = nullptr;

    if (singleInstance == nullptr)
    {
        m_mutex.lock();
        if (singleInstance == nullptr)
        {
            singleInstance = new MatchPPDsThread();
            pMatchPPDsThreadWorkPPDs = new QThread();
            singleInstance->moveToThread(pMatchPPDsThreadWorkPPDs);

            connect(pMatchPPDsThreadWorkPPDs, &QThread::finished, singleInstance, &QObject::deleteLater);
            connect(singleInstance, SIGNAL(start()),
                    singleInstance, SLOT(detectStart()));
            pMatchPPDsThreadWorkPPDs->start();
        }
        m_mutex.unlock();
    }
    return singleInstance;
}

int MatchPPDsThread::initPPDMatch(QString bandName,
                                   QString printerName,
                                   QString suggestPpdName,
                                   ppdPrinterMap data,
                                   ppdPrinterMap extra,
                                   QString &exactMatchPPD,
                                   QStringList &matchResultList)
{
    qDebug() << "匹配中！";

    if(bandName.isEmpty())
    {
        qDebug() << "输入的要查询的厂商名为空！";
        return static_cast<int>(matchRes::INPUT_ERR);
    }
    if(printerName.isEmpty())
    {
        qDebug() << "输入的要查询的打印机名为空！";
        return static_cast<int>(matchRes::INPUT_ERR);
    }
    if(data.isEmpty())
    {
        qDebug() << "输入的ppd与printer关系QMap为空！";
        return static_cast<int>(matchRes::INPUT_ERR);
    }

    if (!suggestPpdName.isEmpty()) {
        for(auto outerIterator = data.begin(); outerIterator != data.end(); ++outerIterator) {
            auto innerMap = outerIterator.value();
            for (auto innerIterator = innerMap.begin(); innerIterator != innerMap.end(); ++innerIterator) {
                auto innerKey = innerIterator.key();
                auto value = innerIterator.value();
                if (value.ppdname == suggestPpdName) {
                    exactMatchPPD = value.ppdname;
                    return 0;
                }
            }
        }
    }

    m_originData = data;
    QStringList printerNameList = (bandName + ' ' + printerName).split(' ');
    m_printerBandName = nullptr;
    m_printerModelName = nullptr;

    m_printerBandName = bandName.toLower();
    if (!m_printerBandName.isEmpty())
    {
        for (int i = 1; i < printerNameList.size(); ++i)
        {
            QString tempLetterAndNum = nullptr;
            tempLetterAndNum = originStringHandle(printerNameList[i]);

            m_printerModelName.append(tempLetterAndNum);
            m_printerModelName.append(' ');
        }
    }

    if (m_originData.find(m_printerBandName) != m_originData.end())
    {
        auto &mfg = m_originData[m_printerBandName];

        int res = exactMatch(m_printerModelName, mfg, exactMatchPPD,matchResultList);
        return res;//EXACT_MATCH = 0,MATCHED_NORMAL = 1,
    }
    else if (hasThisBandName(extra, m_printerBandName))
    {
        auto &mfg = extra[getMfgName(extra, m_printerBandName)];

        int res = exactMatch(m_printerModelName, mfg, exactMatchPPD,matchResultList);
        return res;//EXACT_MATCH = 0,MATCHED_NORMAL = 1,
    }
    else
    {
        auto &mfg = m_originData["generic"];

        int res = genericMatch(m_printerModelName, mfg, exactMatchPPD,matchResultList);
        return res;//MATCHED_GENERIC = 2,
    }
}

int MatchPPDsThread::genericMatch(QString printerModel
                                  , QMap<QString, PPDsAndAttr> map
                                  , QString &exactMatchPPD
                                  , QStringList &matchResultList)
{
    if(!map.size())
    {
        qDebug() << "入参map大小为0";
        return static_cast<int>(matchRes::INPUT_ERR);
    }
    if(printerModel.isEmpty())
    {
        qDebug() << "入参printerModel内容为空";
        return static_cast<int>(matchRes::INPUT_ERR);
    }

    QStringList printerModelList = printerModel.split(' ');
    if (printerModelList.size() <= 2)
    {
        qDebug() << "printerModelList值不符合规范";
        return static_cast<int>(matchRes::INPUT_ERR);
    }
    printerModelList.removeAt(0);

    printerModel.clear();

    foreach (QString it , printerModelList)
    {
        printerModel.append(it);
    }

    QString tempPrinterModel = nullptr;
    modelUniform(printerModel, tempPrinterModel);

    exactMatchPPD.clear();
    matchResultList.clear();
    int numOfMatchedPPD = 0;
    int size = tempPrinterModel.size();
    for (int i = size ; i > 2; i--)
    {
        QString currentStr = tempPrinterModel.left(i);

        foreach (auto keyPPDs, map.keys())
        {
            if (map[keyPPDs].ppdname.contains("driverless"))
            {
                continue;
            }
            else
            {
                QString handledPPDMakeAndModel;
                handledPPDMakeAndModel = originStringHandle(keyPPDs);

                QString tempHandledPPDMakeAndModel = nullptr;
                modelUniform(handledPPDMakeAndModel, tempHandledPPDMakeAndModel);

                handledPPDMakeAndModel.clear();
                handledPPDMakeAndModel = tempHandledPPDMakeAndModel;

                if (handledPPDMakeAndModel.contains(currentStr, Qt::CaseInsensitive))
                {
//                    matchResultList.append(map[keyPPDs].ppdname);
//                    ++numOfMatchedPPD;
                }
                if(5 <= numOfMatchedPPD)
                {
                    break;
                }
            }
        }

        if (5 <= numOfMatchedPPD)
        {
            break;
        }
    }

    qDebug() << "没找到和" << printerModel << "匹配的PPD文件,使用通用文件";
    qDebug() << "找到匹配的通用PPD文件" << matchResultList.size() << "个";

    if(matchResultList.size() == 0)
    {
        qDebug() << "没找到和" << printerModel << "相近的通用驱动文件";

        QMap<QString, PPDsAndAttr>::iterator it = map.begin();
        int i = 0;
        for(;it != map.end();it++)
        {
            if(i < 5)
            {
                if(it.key().contains("text"))
                {
                    matchResultList.append(it.value().ppdname);
                    i++;
                }
            }
            else
            {
                break;
            }
        }
    }
    return static_cast<int>(matchRes::MATCHED_GENERIC);

}

int MatchPPDsThread::exactMatch(QString printerModel
                                , QMap<QString, PPDsAndAttr> map
                                , QString &exactMatchPPD
                                , QStringList &matchResultList)
{
    if(!map.size())
    {
        qDebug() << "入参map大小为0";
        return static_cast<int>(matchRes::INPUT_ERR);
    }
    if(printerModel.isEmpty())
    {
        qDebug() << "入参printerModel内容为空";
        return static_cast<int>(matchRes::INPUT_ERR);
    }

    m_exactMatchFlag = false;

    QStringList printerModelList = printerModel.split(' ');
    if (printerModelList.size() <= 2)
    {
        qDebug() << "printerModelList值不符合规范";
        return static_cast<int>(matchRes::INPUT_ERR);
    }
    QString tempPrinterModel = nullptr;
    modelUniform(printerModel, tempPrinterModel);

    QMap<QString, PPDsAndAttr>::iterator iter = map.begin();
    QString iterList[4] = {};

    int i = 0;
    exactMatchPPD.clear();
    QStringList exactMatchPPDList;
    matchResultList.clear();
    while (iter != map.end())
    {
        //map的key是处理后的match_str，已去掉减号和空格，即按从小到大的顺序排列的。
        if(iter.value().ppdname.contains("driverless"))
        {
            qDebug() << "排除的PPD文件为：" << iter.value().ppdname;
            iter = map.erase(iter);
        }

        QString handledPPDMakeAndModel;
        handledPPDMakeAndModel = originStringHandle(iter.key());

        QString tempHandledPPDMakeAndModel = nullptr;
        modelUniform(handledPPDMakeAndModel, tempHandledPPDMakeAndModel);

        handledPPDMakeAndModel.clear();
        handledPPDMakeAndModel = tempHandledPPDMakeAndModel;

        if(iter == map.begin() && 0 < handledPPDMakeAndModel.compare(tempPrinterModel, Qt::CaseInsensitive))
        {
            m_exactMatchFlag = false;
            exactMatchPPD.clear();

            int count  = (map.count() < 5)?map.count():5;
            for(int i = 0; i < count; ++i)
            {
                if(exactMatchPPD.isEmpty())
                {
                    if(handledPPDMakeAndModel.contains(tempPrinterModel,Qt::CaseInsensitive))
                    {
                        m_exactMatchFlag = true;
                        exactMatchPPD = iter.value().ppdname;
                    }
                }
                matchResultList.append(iter.value().ppdname);
                if(iter != map.end())
                {
                    iter++;
                    if(iter == map.end())
                    {
                        break;
                    }
                }
            }

            //上来最小的都比它大，取最开始的5个PPD，直接结束跳出。
            break;
        }
        else
        {
            if(iter == map.end() && 0 > handledPPDMakeAndModel.compare(tempPrinterModel, Qt::CaseInsensitive))
            {
                m_exactMatchFlag = false;
                int count  = (map.count() < 5)?map.count():5;
                for(int i = 0; i < count; ++i)
                {
                    matchResultList.append(iter.value().ppdname);
                    if(iter != map.begin())
                    {
                        iter--;
                        if(iter == map.begin())
                        {
                            break;
                        }
                    }
                }
                exactMatchPPD.clear();
                //最后的字符串的都比它小，取最后的5个PPD，直接结束跳出。
                break;
            }
            else
            {
                if (handledPPDMakeAndModel.contains(tempPrinterModel, Qt::CaseInsensitive))
                {
                    m_exactMatchFlag = true;

                    exactMatchPPDList.append(iter.value().ppdname);

                    while(iter != map.end())
                    {
                        iter++;
                        if(iter == map.end())
                        {
                            break;
                        }
                        else if(iter.value().match_str.contains(tempPrinterModel, Qt::CaseInsensitive))
                        {
                            exactMatchPPDList.append(iter.value().ppdname);
                        }
                    }

                    break;
                }
                else if(0 > handledPPDMakeAndModel.compare(tempPrinterModel, Qt::CaseInsensitive))
                {
                    iterList[i%4] = iter.value().ppdname;
                    i++;
                    //比其小的字符串正常记录
                }
                else if(0 < handledPPDMakeAndModel.compare(tempPrinterModel, Qt::CaseInsensitive))
                {
                    iterList[i%4] = iter.value().ppdname;
                    i++;
                    if(iter != map.end())
                    {
                        iter++;
                        if(iter == map.end())
                        {
                            break;
                        }
                        else
                        {
                            iterList[i%4] = iter.value().ppdname;
                        }

                        //从比其小变为了比其大，列表中的数据返回即可
                        m_exactMatchFlag = false;
                        for(int j = 0; j < 4; ++j)
                        {
                            matchResultList.append(iterList[j]);
                        }
                        if(iter != map.end())
                        {
                            iter++;
                            if(iter == map.end())
                            {
                                break;
                            }
                            else
                            {
                                matchResultList.append(iter.value().ppdname);
                            }
                        }
                        qSort(matchResultList.begin(),matchResultList.end());
                        break;
                    }
                    else
                    {
                        m_exactMatchFlag = false;

                        for(int j = 0; j < 4; ++j)
                        {
                            matchResultList.append(iterList[j]);
                        }
                        qSort(matchResultList.begin(),matchResultList.end());
                        break;
                    }
                }
            }
        }
        iter++;
    }


    if (!m_exactMatchFlag)
    {
        qDebug() << "没找到和" << printerModel << "精准匹配的PPD文件";
        exactMatchPPD.clear();
        return static_cast<int>(matchRes::MATCHED_NORMAL);
    }
    else
    {
        exactMatchPPD.clear();
        for(auto it : exactMatchPPDList)
        {
            if(!it.contains("foomatic",Qt::CaseInsensitive))
            {
                exactMatchPPD = it;
                break;
            }
        }
        if(exactMatchPPD.isEmpty() && exactMatchPPDList.size())
        {
            exactMatchPPD = exactMatchPPDList.at(0);
        }
        qDebug() << "找到了和" << printerModel << "精准匹配的PPD文件" << exactMatchPPD;
        return static_cast<int>(matchRes::EXACT_MATCH);
    }

}

QString MatchPPDsThread::originStringHandle(QString originString)
{
    if (originString.isEmpty())
    {
        qDebug() << "当前处理字符串为空字符串";
        return originString;
    }
    QString tempHandledString = nullptr;
    foreach (auto &letter, originString)
    {
        if (!letter.isLetterOrNumber())
        {
            tempHandledString.append(' ');
        }
        else
        {
            tempHandledString.append(letter);
            if (letter.isLetter() && (*(&letter + 1)).isDigit())
            {
                tempHandledString.append(' ');
            }
            else if (letter.isDigit() && (*(&letter + 1)).isLetter())
            {
                tempHandledString.append(' ');
            }
        }
    }

    return tempHandledString;
}
void MatchPPDsThread::modelUniform(QString handledStr, QString& outputStr)
{
    foreach (auto &letter, handledStr)
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
}
void MatchPPDsThread::detectStart()
{
    qDebug() << "MatchPPDsThread Thread is:" << QThread::currentThreadId();
}

bool MatchPPDsThread::hasThisBandName(ppdPrinterMap extra, QString inputName)
{
    for (auto key : static_cast<QList<QString> >(extra.keys()))
    {
        if(key.contains(inputName,Qt::CaseInsensitive))
        {
            return true;
        }
    }
    return false;
}

QString MatchPPDsThread::getMfgName(ppdPrinterMap extra, QString inputName)
{
    for (auto key : static_cast<QList<QString> >(extra.keys()))
    {
        if(key.contains(inputName,Qt::CaseInsensitive))
        {
            return key;
        }
    }
    return "";
}
