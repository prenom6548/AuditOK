#include "datetimeutils.h"

#include <QDBusReply>
#include <QDebug>
#include <locale.h>
#include <libintl.h>
DateTimeUtils* DateTimeUtils::m_dataTimeInstance = nullptr;
DateTimeUtils *DateTimeUtils::get_instance()
{
    if(m_dataTimeInstance == nullptr){
        m_dataTimeInstance = new DateTimeUtils;
    }
    return m_dataTimeInstance;
}


DateTimeUtils::DateTimeUtils(QObject *parent /*= nullptr*/): QObject(parent)
{
    WatchingTimeFormat();
}

void DateTimeUtils::WatchingTimeFormat()
{
    //监听系统时间格式转换
    m_dateSessionDbus = new QDBusInterface("com.kylin.kysdk.DateServer",
                                             "/com/kylin/kysdk/Date",
                                             "com.kylin.kysdk.DateInterface",
                                             QDBusConnection::sessionBus(),
                                             this);
    if (m_dateSessionDbus->isValid()) {
        connect(m_dateSessionDbus, SIGNAL(DateSignal(QString)), this, SLOT(TranslationTime(QString)));
        connect(m_dateSessionDbus, SIGNAL(ShortDateSignal(QString)), this, SLOT(TranslationTime(QString)));
        connect(m_dateSessionDbus, SIGNAL(LongDateSignal(QString)), this, SLOT(TranslationTime(QString)));
        connect(m_dateSessionDbus, SIGNAL(TimeSignal(QString)), this, SLOT(TranslationTime(QString)));
    }
//    m_dateFormatOld = "yyyy-MM-dd";
//    m_dateFormatNow = kdk_system_get_shortformat();
//    m_timeFormatOld = "hh:mm:ss";
//    QString newTimeFormat(kdk_system_get_now_timeformat());//这个接口会影响安全中心翻译
//    setLanguage();  //重新加载翻译
//    if (newTimeFormat.contains("12")) {
//        m_timeFormatNow = "ap hh:mm:ss";
//    } else {
//        m_timeFormatNow = "hh:mm:ss";
//    }
}

//void DateTimeUtils::DateFormatChange(QString dateFormat)
//{
//    Q_UNUSED(dateFormat)
//    QString shortDateFormat(kdk_system_get_shortformat());
//    if (m_dateFormatNow != shortDateFormat) {
//        m_dateFormatOld = m_dateFormatNow;
//        m_dateFormatNow = shortDateFormat;
//        emit this->ShortDateSignal();
//        qDebug()<<"emit ShortDateSignal";
//    }
//}

//struct tm DateTimeUtils::QDateTimeToCtm(const QDateTime &datetime)
//{
//    struct tm c_time;
//    c_time.tm_year = datetime.date().year();
//    c_time.tm_mon = datetime.date().month();
//    c_time.tm_mday = datetime.date().day();
//    c_time.tm_hour = datetime.time().hour();
//    c_time.tm_min = datetime.time().minute();
//    c_time.tm_sec = datetime.time().second();

//    return c_time;
//}

QString DateTimeUtils::TranslationTime(QString time)
{
    qInfo()<<"sdk====>TranslationTime function";
    if(time == tr("No information!"))
    {
        return time;
    }
    if(time == NULL)
    {
        return time;
    }
    if(!time.contains(" "))
    {
        return time;
    }
    if(!time.contains("-"))
    {
        return time;
    }
    if(!time.contains(":"))
    {
        return time;
    }
    struct tm timestruct;
//    time = "2022-08-01 22:00:00";
    QStringList timelist = time.split(" ");
    qInfo()<<"timelist is"<<timelist;
    QString datesplit = timelist.at(0);
    QString timesplit = timelist.at(1);
    qInfo()<<datesplit<<timesplit;
    /*日期处理*/
    QStringList datedoublesplit = datesplit.split("-");
    qInfo()<<datedoublesplit;
    QString year = datedoublesplit.at(0);
    timestruct.tm_year = year.toInt();
    QString month = datedoublesplit.at(1);
    timestruct.tm_mon = month.toInt();
    QString day = datedoublesplit.at(2);
    timestruct.tm_mday = day.toInt();
    qInfo()<<timestruct.tm_year<<timestruct.tm_mon<<timestruct.tm_mday;
    char* sdkdate = kdk_system_shortformat_transform(&timestruct);
    qInfo()<<"h:"<<sdkdate;
    QString sdkdatestring = QString::fromUtf8(sdkdate);
    qInfo()<<sdkdatestring;
    if(sdkdatestring.length()==0)
        return time;

    /*时间处理*/
    qInfo()<<"1";
    QStringList timedoublesplit = timesplit.split(":");
    QString hour = timedoublesplit.at(0);
    timestruct.tm_hour = hour.toInt();
    QString minute = timedoublesplit.at(1);
    timestruct.tm_min = minute.toInt();
    QString seconds = timedoublesplit.at(2);
    timestruct.tm_sec = seconds.toInt();
    qInfo()<<"2";
    kdk_timeinfo *kdk_timestruct =kdk_system_timeformat_transform(&timestruct);
    qInfo()<<"3";
//    qDebug()<<kdk_timestruct->time;
//    qDebug()<<kdk_timestruct->timesec;
//    char *kdk_timestruct_second=kdk_timestruct->timesec;
    qInfo()<<"4";
    QByteArray byteArray(kdk_timestruct->timesec, strlen(kdk_timestruct->timesec));
    qInfo()<<"5";
    QString sdktime = QString::fromUtf8(byteArray);
    qDebug()<<sdktime;
    QString sdk_time = sdkdatestring + " " +sdktime;
    qInfo()<<"finish tanslationtime"<<sdk_time;
    if(sdk_time.length()==0)
        return time;
    return sdk_time;
}

DateTimeUtils::~DateTimeUtils()
{
//    if (m_dateSessionDbus)
//        delete m_dateSessionDbus;
}
