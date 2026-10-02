/*
* Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#include <QSqlRecord>
#include <QFile>
#include <QList>
#include <QTemporaryFile>
#include <QProcess>
#include "schedulewidget.h"
#include "calendardatabase.h"
#define  EN_FORMATS                        "en_US"
#define  BO_FORMATS                        "bo"
#define  MN_FORMATS                        "mn"
/**
 * crontab的格式为  *   *   *   *   *   command
 * @param parent  分钟 小时 天  月   周这个不要直接* * * *  指令 即可
 * Notify-send 有的没有关闭按钮先需要运行一下/usr/lib/ukui-notification-daemon/ukui-notifications
 * 因为Kylin-calendar-plugin 已经是常驻进程 所以用QTimer类定时也可以 因为QTimer对象不会被delete 除非用户killall
 * kylin-calendar可以参考一下的del add update函数
 */
CalendarDataBase::CalendarDataBase(QObject *parent) : QObject(parent)
{
    display = getenv("DISPLAY");
    addShell();
    userName = getenv("USER"); //获取当前用户名
    xdg_runtime_dir = getenv("XDG_RUNTIME_DIR");
    XorWayland = getenv("XDG_SESSION_TYPE");
    qDebug() << XorWayland; //wayland 下应该是WAYLAND_DISPLAY=wayland-0 和X11的DISPLAY=:0不一样
    qDebug() << QSqlDatabase::drivers(); //当前环境支持哪些数据库
    m_dataBase = QSqlDatabase::addDatabase("QSQLITE");
    QString dirPath = QString(getenv("HOME")) + "/.config/.kylin_calendar_Schedule2.db";

    appName = tr("calendar");
    Allday = tr("allday");

    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale()); //设置显示中文
    m_dataBase.setDatabaseName(dirPath);

    connect(this, &CalendarDataBase::insertDataSignal, this, &CalendarDataBase::insertDataSlot);
    connect(this, &CalendarDataBase::deleteDataSignal, this, &CalendarDataBase::deleteDataSlot);
    connect(this, &CalendarDataBase::searchDataSignal, this, &CalendarDataBase::searchDataSlot);
    connect(this, &CalendarDataBase::updateDataSignal, this, &CalendarDataBase::updateDataSlot);
}
/**
 * @brief CalendarDataBase::deleteschedule
 * 删除自定义日期过了的日程
 */
void CalendarDataBase::deleteschedule()
{
    QSqlQuery query;
    if (!query.exec("SELECT id FROM Schedule WHERE endrepeat < datetime('now','+1 day')"))
    {
        qDebug() << "Error: Failed to execute query.";
        return;
    }
    QString queryResult;
    while (query.next())
    {
        queryResult += query.value(0).toString() + " ";
    }
    if (!queryResult.isEmpty())
    {
        processExpiredEvents(queryResult.trimmed());
    }
    else
    {
        qDebug() << "No expired events found.";
    }
}
/**
 * @brief CalendarDataBase::processExpiredEvents
 * @param expiredIds
 * 用临时文件去保存已经删除过期日程的crontab,QTemporaryFile会在程序执行结束之后自动删除
 */
void CalendarDataBase::processExpiredEvents(const QString& expiredIds)
{
    QTemporaryFile tempFile;
    if (!tempFile.open())
    {
        qDebug() << "Error: Failed to create temporary file.";
        return;
    }

    QTextStream out(&tempFile);
    QStringList crontabLines = getCurrentCrontab();
    for (const QString& line : crontabLines)
    {
        if(line.trimmed().isEmpty())
        {
            continue;
        }
        QStringList parts = line.split('#');
        if (parts.size() > 1 && expiredIds.contains(parts.last().trimmed()))
        {
            continue;
        }
        out << line << "\n";
    }

    tempFile.close();
    updateCrontabFromFile(tempFile.fileName());
}
/**
 * @brief CalendarDataBase::getCurrentCrontab
 * @return
 * 获取当前的crontab -l
 */
QStringList CalendarDataBase::getCurrentCrontab()
{
    QStringList crontabLines;
    QProcess process;
    process.start("crontab", QStringList() << "-l");
    process.waitForFinished();
    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0)
    {
        QString output = process.readAllStandardOutput();
        crontabLines = output.split('\n');
    }
    else
    {
        qDebug() << "Error: Failed to read current crontab.";
    }
    return crontabLines;
}
/**
 * @brief CalendarDataBase::updateCrontabFromFile
 * @param filename
 * 更新删除完毕的crontab
 *
 */
void CalendarDataBase::updateCrontabFromFile(const QString& filename)
{
    QProcess process;
    process.start("crontab", QStringList() << filename);
    process.waitForFinished();
    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0)
    {
        qDebug() << "Crontab updated successfully.";
    }
    else
    {
        qDebug() << "Error: Failed to update crontab.";
    }
}
/**
 * @brief CalendarDataBase::add
 * @param info
 * 先在数据库里面把原来的 都记录在一个临时文件里面 然后在这个临时文件新写一行
 * info.minute info.hour info.day info.month *星期不要 DISPLAY=:0 /usr/bin/dbus
 */
