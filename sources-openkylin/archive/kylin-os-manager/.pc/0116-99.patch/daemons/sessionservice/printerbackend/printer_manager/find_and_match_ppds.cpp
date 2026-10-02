#include "find_and_match_ppds.h"
#define g_cmdFindPPDs(m_newHttp) FindPPDsThread::getInstance(m_newHttp)
#define g_cmdMatchPPDs MatchPPDsThread::getInstance()
QMutex FindAndMatchPPDs:: m_mutex;
ppdPrinterMap FindAndMatchPPDs:: m_currentPPDsMap;
ppdPrinterMap FindAndMatchPPDs:: m_extraPPDsMap;
/************************************************
* 函数名称：FindAndMatchPPDs
* 功能描述：构造函数
* 输入参数：无
* 输出参数：无
* 修改日期：2021.05.27
* 修改内容：
*   创建  HZH
*
*************************************************/
FindAndMatchPPDs:: FindAndMatchPPDs(QObject *parent)
                 :  QObject(parent),
                 m_isPPDsCollecting(false),
                 m_canMatchPPD(false)
{
    qDebug() << "FindAndMatchPPDs is begin!";
    try
    {
        buildCupsConnection();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        throw;
    }

}
/************************************************
* 函数名称：~FindAndMatchPPDs
* 功能描述：析构函数
* 输入参数：无
* 输出参数：无
* 修改日期：2021.05.27
* 修改内容：
*   创建  HZH
*
*************************************************/
FindAndMatchPPDs::~FindAndMatchPPDs()
{
    destroyCupsConnection();
    destroyGetPPDsConnection();
}
/************************************************
* 函数名称：getInstance
* 功能描述：public接口，用于返回唯一实例化对象的指针
* 输入参数：无
* 输出参数：唯一实例化对象的指针。
* 修改日期：2021.05.27
* 修改内容：
*   创建  HZH
*
*************************************************/
FindAndMatchPPDs *FindAndMatchPPDs::getInstance()
{
    static FindAndMatchPPDs *findAndMatchInstance = nullptr;
    static QThread *pFindAndMatchWorkPPDs = nullptr;
    if (findAndMatchInstance == nullptr)
    {
        m_mutex.lock();
        if (findAndMatchInstance == nullptr)
        {
            try
            {
                findAndMatchInstance = new FindAndMatchPPDs;
                pFindAndMatchWorkPPDs = new QThread();
                findAndMatchInstance->moveToThread(pFindAndMatchWorkPPDs);
                connect(pFindAndMatchWorkPPDs, &QThread::finished, findAndMatchInstance, &QObject::deleteLater);
                connect(findAndMatchInstance, SIGNAL(start()),
                        findAndMatchInstance, SLOT(detectStart()));
                pFindAndMatchWorkPPDs->start();
            }
            catch (const std::runtime_error &re)
            {
                qDebug() << "runtime_error:" << re.what();
                throw;
            }
        }
        m_mutex.unlock();
    }
    return findAndMatchInstance;
}
/************************************************
* 函数名称：buildCupsConnection
* 功能描述：私有函数，Cups的链接建立
* 输入参数：无
* 输出参数：无
* 修改日期：2021.05.27
* 修改内容：
*   创建  HZH
*
*************************************************/
void FindAndMatchPPDs::buildCupsConnection()
{
    //CUPS链接建立
    //*************************************************************************************
    m_cupsConnection = new CupsConnection4PPDs();
    m_newHttp = m_cupsConnection->getCupsConnection(); //建立，并获取http链接

    if (!m_newHttp)
    {
        qDebug() << "CUPsConnection construct failed!";
        destroyCupsConnection();
        throw std::runtime_error("Failed to buildCupsConnection!");
    }
}
/************************************************
* 函数名称：destroyCupsConnection
* 功能描述：私有函数，Cups的链接的销毁
* 输入参数：无
* 输出参数：无
* 修改日期：2021.05.27
* 修改内容：
*   创建  HZH
*
*************************************************/
void FindAndMatchPPDs::destroyCupsConnection()
{
    qDebug() << "destroyCupsConnection";
    if(!m_cupsConnection)
    {
        delete m_cupsConnection;
        m_cupsConnection = nullptr;
    }
}
/************************************************
* 函数名称：updatePPDsMap
* 功能描述：public接口，用于更新PPD的Map
* 输入参数：无
* 输出参数：是否更新了，0表示更新了，1表示Map此时正在更新
* 修改日期：2021.05.27
* 修改内容：
*   创建  HZH
*
*************************************************/
int FindAndMatchPPDs::updatePPDsMap()
{
    qDebug() << "updatePPDsMap ing !";
    if(false == isBusy())
    {
        buildGetPPDsThreadsAndConnections();
        emit startFindPPDs();
        m_isPPDsCollecting = true;
        return static_cast<int>(FindAndMatchStatus::OPERATION_OK);
    }
    else
    {
        return static_cast<int>(FindAndMatchStatus::GETTING_PPD_THREAD_IS_RUNNING);
    }
}
/************************************************
* 函数名称：matchPPDToPrinter
* 功能描述：public接口，用于在PPD的Map中匹配某打印机对应
          的PPD列表
* 输入参数：打印机厂商，打印机型号，打印机链接类型，用于取
          结果的result
* 输出参数：是否匹配了，0表示匹配了，2表示Map此时正在更新
          ，3表示匹配失败。若匹配成功，可在入参的result
          中取到匹配的结果。
* 修改日期：2021.05.27
* 修改内容：
*   创建  HZH
*
*************************************************/
int FindAndMatchPPDs::matchPPDToPrinter(QString vendor,
                                        QString model,
                                        QString suggestPpdName,
                                        QString &exactMatchPPD,
                                        QStringList &matchResultList)
{
    if(vendor.isEmpty()||model.isEmpty())
    {
        qDebug() << "打印机厂商内容为空！";
        return static_cast<int>(FindAndMatchStatus::PPDS_MATCH_FAILED);
    }
    if(vendor.size() > 128 || model.size() > 128)
    {
        qDebug() << "打印机厂商内容为乱码！";
        return static_cast<int>(FindAndMatchStatus::PPDS_MATCH_FAILED);
    }
    qDebug() << "开始匹配" << vendor << "牌" << model << "的打印机";
    if(true == isBusy() || !hasPPDsMap())
    {
        qDebug() << "线程忙，不能匹配！";

        return static_cast<int>(FindAndMatchStatus::PPDS_MAP_IS_NOT_READY);
    }
    else
    {
        qDebug() << "开启匹配线程！";
        buildMatchPPDsThreadsAndConnections();
        int res = g_cmdMatchPPDs->initPPDMatch(vendor,model,suggestPpdName,m_currentPPDsMap,m_extraPPDsMap,exactMatchPPD,matchResultList);
        return res;
    }
}

