#ifndef DESKTOPCLOCKBACKEND_H
#define DESKTOPCLOCKBACKEND_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include <QTimeZone>
#include <QVariantList>

class ClockBackend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString timeDisplayText READ timeDisplayText NOTIFY timeChanged)
    Q_PROPERTY(QString zoneText READ zoneText NOTIFY timeChanged)
    Q_PROPERTY(QString weekText READ weekText NOTIFY timeChanged)
    Q_PROPERTY(QString monthDayText READ monthDayText NOTIFY timeChanged)
    Q_PROPERTY(int hour READ hour NOTIFY timeChanged)
    Q_PROPERTY(int minute READ minute NOTIFY timeChanged)
    Q_PROPERTY(int second READ second NOTIFY timeChanged)

public:
    explicit ClockBackend(QObject *parent = nullptr);

    QString timeDisplayText() const;
    QString zoneText() const;
    QString weekText() const;
    QString monthDayText() const;
    int hour() const;
    int minute() const;
    int second() const;
    Q_INVOKABLE void setWidgetInstanceId(int instanceId);
    Q_INVOKABLE void openTimezoneChooser();
    Q_INVOKABLE void removeInstanceConfig();

Q_SIGNALS:
    void timeChanged();

private Q_SLOTS:
    void updateTime();

private:
    void loadInstanceTimezone();
    void saveInstanceTimezone() const;
    void setTimezoneId(const QString &timezoneId);
    QDateTime effectiveNow() const;
    QTimeZone effectiveTimeZone() const;
    QVariantList buildTimezoneOptions() const;

    QString m_timeDisplayText;
    QString m_zoneText;
    QString m_weekText;
    QString m_monthDayText;
    int m_hour = 0;
    int m_minute = 0;
    int m_second = 0;
    QString m_timezoneId;
    int m_widgetInstanceId = -1;
    QTimer  m_timer;
};

#endif // DESKTOPCLOCKBACKEND_H