void CalendarDataBase::add(const MarkInfo &info)
{
    QProcess *c = new QProcess(this);
    int code = c->execute("crontab -l");
    //如果返回的是0，说明用户从来没用过crontab 这个东西，要先把临时文件里面写入原来应该有的注释 再写自己的cron内容
    c->start("crontab -l");
    c->waitForFinished();
    QString tmpFilePath = "/tmp/" + userName + ".tmp";

    qDebug() << "进来的参数hour" << info.hour << info.Alarm_hour;
    QFile tmpFile(tmpFilePath);
    if (tmpFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream out(&tmpFile);
        qDebug() << "code=" << code;
        if(code == 1) //没成功
        {
            QString old = "# Edit this file to introduce tasks to be run by cron. \n"
                          "#\n"
                          "# Each task to run has to be defined through a single line\n"
                          "# indicating with different fields when the task will be run\n"
                          "# and what command to run for the task\n"
                          "#\n"
                          "# To define the time you can provide concrete values for\n"
                          "# minute (m), hour (h), day of month (dom), month (mon),\n"
                          "# and day of week (dow) or use '*' in these fields (for 'any').\n"
                          "#\n"
                          "# Notice that tasks will be started based on the cron's system\n"
                          "# daemon's notion of time and timezones.\n"
                          "#\n"
                          "# Output of the crontab jobs (including errors) is sent through\n"
                          "# email to the user the crontab file belongs to (unless redirected).\n"
                          "#\n"
                          "# For example, you can run a backup of all your user accounts\n"
                          "# at 5 a.m every week with:\n"
                          "# 0 5 * * 1 tar -zcf /var/backups/home.tgz /home/\n"
                          "#\n"
                          "# For more information see the manual pages of crontab(5) and cron(8)\n"
                          "#\n"
                          "# m h  dom mon dow   command\n";
            out << old << "\n";
        }
        QString cronLine = generateCronLine(formatTimeUnit(info.Alarm_end_minute), formatTimeUnit(info.Alarm_end_hour), formatTimeUnit(info.day), formatTimeUnit(info.month), "*",
                                            xdg_runtime_dir, display, appName, info.m_descript, "", info.timePrefix,
                                            formatTimeUnit(info.displayHour), formatTimeUnit(info.minute), info.m_markId );
        qDebug() << "cronLine" << generateCronLine(formatTimeUnit(info.Alarm_end_minute), formatTimeUnit(info.Alarm_end_hour), formatTimeUnit(info.day), formatTimeUnit(info.month), "*",
                                                   xdg_runtime_dir, display, appName, info.m_descript, "", info.timePrefix,
                                                   formatTimeUnit(info.displayHour), formatTimeUnit(info.minute), info.m_markId ) <<
                    info.m_descript;
        QString cronLine_RRule_Day_UnAllday = generateCronLine(formatTimeUnit(info.Alarm_end_minute), formatTimeUnit(info.Alarm_end_hour), "*", "*", "*",
                                                               xdg_runtime_dir, display, appName, info.m_descript, info.m_markStartDate.toString("yyyy/MM/dd"), info.timePrefix,
                                                               formatTimeUnit(info.displayHour), formatTimeUnit(info.minute), info.m_markId );
        QString cronLine_RRule_Week_UnAllday = generateCronLine(formatTimeUnit(info.Alarm_end_minute), formatTimeUnit(info.Alarm_end_hour), "*", "*", formatTimeUnit(info.week),
                                                                xdg_runtime_dir, display, appName, info.m_descript, info.m_markStartDate.toString("yyyy/MM/dd"), info.timePrefix,
                                                                formatTimeUnit(info.displayHour), formatTimeUnit(info.minute), info.m_markId );
        QString cronLine_RRule_Month_UnAllday = generateCronLine(formatTimeUnit(info.Alarm_end_minute), formatTimeUnit(info.Alarm_end_hour), formatTimeUnit(info.day), "*", "*",
                                                                 xdg_runtime_dir, display, appName, info.m_descript, info.m_markStartDate.toString("yyyy/MM/dd"), info.timePrefix,
                                                                 formatTimeUnit(info.displayHour), formatTimeUnit(info.minute), info.m_markId );
        QString cronLine_RRule_Year_UnAllday = generateCronLine(formatTimeUnit(info.Alarm_end_minute), formatTimeUnit(info.Alarm_end_hour), formatTimeUnit(info.day), formatTimeUnit(info.month), "*",
                                                                xdg_runtime_dir, display, appName, info.m_descript, info.m_markStartDate.toString("yyyy/MM/dd"), info.timePrefix,
                                                                formatTimeUnit(info.displayHour), formatTimeUnit(info.minute), info.m_markId );
        QString cronLine_RRule_Day_Allday = generateCronLine("00", "09", "*", "*", "*",
                                                             xdg_runtime_dir, display, appName, info.m_descript, info.m_markStartDate.toString("yyyy/MM/dd"), Allday,
                                                             "", "", info.m_markId );
        QString cronLine_RRule_Week_Allday = generateCronLine("00", "09", "*", "*", formatTimeUnit(info.week),
                                                              xdg_runtime_dir, display, appName, info.m_descript, info.m_markStartDate.toString("yyyy/MM/dd"), Allday,
                                                              "", "", info.m_markId );
        QString cronLine_RRule_Month_Allday = generateCronLine("00", "09", formatTimeUnit(info.day), "*", "*",
                                                               xdg_runtime_dir, display, appName, info.m_descript, info.m_markStartDate.toString("yyyy/MM/dd"), Allday,
                                                               "", "", info.m_markId );
        QString cronLine_RRule_Year_Allday = generateCronLine("00", "09", formatTimeUnit(info.day), formatTimeUnit(info.month), "*",
                                                              xdg_runtime_dir, display, appName, info.m_descript, info.m_markStartDate.toString("yyyy/MM/dd"), Allday,
                                                              "", "", info.m_markId );

        if(m_AlarmType == QString("No prompt"))
        {
            cronLine = "";
        }
        if(m_RRuleType == QString("Daily"))
        {
            cronLine = cronLine_RRule_Day_UnAllday;
        }
        else if(m_RRuleType == QString("Weekly"))
        {
            cronLine = cronLine_RRule_Week_UnAllday;
        }
        else if(m_RRuleType == QString("Monthly"))
        {
            cronLine = cronLine_RRule_Month_UnAllday;
        }
        else if(m_RRuleType == QString("Yearly"))
        {
            cronLine = cronLine_RRule_Year_UnAllday;
        }
        //提前1天
        if(m_AlarmType == QString("1 day in advance"))
        {
            cronLine = generateCronLine(formatTimeUnit(info.Alarm_end_minute), formatTimeUnit(info.Alarm_end_hour), formatTimeUnit(info.day), formatTimeUnit(info.month), "*",
                                        xdg_runtime_dir, display, appName, info.m_descript, "", info.timePrefix,
                                        formatTimeUnit(info.displayHour), formatTimeUnit(info.minute), info.m_markId );
            if(m_RRuleType == QString("Daily"))
            {
                cronLine = cronLine_RRule_Day_UnAllday;
            }
            else if(m_RRuleType == QString("Weekly"))
            {
                cronLine = cronLine_RRule_Week_UnAllday;
            }
            else if(m_RRuleType == QString("Monthly"))
            {
                cronLine = cronLine_RRule_Month_UnAllday;
            }
            else if(m_RRuleType == QString("Yearly"))
            {
                cronLine = cronLine_RRule_Year_UnAllday;
            }

        }
        //提前2天
        if(m_AlarmType == QString("2 day in advance"))
        {
            cronLine = generateCronLine(formatTimeUnit(info.Alarm_end_minute), formatTimeUnit(info.Alarm_end_hour), formatTimeUnit(info.day), formatTimeUnit(info.month), "*",
                                        xdg_runtime_dir, display, appName, info.m_descript, info.m_markStartDate.toString("yyyy/MM/dd"), info.timePrefix,
                                        formatTimeUnit(info.displayHour), formatTimeUnit(info.minute), info.m_markId );
            if(m_RRuleType == QString("Daily"))
            {
                cronLine = cronLine_RRule_Day_UnAllday;
            }
            else if(m_RRuleType == QString("Weekly"))
            {
                cronLine = cronLine_RRule_Week_UnAllday;
            }
            else if(m_RRuleType == QString("Monthly"))
            {
                cronLine = cronLine_RRule_Month_UnAllday;
            }
            else if(m_RRuleType == QString("Yearly"))
            {
                cronLine = cronLine_RRule_Year_UnAllday;
            }
        }
        //1天前上午9点
        if(m_AlarmType == QString("1 day ago at 9am"))
        {
            cronLine = generateCronLine("00", "09", formatTimeUnit(info.day), formatTimeUnit(info.month), "*",
                                        xdg_runtime_dir, display, appName, info.m_descript, info.timePrefix, Allday,
                                        "", "", info.m_markId );
            if(m_RRuleType == QString("Daily"))
            {
                cronLine = cronLine_RRule_Day_Allday;
            }
            else if(m_RRuleType == QString("Weekly"))
            {
                cronLine = cronLine_RRule_Week_Allday;
            }
            else if(m_RRuleType == QString("Monthly"))
            {
                cronLine = cronLine_RRule_Month_Allday;
            }
            else if(m_RRuleType == QString("Yearly"))
            {
                cronLine = cronLine_RRule_Year_Allday;
            }
        }
        if(m_AlarmType == QString("At 9am on the same day of the schedule"))
        {
            cronLine = generateCronLine("00", "09", formatTimeUnit(info.day), formatTimeUnit(info.month), "*",
                                        xdg_runtime_dir, display, appName, info.m_descript, info.timePrefix, Allday,
                                        "", "", info.m_markId );
            if(m_RRuleType == QString("Daily"))
            {
                cronLine = cronLine_RRule_Day_Allday;
            }
            else if(m_RRuleType == QString("Weekly"))
            {
                cronLine = cronLine_RRule_Week_Allday;
            }
            else if(m_RRuleType == QString("Monthly"))
            {
                cronLine = cronLine_RRule_Month_Allday;
            }
            else if(m_RRuleType == QString("Yearly"))
            {
                cronLine = cronLine_RRule_Year_Allday;
            }
        }
        if(m_AlarmType == QString("2 days ago at 9am") || m_AlarmType == QString("1 week ago at 9am"))
        {
            cronLine = generateCronLine("00", "09", formatTimeUnit(info.day), formatTimeUnit(info.month), "*",
                                        xdg_runtime_dir, display, appName, info.m_descript, info.m_markStartDate.toString("yyyy/MM/dd"), Allday,
                                        "", "", info.m_markId );
            if(m_RRuleType == QString("Daily"))
            {
                cronLine = cronLine_RRule_Day_Allday;
            }
            else if(m_RRuleType == QString("Weekly"))
            {
                cronLine = cronLine_RRule_Week_Allday;
            }
            else if(m_RRuleType == QString("Monthly"))
            {
                cronLine = cronLine_RRule_Month_Allday;
            }
            else if(m_RRuleType == QString("Yearly"))
            {
                cronLine = cronLine_RRule_Year_Allday;
            }

        }
        out << cronLine << "\n";
        tmpFile.close();
    }
    else
    {
        qDebug() << "无法新建临时文件" << tmpFile.errorString();
    }
    // 步骤3: 使用crontab命令将tmp文件内容设置为当前用户的crontab
    QProcess *process = new QProcess(this);
    QString safetmpFilePath = shellEscape(tmpFilePath);
    QString command = QString("(crontab -l 2>/dev/null; cat %1) | crontab -").arg(safetmpFilePath);
    QString program = "sh";
    QStringList arguments;
    arguments << "-c" << command;
    process->start(program, arguments);
    // 等待进程结束并检查其退出状态
    if (!process->waitForFinished())
    {
        qDebug() << "Error occurred while executing crontab command.";
        return;
    }
    else
    {
        int exitCode = process->exitCode();
        QString standardOutput = process->readAllStandardOutput();
        QString standardError = process->readAllStandardError();
        if (exitCode != 0)
        {
            qDebug() << "crontab command exited with code" << exitCode;
            qDebug() << "Standard Output:" << standardOutput;
            qDebug() << "Standard Error:" << standardError;
            return ;
        }
        else
        {
            qDebug() << "crontab command executed successfully.";
        }
        QFile::remove(tmpFilePath);
        delete process; //清对象
        delete c;
    }
}
/**
 * @brief CalendarDataBase::update
 * @param info
 * 因为保存的格式是 * * * * * DISPLAY=:0 /usr/bin/dbus #id 遍历数据库的#后面 匹配到日程id 就删除这一行
 * 然后加上新的一行 再把这个tmp临时文件crontab txt保存
 *
 */