/************************************************
* 函数名称：gotPPDsMap
* 功能描述：私有函数，获取到了所有PPD的map，相关标志位置位，
          销毁PPD链接
* 输入参数：无
* 输出参数：无
* 修改日期：2021.05.27
* 修改内容：
*   创建  HZH
*
*************************************************/
void FindAndMatchPPDs::gotPPDsMap(const ppdPrinterMap &temp,const ppdPrinterMap &extra)
{
    m_currentPPDsMap = temp;
    m_extraPPDsMap = extra;

    m_canMatchPPD = true;
    m_isPPDsCollecting = false;

    destroyGetPPDsConnection();
    qDebug() << "匹配完毕，成功!";
    emit updateFinished();
}

/************************************************
* 函数名称：getPPDsErr
* 功能描述：私有函数，获取所有PPD的map失败，相关标志位置位，
          销毁PPD链接
* 输入参数：无
* 输出参数：无
* 修改日期：2021.05.27
* 修改内容：
*   创建  HZH
*
*************************************************/
void FindAndMatchPPDs::getPPDsErr(int errCode)//RequestError errCode)
{
    m_canMatchPPD =false;
    m_isPPDsCollecting = false;

    destroyGetPPDsConnection();
    switch(errCode)
    {
        case static_cast<int>(RequestError::CUPS_REQUEST_ERROR):
            qDebug() << "CUPS请求失败！";
            break;
        case static_cast<int>(RequestError::IPP_REQUEST_ERROR):
            qDebug() << "IPP请求失败！";
            break;
        case static_cast<int>(RequestError::GOT_NO_PPDS):
            qDebug() << "没获取到PPD文件！";
            break;

        default:
            break;
    }
    qDebug() << "匹配完毕，失败!";
    emit updateFailed();
}

/************************************************
* 函数名称：buildGetPPDsThreadsAndConnections
* 功能描述：私有函数，建立PPD查找线程链接
* 输入参数：无
* 输出参数：无
* 修改日期：2021.05.27
* 修改内容：
*   创建  HZH
*
*************************************************/
void FindAndMatchPPDs::buildGetPPDsThreadsAndConnections()
{
    //*************************************************************************************
    //建立通过http链接获取PPDs的线程
    //*************************************************************************************
    qDebug() << "建立通过http链接获取PPDs的线程";
    g_cmdFindPPDs(m_newHttp)->start();
    qRegisterMetaType<ppdPrinterMap>("ppdPrinterMap"); //注册自己的类型，必须！！！！！

    connect(this, &FindAndMatchPPDs::startFindPPDs, g_cmdFindPPDs(m_newHttp), &FindPPDsThread::initPPDMapConstruct,Qt::UniqueConnection);//查找并构建ppd容器
    connect(g_cmdFindPPDs(m_newHttp), SIGNAL(ippRequestError(int)),this,SLOT(getPPDsErr(int)),Qt::UniqueConnection);
    connect(g_cmdFindPPDs(m_newHttp), SIGNAL(gotAllHandledPPDs(ppdPrinterMap,ppdPrinterMap)),this, SLOT(gotPPDsMap(ppdPrinterMap,ppdPrinterMap)),Qt::UniqueConnection); //获取到所有PPD文件列表的初始map

    //*************************************************************************************
}

