#ifndef DATETIMEUTILS_H
#define DATETIMEUTILS_H

#include <QObject>
#include <QDBusInterface>
#include <QDateTime>
#include <libkydate.h>

class DateTimeUtils : public QObject
{
    Q_OBJECT
public:
    DateTimeUtils(QObject *parent = nullptr);
    virtual ~DateTimeUtils();

signals:
    void ShortDateSignal();

public slots:
    void DateFormatChange(QString dateFormat);
    void TimeFormatChange(QString timeFormat);

public:
    QString TranslateDateFormat(const QDateTime &datetime);
    QString TranslateDateFormat(const QString &dateTime, const QString &oldFormat = "yyyy-MM-dd hh:mm:ss");
    QString getOldDateFormat() const {
        return m_dateFormatOld + " " + m_timeFormatOld;
    }

private:
    struct tm QDateTimeToCtm(const QDateTime &datetime);

private:
    QString m_dateFormatNow;
    QString m_dateFormatOld;
    QString m_timeFormatNow;
    QString m_timeFormatOld;

    // 监控是否平板的dbus接口
    QDBusInterface * m_dateSessionDbus = nullptr;
};

#endif // DATETIMEUTILS_H