void CalendarDataBase::update(const MarkInfo &info)
{
    QString id = info.m_markId;
    del(info);
    add(info);
}

void CalendarDataBase::refreshCronTab(QString timemodel)
{
    int start_day;
    int start_month;
    int start_hour;
    int start_minute;
    int end_hour;
    int end_minute;
    QString timeLong;

    MarkInfo info;
    if(!m_dataBase.open())
    {
        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return;
    }

    /*=============================查询=============================*/
    QString searchCmd = QString("select id,start_date,start_day,start_month,start_hour,start_minute,end_hour,end_minute,descript,remind,"
                                "repeat,beginrepeat,endrepeat,beginDateTime,endDateTime,isAllDay,isLunar,end_date,"
                                "allDayremind from Schedule WHERE endrepeat > datetime('now','+1 day')");
    //WHERE后面的意义是只刷新没有过期的日程，已经过期的日程就不用刷新了，不然会把数据库里面还存在但是不在crontab里面的都刷新回来了。
    QSqlQuery searchQuery;
    if(!searchQuery.exec(searchCmd))
    {
        qDebug() << "query error :" << searchQuery.lastError();
        m_dbStatus = DB_OP_GET_FAILED;
        return;
    }
    /*=============================清空=============================*/
    QProcess *c = new QProcess(this);
    c->execute("crontab -r");
    c->start("crontab -r");
    c->waitForFinished();
    /*=============================解析=============================*/
    while(searchQuery.next())
    {
        start_day = searchQuery.value(2).toInt();
        start_month = searchQuery.value(3).toInt();
        start_hour = searchQuery.value(4).toInt(); //9
        start_minute = searchQuery.value(5).toInt(); //30
        end_hour = searchQuery.value(6).toInt(); //10
        end_minute = searchQuery.value(7).toInt(); //0
        timeLong = handleTimeLong(intToTime(start_hour, start_minute), intToTime(end_hour, end_minute));
        info.m_markId = searchQuery.value(0).toString();
        qDebug() << "搜到的已经过期的日程id" << searchQuery.value(0).toString();
        info.m_markStartDate = searchQuery.value(1).toDate();
        info.day = start_day;
        info.month = start_month;
        info.hour = start_hour;
        info.minute = start_minute;
        info.Alarm_end_minute = start_minute;
        info.Alarm_end_hour = start_hour;
        info.end_hour = end_hour;
        info.end_minute = end_minute;
        info.timeLong = timeLong;
        info.m_descript = searchQuery.value(8).toString();
        qDebug() << "搜到的已经过期的日程描述" << searchQuery.value(8).toString();
        info.remind = searchQuery.value(9).toString();
        info.repeat = searchQuery.value(10).toString();
        info.beginrepeat = searchQuery.value(11).toString();
        info.endrepeat = searchQuery.value(12).toDateTime();
        info.beginDateTime = searchQuery.value(13).toDateTime();
        info.endDateTime = searchQuery.value(14).toDateTime();
        info.m_isAllDay = searchQuery.value(15).toBool();
        info.m_isLunar = searchQuery.value(16).toBool();
        info.m_markEndDate = searchQuery.value(17).toDate();
        info.AlldayRemind = searchQuery.value(18).toString();
        info.week = info.beginDateTime.date().dayOfWeek();

        //解析,这块要做好需要动数据结构和日程的读取逻辑，改动太大，先按现有结构解析
        if(timemodel == "12")
        {
            if(info.hour <= 12)
                info.timePrefix = tr("morning");
            else
                info.timePrefix = tr("afternoon");
        }
        else if(timemodel == "24")
        {
            info.timePrefix = tr("today");
        }

        if(info.remind == tr("No prompt"))
            setAlarmType("No prompt");
        else if(info.remind == tr("Schedule Begin"))
        {
            setAlarmType("Schedule Begin");
            QString strToday = tr("today");
            if(timemodel == "12")
            {
                QString str = strToday + " ";
                info.timePrefix.prepend(str);
            }
        }
        else if(info.remind == tr("5 minutes in advance"))
        {
            setAlarmType("5 minutes in advance");
            if (info.minute >= 5)
            {
                info.Alarm_end_minute = info.minute - 5;
            }
            else
            {
                // 处理分钟数变为负数的情况
                // 60分钟减去提前的5分钟
                info.Alarm_end_minute = info.minute + 55;
                if (info.hour > 0)
                {
                    info.Alarm_end_hour = info.hour - 1;
                }
                else
                {
                    // 如果小时为0，则调整为23
                    info.Alarm_end_hour = 23;
                    // 还需要调整日期
                    QDate adjustedDate = info.m_markStartDate.addDays(-1);
                    info.day = adjustedDate.day();
                    info.month = adjustedDate.month();
                }
            }
            QString strToday = tr("today");
            if(timemodel == "12")
            {
                QString str = strToday + " ";
                info.timePrefix.prepend(str);
            }
        }
        else if(info.remind == tr("10 minutes in advance"))
        {
            setAlarmType("10 minutes in advance");
            if (info.minute >= 10)
            {
                info.Alarm_end_minute = info.minute - 10;
            }
            else
            {
                // 处理分钟数变为负数的情况
                info.Alarm_end_minute = info.minute + 50; // 60分钟减去提前的10分钟
                if (info.hour > 0)
                {
                    info.Alarm_end_hour = info.hour - 1;
                }
                else
                {
                    info.Alarm_end_hour = 23; // 如果小时为0，则调整为23
                    // 还需要调整日期
                    QDate adjustedDate = info.m_markStartDate.addDays(-1);
                    info.day = adjustedDate.day();
                    info.month = adjustedDate.month();
                }
            }
            QString strToday = tr("today");
            if(timemodel == "12")
            {
                QString str = strToday + " ";
                info.timePrefix.prepend(str);
            }
        }
        else if(info.remind == tr("15 minutes in advance"))
        {
            setAlarmType("15 minutes in advance");
            if(info.minute >= 15)
            {
                info.Alarm_end_minute = info.minute - 15;
            }
            else
            {
                // 处理分钟数变为负数的情况
                info.Alarm_end_minute = info.minute + 45; // 60分钟减去提前的15分钟
                if (info.hour > 0)
                {
                    info.Alarm_end_hour = info.hour - 1;
                }
                else
                {
                    info.Alarm_end_hour = 23; // 如果小时为0，则调整为23
                    // 还需要调整日期
                    QDate adjustedDate = info.m_markStartDate.addDays(-1);
                    info.day = adjustedDate.day();
                    info.month = adjustedDate.month();
                }
            }
            QString strToday = tr("today");
            if(timemodel == "12")
            {
                QString str = strToday + " ";
                info.timePrefix.prepend(str);
            }
        }
        else if(info.remind == tr("30 minutes in advance"))
        {
            setAlarmType("30 minutes in advance");
            if(info.minute >= 30)
            {
                info.Alarm_end_minute = info.minute - 30;
            }
            else
            {
                // 处理分钟数变为负数的情况
                info.Alarm_end_minute = info.minute + 30; // 60分钟减去提前的30分钟
                if (info.hour > 0)
                {
                    info.Alarm_end_hour = info.hour - 1;
                }
                else
                {
                    info.Alarm_end_hour = 23; // 如果小时为0，则调整为23
                    // 还需要调整日期
                    QDate adjustedDate = info.m_markStartDate.addDays(-1);
                    info.day = adjustedDate.day();
                    info.month = adjustedDate.month();
                }
            }
            QString strToday = tr("today");
            if(timemodel == "12")
            {
                QString str = strToday + " ";
                info.timePrefix.prepend(str);
            }
        }
        else if(info.remind == tr("1 hour in advance"))
        {
            setAlarmType("1 hour in advance");
            if(info.hour >= 1)
            {
                info.Alarm_end_hour = info.hour - 1;
            }
            else
            {
                // 处理小时数变为负数的情况
                info.Alarm_end_hour = info.hour + 23; // 24小时减去提前的1小时
                // 还需要调整日期
                QDate adjustedDate = info.m_markStartDate.addDays(-1);
                info.day = adjustedDate.day();
                info.month = adjustedDate.month();
            }
            QString strToday = tr("today");
            if(timemodel == "12")
            {
                QString str = strToday + " ";
                info.timePrefix.prepend(str);
            }
        }
        else if(info.remind == tr("1 day in advance"))
        {
            setAlarmType("1 day in advance");
            // 处理提前一天的情况
            QDate adjustedDate = info.m_markStartDate.addDays(-1);
            info.day = adjustedDate.day();
            info.month = adjustedDate.month();
            QString strTomorrow = tr("tomorrow");
            if(timemodel == "12")
            {
                QString str = strTomorrow + " ";
                info.timePrefix.prepend(str);
            }
            else
            {
                info.timePrefix = strTomorrow;
            }

        }
        else if(info.remind == tr("2 day in advance"))
        {
            setAlarmType("2 day in advance");
            // 处理提前两天的情况
            QDate adjustedDate = info.m_markStartDate.addDays(-2);
            info.day = adjustedDate.day();
            info.month = adjustedDate.month();
            if(timemodel == "24")
            {
                info.timePrefix = "";
            }
        }


        if(info.m_isAllDay)
        {
            if(info.AlldayRemind == tr("1 day ago at 9am"))
            {
                setAlarmType("1 day ago at 9am");
                QDate currentDate = info.m_markStartDate;
                QDateTime dateTime(currentDate.addDays(-1)); // 减少一天
                dateTime.setTime(QTime(9, 0)); // 设置时间为上午9点
                info.day = dateTime.date().day();
                info.month = dateTime.date().month();
                info.hour = 0; //因为是全天的 所以开始时间和结束时间就从00:00 - 23:59这样就避免全天的也会置灰现象
                info.minute = 0;
                info.end_hour = 23;
                info.end_minute = 59;
                info.timePrefix = tr("tomorrow");
            }
            else if(info.AlldayRemind == tr("At 9am on the same day of the schedule"))
            {
                setAlarmType("At 9am on the same day of the schedule");
                QDate currentDate = info.m_markStartDate;
                QDateTime dateTime(currentDate); // 使用当前日期
                dateTime.setTime(QTime(9, 0)); // 设置时间为上午9点
                info.day = dateTime.date().day();
                info.month = dateTime.date().month();
                info.hour = 0;//因为是全天的 所以开始时间和结束时间就从00:00 - 23:59这样就避免全天的也会置灰现象
                info.minute = 0;
                info.end_hour = 23;
                info.end_minute = 59;
                info.timePrefix = tr("today");
            }
            else if(info.AlldayRemind == tr("2 days ago at 9am"))
            {
                setAlarmType("2 days ago at 9am");
                QDate currentDate = info.m_markStartDate;
                QDateTime dateTime(currentDate.addDays(-2)); // 减少一天
                dateTime.setTime(QTime(9, 0)); // 设置时间为上午9点
                info.day = dateTime.date().day();
                info.month = dateTime.date().month();
                info.hour = 0;//因为是全天的 所以开始时间和结束时间就从00:00 - 23:59这样就避免全天的也会置灰现象
                info.minute = 0;
                info.end_hour = 23;
                info.end_minute = 59;
                if(timemodel == "24")
                {
                    info.timePrefix = "";
                }
            }
            else if(info.AlldayRemind == tr("1 week ago at 9am"))
            {
                setAlarmType("1 week ago at 9am");
                QDate currentDate = info.m_markStartDate;
                QDateTime dateTime(currentDate.addDays(-7)); // 减少一天
                dateTime.setTime(QTime(9, 0)); // 设置时间为上午9点
                info.day = dateTime.date().day();
                info.month = dateTime.date().month();
                info.hour = 0;//因为是全天的 所以开始时间和结束时间就从00:00 - 23:59这样就避免全天的也会置灰现象
                info.minute = 0;
                info.end_hour = 23;
                info.end_minute = 59;
            }
        }

        //解析重复规则
        if(info.repeat == "不重复")
            setRRuleType("No Repeat");
        else if(info.repeat == "每天")
            setRRuleType("Daily");
        else if(info.repeat == "每周")
            setRRuleType("Weekly");
        else if(info.repeat == "每月")
            setRRuleType("Monthly");
        else if(info.repeat == "每年")
            setRRuleType("Yearly");

        if(m_RRuleType != QString("No Repeat"))
        {
            if(timemodel == "12")
            {
                if(info.hour <= 12)
                    info.timePrefix = tr("morning");
                else
                    info.timePrefix = tr("afternoon");
            }
            else if(timemodel == "24")
            {
                info.timePrefix = tr("");
            }
        }
        //12小时制格式转换
        if(timemodel == "12" && info.hour > 12)
        {
            info.displayHour = info.hour - 12;
        }
        else
        {
            info.displayHour = info.hour;
        }

        add(info);
    }
}

