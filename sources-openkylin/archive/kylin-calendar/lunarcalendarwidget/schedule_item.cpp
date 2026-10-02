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
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScreen>
#include <QLabel>
#include <QPainter>
#include <QAction>
#include <QMouseEvent>
#include <QGuiApplication>
#include <QMenu>
#include "calendardatabase.h"
#include <QColor>
#include <QGSettings>
#include "schedule_item.h"
#include "schedulewidget.h"
#include "calendarcolor.h"
#include "colorarea.h"
#include "schedulewidget.h"
#define THEME_SCHAME "org.ukui.style"
#define COLOR_THEME "themeColor"
#define THEME_SCHAME "org.ukui.style"
#define COLOR_THEME "styleName"
#ifndef Q_UNUSED


    #define Q_UNUSED(x) (void)x;

#endif
schedule_item::schedule_item(QWidget *parent, bool ishasSchedule) : QWidget(parent)
{
    // 初始化布局
    mainLayout = new QHBoxLayout(this);
    this->setMouseTracking(true);
    setAccessibleName("kylin-calendar_plugin_scheduleItem_widget");
    setAccessibleDescription("every schedule body in calendar ui");
    // 创建颜色区域
    colorArea = new ColorArea(this);
    colorArea->setAccessibleName("kylin-calendar-plugin_scheduleItem_color_area_widget");
    colorArea->setAccessibleDescription("every schedule part left color from syetem theme color");
    colorArea->setFixedSize(4, 44); // 设置大小
    //实时监听主题变化

    mainLayout->addWidget(colorArea);
    hasSchdule = ishasSchedule;
    lbl_dateTimeLabel = new QLabel(this);
    lbl_dateTimeLabel->setAccessibleName("kylin-calendar-plugin_QLabel_date_time_label");
    lbl_dateTimeLabel->setAccessibleDescription("the first line of every schedule");
    lbl_dateTimeLabel->setFixedHeight(25);
    lbl_dateTimeLabel->setMargin(0); // 设置内边距
    id = new QLabel(this);
    id->setAccessibleName("kylin-calendar-plugin_QLabel_schedule_id_label");
    id->setAccessibleDescription("the unique id of every schedule index");
    id->hide();
    lbl_eventLabel = new QLabel(this);
    lbl_eventLabel->setAccessibleName("kylin-calendar-plugin_QLabel_event_label");
    lbl_eventLabel->setAccessibleDescription("the second line of every schedule");
    lbl_eventLabel->setMargin(0); // 设置内边距

    // 添加间距
    mainLayout->addSpacing(4);

    // 创建水平布局用于日期、时间和日程内容
    eventLayout = new QVBoxLayout();

    textLayout = new QVBoxLayout();

    textLayout->addWidget(lbl_dateTimeLabel);
    textLayout->addWidget(lbl_eventLabel);

    eventLayout->addLayout(textLayout);

    mainLayout->addLayout(eventLayout);

    // 添加间距
    //eventLayout->addSpacing(0);
    // 设置布局
    setLayout(mainLayout);

}
void schedule_item::enterEvent(QEvent *event)
{

}
void schedule_item::setId(const QString &idd)
{
    id->setText(idd);
}
void schedule_item::setStartTime(const QString &startTime)
{
    QFont font;
    QFontMetrics metrics(font);
    QString descriptionTrimmed = startTime.trimmed(); // 去除两端空白字符，确保准确比较长度
    QString elided = metrics.elidedText(descriptionTrimmed, Qt::ElideRight, 330);
    lbl_dateTimeLabel->setText(tr("Schedule:") + elided);
    //    lbl_dateTimeLabel->setToolTip(QString(tr("Schedule:") + startTime));
    //    lbl_dateTimeLabel->setStyleSheet(styleSheet);
    if(!hasSchdule)
    {
        lbl_dateTimeLabel->setText(tr(" No Schedule:") + elided);
    }
    lbl_dateTimeLabel->setStyleSheet(styleSheet);
}
//设置 已经结束了的日程置灰 但是还没有做到日程ASC功能
void schedule_item::setEndTime(int end_hour, int end_minute, QDate date, QString shangwu)
{
    //    if(shangwu == "afternoon")
    if(timemode == "12")
    {
        if(end_hour < 12)
        {
            if(shangwu == "下午")
            {
                end_hour = end_hour + 12;
            }
        }
        else
        {
            //            end_hour = end_hour+12;
        }
    }
    // 将结束时间字符串转换为 QDateTime 对象
    QDateTime endTimeObj = QDateTime(QDate::currentDate(), QTime(end_hour, end_minute), Qt::LocalTime);
    QDateTime currentTime = QDateTime::currentDateTime();
    //    QDateTime endTimeObj = QDateTime::fromString(endTime, "hh:mm");
    if(date == QDate::currentDate())
    {
        if (currentTime > endTimeObj)
        {
            wasSchedule = true;
        }
    }
    else if(date > QDate::currentDate())
    {
        wasSchedule = false;
    }
    if(wasSchedule)
    {
        // 遍历所有子控件，查找并置灰标签
        lbl_eventLabel->setStyleSheet("color: gray;"); // 将标签文字置灰
        lbl_dateTimeLabel->setStyleSheet("color: grey ;");
    }


}
void schedule_item::adjustDate(QDate date)
{
    qDebug() << "传进来的date是" << date << " QDate::currentDate()=" << QDate::currentDate();
    if(date < QDate::currentDate() )
    {
        wasSchedule = true;
    }
    else if (date > QDate::currentDate())
    {
        qDebug() << "false";
        wasSchedule = false;
    }
}
//修改超过显示区域用省略号表示，并只有显示省略号的内容才显示悬浮框
void schedule_item::setDescription(const QString &description)
{
    QFont font;
    //    font.setPointSize(14);
    QFontMetrics metrics(font);
    lbl_eventLabel->setFont(font);
    lbl_eventLabel->setFixedHeight(26);
    QString descriptionTrimmed = description.trimmed(); // 去除两端空白字符，确保准确比较长度
    QString elided = metrics.elidedText(descriptionTrimmed, Qt::ElideRight, 400);//400后用...省略

    lbl_eventLabel->setText(elided);

    const QByteArray i("org.ukui.style");
    QGSettings *fontSetting3 = new QGSettings(i, QByteArray(), this);
    QColor color1 = fontSetting3->get("themeColor").toString();
    color1 = CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED);

    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id))
    {
        styleGsettings = new QGSettings(style_id, QByteArray(), this);
        QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
        if (currentTheme == "ukui-dark" || currentTheme == "ukui-default")
        {
            styleSheet = QString("QLabel { }").arg(color1.name());
            if(wasSchedule)
            {
                styleSheet = QString("QLabel { color: gray; }").arg(color1.name());
            }
        }
        else
        {
            styleSheet = QString("QLabel { }").arg(color1.name());
            if(wasSchedule)
            {
                styleSheet = QString("QLabel { color: gray; }").arg(color1.name());
            }
        }
    }
    lbl_eventLabel->setStyleSheet(styleSheet);

    if (elided != descriptionTrimmed)
    {
        int maxToolTipWidth = 440;
        QString wrappedText;
        QString currentLine;

        for (const QChar &ch : descriptionTrimmed) {
            QString testLine = currentLine + ch;
            int textWidth = metrics.horizontalAdvance(testLine);

            // 如果当前行宽度超过限制，则插入换行符
            if (textWidth > maxToolTipWidth && !currentLine.isEmpty()) {
                wrappedText += currentLine + '\n';
                currentLine.clear();
            }
            currentLine += ch;
        }
        // 添加最后一行
        if (!currentLine.isEmpty()) {
            wrappedText += currentLine;
        }

        lbl_eventLabel->setToolTip(wrappedText);
    }
}

