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

#ifndef GLOBALDATEFORMAT_H
#define GLOBALDATEFORMAT_H

#include <QObject>
#include <QDBusInterface>

/**
 *  GlobalDateFormat类主要监控ukui-control-center中的
 *  “日期短格式”、“时间” 两种格式
 */
class GlobalDateFormat : public QObject
{
    Q_OBJECT
public:
    ~GlobalDateFormat();

public:
    static GlobalDateFormat *getInstance();

    QString TranslateDateAndTimeFormat(QString dateTime, QString strToDateTimeFormat, QString dateTimeToStrFormat);
    struct tm QDateTimeToCtm(const QDateTime &datetime);
    QString TranslateDateFormat(const QDateTime &datetime);

Q_SIGNALS:
    void sigShortDateFormatChange();
    void sigTimeFormatChange(QString);

private:
    GlobalDateFormat();
    void init(void);

    QString m_shortDateFormat;  /* "日期短格式" 的转化格式 */
    QString m_timeFormat;       /* "时间" 的转化格式 */
    QString m_timeSystem;       /* 12小时制度或24小时制度 */
    QDBusInterface *m_dateFormatSessionDbus = nullptr;


private Q_SLOTS:
    void slotDateFormatChange(QString);
    void slotShortDateFormatChange(QString);    /* 日期短格式 (如：23-6-25) */
    void slotLongDataFormatChange(QString);     /* 日期长格式 (如：23年6月25日) */
    void slotTimeFormatChange(QString);         /* 时间 (如：12小时) */
};

#endif // GLOBALDATEFORMAT_H