/**
 * @brief CalendarDataBase::del
 * @param info
 * 遍历crontab 的#id后面的数字 如果相同 遍历每一行的#id
 */
void CalendarDataBase::del(const MarkInfo &info)
{
    QProcess *c = new QProcess(this);
    c->start("crontab -l");
    c->waitForFinished();
    QString cronFilePath = "/var/spool/cron/crontabs/" + userName;
    QString tmpFilePath = "/tmp/" + userName + ".tmp";
    QString crontabContent = c->readAllStandardOutput();
    QString id = info.m_markId;
    QString  fileName = "/var/spool/cron/crontabs/" + userName;
    //步骤一 打开文件
    QFile file(fileName);
    //步骤二 创建一个临时文件用于存储不包含指定ID的行
    QFile tempFile("/tmp/" + userName + ".tmp");
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "无法创建临时文件";
        file.close();
        return;
    }
    //步骤三 准备遍历
    QTextStream out(&tempFile);
    QStringList lines = crontabContent.split('\n');

    QString targetId = "#" + info.m_markId;

    for (const QString &line : lines) {
        // 精确匹配：行必须以#id结尾
        if (line.trimmed().endsWith(targetId)) {
            continue; // 跳过匹配行（删除）
        }
        out << line << "\n";  // 保留其他行
    }

    tempFile.close();
    // 删除原文件并将临时文件重命名为原文件名
    QProcess *process = new QProcess(this);
    QString  command = QString("crontab %1").arg(tmpFilePath);
    QString program = "sh";
    QStringList arguments;
    arguments << "-c" << command;
    process->start(program, arguments);
    // 等待进程结束并检查其退出状态
    if (!process->waitForFinished())
    {
        qDebug() << "Error occurred while executing crontab command.";
        return;
    }
    else
    {
        int exitCode = process->exitCode();
        QString standardOutput = process->readAllStandardOutput();
        QString standardError = process->readAllStandardError();
        if (exitCode != 0)
        {
            qDebug() << "crontab command exited with code" << exitCode;
            qDebug() << "Standard Output:" << standardOutput;
            qDebug() << "Standard Error:" << standardError;
            return ;
        }
        else
        {
            qDebug() << "crontab command executed successfully.";
        }
        QFile::remove(tmpFilePath);
        delete process; //清对象
    }
}