void schedule_item::mousePressEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        if(hasSchdule)
        {
            this_id = getId();
            qDebug() << "this_id" << this_id;
            mnu_menu = new QMenu(this);
            mnu_menu->setAccessibleName("kylin-calendar-plugin_QMenu_schedule_menu");
            mnu_menu->setAccessibleDescription("right menu of schedule");
            const QByteArray i("org.ukui.style");
            QGSettings *fontSetting3 = new QGSettings(i, QByteArray(), this);
            QColor color1 = fontSetting3->get("themeColor").toString();
            QPalette pal = palette();
            color1 = CalendarColor::CalendarColor::getThemeColor(CalendarColor::CLICKED);

            QString styleSheet;
            const QByteArray style_id(THEME_SCHAME);
            if (QGSettings::isSchemaInstalled(style_id))
            {
                styleGsettings = new QGSettings(style_id, QByteArray(), this);
                QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
                if (currentTheme == "ukui-dark" || currentTheme == "ukui-default")
                {
                    styleSheet = QString("QMenu {"
                                         "    background-color: #222222;"
                                         "    border: 1px solid #888888;" // 设置一个透明的边框，用于显示渐变效果
                                         "    border-radius: 10px;" // 设置边框圆角半径
                                         "}"
                                         "QMenu::item {"
                                         "    padding-left: 5px; "
                                         "    padding-right: 80px;"
                                         "    background-color: transparent;"
                                         "}"
                                         "QMenu::item:selected {"
                                         "    background-color: %1;"
                                         "    border-radius: 4px;"
                                         "    color: white;"
                                         "}").arg(color1.name());
                }
                else
                {
                    styleSheet = QString("QMenu {"
                                         "    background-color: white;"
                                         "    border: 1px solid #C0C0C0;" // 设置一个透明的边框，用于显示渐变效果
                                         "    border-radius: 10px;" // 设置边框圆角半径
                                         "}"
                                         "QMenu::item {"
                                         "    padding-left : 5px; "
                                         "    padding-right: 80px;"
                                         "    background-color: transparent;"
                                         "}"
                                         "QMenu::item:selected {"
                                         "    background-color: %1;"
                                         "    border-radius: 4px;"
                                         "    color: black;"
                                         "}").arg(color1.name());
                }
            }
            //            mnu_menu->setStyleSheet(styleSheet);

            // 添加“编辑日程”动作
            QAction *editAction = new QAction(tr("Edit"));
            mnu_menu->addAction(editAction);
            // 添加“删除日程”动作
            QAction *deleteAction =  new QAction(tr("Delete"));
            mnu_menu->addAction(deleteAction);
            // 显示菜单
            // 检查用户选择了哪个动作
            connect(editAction, &QAction::triggered, this, &schedule_item::slotEditSchedule);
            connect(deleteAction, &QAction::triggered, this, &schedule_item::slotDeleteSchedule);
            mnu_menu->exec(event->globalPos());
        }
        else if(!hasSchdule)
        {

        }
    }
}
void schedule_item::mouseDoubleClickEvent(QMouseEvent *event)
{
    QString id = getId();
    if(event->button() == Qt::LeftButton)
    {
        Q_EMIT ScheduleClick(id);
    }

}
void schedule_item::slotEditSchedule()
{
    Q_EMIT EditSchedule(this_id);
}
void schedule_item::slotDeleteSchedule()
{
    qDebug() << "传出来的" << this_id;
    Q_EMIT updateWindow(this_id);



}
QString schedule_item::getId()
{
    return id->text();
}
