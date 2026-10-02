#ifndef DATETIMEUTILS_H
#define DATETIMEUTILS_H

#include <QObject>
#include <QDBusInterface>
#include <libkydate.h>
#include <QDateTime>
class DateTimeUtils : public QObject
{
    Q_OBJECT
public:
    DateTimeUtils(QObject *parent = nullptr);
    virtual ~DateTimeUtils();
    void WatchingTimeFormat();
    static DateTimeUtils* get_instance();
    QString TranslationTime(QString time);
public slots:
    void DateFormatChange(QString dateFormat);
    void TimeFormatChange(QString timeFormat);
//    QString TranslationTime(QString time);
private:
    static DateTimeUtils *m_dataTimeInstance;
    QString m_dateFormatNow;
    QString m_dateFormatOld;
    QString m_timeFormatNow;
    QString m_timeFormatOld;
    struct tm QDateTimeToCtm(const QDateTime &datetime);


    QDBusInterface * m_dateSessionDbus = nullptr;
signals:
    void ShortDateSignal();
};

#endif // DATETIMEUTILS_H