int CalendarDataBase::initDataBase()
{
    if(!m_dataBase.open())
    {
        //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;
    }
    else
    {
        QSqlQuery query;
        QString creatTableStr = "CREATE TABLE IF NOT EXISTS Schedule  "
                                "(id          TEXT  NOT NULL,"
                                "start_day    TEXT  NOT NULL,"
                                "start_month  TEXT  NOT NULL,"
                                "start_hour   TEXT  NOT NULL,"
                                "start_minute TEXT  NOT NULL,"
                                "end_hour     TEXT  NOT NULL,"
                                "end_minute   TEXT  NOT NULL,"
                                "end_day      TEXT  NOT NULL,"
                                "end_month    TEXT  NOT NULL,"
                                "descript     TEXT  NOT NULL,"
                                "start_date   TEXT  NOT NULL,"
                                "end_date     TEXT  NOT NULL,"
                                "remind       TEXT  NOT NULL,"
                                "repeat       TEXT  NOT NULL,"
                                "beginrepeat  TEXT  NOT NULL,"
                                "endrepeat    DATETIME  NOT NULL,"
                                "beginDateTime DATETIME NOT NULL,"
                                "endDateTime   DATETIME NOT NULL,"
                                "isAllDay     BOOLEAN NOT NULL,"
                                "isLunar      BOOLEAN NOT NULL,"
                                "allDayremind TEXT   NOT NULL);";

        query.prepare(creatTableStr);
        if (!query.exec())
        {
            qDebug() << "建表失败!" << query.lastError();
            m_dbStatus = CREATE_TABLE_FAILED;
            return CREATE_TABLE_FAILED;
        }
        else
        {
            qDebug() << "建表成功!";
            m_dbStatus = DB_OP_SUCC;
            return DB_OP_SUCC;
        }
    }
}
int CalendarDataBase::insertDataSlot(const MarkInfo &info)
{
    QString uniqueId = info.m_markId;
    QString startDateStr = info.m_markStartDate.toString("yyyy-MM-dd");
    QString endDateStr = info.m_markEndDate.toString("yyyy-MM-dd");
    int day =  info.day;
    int month = info.month;
    int hour =  info.hour;
    int minute =  info.minute;
    int end_hour = info.end_hour;
    int end_minute = info.end_minute;
    int end_day = info.end_day;
    int end_month = info.end_month;
    QString longtime = info.timeLong;
    QString descript = info.m_descript;
    QString remind = info.remind;
    QString repeat = info.repeat;
    QString beginrepeat = info.beginrepeat;
    QDateTime endrepeat = info.endrepeat;
    QDateTime beginDateTime = info.beginDateTime;
    QDateTime endDateTime = info.endDateTime;
    bool isAllDay = info.m_isAllDay;
    bool isLunar = info.m_isLunar;
    QString alldayremind = info.AlldayRemind;

    // 参数化查询
    QString cmd = QString("insert into Schedule values(:uniqueId, :day, :month, :hour, :minute, :end_hour, :end_minute, :end_day, :end_month, :descript, :startDateStr, "
                          ":endDateStr, :remind, :repeat, :beginrepeat, :endrepeat, :beginDateTime, :endDateTime, :isAllDay, :isLunar, :alldayremind)").arg(ITEM_FIELD);
    QSqlQuery query;
    if(!query.prepare(cmd))
    {
        qDebug() << "Prepare failed:" <<query.lastError();
        return 0;
    }

    query.bindValue(":uniqueId", uniqueId);
    query.bindValue(":day", day);
    query.bindValue(":month", month);
    query.bindValue(":hour", hour);
    query.bindValue(":minute", minute);
    query.bindValue(":end_hour", end_hour);
    query.bindValue(":end_minute", end_minute);
    query.bindValue(":end_day", end_day);
    query.bindValue(":end_month", end_month);
    query.bindValue(":descript", descript);
    query.bindValue(":startDateStr", startDateStr);
    query.bindValue(":endDateStr", endDateStr);
    query.bindValue(":remind", remind);
    query.bindValue(":repeat", repeat);
    query.bindValue(":beginrepeat", beginrepeat);
    query.bindValue(":endrepeat", endrepeat.toString(Qt::ISODate));
    query.bindValue(":beginDateTime", beginDateTime.toString(Qt::ISODate));
    query.bindValue(":endDateTime", endDateTime.toString(Qt::ISODate));
    query.bindValue(":isAllDay", QString::number(isAllDay));
    query.bindValue(":isLunar", QString::number(isLunar));
    query.bindValue(":alldayremind", alldayremind);

    if (!query.exec())
    {
        qDebug() << "query error :" << query.lastError();
        m_dbStatus = DB_OP_ADD_FAILED;
        return DB_OP_ADD_FAILED;
    }
    else
    {
        qDebug() << "插入成功!";
        m_dbStatus = DB_OP_SUCC;
        return DB_OP_SUCC;
    }
}
//当天查看当天的日程 然后把m_markInfoList 传出来 再把每个日程信息拿出来传给主窗口的日程信息
int CalendarDataBase::searchDataSlot(const QDate &date)
{
    //这个是当天日期，也就是说，这个日期要在开始日期结束日期之间，就要加进去
    QString startDateStr = date.toString("yyyy-MM-dd");
    QString searchId;
    QDate startDate;
    QDate endDate;
    int start_hour;
    int start_minute;
    int end_hour;
    int end_minute;
    QString timeLong;
    QString descript;
    QString remind;
    QString repeat;
    QString beginrepeat;
    QDateTime endrepeat;
    QDateTime beginDateTime;
    QDateTime endDateTime;
    QString alldayremind;
    bool isAllDay;
    bool isLunar;
    MarkInfo resultInfo;
    if (!m_dataBase.open())   //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
    {
        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;
    }
    else
    {
        //日程的展示需要的是开始时间-结束时间 时长 内容  例如09:30-10:00 30分钟 讨论会
        QString searchCmd =QString("select id,start_date,start_hour,start_minute,end_hour,end_minute,descript,remind,repeat,beginrepeat,endrepeat,beginDateTime,"
                                   "endDateTime,isAllDay,isLunar,end_date,allDayremind from Schedule where (start_date <=:start_date AND end_date >=:end_date)");
        QSqlQuery searchQuery;
        if(!searchQuery.prepare(searchCmd))
        {
            qDebug() << "Prepare failed:" <<searchQuery.lastError();
            return 0;
        }
        searchQuery.bindValue(":start_date",startDateStr);
        searchQuery.bindValue(":end_date",startDateStr);
        searchQuery.exec();
        if (!searchQuery.exec())
        {
            qDebug() << "query error :" << searchQuery.lastError();
            m_dbStatus = DB_OP_GET_FAILED;
            return DB_OP_GET_FAILED;
        }
        else
        {
            m_markIdList.clear();
            m_markInfoList.clear();
            while (searchQuery.next())
            {
                searchId = searchQuery.value(0).toString();
                startDate = searchQuery.value(1).toDate();
                start_hour = searchQuery.value(2).toInt(); //9
                start_minute = searchQuery.value(3).toInt(); //30
                end_hour = searchQuery.value(4).toInt(); //10
                end_minute = searchQuery.value(5).toInt(); //0
                timeLong = handleTimeLong(intToTime(start_hour, start_minute), intToTime(end_hour, end_minute));
                //                endDate = searchQuery.value(2).toDate();
                descript = searchQuery.value(6).toString();
                remind = searchQuery.value(7).toString();
                repeat = searchQuery.value(8).toString();
                beginrepeat = searchQuery.value(9).toString();
                endrepeat = searchQuery.value(10).toDateTime();
                beginDateTime =  searchQuery.value(11).toDateTime();
                endDateTime =  searchQuery.value(12).toDateTime();
                isAllDay = searchQuery.value(13).toBool();
                isLunar = searchQuery.value(14).toBool();
                endDate = searchQuery.value(15).toDate();
                //跨天
                if(startDate != endDate)
                {
                    resultInfo.isKuaDay = true;
                }
                else
                {
                    resultInfo.isKuaDay = false;
                }
                alldayremind = searchQuery.value(16).toString();
                resultInfo.m_markId = searchId;
                resultInfo.m_markStartDate = startDate;
                resultInfo.hour = start_hour;
                resultInfo.minute = start_minute;
                resultInfo.end_hour = end_hour;
                resultInfo.end_minute = end_minute;
                resultInfo.timeLong = timeLong;
                resultInfo.m_descript = descript;
                resultInfo.remind = remind;
                resultInfo.repeat = repeat;
                resultInfo.beginrepeat = beginrepeat;
                resultInfo.endrepeat = endrepeat;
                resultInfo.beginDateTime = beginDateTime;
                resultInfo.endDateTime = endDateTime;
                resultInfo.m_isAllDay = isAllDay;
                resultInfo.m_isLunar = isLunar;
                resultInfo.m_markEndDate = endDate;
                resultInfo.AlldayRemind = alldayremind;

                m_markIdList.append(searchId);
                m_markInfoList.append(resultInfo);
            }
            qDebug() << m_markIdList;
            if (searchId != "")
            {
                qDebug() << "找到符合条件的id一共有" << m_markIdList.count() << "条数据";
                m_dbStatus = DB_OP_SUCC;
                return DB_OP_SUCC;
            }
            else
            {
                m_dbStatus = DB_OP_GET_FAILED;
                return DB_OP_GET_FAILED;
            }
        }
    }
}
QList<MarkInfo> CalendarDataBase::getAllMarkIds()
{
    // 注意：此函数返回的对象仅包含m_markId字段
    // 其他字段为默认值，不可用于需要完整信息的场景
    //    QStringList markIdList;
    QList<MarkInfo> all_markInfoList;
    MarkInfo resultInfo;
    QSqlQuery query;
    QString selectCmd = "SELECT id FROM Schedule";

    // 打开数据库连接（如果尚未打开）
    if (!m_dataBase.isOpen())
    {
        if (!m_dataBase.open())
        {
            qDebug() << "Failed to open database:" << m_dataBase.lastError().text();
            m_dbStatus = DB_UNCONNECT; // 假设这是一个枚举值，表示数据库未连接
            return all_markInfoList; // 返回空列表，因为无法连接数据库
        }
    }

    // 执行查询
    if (!query.exec(selectCmd))
    {
        qDebug() << "Query error:" << query.lastError();
        m_dbStatus = DB_OP_GET_FAILED; // 假设这是一个枚举值，表示操作失败
        return all_markInfoList; // 返回空列表，因为查询失败
    }

    // 遍历查询结果并收集 markid
    while (query.next())
    {
        resultInfo.m_markId = query.value(0).toString(); // 假设 id 是第一列
        all_markInfoList.append(resultInfo);
    }

    // 如果成功获取数据，则设置数据库状态为成功
    m_dbStatus = DB_OP_SUCC; // 假设这是一个枚举值，表示操作成功
    return all_markInfoList; // 返回包含所有 markid 的列表
}
int CalendarDataBase::deleteDataSlot(const MarkInfo &info)
{
    QString markId = info.m_markId;
    if (!m_dataBase.open())   //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
    {

        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;

    }
    else
    {

        if (markId != "")
        {
            QString cmd = QString("delete from Schedule where id =:markId");
            QSqlQuery query;
            if(!query.prepare(cmd))
            {
                qDebug() << "Prepare failed:" <<query.lastError();
                return 0;
            }
            query.bindValue(":markId", markId);
            if (!query.exec())
            {
                qDebug() << "query error :" << query.lastError();
                m_dbStatus = DB_OP_DEL_FAILED;
                return DB_OP_DEL_FAILED;
            }
            else
            {
                qDebug() << "删除成功!";
                m_dbStatus = DB_OP_SUCC;
                return DB_OP_SUCC;
            }
        }
        else
        {
            m_dbStatus = SCHEDULE_UNFOUND;
            return SCHEDULE_UNFOUND;
        }
    }
}
/**
 * @brief CalendarDataBase::updateDataSlot
 * @param info
 * @return 更新主要改的是开始时间和结束时间，未来的kylin-calendar 的所有日程的item 用的是数据库里面的开始时间和结束时间来计算长度的
 * 每一次更新 /删除 /添加都是重新 获取一遍数据库的内容来重新渲染当前页面 包括视图 切换 也是堆栈Widget再重新获取数据渲染
 */