/************************************************
* 函数名称：destroyGetPPDsConnection
* 功能描述：私有函数，销毁PPD查找线程链接
* 输入参数：无
* 输出参数：无
* 修改日期：2021.05.27
* 修改内容：
*   创建  HZH
*
*************************************************/
void FindAndMatchPPDs::destroyGetPPDsConnection()
{
    qDebug() << "destroyGetPPDsConnection";
    disconnect(this, &FindAndMatchPPDs::startFindPPDs, g_cmdFindPPDs(m_newHttp), &FindPPDsThread::initPPDMapConstruct);//查找并构建ppd容器
    disconnect(g_cmdFindPPDs(m_newHttp), SIGNAL(ippRequestError(int)),this,SLOT(getPPDsErr(int)));
    disconnect(g_cmdFindPPDs(m_newHttp), SIGNAL(gotAllHandledPPDs(ppdPrinterMap,ppdPrinterMap)),this, SLOT(gotPPDsMap(ppdPrinterMap,ppdPrinterMap))); //获取到所有PPD文件列表的初始map
}

/************************************************
* 函数名称：buildMatchPPDsThreadsAndConnections
* 功能描述：私有函数，建立PPD匹配线程链接
* 输入参数：无
* 输出参数：无
* 修改日期：2021.05.27
* 修改内容：
*   创建  HZH
*
*************************************************/
void FindAndMatchPPDs::buildMatchPPDsThreadsAndConnections()
{
    qDebug() << "建立打印机匹配PPDs的线程";
    g_cmdMatchPPDs->start();
}

/************************************************
* 函数名称：destroyMatchPPDsConnections
* 功能描述：私有函数，销毁PPD匹配线程链接
* 输入参数：无
* 输出参数：无
* 修改日期：2021.05.27
* 修改内容：
*   创建  HZH
*
*************************************************/
void FindAndMatchPPDs::destroyMatchPPDsConnections()
{
    qDebug() << "destroyMatchPPDsConnections";
}
/************************************************
* 函数名称：getCurrentPPDsMap
* 功能描述：public接口，用于只读当前的PPD列表容器
* 输入参数：ppdMap容器
* 输出参数：是否取到了，0表示取到了，2表示Map此时正在更新
          。若取成功，可在入参的ppdMap中取到PPD的Map。
* 修改日期：2021.05.27
* 修改内容：
*   创建  HZH
*
*************************************************/
int FindAndMatchPPDs::getCurrentPPDsMap(ppdPrinterMap& ppdMap) const
{
    qDebug() << "获取当前的PPD文件Map";
    m_mutex.lock();                       //会出现死等
    if((false == m_isPPDsCollecting)  && (true == m_canMatchPPD))
    {
        ppdMap = m_currentPPDsMap;
        m_mutex.unlock();
        return static_cast<int>(FindAndMatchStatus::OPERATION_OK);
    }
    else
    {
        m_mutex.unlock();
        return static_cast<int>(FindAndMatchStatus::PPDS_MAP_IS_NOT_READY);
    }
}
/************************************************
* 函数名称：isBusy
* 功能描述：public接口，用于只读当前是否可以进行PPD列表更
          新
* 输入参数：无
* 输出参数：是否可以进行PPD列表更新，false表示可以，true表示不可以。
* 修改日期：2021.05.27
* 修改内容：
*   创建  HZH
*
*************************************************/
bool FindAndMatchPPDs::isBusy() const
{
    return m_isPPDsCollecting;
}
/************************************************
* 函数名称：hasPPDsMap
* 功能描述：public接口，用于只读当前是否可以进行PPD匹配
* 输入参数：无
* 输出参数：是否可以进行PPD匹配，false表示不可以，true表示可以。
* 修改日期：2021.05.27
* 修改内容：
*   创建  HZH
*
*************************************************/
bool FindAndMatchPPDs::hasPPDsMap() const
{
    return m_canMatchPPD;
}

void FindAndMatchPPDs::detectStart()
{
    qDebug() << "FindAndMatchPPDs Thread is:" << QThread::currentThreadId();
}