int CalendarDataBase::updateDataSlot(const MarkInfo &info)
{
    QString uniqueId = info.m_markId;
    QString startDateStr = info.m_markStartDate.toString("yyyy-MM-dd");
    QString endDateStr = info.m_markEndDate.toString("yyyy-MM-dd");
    int day =  info.day;
    int month = info.month;
    int hour =  info.hour;
    int minute =  info.minute;
    int end_hour = info.end_hour;
    int end_minute = info.end_minute;
    int end_day = info.end_day;
    int end_month = info.end_month;
    QString descript = info.m_descript;
    QString longtime = info.timeLong;
    QString remind = info.remind;
    QString repeat = info.repeat;
    QString beginrepeat = info.beginrepeat;
    QDateTime endrepeat = info.endrepeat;
    QDateTime beginDateTime = info.beginDateTime;
    QDateTime endDateTime = info.endDateTime;
    bool isAllDay = info.m_isAllDay;
    bool isLunar = info.m_isLunar;
    QString alldayremind = info.AlldayRemind;

    if (!m_dataBase.open())   //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
    {
        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;
    }
    else
    {
        if (uniqueId != "")
        {
            QString cmd = QString("update Schedule set start_day =:start_day , start_month =:start_month, start_hour =:start_hour, start_minute =:start_minute, end_hour =:end_hour,"
                                  " end_minute =:end_minute, end_day =:end_day, end_month =:end_month, descript =:descript, start_date =:startDateStr, end_date =:endDateStr, "
                                  "remind =:remind, repeat =:repeat, beginrepeat =:beginrepeat, endrepeat =:endrepeat, beginDateTime =:beginDateTime, endDateTime =:endDateTime,"
                                  "isAllDay =:isAllDay, isLunar =:isLunar, allDayremind =:alldayremind where id =:uniqueId");
            QSqlQuery query;
            if(!query.prepare(cmd))
            {
                qDebug() << "Prepare failed:" <<query.lastError();
                return 0;
            }

            query.bindValue(":start_day", day);
            query.bindValue(":start_month", month);
            query.bindValue(":start_hour", hour);
            query.bindValue(":start_minute", minute);
            query.bindValue(":end_hour", end_hour);
            query.bindValue(":end_minute", end_minute);
            query.bindValue(":end_day", end_day);
            query.bindValue(":end_month", end_month);
            query.bindValue(":descript", descript);
            query.bindValue(":startDateStr", startDateStr);
            query.bindValue(":endDateStr", endDateStr);
            query.bindValue(":remind", remind);
            query.bindValue(":repeat", repeat);
            query.bindValue(":beginrepeat", beginrepeat);
            query.bindValue(":endrepeat", endrepeat.toString(Qt::ISODate));
            query.bindValue(":beginDateTime", beginDateTime.toString(Qt::ISODate));
            query.bindValue(":endDateTime", endDateTime.toString(Qt::ISODate));
            query.bindValue(":isAllDay", QString::number(isAllDay));
            query.bindValue(":isLunar", QString::number(isLunar));
            query.bindValue(":alldayremind", alldayremind);
            query.bindValue(":uniqueId", uniqueId);

            qDebug() << "cmd:" << cmd;
            if (!query.exec())
            {
                qDebug() << "query error :" << query.lastError();
                m_dbStatus = DB_OP_ALTER_FAILED;
                return DB_OP_ALTER_FAILED;
            }
            else
            {
                qDebug() << "修改成功!";
                m_dbStatus = DB_OP_SUCC;
                return DB_OP_SUCC;
            }
        }
        else
        {

            m_dbStatus = SCHEDULE_UNFOUND;
            return SCHEDULE_UNFOUND;
        }
    }
}
/**
 * @brief CalendarDataBase::convertToCronFormat
 * @param record
 * @return 将得到的数据库内容转换成crontab格式 如果要添加日程id 可以在最后加一个#id
 * 类似于* * * * * command #123 这样123不会影响正常cron服务，以后直接判断#后面的id 就删除单行
 */
QString CalendarDataBase::convertToCronFormat(const QSqlRecord &record)
{
    QString minute = record.value("minute").toString();
    QString hour = record.value("hour").toString();
    QString dayOfMonth = record.value("day_of_month").toString();
    QString month = record.value("month").toString();
    QString dayOfWeek = record.value("day_of_week").toString();
    QString command = record.value("command").toString();

    // 如果某个字段为空，则使用星号(*)作为占位符
    if (minute.isEmpty()) minute = "*";
    if (hour.isEmpty()) hour = "*";
    if (dayOfMonth.isEmpty()) dayOfMonth = "*";
    if (month.isEmpty()) month = "*";
    if (dayOfWeek.isEmpty()) dayOfWeek = "*";

    // 构建cron格式的字符串
    QString cronString = QString("%1 %2 %3 %4 %5 %6")
            .arg(minute)
            .arg(hour)
            .arg(dayOfMonth)
            .arg(month)
            .arg(dayOfWeek)
            .arg(command);
    return cronString;
}
/**
 * @brief CalendarDataBase::getAllCronJobs
 * @return 获取所有的数据
 */
QStringList CalendarDataBase::getAllCronJobs()
{
    QSqlQuery query;
    QString selectCronJobsStr = "SELECT start_time, end_time, start_date, end_date, command FROM Schedule;";
    query.prepare(selectCronJobsStr);

    if (!query.exec())
    {
        qDebug() << "查询cron作业失败!";
        m_dbStatus = CREATE_TABLE_FAILED;
        return QStringList(); //返回一个空的
    }
    else
    {
        QStringList cronJobs;
        while (query.next())
        {
            QString cronJob = convertToCronFormat(query.record());
            cronJobs.append(cronJob);
        }
        qDebug() << "查询cron作业成功!";
        m_dbStatus = DB_OP_SUCC;
        return cronJobs;
    }
}
DB_RETURN_STATUS CalendarDataBase::getStatus()
{
    return m_dbStatus;
}

void CalendarDataBase::setMarkInfo(const MarkInfo &info)
{
    m_info = info;
}

MarkInfo CalendarDataBase::getMarkInfo()
{
    return m_info;
}

QStringList CalendarDataBase::getMarkIdList()
{
    return m_markIdList;
}

QList<MarkInfo> CalendarDataBase::getMarkInfoList()
{
    return m_markInfoList;
}

CalendarDataBase::~CalendarDataBase()
{
    qDebug() << "析构RecorderDataBase";
    if (m_dataBase.open())
    {
        m_dataBase.close();
    }
}
/**
 * @brief CalendarDataBase::intToTime
 * @param hour
 * @param minute
 * @return 时间和小时组合成时间
 */
QTime CalendarDataBase::intToTime(int hour, int minute)
{
    QTime time(hour, minute); //9,0 -> 9:00
    return time;
}
/**
 * @brief CalendarDataBase::handleTimeLong
 * @param time1
 * @param time2
 * @return
 */
QString CalendarDataBase::handleTimeLong(QTime time1, QTime time2)
{
    //确保结束时间大于开始时间
    if (time1 > time2)
    {
        QTime temp = time1;
        time1 = time2;
        time2 = temp;
    }
    // 计算时间差（以毫秒为单位）
    int msecs = time1.msecsTo(time2);
    // 转换为小时和分钟
    int hours = msecs / (1000 * 60 * 60);
    int mins = (msecs % (1000 * 60 * 60)) / (1000 * 60);
    QString hour = QString::number(hours);
    QString min = QString::number(mins);
    //       QString result = hour + "小时" + min +"分钟";
    //构造结果字符串
    QString result;
    if (hours > 0)
    {
        result = hour + tr("hour") + min + tr("minute");
    }
    else
    {
        result = min + tr("minute");
    }
    if(mins == 0)
    {
        result = hour + tr("hour");
    }
    return result;
}
QStringList CalendarDataBase::getIdList()
{
    return m_markIdList;
}
QList<MarkInfo> CalendarDataBase::getInfoList()
{
    return m_markInfoList;
}
void CalendarDataBase::addShell()
{
}

QString CalendarDataBase::shellEscape(const QString& input)
{
    if (input.isEmpty())
    {
        return QString("''");
    }

    QString escaped;
    escaped.reserve(input.length() * 2);

    for (QChar c : input)
    {
        switch (c.unicode())
        {
        case '\\': escaped.append("\\\\"); break;
        case '\'': escaped.append("\\'");  break;
        case '"':  escaped.append("\\\""); break;
        case '`':  escaped.append("\\`");  break;
        case '$':  escaped.append("\\$");  break;
        case '|':  escaped.append("\\|");  break;
        case ';':  escaped.append("\\;");  break;
        case '&':  escaped.append("\\&");  break;
        case '(':  escaped.append("\\(");  break;
        case ')':  escaped.append("\\)");  break;
        case '<':  escaped.append("\\<");  break;
        case '>':  escaped.append("\\>");  break;
        case ' ':  escaped.append("\\ ");  break;
        case '*':  escaped.append("\\*");  break;
        case '?':  escaped.append("\\?");  break;
        case '{':  escaped.append("\\{");  break;
        case '}':  escaped.append("\\}");  break;
        case '[':  escaped.append("\\[");  break;
        case ']':  escaped.append("\\]");  break;
        case '#':  escaped.append("\\#");  break;
        case '\n': escaped.append("\\n");  break;
        case '\t': escaped.append("\\t");  break;
        case '\r': escaped.append("\\r");  break;
        default:
            escaped.append(c);
            break;
        }
    }
    return escaped;
}

QString CalendarDataBase::generateCronLine(const QString& minute, const QString& hour, const QString& day, const QString& month, const QString& week, const QString& xdg_runtime_dir, const QString& display, const QString& appName, const QString& description, const QString& date, const QString& shangwu, const QString& cronHour, const QString& cronMinute,  const QString& markId)
{
    QString descript = shellEscape(description);
    if(m_AlarmType == QString("1 day ago at 9am") || m_AlarmType == QString("At 9am on the same day of the schedule") || m_AlarmType == QString("2 days ago at 9am") || m_AlarmType == QString("1 week ago at 9am"))
    {
        QString  command =  QString("%1 %2 %3 %4 %5 XDG_RUNTIME_DIR=%6 DISPLAY=%7 /usr/bin/notify-send -i kylin-calendar -h string:desktop-entry:calendar --app-name=%8 \":descript\" \":date :allday \" #:markId");
        command.replace("%1", minute)
                .replace("%2", hour)
                .replace("%3", day)
                .replace("%4", month)
                .replace("%5", week)
                .replace("%6", xdg_runtime_dir)
                .replace("%7", display)
                .replace("%8", appName)
                .replace(":descript", descript)
                .replace(":date", date)
                .replace(":allday", Allday)
                .replace(":markId", markId);
        return command;
    }
    else
    {
        QString command =  QString("%1 %2 %3 %4 %5 XDG_RUNTIME_DIR=%6 DISPLAY=%7 /usr/bin/notify-send -i kylin-calendar -h string:desktop-entry:calendar --app-name=%8 \":descript\" \":date:shangwu :cronHour::cronMinute\" #:markId");
        command.replace("%1", minute)
                .replace("%2", hour)
                .replace("%3", day)
                .replace("%4", month)
                .replace("%5", week)
                .replace("%6", xdg_runtime_dir)
                .replace("%7", display)
                .replace("%8", appName)
                .replace(":descript", descript)
                .replace(":date", date)
                .replace(":shangwu", shangwu)
                .replace(":cronHour", cronHour)
                .replace(":cronMinute", cronMinute)
                .replace(":markId", markId);
        return command;
    }
}
QString CalendarDataBase::formatTimeUnit(int unit)
{
    return unit < 10 ? "0" + QString::number(unit) : QString::number(unit);
}
void CalendarDataBase::setRRuleType(QString RRuleType)
{
    m_RRuleType = RRuleType;
}
void CalendarDataBase::setAlarmType(QString AlarmType)
{
    m_AlarmType = AlarmType;
}
int CalendarDataBase::deleteDataSlotfromMarkID(QString markID)
{
    QString markId = markID;
    if (!m_dataBase.open())   //如果数据库打开失败，会弹出一个警告窗口;由于此类非指针类型因此是0
    {

        QMessageBox::warning(0, QObject::tr("Database Error"), m_dataBase.lastError().text());
        m_dbStatus = DB_UNCONNECT;
        return DB_UNCONNECT;

    }
    else
    {

        if (markId != "")
        {

            QString cmd = QString("delete from Schedule where id = '%1'").arg(markId);
            QSqlQuery query;
            if (!query.exec(cmd))
            {
                qDebug() << "query error :" << query.lastError();
                m_dbStatus = DB_OP_DEL_FAILED;
                return DB_OP_DEL_FAILED;
            }
            else
            {
                qDebug() << "删除成功!";
                m_dbStatus = DB_OP_SUCC;
                return DB_OP_SUCC;
            }

        }
        else
        {

            m_dbStatus = SCHEDULE_UNFOUND;
            return SCHEDULE_UNFOUND;
        }
    }
}
int  CalendarDataBase::deleteCrontabfromMarkID(QString markID)
{
    QProcess *c = new QProcess(this);
    c->start("crontab -l");
    c->waitForFinished();
    QString tmpFilePath = "/tmp/" + userName + ".tmp";
    QString crontabContent = c->readAllStandardOutput();
    QString id = markID;
    QString  fileName = "/var/spool/cron/crontabs/" + userName;
    //步骤一 打开文件
    QFile file(fileName);
    //步骤二 创建一个临时文件用于存储不包含指定ID的行
    QFile tempFile("/tmp/" + userName + ".tmp");
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "无法创建临时文件";
        file.close();
        return 0;
    }
    //步骤三 准备遍历
    QTextStream out(&tempFile);
    QStringList lines = crontabContent.split('\n');
    QString targetId = "#" + id; // 加上#前缀来匹配文件中的格式
    //开始遍历
    for (const QString &line : lines)
    {
        if (!line.contains(targetId) || line.startsWith("#"))   // 排除注释行 并且保留最开始的注释
        {
            out << line << "\n";
        }
    }
    tempFile.close();
    // 删除原文件并将临时文件重命名为原文件名
    QProcess *process = new QProcess(this);
    QString  command = QString("crontab %1").arg(tmpFilePath);
    QString program = "sh";
    QStringList arguments;
    arguments << "-c" << command;
    process->start(program, arguments);
    // 等待进程结束并检查其退出状态
    if (!process->waitForFinished())
    {
        qDebug() << "Error occurred while executing crontab command.";
        return DB_OP_DEL_FAILED;
    }
    else
    {
        int exitCode = process->exitCode();
        QString standardOutput = process->readAllStandardOutput();
        QString standardError = process->readAllStandardError();
        if (exitCode != 0)
        {
            qDebug() << "crontab command exited with code" << exitCode;
            qDebug() << "Standard Output:" << standardOutput;
            qDebug() << "Standard Error:" << standardError;
            return DB_OP_DEL_FAILED;
        }
        else
        {
            qDebug() << "crontab command executed successfully.";
        }
        QFile::remove(tmpFilePath);
        delete process; //清对象
        return DB_OP_SUCC;
    }
}
