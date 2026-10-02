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
#include <QStandardItemModel>
#include <QObject>
#include <QMessageBox>
#include <QPalette>
#include <QCalendarWidget>
#include <QHBoxLayout>
#include <QIcon>
#include <QString>
#include <QEventLoop>
#include <QFrame>
#include <QStringList>
#include <QPalette>
#include <QTimer>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <QApplication>
#include <QIntValidator>
#include <QTextBlock>
#include <unistd.h>
#include <QToolButton>
#include <cstdlib>
#include <QProcess>
#include <QTime>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QListView>
#include <QScrollBar>
#include <QMessageBox>
#include <QToolButton>
#include <KWindowSystem>
#include <QAbstractItemView>
#include <QRadioButton>
#include <QShortcut>
#include "custommessagebox.h"
#include "schedulewidget.h"
#include "kysdk/kysdk-system/libkydate.h"
#include "kysdk/applications/klunardateedit.h"
#include <libkydatacollect.h>
#include "calendardatabase.h"

const QString websiteTemplate = R"(<a href="%1">%2</a>)";
#define SHADOW_WIDTH 0    // 窗口阴影宽度;
#define TRIANGLE_WIDTH 15  // 小三角的宽度;
#define TRIANGLE_HEIGHT 10 // 小三角的高度;
#define BORDER_RADIUS 5    // 窗口边角的弧度;
#define THEME_SCHAME "org.ukui.style"
#define ICON_NAME "iconThemeName"
#define COLOR_THEME "styleName"
#define LUNAR_KEY                 "calendar"
#define FIRST_DAY_KEY "firstday"
#define PANEL_CONTROL_IN_CALENDAR "org.ukui.control-center.panel.plugins"
#define SDK2CPP(func) []()->QString { char* p = func(); QString str(p); free(p); return str; }

CSchceduleDlg::CSchceduleDlg(QWidget *parent)
    : KDialog(parent), m_startX(50), m_triangleWidth(TRIANGLE_WIDTH), m_triangleHeight(TRIANGLE_HEIGHT)
{
    m_type = 1;
    m_date = QDate::currentDate();
    initWindow();
    setAccessibleName("kylin-calendar-plugin_CSchduleDlg_schedule_dialog");
    setAccessibleDescription("this is dialog to operate the schedule create or edit");
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id))
    {
        styleGsettings = new QGSettings(style_id, QByteArray(), this);
        QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
        if(currentTheme == "ukui-default")
        {
            isDefault = true;
        }
        else if(currentTheme == "ukui-dark")
        {
            isDark = true;
        }
        connect(styleGsettings, &QGSettings::changed, [ = ](QString key)
        {
            if ("styleName" == key)
            {
                onPaletteChanged();
            }
            if(key == ICON_NAME)
            {
                this->setWindowIcon(QIcon::fromTheme("kylin-calendar", QIcon(":/png/png/calendar_32.png")));
            }
        });
    }
    if(isDefault)
    {
        QPalette pal ;
        const QByteArray style_id(THEME_SCHAME);
        if (QGSettings::isSchemaInstalled(style_id))
        {
            styleGsettings = new QGSettings(style_id);
            QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
            if(currentTheme == "ukui-default")
            {
                pal = lightPalette(this);

            }
        }
        pal.setColor(QPalette::Background, pal.background().color());
        pal.setColor(QPalette::Base, pal.base().color());
        //        pal.setColor(QPalette::Text, pal.text().color());
        pal.setColor(QPalette::Button, pal.button().color());
        pal.setColor(QPalette::ButtonText, pal.buttonText().color());
        setFramePalette(this, pal);
        bindConnect();
        setFocusPolicy(Qt::WheelFocus);
    }
    const QByteArray id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(id))
    {
        colorGsettings = new QGSettings(id, QByteArray(), this);
        connect(colorGsettings, &QGSettings::changed, this, [ = ](QString key)
        {
            if ("themeColor" == key)
            {
                onPaletteChanged();
            }
        });
    }

    const QByteArray calendar_id(PANEL_CONTROL_IN_CALENDAR);
    if(QGSettings::isSchemaInstalled(calendar_id))
    {
        calendar_gsettings = new QGSettings(calendar_id);
    }
    if(calendar_gsettings != nullptr)
    {
        //农历切换监听与日期显示格式
        connect(calendar_gsettings, &QGSettings::changed, this, [ = ] (const QString & key)
        {
            //后期可能在控制面板农历公历有用
            if(key == LUNAR_KEY)
            {
            }
            else if(key == "date")
            {
            }
            else if (key == FIRST_DAY_KEY)
            {
            }
        });
        //监听12/24小时制
        connect(calendar_gsettings, &QGSettings::changed, this, [ = ] (const QString & keys)
        {
            timemodel = calendar_gsettings->get("hoursystem").toString();
        });
        timemodel = calendar_gsettings->get("hoursystem").toString();

    }

    bindConnect();
    setFocusPolicy(Qt::WheelFocus);
    initTooltipStyle();
    m_showLunar = getShowLunar();
}

CSchceduleDlg::~CSchceduleDlg()
{
    if(calendar_gsettings)
    {
        delete calendar_gsettings;
        calendar_gsettings = nullptr;
    }
    if(colorGsettings)
    {
        delete colorGsettings;
        colorGsettings = nullptr;
    }
    if(styleGsettings)
    {
        delete styleGsettings;
        styleGsettings = nullptr;
    }
    qDebug() << "~CSchceduleDlg()";
}

void CSchceduleDlg::setProperty(int type, bool isAllDay)
{
    //1,新建日程 0.编辑日程
    m_type = type;
    m_createAllDay = isAllDay;

    //添加日历图标图片
    if(m_type == 0)
        this->setWindowTitle(tr("Edit Event"));
    else
        this->setWindowTitle(tr("New Event"));
}

void CSchceduleDlg::paintEvent(QPaintEvent *)
{
    //    QPainter painter(this);
    //    painter.setRenderHint(QPainter::Antialiasing, true);
    //    painter.setPen(Qt::NoPen);

    //    painter.setBrush(QColor(255, 255, 255));
    QPalette pal ;
    if(styleGsettings != nullptr)
    {
        QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
        if(currentTheme == "ukui-default" )
        {
            pal.setColor(QPalette::Background, Qt::white);
        }
        else if(currentTheme == "ukui-dark")
        {
            pal.setColor(QPalette::Background, QColor(29, 29, 29));
        }
        else if (currentTheme == "ukui-light")
        {
            pal.setColor(QPalette::Background, Qt::white);
        }
    }
    this->setPalette(pal);
}

void CSchceduleDlg::closeEvent(QCloseEvent *e)
{
    emit sigWindowClosed(); // 发出窗口关闭信号
    e->accept(); // 接受关闭事件
}

/**
 * @brief CSchceduleDlg::addRemindMe
 * 添加提醒时间
 */
void CSchceduleDlg::addRemindMe()
{
    cmb_rmindCombox->addItem(tr("No prompt"));
    cmb_rmindCombox->addItem(tr("Schedule Begin"));
    cmb_rmindCombox->addItem(tr("5 minutes in advance"));
    cmb_rmindCombox->addItem(tr("10 minutes in advance"));
    cmb_rmindCombox->addItem(tr("15 minutes in advance"));
    cmb_rmindCombox->addItem(tr("30 minutes in advance"));
    cmb_rmindCombox->addItem(tr("1 hour in advance"));
    cmb_rmindCombox->addItem(tr("1 day in advance"));
    cmb_rmindCombox->addItem(tr("2 day in advance"));
    setAlarmType(AlarmType::Alarm_None);
    CalendarDataBase::getInstance().setAlarmType("No prompt");//0
    connect(cmb_rmindCombox, SIGNAL(currentTextChanged(QString)), this, SLOT(handleComboBoxTextChanged(QString)));
}

//点击全天按钮后的提醒下拉框
void CSchceduleDlg::addRemindMeallday()
{
    cmb_rmindComboxallday->addItem(tr("1 day ago at 9am"));
    cmb_rmindComboxallday->addItem(tr("No prompt"));
    cmb_rmindComboxallday->addItem(tr("At 9am on the same day of the schedule"));
    cmb_rmindComboxallday->addItem(tr("2 days ago at 9am"));
    cmb_rmindComboxallday->addItem(tr("1 week ago at 9am"));
    setAlarmType(AlarmType::Alarm_1Day9_Front);
    CalendarDataBase::getInstance().setAlarmType("1 day ago at 9am");//9
    connect(cmb_rmindComboxallday, SIGNAL(currentTextChanged(QString)), this, SLOT(handleComboBoxTextChangedallday(QString)));
}

void CSchceduleDlg::handleComboBoxTextChanged(const QString &text)
{
    if(text == tr("No prompt"))
    {
        setAlarmType(Alarm_None);
        CalendarDataBase::getInstance().setAlarmType("No prompt");//0
    }
    else if(text == tr("Schedule Begin"))
    {
        setAlarmType(Alarm_Begin);
        CalendarDataBase::getInstance().setAlarmType("Schedule Begin");//1
    }
    else if(text == tr("5 minutes in advance"))
    {
        setAlarmType(Alarm_5Min_Front);
        CalendarDataBase::getInstance().setAlarmType("5 minutes in advance");//2
    }
    else if(text == tr("10 minutes in advance"))
    {
        setAlarmType(Alarm_10Min_Front);
        CalendarDataBase::getInstance().setAlarmType("10 minutes in advance");//3
    }
    else if(text == tr("15 minutes in advance"))
    {
        setAlarmType(Alarm_15Min_Front);
        CalendarDataBase::getInstance().setAlarmType("15 minutes in advance");//4
    }
    else if(text == tr("30 minutes in advance"))
    {
        setAlarmType(Alarm_30Min_Front);
        CalendarDataBase::getInstance().setAlarmType("30 minutes in advance");//5
    }
    else if(text == tr("1 hour in advance"))
    {
        setAlarmType(Alarm_1Hour_Front);
        CalendarDataBase::getInstance().setAlarmType("1 hour in advance");//6
    }
    else if(text == tr("1 day in advance"))
    {
        setAlarmType(Alarm_1Day_Front);
        CalendarDataBase::getInstance().setAlarmType("1 day in advance");//7;
    }
    else if(text == tr("2 day in advance"))
    {
        setAlarmType(Alarm_2Day_Front);
        CalendarDataBase::getInstance().setAlarmType("2 day in advance");//8
    }
}

void CSchceduleDlg::handleComboBoxTextChangedallday(const QString &text)
{

    if(text == tr("1 day ago at 9am"))
    {
        setAlarmType(Alarm_1Day9_Front);
        CalendarDataBase::getInstance().setAlarmType("1 day ago at 9am");//9
    }
    else if(text == tr("No prompt"))
    {
        setAlarmType(Alarm_None);
        CalendarDataBase::getInstance().setAlarmType("No prompt");//0
    }
    else if(text == tr("At 9am on the same day of the schedule"))
    {
        setAlarmType(Alarm_Begin9_Front);
        CalendarDataBase::getInstance().setAlarmType("At 9am on the same day of the schedule");//10
    }
    else if(text == tr("2 days ago at 9am"))
    {
        setAlarmType(Alarm_2Day9_Front);
        CalendarDataBase::getInstance().setAlarmType("2 days ago at 9am");//11
    }
    else if(text == tr("1 week ago at 9am"))
    {
        setAlarmType(Alarm_1Week9_Front);
        CalendarDataBase::getInstance().setAlarmType("1 week ago at 9am");//12
    }
}

/**
 * @brief CSchceduleDlg::setStartTimeMinute
 * @param startTime
 * 设置开始时间的分钟
 */
void CSchceduleDlg::setStartTimeMinute(const QString startTime)
{
    QTime time = QTime::fromString(startTime, "h:mm");
    QTime end = time.addSecs(1800);
    m_endHour = end.hour();//设置默认
    m_endMinute = end.minute();//设置默认

    m_beginMinute = time.minute();
}
/**
 * @brief CSchceduleDlg::setStartTimeHour
 * @param startTime
 * 设置开始时间的小时
 */
void  CSchceduleDlg::setStartTimeHour(const QString startTime)
{
    QTime time = QTime::fromString(startTime, "h:mm");
    m_beginHour = time.hour();
}
void CSchceduleDlg::setStartTime(const QString &startTime)
{

    m_startTime = QTime::fromString(startTime, "hh:mm");
    setStartTimeHour(startTime);
    setStartTimeMinute(startTime);
}
/**
 * @brief CSchceduleDlg::setEndTimeHour
 * @param startTime
 * 设置结束时间的小时
 */
void CSchceduleDlg::setEndTimeHour(const QString endTime)
{
    QTime time = QTime::fromString(endTime, "h:mm");
    m_endHour = time.hour();
}
/**
 * @brief CSchceduleDlg::setEndTimeMinute
 * @param startTime
 * 设置结束时间的分钟
 */
void CSchceduleDlg::setEndTimeMinute(const QString endTime)
{
    QTime time = QTime::fromString(endTime, "h:mm");
    m_endMinute = time.minute();
}
void CSchceduleDlg::setEndTime(const QString &endTime)
{

    m_endTime = QTime::fromString(endTime, "hh:mm");
    setEndTimeHour(endTime);
    setEndTimeMinute(endTime);
}

//检查手动输入结束时间，结束时间小于开始时间弹出警告框，点击确认后结束时间恢复为比开始时间晚半小时的
void CSchceduleDlg::updateEndTimeValidation()
{
    QString beginTimeStr = cmb_beginTimeEdit->currentText();
    QString endTimeStr = cmb_endTimeEdit->currentText();
    QTime userEndTime = QTime::fromString(endTimeStr, "hh:mm");
    QTime selectedStartTime = QTime::fromString(beginTimeStr, "hh:mm");
    QTime halfAnHourLater = selectedStartTime.addSecs(30 * 60);
    QDateTime beginDateTime = m_pBeginDateEdit->getDateEdit()->dateTime();
    QDateTime endDateTime = m_pEndDateEdit->getDateEdit()->dateTime();
    // 检查结束时间是否早于开始时间
    if(beginDateTime.date() < endDateTime.date())
    {
    }
    else
    {
        if(tanchuang == false)
        {
            tanchuang = true;
            if (endDateTime == beginDateTime && userEndTime.isValid() && userEndTime < m_startTime)
            {
                CustomMessageBox *msgBox = new CustomMessageBox(0, this);
                connect(msgBox, &CustomMessageBox::yesClicked, [&]()
                {
                    msgBox->accept();
                });
                connect(msgBox, &CustomMessageBox::noClicked, [&]()
                {
                    msgBox->accept();
                });
                QString defaultEndTimeStr = halfAnHourLater.toString("hh:mm");
                cmb_endTimeEdit->setCurrentText(defaultEndTimeStr);
                msgBox->exec();

            }
        }
        else
        {
            tanchuang = false;
            return ;
        }
    }

    // 当开始时间发生变化时触发
    QObject::connect(cmb_beginTimeEdit, QOverload<const QString &>::of(&QComboBox::currentTextChanged), [ = ]()
    {
        isStartTimeChanged = true; // 标记开始时间已改变
        m_startTime = QTime::fromString(cmb_beginTimeEdit->lineEdit()->text(), "hh:mm");
        QTime halfAnHourLater = m_startTime.addSecs(30 * 60);
        QString defaultEndTimeStr = halfAnHourLater.toString("hh:mm");
        cmb_endTimeEdit->setCurrentText(defaultEndTimeStr);
    });

    // 当结束时间从下拉框中选择时触发
    connect(cmb_endTimeEdit, QOverload<int>::of(&QComboBox::activated), [ = ](int)
    {
        updateEndTimeValidation();
    });
    connect(cmb_endTimeEdit->lineEdit(), &QLineEdit::editingFinished, [ = ]()
    {
        if (isStartTimeChanged == true && tanchuang == false)   // 检查开始时间是否刚被改变
        {
            updateEndTimeValidation();
        }
    });
}

/**
 * @brief CSchceduleDlg::updateEndTime
 * @param startTime
 * 当选择完开始时间，结束时间要始终比开始时间晚半小时，结束时间下拉框更新为只显示开始时间之后的时间
 */
void CSchceduleDlg::updateEndTime(const QString &startTime)
{
    m_startTime = QTime::fromString(startTime, "hh:mm");
    setStartTimeMinute(startTime);
    setStartTimeHour(startTime);

    QTime selectedStartTime = QTime::fromString(startTime, "hh:mm");
    QTime halfAnHourLater = selectedStartTime.addSecs(1800); // 30分钟
    QDateTime beginDateTime = m_pBeginDateEdit->getDateEdit()->dateTime();
    QDateTime endDateTime = m_pEndDateEdit->getDateEdit()->dateTime();

    cmb_endTimeEdit->clear();

    // 判断结束日期是否大于开始日期
    if (endDateTime.date() > beginDateTime.date()) {
        // 结束日期 > 开始日期：添加全天选项（每半小时）
        for (int hour = 0; hour <= 23; ++hour) {
            for (int minute : {0, 30}) { // 只添加00分和30分
                QString timeStr = QStringLiteral("%1:%2")
                        .arg(hour, 2, 10, QLatin1Char('0'))
                        .arg(minute, 2, 10, QLatin1Char('0'));
                cmb_endTimeEdit->addItem(timeStr);
            }
        }
    } else {
        // 结束日期 = 开始日期：只添加开始时间之后的选项（每半小时）
        for (int hour = selectedStartTime.hour(); hour <= 23; ++hour) {
            // 确定起始分钟
            int startMinute = (hour == selectedStartTime.hour())
                    ? selectedStartTime.minute()
                    : 0;

            // 添加符合条件的选项
            for (int minute : {0, 30}) {
                if (hour == selectedStartTime.hour() && minute < startMinute)
                    continue;

                QString timeStr = QStringLiteral("%1:%2")
                        .arg(hour, 2, 10, QLatin1Char('0'))
                        .arg(minute, 2, 10, QLatin1Char('0'));
                cmb_endTimeEdit->addItem(timeStr);
            }
        }
    }

    // 设置结束时间下拉框的当前选择为比开始时间晚半小时的时间（如果有）
    QString endTimeStr = halfAnHourLater.toString("hh:mm");
    int index = cmb_endTimeEdit->findText(endTimeStr);
    if (index != -1) {
        cmb_endTimeEdit->setCurrentIndex(index);
    }

    //更改开始时间后，输入结束时间，点击回车保存时确认合法性
    QObject::connect(cmb_endTimeEdit, QOverload<const QString &>::of(&QComboBox::activated), [ = ]()
    {
        updateEndTimeValidation();
    });

    connect(cmb_endTimeEdit, &QComboBox::currentTextChanged, this, &CSchceduleDlg::setEndTime);
}

void CSchceduleDlg::validateEndTimeFormat1()
{
    const auto inputText = cmb_beginTimeEdit->currentText();
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString currentEndTimeText = currentDateTime.time().toString("HH:mm"); // 只获取当前时间部分

    if(tanchuang == false)
    {
        tanchuang = true;
        //        if (!inputText.isEmpty()) {
        QTime time = QTime::fromString(inputText, "HH:mm");
        if (!time.isValid() || time.hour() > 23 || time.minute() > 59)
        {
            CustomMessageBox *msgBox = new CustomMessageBox(2, this);
            connect(msgBox, &CustomMessageBox::yesClicked, [&]()
            {
                msgBox->accept();
            });
            connect(msgBox, &CustomMessageBox::noClicked, [&]()
            {
                msgBox->accept();
            });
            tanchuang = true;
            cmb_beginTimeEdit->clearEditText();
            cmb_beginTimeEdit->setFocus();

            // 设置为当前时间
            cmb_beginTimeEdit->addItem(currentEndTimeText);
            cmb_beginTimeEdit->setCurrentText(currentEndTimeText); // 设置为当前选中项
            QString beginTimeStr = cmb_beginTimeEdit->currentText();
            QTime selectedStartTime = QTime::fromString(beginTimeStr, "hh:mm");
            QTime halfAnHourLater = selectedStartTime.addSecs(30 * 60);
            QString defaultEndTimeStr = halfAnHourLater.toString("hh:mm");
            cmb_endTimeEdit->setCurrentText(defaultEndTimeStr);
            msgBox->exec();
            //                QMessageBox::warning(nullptr, tr("Warning"), tr("The time entered is not in the correct format, please make sure that the hours are between 0-23 and the minutes are between 0-59."));

        }
        //        }else{
        //            tanchuang = false;
        //            return;
        //        }
    }
    tanchuang = false;
}

void CSchceduleDlg::validateEndTimeFormat2()
{
    const auto inputText = cmb_endTimeEdit->currentText();
    if(tanchuang == false)
    {
        tanchuang = true;
        //        if (!inputText.isEmpty()) {
        QTime time = QTime::fromString(inputText, "HH:mm");
        if (!time.isValid() || time.hour() > 23 || time.minute() > 59)
        {
            CustomMessageBox *msgBox = new CustomMessageBox(2, this);
            connect(msgBox, &CustomMessageBox::yesClicked, [&]()
            {
                msgBox->accept();
            });
            connect(msgBox, &CustomMessageBox::noClicked, [&]()
            {
                msgBox->accept();
            });
            cmb_endTimeEdit->clearEditText();
            cmb_endTimeEdit->setFocus();
            tanchuang = true;
            // 设置为当前时间
            QString beginTimeStr = cmb_beginTimeEdit->currentText();
            QTime selectedStartTime = QTime::fromString(beginTimeStr, "hh:mm");
            QTime halfAnHourLater = selectedStartTime.addSecs(30 * 60);
            QString defaultEndTimeStr = halfAnHourLater.toString("hh:mm");
            cmb_endTimeEdit->setCurrentText(defaultEndTimeStr);
            tanchuang = false;
            msgBox->exec();
            //                QMessageBox::warning(nullptr, tr("Warning"), tr("The time entered is not in the correct format, please make sure that the hours are between 0-23 and the minutes are between 0-59."));

        }

    }
    tanchuang = false;
}

void CSchceduleDlg::initWindow()
{
    this->setModal(false);
    this->setWindowIcon(QIcon::fromTheme("kylin-calendar", QIcon(":/png/png/calendar_32.png")));
    this->setWindowFlag(Qt::WindowMinimizeButtonHint, false);
    this->setFixedWidth(450);
    this->setMaximumHeight(0);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    this->setWindowTitle(tr("New Event"));
    QPushButton *btn = closeButton();
    btn->setToolTip(tr("Close"));
    m_pMainWidget = new QWidget(this);
    m_pMainWidget->setAccessibleName("kylin-calendar-plugin_CSchduleDlg_main_widget");
    m_pMainWidget->setAccessibleDescription("the text edit in schedule dialog");

    m_pTextContentlayout = new QHBoxLayout();
    m_pTextContentlayout->setContentsMargins(0, 0, 0, 0);
    m_textEdit = new TextEdit(this);
    m_textEdit->createStandardContextMenu()->setAccessibleName("kylin-calendar-plugin_QTextEdit_text_edit_menu");
    m_textEdit->viewport()->setAccessibleName("kylin-calendar-plugin_QTextEdit_text_edit_viewport");
    int index = 1;
    QList<QWidget *>children = m_textEdit->findChildren<QWidget *>();
    for(QWidget *child : children)
    {
        child->setAccessibleName(QString("kylin-calendar-plugin_QTextEdit_text_edit_children_%1").arg(index));
        index ++;
    }
    m_textEdit->horizontalScrollBar()->setAccessibleName("kylin-calendar-plugin_QTextEdit_text_edit_horizontalScrollBar");
    m_textEdit->verticalScrollBar()->setAccessibleName("kylin-calendar-plugin_QTextEdit_text_edit_verticalScrollBar");
    m_textEdit->setAccessibleName("kylin-calendar-plugin_QTextEdit_text_edit");
    m_textEdit->setAccessibleDescription("the text edit in schedule dialog");
    m_textEdit->setFixedHeight(142);
    m_textEdit->setAcceptRichText(false);
    m_textEdit->setWordWrapMode(QTextOption::WrapAnywhere);
    //设置事件过滤器
    m_textEdit->installEventFilter(this);
    m_pTextContentlayout->addWidget(m_textEdit);
    // 如果是占位符文本
    m_textEdit->setPlaceholderText(tr("Enter the content of the event"));
    // 连接textChanged信号和槽函数
    connect(m_textEdit, &QTextEdit::textChanged, this, &CSchceduleDlg::slotTextChanged, Qt::QueuedConnection );

    //添加全天和农历选择框
    lbl_allDayLabellayout = new QHBoxLayout();
    lbl_allDayLabellayout->setSpacing(0);
    lbl_allDayLabellayout->setMargin(0);
    lbl_allDayLabel = new QLabel();
    lbl_allDayLabel->setAccessibleName("kylin-calendar-plugin_QLabel_allday_label");
    lbl_allDayLabel->setAccessibleDescription("allday label of schedule dialog");
    lbl_allDayLabel->setText(tr("All Day"));
    qDebug() << "lbl_allDayLabel font point size:" << lbl_allDayLabel->font().pointSize();
    qDebug() << "lbl_allDayLabel font pixel size:" << lbl_allDayLabel->font().pixelSize();
    lbl_allDayLabel->setFixedSize(65, 25);

    chk_allDaycheckBox = new QCheckBox(this);
    chk_allDaycheckBox->setAccessibleName("kylin-calendar-plugin_QCheckBox_allday_checkbox");
    chk_allDaycheckBox->setAccessibleDescription("allday checkbox of schedule dialog");
    chk_allDaycheckBox->setFixedSize(18, 18);
    chk_allDaycheckBox->installEventFilter(this);
    m_lunarLabel = new QLabel();
    m_lunarLabel->setText(tr("Lunar"));
    m_lunarLabel->setFixedSize(65, 25);
    m_lunarcheckBox = new QCheckBox(this);
    m_lunarcheckBox->setAccessibleName("kylin-calendar-plugin_QCheckBox_lunarBox");
    m_lunarcheckBox->setFixedSize(18, 18);
    lbl_allDayLabellayout->addWidget(chk_allDaycheckBox);
    lbl_allDayLabellayout->addSpacing(10);
    lbl_allDayLabellayout->addWidget(lbl_allDayLabel);
    lbl_allDayLabellayout->addWidget(m_lunarcheckBox);
    lbl_allDayLabellayout->addSpacing(10);
    lbl_allDayLabellayout->addWidget(m_lunarLabel);
    lbl_allDayLabellayout->setContentsMargins(102, 0, 0, 0);
    lbl_allDayLabellayout->addStretch();

    m_pBeginTimelayout = new QHBoxLayout();
    m_pBeginTimelayout->setSpacing(0);
    m_pBeginTimelayout->setMargin(0);
    lbl_beginTimeLabel = new QLabel(tr("Starts:"));
    lbl_beginTimeLabel->setToolTip(tr("Starts:"));
    lbl_beginTimeLabel->setAccessibleName("kylin-calendar-plugin_QLabel_begin_time_label");
    lbl_beginTimeLabel->setAccessibleDescription("begintime label of schedule dialog");
    lbl_beginTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lbl_beginTimeLabel->setFixedWidth(75);

    m_pBeginDateEdit = new KLunarDateEdit(this);
    m_pBeginDateEdit->setAccessibleName("kylin-calendar-plugin_QDateTimeEdit_begin_date_edit");
    m_pBeginDateEdit->setAccessibleDescription("begin date chocie of schedule dialog");
    m_pBeginDateEdit->setContextMenuPolicy(Qt::NoContextMenu);

    m_pEndDateEdit = new KLunarDateEdit(this);
    m_pEndDateEdit->setAccessibleName("kylin-calendar-plugin_QDateTimeEdit_end_date_edit");
    m_pEndDateEdit->setAccessibleDescription("end date chocie of schedule dialog");
    m_pEndDateEdit->setContextMenuPolicy(Qt::NoContextMenu);
    connect(m_pEndDateEdit->getDateEdit(), &QDateTimeEdit::dateTimeChanged, this, &CSchceduleDlg::updateEndTimeValidation);
    m_pBeginDateEdit->setFixedSize(153, 36);
    m_pBeginDateEdit->getDateEdit()->setCalendarPopup(true);
    m_pBeginDateEdit->getDateEdit()->setDisplayFormat("yyyy-MM-dd");
    // 假设你已经有了QDateEdit的指针，名为dateEdit
    QDateTime currentDateTime = QDateTime::currentDateTime();
    // 设置最小日期时间
    m_pBeginDateEdit->getDateEdit()->setMinimumDateTime(currentDateTime);

    // 设置日期时间范围，不允许选择早于当前的时间
    m_pBeginDateEdit->getDateEdit()->setDateTimeRange(currentDateTime, QDateTime::currentDateTime().addDays(365)); // 或者使用QDateTime::max()来允许选择任意未来时间

    // 更新界面
    m_pBeginDateEdit->update();


    //解决开始时间显示不全问题
    m_pBeginDateEdit->getDateEdit()->setDate(m_date);
    //重新绘制time选择下拉框
    cmb_beginTimeEdit = new QComboBox(this);
    cmb_beginTimeEdit->setAccessibleName("kylin-calendar-plugin_QComboBox_begin_time_edit");
    cmb_beginTimeEdit->view()->setAccessibleName("kylin-calendar-plugin_QComboBox_begin_time_edit_view");
    cmb_beginTimeEdit->setAccessibleDescription("begin time to choice in schedule dialog");
    cmb_beginTimeEdit->setContextMenuPolicy(Qt::NoContextMenu);
    cmb_beginTimeEdit->installEventFilter(this);
    cmb_beginTimeEdit->setFixedSize(125, 36);
    cmb_beginTimeEdit->setEditable(true);


    //重新绘制time选择下拉框
    cmb_endTimeEdit = new QComboBox(this);
    cmb_endTimeEdit->setAccessibleName("kylin-calendar-plugin_QComboBox_end_time_edit");
    cmb_endTimeEdit->view()->setAccessibleName("kylin-calendar-plugin_QComboBox_end_time_edit_view");
    cmb_endTimeEdit->setAccessibleDescription("end time to choice in schedule dialog");
    cmb_endTimeEdit->setContextMenuPolicy(Qt::NoContextMenu);

    cmb_endTimeEdit->setFixedSize(125, 36);
    cmb_endTimeEdit->setEditable(true);

    connect(cmb_beginTimeEdit, &QComboBox::customContextMenuRequested, this, &CSchceduleDlg::handleComboBoxDoubleClick);
    connect(cmb_endTimeEdit, &QComboBox::customContextMenuRequested, this, &CSchceduleDlg::handleComboBoxDoubleClick);
    addPeriodOfTime();

    m_pBeginTimelayout->addWidget(lbl_beginTimeLabel);
    m_pBeginTimelayout->addSpacing(28);
    m_pBeginTimelayout->addWidget(m_pBeginDateEdit);
    m_pBeginTimelayout->addSpacing(11);
    m_pBeginTimelayout->addWidget(cmb_beginTimeEdit);

    m_pEndTimelayout = new QHBoxLayout();
    m_pEndTimelayout->setSpacing(0);
    m_pEndTimelayout->setMargin(0);
    lbl_endTimeLabel = new QLabel(tr("Ends:"));
    lbl_endTimeLabel->setToolTip(tr("Ends:"));
    lbl_endTimeLabel->setAccessibleName("kylin-calendar-plugin_QLabel_end_time_label");
    lbl_endTimeLabel->setAccessibleDescription("end time label of schedule dialog");
    lbl_endTimeLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lbl_endTimeLabel->setFixedWidth(75);


    m_pEndDateEdit->setFixedSize(153, 36);
    m_pEndDateEdit->getDateEdit()->setCalendarPopup(true);
    m_pEndDateEdit->getDateEdit()->setDisplayFormat("yyyy-MM-dd");

    //解决开始时间显示不全问题
    m_pEndDateEdit->getDateEdit()->setDate(m_date);
    m_pBeginDateEdit->getDateEdit()->setDate(m_date);
    QDateTime newStartTime = m_pBeginDateEdit->getDateEdit()->dateTime();
    // 保持结束时间的小时、分钟和秒不变，仅更新日期部分
    QDateTime newEndTime = QDateTime(newStartTime.date(), m_pEndDateEdit->getDateEdit()->time());
    m_pEndDateEdit->getDateEdit()->setDateTime(newEndTime);
    m_pEndDateEdit->getDateEdit()->setMinimumDateTime(m_pBeginDateEdit->getDateEdit()->dateTime());

    // 连接开始时间更改信号到槽函数,保证结束时间自动更新为和开始时间一样，并不能早于开始时间
    QObject::connect(m_pBeginDateEdit->getDateEdit(), &QDateTimeEdit::dateTimeChanged, [ = ]()
    {
        QDateTime newStartTime = m_pBeginDateEdit->getDateEdit()->dateTime();
        // 保持结束时间的小时、分钟和秒不变，仅更新日期部分
        QDateTime newEndTime = QDateTime(newStartTime.date(), m_pEndDateEdit->getDateEdit()->time());
        m_pEndDateEdit->getDateEdit()->setDateTime(newEndTime);
        m_pEndDateEdit->getDateEdit()->setMinimumDateTime(m_pBeginDateEdit->getDateEdit()->dateTime());
    });

    m_pEndTimelayout->addWidget(lbl_endTimeLabel);
    m_pEndTimelayout->addSpacing(28);
    m_pEndTimelayout->addWidget(m_pEndDateEdit);
    m_pEndTimelayout->addSpacing(11);
    m_pEndTimelayout->addWidget(cmb_endTimeEdit);

    rminWidget = new QWidget(this);
    rminWidget->setFixedHeight(36);
    rminWidget->setAccessibleName("kylin-calendar-plugin_QDialog_rminWidget");
    QHBoxLayout *rminQLabellayout = new QHBoxLayout(rminWidget);
    rminQLabellayout->setSpacing(0);
    rminQLabellayout->setMargin(0);
    lbl_remindSetLabel = new QLabel(rminWidget);
    lbl_remindSetLabel->setAccessibleName("kylin-calendar-plugin_QLabel_remind_label");
    lbl_remindSetLabel->setAccessibleDescription("remind label in scheudle dialog");
    lbl_remindSetLabel->setText(tr("Remind Me:"));
    lbl_remindSetLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lbl_remindSetLabel->setFixedWidth(75);

    cmb_rmindCombox = new QComboBox(rminWidget);
    cmb_rmindCombox->setAccessibleName("kylin-calendar-plugin_QComboBox_remind_comboBox");
    cmb_rmindCombox->view()->setAccessibleName("kylin-calendar-plugin_QComboBox_remind_comboBox_view");
    cmb_rmindCombox->setAccessibleDescription("remind choice in schedule dialog");
    cmb_rmindCombox->setFixedHeight(36);
    cmb_rmindCombox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    cmb_rmindComboxallday = new QComboBox(rminWidget);
    cmb_rmindComboxallday->setAccessibleName("kylin-calendar-plugin_QComboBox_remind_comboBox_allday");
    cmb_rmindComboxallday->view()->setAccessibleName("kylin-calendar-plugin_QComboBox_remind_comboBox_allday_view");
    cmb_rmindComboxallday->setAccessibleDescription("remind choice of allday in schedule dialog");
    cmb_rmindComboxallday->setFixedHeight(36);
    cmb_rmindComboxallday->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    rminQLabellayout->addWidget(lbl_remindSetLabel);
    rminQLabellayout->setSpacing(28);
    rminQLabellayout->addWidget(cmb_rmindCombox);
    rminQLabellayout->addWidget(cmb_rmindComboxallday); // 注意：这里也需要添加cmb_rmindComboxallday，但设置为不可见

    cmb_rmindComboxallday->setVisible(false);
    cmb_rmindCombox->setVisible(true);
    addRemindMe();

    connect(chk_allDaycheckBox, &QCheckBox::stateChanged, [ = ](int state)
    {
        if (state == Qt::Checked)
        {
            cmb_rmindComboxallday->clear();
            cmb_rmindCombox->setVisible(false);
            cmb_rmindComboxallday->setVisible(true);
            addRemindMeallday();
            //            addPeriodOfTime();
        }
        else
        {
            cmb_rmindCombox->clear();
            cmb_rmindComboxallday->setVisible(false);
            cmb_rmindCombox->setVisible(true);
            addRemindMe();
            //            addPeriodOfTime();
        }
    });

    connect(m_lunarcheckBox, &QCheckBox::stateChanged, [ = ](int state)
    {
        if(state == Qt::Checked)
        {
            m_pBeginDateEdit->setShowLunar(true);
            m_pEndDateEdit->setShowLunar(true);
            m_pFrequencyTimeEdit->setShowLunar(true);
        }
        else
        {
            m_pBeginDateEdit->setShowLunar(false);
            m_pEndDateEdit->setShowLunar(false);
            m_pFrequencyTimeEdit->setShowLunar(false);
        }
    });

    repeatWidget = new QWidget(this);
    repeatWidget->setFixedHeight(36);
    repeatWidget->setAccessibleName("kylin-calendar-plugin_QDialog_repeatWidget");
    QHBoxLayout *repeatLabellayout = new QHBoxLayout(repeatWidget);
    repeatLabellayout->setSpacing(0);
    repeatLabellayout->setMargin(0);
    lbl_beginrepeatLabel = new QLabel(tr("Repeat:"), repeatWidget);
    lbl_beginrepeatLabel->setAccessibleName("kylin-calendar-plugin_QLabel_beginrepeat_label");
    lbl_beginrepeatLabel->setAccessibleDescription("begin repeat label in schedule dialog");
    lbl_beginrepeatLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lbl_beginrepeatLabel->setFixedWidth(75);

    cmb_repeatCombox = new QComboBox(repeatWidget);
    cmb_repeatCombox->setAccessibleName("kylin-calendar-plugin_QComboBox_repeat_comboBox");
    cmb_repeatCombox->view()->setAccessibleName("kylin-calendar-plugin_QComboBox_repeat_comboBox_view");
    cmb_repeatCombox->setAccessibleDescription("repeat choice of allday in schedule dialog");
    cmb_repeatCombox->setFixedHeight(36);
    cmb_repeatCombox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    cmb_repeatCombox->addItem(tr("No Repeat"));
    cmb_repeatCombox->addItem(tr("Daily"));
    cmb_repeatCombox->addItem(tr("Weekly"));
    cmb_repeatCombox->addItem(tr("Monthly"));
    cmb_repeatCombox->addItem(tr("Yearly"));
    setRRuleType(RRuleType::RRule_None);
    CalendarDataBase::getInstance().setRRuleType("No Repeat");
    connect(cmb_repeatCombox, &QComboBox::currentTextChanged, this, &CSchceduleDlg::slotfrequencyBox);

    repeatLabellayout->addWidget(lbl_beginrepeatLabel);
    repeatLabellayout->setSpacing(28);
    repeatLabellayout->addWidget(cmb_repeatCombox);

    frequencyWidget = new QWidget(this);
    frequencyWidget->setAccessibleName("kylin-calendar-plugin_QDialog_frequencyWidget");
    frequencyLayout = new QHBoxLayout(frequencyWidget);
    frequencyLayout->setContentsMargins(0,0,0,0);
    lbl_frequencyLabel = new QLabel(frequencyWidget);
    lbl_frequencyLabel->setAccessibleName("kylin-calendar-plugin_QLabel_frequency_label");
    lbl_frequencyLabel->setAccessibleDescription("frequency label in schedule dialog");
    cmb_frequencyBox = new QComboBox(frequencyWidget);
    cmb_frequencyBox->setFixedSize(290,36);
    cmb_frequencyBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    cmb_frequencyBox->setAccessibleName("kylin-calendar-plugin_QComboBox_frequency_comboBox");
    cmb_frequencyBox->view()->setAccessibleName("kylin-calendar-plugin_QComboBox_frequency_comboBox_view");
    cmb_frequencyBox->setAccessibleDescription("frequency label in schedule dialog");
    cmb_frequencyBox->setDisabled(false);
    cmb_frequencyBox->addItem(tr("Infinite repetition"));
    cmb_frequencyBox->addItem(tr("Customize"));

    m_pFrequencyTimeEdit = new KLunarDateEdit(frequencyWidget);
    m_pFrequencyTimeEdit->setAccessibleName("kylin-calendar-plugin_QDateTimeEdit_frequency_dateTime_edit");
    m_pFrequencyTimeEdit->setAccessibleDescription("frequency label in schedule dialog");
    m_pFrequencyTimeEdit->setContextMenuPolicy(Qt::NoContextMenu);
    m_pFrequencyTimeEdit->setFixedSize(143, 36);
    m_pFrequencyTimeEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_pFrequencyTimeEdit->getDateEdit()->setCalendarPopup(true);
    m_pFrequencyTimeEdit->getDateEdit()->setDisplayFormat("yyyy-MM-dd");
    // 初始化完成后立即更新结束时间
    updateEndDateFromBeginDate();
    // 然后再连接开始时间控件的dateTimeChanged信号
    QObject::connect(m_pBeginDateEdit->getDateEdit(), &QDateTimeEdit::dateTimeChanged, this, [ = ]()
    {
        updateEndDateFromBeginDate();
    });

    //解决日历下拉框多语言适配
    const auto locale = QLocale::system();
    if (locale.name().startsWith(QLatin1String("zh")))
    {
        m_pBeginDateEdit->setLocale(QLocale(QLocale::Chinese));
        m_pEndDateEdit->setLocale(QLocale(QLocale::Chinese));
        m_pFrequencyTimeEdit->setLocale(QLocale(QLocale::Chinese));
    }
    else if(locale.name().startsWith(QLatin1String("en")))
    {
        m_pBeginDateEdit->setLocale(QLocale(QLocale::English));
        m_pEndDateEdit->setLocale(QLocale(QLocale::English));
        m_pFrequencyTimeEdit->setLocale(QLocale(QLocale::English));
    }
    else if(locale.name().startsWith(QLatin1String("bo")))
    {
        m_pBeginDateEdit->setLocale(QLocale(QLocale::Tibetan));
        m_pEndDateEdit->setLocale(QLocale(QLocale::Tibetan));
        m_pFrequencyTimeEdit->setLocale(QLocale(QLocale::Tibetan));
    }
    else if(locale.name().startsWith(QLatin1String("mn")))
    {
        m_pBeginDateEdit->setLocale(QLocale(QLocale::Mongolian));
        m_pEndDateEdit->setLocale(QLocale(QLocale::Mongolian));
        m_pFrequencyTimeEdit->setLocale(QLocale(QLocale::Mongolian));
    }

    cmb_frequencyBox->setVisible(false);  //默认不显示
    lbl_frequencyLabel->setVisible(false);  //默认不显示
    m_pFrequencyTimeEdit->setVisible(false);  //默认不显示

    lbl_frequencyLabel->setText(tr("Frequency:"));

    frequencyLayout->addWidget(lbl_frequencyLabel);
    frequencyLayout->addWidget(cmb_frequencyBox);
    frequencyLayout->addWidget(m_pFrequencyTimeEdit);

    buttonWidget = new QWidget(this);
    buttonWidget->setAccessibleName("kylin-calendar-plugin_QDialog_ButtonWIdget");
    buttonLayout = new QHBoxLayout(buttonWidget);

    //调整确认取消按钮布局排列及按钮三态
    btn_queding = new QPushButton(tr("Save"), buttonWidget);
    btn_queding->setAccessibleName("kylin-calendar-plugin_QPushButton_comfirm_btn");
    btn_queding->setAccessibleDescription("button of ok");
    btn_queding->setFixedSize(108, 36);
    connect(btn_queding, &QPushButton::clicked, this, &CSchceduleDlg::slotConfirmClick);

    btn_quxiao = new QPushButton(tr("Cancel"), buttonWidget);
    btn_quxiao->setAccessibleName("kylin-calendar-plugin_QPushButton_cancel_btn");
    btn_quxiao->setAccessibleDescription("button of cancel");
    btn_quxiao->setFixedSize(108, 36);
    connect(btn_quxiao, &QPushButton::clicked, [&]()
    {
        resetToDefault();
        this->close();
    });

    buttonLayout->setContentsMargins(0, 0, 0, 0); // 清除左右两侧的默认内边距
    buttonLayout->setAlignment(Qt::AlignRight); // 设置整体布局右对齐
    // 让按钮之间有18像素间距
    QSpacerItem *spacer = new QSpacerItem(18, 0, QSizePolicy::Fixed, QSizePolicy::Ignored);
    buttonLayout->addItem(spacer);
    buttonLayout->addWidget(btn_quxiao);
    buttonLayout->addWidget(btn_queding);

    vlayout = new QVBoxLayout();
    vlayout->setSpacing(8);
    vlayout->addLayout(m_pTextContentlayout);
    vlayout->addSpacing(7);
    vlayout->addLayout(lbl_allDayLabellayout);
    vlayout->addSpacing(7);
    vlayout->addLayout(m_pBeginTimelayout);
    vlayout->addLayout(m_pEndTimelayout);
    vlayout->addWidget(rminWidget);
    vlayout->addWidget(repeatWidget);
    vlayout->addWidget(frequencyWidget);
    vlayout->addSpacing(24);
    vlayout->addWidget(buttonWidget);
    //设置重复时，才显示结束重复
    connect(cmb_repeatCombox, qOverload<const QString &>(&QComboBox::currentTextChanged),
            this, &CSchceduleDlg::onRepeatComboxTextChanged);
    connect(cmb_frequencyBox, qOverload<const QString &>(&QComboBox::currentTextChanged),
            this, &CSchceduleDlg::onfrequencyComboxTextChanged);

    m_pMainWidget->setLayout(vlayout);
    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->setContentsMargins(22,20,22,25);
    hlayout->addWidget(m_pMainWidget);
    this->KDialog::mainWidget()->setLayout(hlayout);
    this->adjustSize();
    setFormat(getFormat());
}

//设置自定义结束重复日期始终是开始日期的后一天
void CSchceduleDlg::updateEndDateFromBeginDate()
{
    // 同样获取开始时间并计算结束时间，然后设置到m_pFrequencyTimeEdit
    QDateTime beginDateTime = m_pBeginDateEdit->getDateEdit()->dateTime();
    QDate nextDay = beginDateTime.date().addDays(1);
    QDateTime endDate = QDateTime(nextDay, beginDateTime.time());
    m_pFrequencyTimeEdit->getDateEdit()->setDateTime(endDate);
    m_pFrequencyTimeEdit->getDateEdit()->setMinimumDateTime(m_pBeginDateEdit->getDateEdit()->dateTime());
}

//实现双击时间进入编辑状态
void CSchceduleDlg::handleComboBoxDoubleClick(const QPoint &pos)
{
    //if (cmb_beginTimeEdit->view()->rect().contains(pos))
    //    return; // 如果双击在下拉列表内，忽略此次事件

    cmb_beginTimeEdit->lineEdit()->selectAll();
    cmb_beginTimeEdit->lineEdit()->setFocus(Qt::OtherFocusReason);
}


void CSchceduleDlg::addPeriodOfTime()
{
    // 初始化不改变开始时间时更改结束时间立即开始检查合法性
    updateEndTimeValidation();
    QDateTime now = QDateTime::currentDateTime();
    QTime currentTime = now.time();

    // 设置默认开始时间
    m_beginHour = currentTime.hour();
    m_beginMinute = currentTime.minute();

    // 生成开始时间下拉框选项（每半小时）
    cmb_beginTimeEdit->clear();
    for (int hour = 0; hour <= 23; ++hour) {
        for (int minute : {0, 30}) { // 只添加00分和30分
            QString timeStr = QStringLiteral("%1:%2")
                    .arg(hour, 2, 10, QLatin1Char('0'))
                    .arg(minute, 2, 10, QLatin1Char('0'));
            cmb_beginTimeEdit->addItem(timeStr);
        }
    }

    QString defaultBeginTimeStr = currentTime.toString("hh:mm");
    cmb_beginTimeEdit->setCurrentIndex(-1);
    cmb_beginTimeEdit->setCurrentText(defaultBeginTimeStr);

    // 设置默认结束时间（开始时间+30分钟）
    QTime defaultEndTime = currentTime.addSecs(1800);
    m_endHour = defaultEndTime.hour();
    m_endMinute = defaultEndTime.minute();

    // 生成结束时间下拉框选项（每半小时）
    cmb_endTimeEdit->clear();
    for (int hour = 0; hour <= 23; ++hour) {
        for (int minute : {0, 30}) { // 只添加00分和30分
            QString timeStr = QStringLiteral("%1:%2")
                    .arg(hour, 2, 10, QLatin1Char('0'))
                    .arg(minute, 2, 10, QLatin1Char('0'));
            cmb_endTimeEdit->addItem(timeStr);
        }
    }

    QString defaultEndTimeStr = defaultEndTime.toString("hh:mm");
    cmb_endTimeEdit->setCurrentIndex(-1);
    cmb_endTimeEdit->setCurrentText(defaultEndTimeStr);

    tanchuang = false;
    QObject::connect(cmb_beginTimeEdit->lineEdit(), &QLineEdit::editingFinished, [ = ]()
    {
        tanchuang = false;
        validateEndTimeFormat1();
    });
    QObject::connect(cmb_endTimeEdit->lineEdit(), &QLineEdit::editingFinished, [ = ]()
    {
        tanchuang = false;
        validateEndTimeFormat2();
    });
    connect(cmb_beginTimeEdit, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(updateEndTime(const QString&)));
    connect(cmb_beginTimeEdit, &QComboBox::currentTextChanged, this, &CSchceduleDlg::setStartTime);
    connect(cmb_endTimeEdit, &QComboBox::currentTextChanged, this, &CSchceduleDlg::setEndTime);
    cmb_beginTimeEdit->lineEdit()->setAccessibleName("kylin-calendar-plugin_QComboBox_begin_time_edit_LineEdit");
    cmb_endTimeEdit->lineEdit()->setAccessibleName("kylin-calendar-plugin_QComboBox_end_time_edit_LineEdit");
}

//判断重复选项是否变化为重复，重复时出现结束重复layout
void CSchceduleDlg::onRepeatComboxTextChanged(const QString &text)
{
    bool showFrequencyLayout = (text != tr("No Repeat")); // 当选项不是"不重复"时显示frequencyLayout
    this->setMaximumHeight(this->height());
    cmb_frequencyBox->setVisible(showFrequencyLayout);
    lbl_frequencyLabel->setVisible(showFrequencyLayout);
    qDebug()<<"=====================================:"<<showFrequencyLayout;
    if(cmb_frequencyBox->currentText() == tr("Customize") && showFrequencyLayout)
    {
        cmb_frequencyBox->setFixedSize(138, 36);
        m_pFrequencyTimeEdit->setVisible(true);
    }
    else
    {
        m_pFrequencyTimeEdit->setVisible(false);
    }

    QTimer::singleShot(0, this, [this](){
        this->setMaximumHeight(0);
        this->adjustSize();
    });
}

//判断结束重复选项是否变化为自定义，自定义时出现时间box
void CSchceduleDlg::onfrequencyComboxTextChanged(const QString &text)
{
    bool showFrequencyLayout = (text == tr("Customize")); // 当选项不是"不重复"时显示frequencyLayout

    this->setMaximumHeight(this->height());
    m_pFrequencyTimeEdit->setVisible(showFrequencyLayout);

    cmb_frequencyBox->setFixedSize(138, 36);
    if(!showFrequencyLayout)
    {
        cmb_frequencyBox->setFixedSize(290, 36);
    }
    QTimer::singleShot(0, this, [this](){
        this->setMaximumHeight(0);
        this->adjustSize();
    });
}

/**
 * @brief CSchceduleDlg::slotfrequencyBox
 * @param text
 * 设置重复规则的
 */
void CSchceduleDlg::slotfrequencyBox(const QString &text)
{

    if(text.compare(tr("No Repeat")) == 0)
    {
        CalendarDataBase::getInstance().setRRuleType("No Repeat");//0
        setRRuleType(RRule_None);
    }
    else if(text.compare(tr("Daily")) == 0)
    {
        CalendarDataBase::getInstance().setRRuleType("Daily");//1
        setRRuleType(RRule_Day);
    }
    else if(text.compare(tr("Weekly")) == 0)
    {
        CalendarDataBase::getInstance().setRRuleType("Weekly");//3
        setRRuleType(RRule_Week);
    }
    else if(text.compare(tr("Monthly")) == 0)
    {
        CalendarDataBase::getInstance().setRRuleType("Monthly");//4
        setRRuleType(RRule_Month);
    }
    else if(text.compare(tr("Yearly")) == 0)
    {
        CalendarDataBase::getInstance().setRRuleType("Yearly");//5
        setRRuleType(RRule_Year);
    }
}
/**
 * @brief CSchceduleDlg::slotConfirmClick
 * 点击"确定"，保存当前日程到数据库 并创建QTimer ，到时间后调用通知DBus信号发送通知
 */
void CSchceduleDlg::slotConfirmClick()
{
    if(m_textEdit->toPlainText().trimmed().isEmpty())
        m_textEdit->setText(tr("Enter the content of the event"));

    QString uniqueId = QDateTime::currentDateTime().toString("yyyyMMddhhmmss"); //日程id为14位
    qDebug() << "日程的唯一id是" << uniqueId;

    AlarmType curAlarmType = getAlarmType();
    RRuleType curRRuleType = getRRuleType();

    MarkInfo info;
    info.m_markId = uniqueId;
    info.m_descript = getRealText(m_textEdit);
    info.hour = info.Alarm_end_hour = m_beginHour;
    info.minute = info.Alarm_end_minute = m_beginMinute;
    info.day = m_pBeginDateEdit->getDateEdit()->date().day();
    info.week = m_pBeginDateEdit->getDateEdit()->date().dayOfWeek();
    info.month = m_pBeginDateEdit->getDateEdit()->date().month();
    info.end_day = m_pEndDateEdit->getDateEdit()->date().day();
    info.end_month = m_pEndDateEdit->getDateEdit()->date().month();
    info.m_markStartDate = m_pBeginDateEdit->getDateEdit()->date();
    info.m_markEndDate = m_pEndDateEdit->getDateEdit()->date();
    info.end_hour = m_endHour;
    info.end_minute = m_endMinute;
    info.startTime = m_startTime;
    info.endTime = m_endTime;
    info.beginDateTime = m_pBeginDateEdit->getDateEdit()->dateTime();
    info.endDateTime = m_pEndDateEdit->getDateEdit()->dateTime();
    info.m_isAllDay = chk_allDaycheckBox->isChecked();
    info.m_isLunar = m_lunarcheckBox->isChecked();
    int secs1 = m_startTime.secsTo(QTime(0, 0, 0));
    int secs2 = m_endTime.secsTo(QTime(0, 0, 0));
    int timeDiffSecs = secs2 - secs1;
    // 转换为小时、分钟和秒
    int hours = timeDiffSecs / 3600;
    int minutes = (timeDiffSecs % 3600) / 60;
    timeLong = QString("%1 hours,%2 minutes").arg(hours).arg(minutes);

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

    switch (curAlarmType)
    {
    case Alarm_Begin:
    {
        QString strToday = tr("today");
        if(timemodel == "12")
        {
            QString str = strToday + " ";
            info.timePrefix.prepend(str);
        }
        break;
    }
    case Alarm_5Min_Front:
    {
        if (m_beginMinute >= 5)
        {
            info.Alarm_end_minute = m_beginMinute - 5;
        }
        else
        {
            // 处理分钟数变为负数的情况
            // 60分钟减去提前的5分钟
            info.Alarm_end_minute = m_beginMinute + 55;
            if (m_beginHour > 0)
            {
                info.Alarm_end_hour = m_beginHour - 1;
            }
            else
            {
                // 如果小时为0，则调整为23
                info.Alarm_end_hour = 23;
                // 还需要调整日期
                QDate adjustedDate = m_pBeginDateEdit->getDateEdit()->date().addDays(-1);
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
        break;
    }
    case Alarm_10Min_Front:
    {
        if (m_beginMinute >= 10)
        {
            info.Alarm_end_minute = m_beginMinute - 10;
        }
        else
        {
            // 处理分钟数变为负数的情况
            info.Alarm_end_minute = m_beginMinute + 50; // 60分钟减去提前的10分钟
            if (m_beginHour > 0)
            {
                info.Alarm_end_hour = m_beginHour - 1;
            }
            else
            {
                info.Alarm_end_hour = 23; // 如果小时为0，则调整为23
                // 还需要调整日期
                QDate adjustedDate = m_pBeginDateEdit->getDateEdit()->date().addDays(-1);
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
        break;
    }
    case Alarm_15Min_Front:
    {
        if(m_beginMinute >= 15)
        {
            info.Alarm_end_minute = m_beginMinute - 15;
        }
        else
        {
            // 处理分钟数变为负数的情况
            info.Alarm_end_minute = m_beginMinute + 45; // 60分钟减去提前的15分钟
            if (m_beginHour > 0)
            {
                info.Alarm_end_hour = m_beginHour - 1;
            }
            else
            {
                info.Alarm_end_hour = 23; // 如果小时为0，则调整为23
                // 还需要调整日期
                QDate adjustedDate = m_pBeginDateEdit->getDateEdit()->date().addDays(-1);
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
        break;
    }
    case Alarm_30Min_Front:
    {
        if(m_beginMinute >= 30)
        {
            info.Alarm_end_minute = m_beginMinute - 30;
        }
        else
        {
            // 处理分钟数变为负数的情况
            info.Alarm_end_minute = m_beginMinute + 30; // 60分钟减去提前的30分钟
            if (m_beginHour > 0)
            {
                info.Alarm_end_hour = m_beginHour - 1;
            }
            else
            {
                info.Alarm_end_hour = 23; // 如果小时为0，则调整为23
                // 还需要调整日期
                QDate adjustedDate = m_pBeginDateEdit->getDateEdit()->date().addDays(-1);
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
        break;
    }
    case Alarm_1Hour_Front:
    {
        if(m_beginHour >= 1)
        {
            info.Alarm_end_hour = m_beginHour - 1;
        }
        else
        {
            // 处理小时数变为负数的情况
            info.Alarm_end_hour = m_beginHour + 23; // 24小时减去提前的1小时
            // 还需要调整日期
            QDate adjustedDate = m_pBeginDateEdit->getDateEdit()->date().addDays(-1);
            info.day = adjustedDate.day();
            info.month = adjustedDate.month();
        }
        QString strToday = tr("today");
        if(timemodel == "12")
        {
            QString str = strToday + " ";
            info.timePrefix.prepend(str);
        }
        break;
    }
    case Alarm_1Day_Front:
    {
        // 处理提前一天的情况
        QDate adjustedDate = m_pBeginDateEdit->getDateEdit()->date().addDays(-1);
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
        break;
    }
    case Alarm_2Day_Front:
    {
        // 处理提前两天的情况
        QDate adjustedDate = m_pBeginDateEdit->getDateEdit()->date().addDays(-2);
        info.day = adjustedDate.day();
        info.month = adjustedDate.month();
        if(timemodel == "24")
        {
            info.timePrefix = "";
        }
        break;
    }
    case Alarm_1Day9_Front:
    {
        QDate currentDate = m_pBeginDateEdit->getDateEdit()->date();
        QDateTime dateTime(currentDate.addDays(-1)); // 减少一天
        dateTime.setTime(QTime(9, 0)); // 设置时间为上午9点
        info.day = dateTime.date().day();
        info.month = dateTime.date().month();
        info.hour = 0; //因为是全天的 所以开始时间和结束时间就从00:00 - 23:59这样就避免全天的也会置灰现象
        info.minute = 0;
        info.end_hour = 23;
        info.end_minute = 59;
        info.timePrefix = tr("tomorrow");
        break;
    }
    case Alarm_Begin9_Front:
    {
        QDate currentDate = m_pBeginDateEdit->getDateEdit()->date();
        QDateTime dateTime(currentDate); // 使用当前日期
        dateTime.setTime(QTime(9, 0)); // 设置时间为上午9点
        info.day = dateTime.date().day();
        info.month = dateTime.date().month();
        info.hour = 0;//因为是全天的 所以开始时间和结束时间就从00:00 - 23:59这样就避免全天的也会置灰现象
        info.minute = 0;
        info.end_hour = 23;
        info.end_minute = 59;
        info.timePrefix = tr("today");
        break;
    }
    case Alarm_2Day9_Front:
    {
        QDate currentDate = m_pBeginDateEdit->getDateEdit()->date();
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
        break;
    }
    case Alarm_1Week9_Front:
    {
        QDate currentDate = m_pBeginDateEdit->getDateEdit()->date();
        QDateTime dateTime(currentDate.addDays(-7)); // 减少一天
        dateTime.setTime(QTime(9, 0)); // 设置时间为上午9点
        info.day = dateTime.date().day();
        info.month = dateTime.date().month();
        info.hour = 0;//因为是全天的 所以开始时间和结束时间就从00:00 - 23:59这样就避免全天的也会置灰现象
        info.minute = 0;
        info.end_hour = 23;
        info.end_minute = 59;
        break;
    }
    default:
        break;
    }

    //重复提醒判断
    if(curRRuleType != RRule_None)
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

    info.m_isAllDay = chk_allDaycheckBox->isChecked();
    info.remind = cmb_rmindCombox->currentText();
    info.AlldayRemind = cmb_rmindComboxallday->currentText();
    if(info.remind == "")
        info.remind = "No prompt";
    else if(info.AlldayRemind == "")
        info.AlldayRemind = "No prompt";
    info.repeat = cmb_repeatCombox->currentText();
    info.beginrepeat = cmb_frequencyBox->currentText();
    info.endrepeat = m_pFrequencyTimeEdit->getDateEdit()->dateTime();
    if(cmb_frequencyBox->currentText() == tr("Infinite repetition"))
    {
        info.endrepeat = QDateTime::fromString("9999-12-31", "yyyy-MM-dd");
    }
    if(info.m_descript.contains("\n"))
    {
        qDebug() << "包含\n,不符合crontab标准,消除所有的\n";
        info.m_descript = info.m_descript.replace("\n", " "); //和设计沟通过了 可以用空格
    }
    if(m_type == 1)
    {
        if(curAlarmType == 0)
        {
            CalendarDataBase::getInstance().insertDataSlot(info);
            CalendarDataBase::getInstance().searchDataSlot(info.m_markStartDate);
        }
        else
        {
            CalendarDataBase::getInstance().add(info); //crontab
            CalendarDataBase::getInstance().insertDataSlot(info);
            CalendarDataBase::getInstance().searchDataSlot(info.m_markStartDate);
        }
    }
    else if(m_type == 0)
    {
        info.m_markId = idd;
        if(curAlarmType == 0)
        {
            CalendarDataBase::getInstance().update(info);
            CalendarDataBase::getInstance().updateDataSlot(info);
        }
        else
        {
            CalendarDataBase::getInstance().update(info);
            CalendarDataBase::getInstance().updateDataSlot(info);
        }
    }
    resetToDefault();
    this->close();
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    //传入事件描述“打开文件”，所在界面“主界面”后，上传事件
    kdk_dia_upload_default(node, "kylin_calendar_add_save_schedule", "ScheduleDialog");
    //释放内存
    kdk_dia_data_free(node);

}
void CSchceduleDlg::setStartDate(const QDate &date)
{
    m_pBeginDateEdit->getDateEdit()->setDate(date);
}

void CSchceduleDlg::setEndDate(const QDate &date)
{
    m_pEndDateEdit->getDateEdit()->setDate(date);
}
QString CSchceduleDlg::getText()
{
    return m_textEdit->toPlainText();
}
void CSchceduleDlg::bindConnect()
{
    connect(chk_allDaycheckBox, &QCheckBox::toggled, this, &CSchceduleDlg::setAllDaySlot);
}

//设置打开全天，不能操作时间，且时间选择框置灰
void CSchceduleDlg::setAllDaySlot(bool isChecked)
{
    if (isChecked)
    {
        cmb_beginTimeEdit->setDisabled(true);
        cmb_endTimeEdit->setDisabled(true);
    }
    else
    {
        cmb_beginTimeEdit->setDisabled(false);
        cmb_endTimeEdit->setDisabled(false);
    }
}

void CSchceduleDlg::setRepeatSlot(bool isChecked)
{
    if (isChecked)
    {
        cmb_frequencyBox->setDisabled(false);
        m_pFrequencyTimeEdit->setDisabled(false);
    }
    else
    {
        cmb_frequencyBox->setDisabled(true);
    }
}

void CSchceduleDlg::updateScheduleInfo(const MarkInfo &info)
{
    m_info = info;
    m_pBeginDateEdit->getDateEdit()->setDate(info.m_markStartDate);
    m_pEndDateEdit->getDateEdit()->setDate(info.m_markEndDate);
    m_textEdit->setText(info.m_descript);
}
/**
 * @brief CSchceduleDlg::Setinfo
 * 给弹窗设置info，这样可以弹出来一个已知的日程信息.
 */
void CSchceduleDlg::Setinfo(MarkInfo info)
{
    m_info = info;
}

void CSchceduleDlg::setId(QString idd)
{
    this->idd = idd;
}
//BUG:218784
void CSchceduleDlg::setStatusFromId(QString id)
{
    QString date;
    QString searchCmd =
            QString("select start_date from Schedule where id = '%1'").arg(id);
    QSqlQuery searchQuery;
    searchQuery.exec(searchCmd);
    if (!searchQuery.exec(searchCmd))
    {
        qDebug() << "query error :" << searchQuery.lastError();
    }
    else
    {
        while (searchQuery.next())
        {
            date = searchQuery.value(0).toString();
        }
    }
    QDate mubiaoDate = QDate::fromString(date, "yyyy-MM-dd");
    int x = QDate::currentDate().daysTo(mubiaoDate);
    CalendarDataBase::getInstance().searchDataSlot(QDate::currentDate().addDays(x));
    m_markInfoList = CalendarDataBase::getInstance().m_markInfoList;
    foreach (MarkInfo info, m_markInfoList)
    {
        if(info.m_markId == id)
        {
            m_textEdit->setText(info.m_descript);
            m_pBeginDateEdit->getDateEdit()->setDateTime(info.beginDateTime);
            m_pEndDateEdit->getDateEdit()->setDateTime(info.endDateTime);

            QString starttimeString = QString::number(info.hour) + ":" + QString::number(info.minute); // 确保这是有效的格式
            QTime m_startTime = QTime::fromString(starttimeString, "h:m");
            QString start = m_startTime.toString("hh:mm");
            int index = cmb_beginTimeEdit->findText(start);
            cmb_beginTimeEdit->setCurrentIndex(index);
            m_beginHour = info.hour;
            m_beginMinute = info.minute;
            QString begintime = QString("%1:%2").arg(m_beginHour,2,10,QLatin1Char('0')).arg(m_beginMinute,2,10,QLatin1Char('0'));
            cmb_beginTimeEdit->lineEdit()->setText(begintime);

            QString timeString = QString::number(info.end_hour) + ":" + QString::number(info.end_minute); // 确保这是有效的格式
            QTime m_endTime = QTime::fromString(timeString, "h:m");
            QString end = m_endTime.toString("hh:mm");
            int index1 = cmb_endTimeEdit->findText(end);
            cmb_endTimeEdit->setCurrentIndex(index1);
            m_endHour = info.end_hour;
            m_endMinute = info.end_minute;
            QString endtime = QString("%1:%2").arg(m_endHour,2,10,QLatin1Char('0')).arg(m_endMinute,2,10,QLatin1Char('0'));
            cmb_endTimeEdit->lineEdit()->setText(endtime);
            if(info.m_isAllDay)
            {
                addPeriodOfTime();
            }

            chk_allDaycheckBox->setChecked(info.m_isAllDay);

            if(info.m_isLunar && m_showLunar)
            {
                m_lunarcheckBox->setChecked(info.m_isLunar);
                qDebug() << "m_isLunar1 and m_showLunar";
                m_pBeginDateEdit->setShowLunar(true);
                m_pEndDateEdit->setShowLunar(true);
                m_pFrequencyTimeEdit->setShowLunar(true);
            }

            QString transRemind = tr(info.remind.toUtf8().constData());
            int index2 = cmb_rmindCombox->findText(transRemind);

            cmb_rmindCombox->setCurrentIndex(index2);
            QString transAllDayRemind = tr(info.AlldayRemind.toUtf8().constData());
            int index3 = cmb_rmindComboxallday->findText(transAllDayRemind);
            cmb_rmindComboxallday->setCurrentIndex(index3);

            QString transRepeat = tr(info.repeat.toUtf8().constData());
            int index4 = cmb_repeatCombox->findText(transRepeat);
            cmb_repeatCombox->setCurrentIndex(index4);
            if(index4 == 0)
            {
                return;

            }

            QString transBeginRepeat = tr(info.beginrepeat.toUtf8().constData());
            int index5 = cmb_frequencyBox->findText(transBeginRepeat);
            cmb_frequencyBox->setCurrentIndex(index5);

            if(cmb_frequencyBox->currentText() == tr("Infinite repetition"))
            {
                m_pFrequencyTimeEdit->getDateEdit()->setDateTime(QDateTime::currentDateTime().addDays(1));
            }
            else
            {
                m_pFrequencyTimeEdit->getDateEdit()->setDateTime(info.endrepeat);
            }
            qDebug() << info.m_descript << info.m_isAllDay << info.remind << info.AlldayRemind << info.repeat << info.beginrepeat << info.beginDateTime << info.endDateTime << info.endrepeat;

        }
    }
}

QString CSchceduleDlg::escapeUrlToHtmlTags(QString &text)
{
    QString escapedText = text;

    QRegularExpression regExp(R"((?:https|http|ftp)://[a-zA-Z0-9\-\.]+\.[a-zA-Z]{2,}(?:[0-9]{1,5})?(?:/[\S]*)?)");
    QRegularExpressionMatchIterator iterator = regExp.globalMatch(text);
    while (iterator.hasNext())
    {
        QStringList capturedTextList = iterator.next().capturedTexts();
        QString url = capturedTextList.value(0);

        escapedText.replace(url, websiteTemplate.arg(url, url));
    }

    text = escapedText;
    return text;
}

void CSchceduleDlg::resetToDefault()
{
    // 重置文本编辑框
    m_textEdit->clear();
    m_textEdit->setPlaceholderText(tr("Enter the content of the event"));

    // 重置复选框
    chk_allDaycheckBox->setChecked(false);
    m_lunarcheckBox->setChecked(false);

    // 重置日期控件
    QDateTime currentDateTime = QDateTime::currentDateTime();
    m_pBeginDateEdit->getDateEdit()->setMinimumDateTime(currentDateTime);
    m_pBeginDateEdit->getDateEdit()->setDateTimeRange(currentDateTime, QDateTime::currentDateTime().addDays(365));
    m_pBeginDateEdit->getDateEdit()->setDate(currentDateTime.date());

    m_pEndDateEdit->getDateEdit()->setDate(currentDateTime.date());
    QDateTime newStartTime = m_pBeginDateEdit->getDateEdit()->dateTime();
    QDateTime newEndTime = QDateTime(newStartTime.date(), m_pEndDateEdit->getDateEdit()->time());
    m_pEndDateEdit->getDateEdit()->setDateTime(newEndTime);
    m_pEndDateEdit->getDateEdit()->setMinimumDateTime(m_pBeginDateEdit->getDateEdit()->dateTime());

    // 重置时间下拉框
    addPeriodOfTime(); // 这会重置时间选择框到默认值

    // 重置提醒下拉框
    cmb_rmindCombox->setCurrentIndex(0);
    cmb_rmindComboxallday->setCurrentIndex(0);
    cmb_rmindCombox->setVisible(true);
    cmb_rmindComboxallday->setVisible(false);

    // 重置重复下拉框
    cmb_repeatCombox->setCurrentIndex(0);
    cmb_frequencyBox->setCurrentIndex(0);

    // 重置重复设置控件
    cmb_frequencyBox->setVisible(false);
    lbl_frequencyLabel->setVisible(false);
    m_pFrequencyTimeEdit->setVisible(false);

    // 更新结束日期
    updateEndDateFromBeginDate();

    // 确保UI正确更新
    onRepeatComboxTextChanged(cmb_repeatCombox->currentText());
    onfrequencyComboxTextChanged(cmb_frequencyBox->currentText());
}

void CSchceduleDlg::slotTextChanged()
{
    QTextCursor cursor = m_textEdit->textCursor();
    int previousPosition = cursor.position();

    m_textEdit->setPlaceholderText(this->m_textEdit->toPlainText().isEmpty() ? tr("Enter the content of the event") : "");
    m_textEdit->blockSignals(true);

    QString plainText = m_textEdit->toPlainText();
    plainText.replace(" ", "&nbsp;");
    plainText = escapeUrlToHtmlTags(plainText);

    plainText.replace("\n", "<br>");
    m_textEdit->setHtml(plainText);

    //    m_textEdit->moveCursor(QTextCursor::End);
    cursor.setPosition(previousPosition);
    m_textEdit->setTextCursor(cursor);


    m_textEdit->blockSignals(false);
}
void CSchceduleDlg::onPaletteChanged()
{
    QPalette pal = qApp->palette();

    QGSettings * styleGsettings = nullptr;
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id))
    {
        styleGsettings = new QGSettings(style_id);
        QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
        if(currentTheme == "ukui-default" || currentTheme == "ukui-light" )
        {
            QPalette pal ;
            pal = lightPalette(this);
            pal.setColor(QPalette::Background, pal.background().color());
            pal.setColor(QPalette::Base, pal.base().color());
            pal.setColor(QPalette::Button, pal.button().color());

            pal.setColor(QPalette::ToolTipBase, pal.toolTipBase().color());
            pal.setColor(QPalette::ToolTipText, pal.toolTipText().color());
            pal.setColor(QPalette::ButtonText, pal.buttonText().color());
            setFramePalette(this, pal);
        }
        if(currentTheme == "ukui-dark")
        {
            setFramePalette(this, pal);
        }
    }
    this->setPalette(pal);
    if (styleGsettings != nullptr)
    {
        delete styleGsettings;
        styleGsettings = nullptr;
    }
}

void CSchceduleDlg::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        qDebug() << "event->key() == Qt::Key_Return ||";
        event->ignore();
    }
    else
    {
        QWidget::keyPressEvent(event);
    }
}
void CSchceduleDlg::showEvent(QShowEvent *event)
{
    KWindowSystem::setState(winId(), NET::SkipTaskbar);
}
void CSchceduleDlg::setAlarmType(AlarmType type)
{
    alarm = type;
}
void CSchceduleDlg::setRRuleType(RRuleType type)
{
    rrule = type;
}
AlarmType CSchceduleDlg::getAlarmType()
{
    return alarm;
}
RRuleType CSchceduleDlg::getRRuleType()
{
    return rrule;
}
void CSchceduleDlg::setFramePalette(QWidget  *widget, QPalette &pal)
{
    QList<QLabel *> labelList = widget->findChildren<QLabel *>();
    for (int i = 0; i < labelList.count(); ++i)
    {
        labelList.at(i)->setPalette(pal);
    }
    QList<QComboBox *> comboBoxList = widget->findChildren<QComboBox *>();
    for (int i = 0; i < comboBoxList.count(); ++i)
    {
        comboBoxList.at(i)->setPalette(pal);
        if (comboBoxList.at(i)->view())
        {
            comboBoxList.at(i)->view()->setPalette(pal);
        }
    }
    QList<QPushButton *> buttonList = widget->findChildren<QPushButton *>();
    for (int i = 0; i < buttonList.count(); ++i)
    {
        buttonList.at(i)->setPalette(pal);
    }
    QList<QDateTimeEdit *> dateList = widget->findChildren<QDateTimeEdit *>();
    for (int i = 0; i < dateList.count(); ++i)
    {
        dateList.at(i)->setPalette(pal);
    }
    QList<QTextEdit *> editList = widget->findChildren<QTextEdit *>();
    for (int i = 0; i < editList.count(); ++i)
    {
        editList.at(i)->setPalette(pal);
        editList.at(i)->setContextMenuPolicy(Qt::DefaultContextMenu);
    }
    QList<QCheckBox *> checnboxList = widget->findChildren<QCheckBox *>();
    for (int i = 0; i < checnboxList.count(); ++i)
    {
        checnboxList.at(i)->setPalette(pal);
    }

}
QPalette CSchceduleDlg::lightPalette(QWidget *widget)
{
    QPalette palette = qApp->palette();

    //ukui-light palette UKUI3.1 亮主题色板
    QColor windowText_at(38, 38, 38),
            windowText_iat(0, 0, 0, 255 * 0.55),
            windowText_dis(0, 0, 0, 255 * 0.3),
            button_at(230, 230, 230),
            button_iat(230, 230, 230),
            button_dis(233, 233, 233),
            light_at(255, 255, 255),
            light_iat(255, 255, 255),
            light_dis(242, 242, 242),
            midlight_at(218, 218, 218),
            midlight_iat(218, 218, 218),
            midlight_dis(230, 230, 230),
            dark_at(77, 77, 77),
            dark_iat(77, 77, 77),
            dark_dis(64, 64, 64),
            mid_at(115, 115, 115),
            mid_iat(115, 115, 115),
            mid_dis(102, 102, 102),
            text_at(38, 38, 38),
            text_iat(38, 38, 38),
            text_dis(0, 0, 0, 255 * 0.3),
            brightText_at(0, 0, 0),
            brightText_iat(0, 0, 0),
            brightText_dis(0, 0, 0),
            buttonText_at(38, 38, 38),
            buttonText_iat(38, 38, 38),
            buttonText_dis(150, 150, 150),
            base_at(255, 255, 255),
            base_iat(245, 245, 245),
            base_dis(237, 237, 237),
            window_at(245, 245, 245),
            window_iat(237, 237, 237),
            window_dis(230, 230, 230),
            shadow_at(0, 0, 0, 255 * 0.16),
            shadow_iat(0, 0, 0, 255 * 0.16),
            shadow_dis(0, 0, 0, 255 * 0.21),
            highLightText_at(255, 255, 255),
            highLightText_iat(255, 255, 255),
            highLightText_dis(179, 179, 179),
            alternateBase_at(245, 245, 245),
            alternateBase_iat(245, 245, 245),
            alternateBase_dis(245, 245, 245),
            noRale_at(240, 240, 240),
            noRole_iat(240, 240, 240),
            noRole_dis(217, 217, 217),
            toolTipBase_at(255, 255, 255),
            toolTipBase_iat(255, 255, 255),
            toolTipBase_dis(255, 255, 255),
            toolTipText_at(38, 38, 38),
            toolTipText_iat(38, 38, 38),
            toolTipText_dis(38, 38, 38),
            placeholderText_at(0, 0, 0, 255 * 0.35),
            placeholderText_iat(0, 0, 0, 255 * 0.35),
            placeholderText_dis(0, 0, 0, 255 * 0.3);

    palette.setColor(QPalette::Active, QPalette::WindowText, windowText_at);
    palette.setColor(QPalette::Inactive, QPalette::WindowText, windowText_iat);
    palette.setColor(QPalette::Disabled, QPalette::WindowText, windowText_dis);

    palette.setColor(QPalette::Active, QPalette::Button, button_at);
    palette.setColor(QPalette::Inactive, QPalette::Button, button_iat);
    palette.setColor(QPalette::Disabled, QPalette::Button, button_dis);

    palette.setColor(QPalette::Active, QPalette::Light, light_at);
    palette.setColor(QPalette::Inactive, QPalette::Light, light_iat);
    palette.setColor(QPalette::Disabled, QPalette::Light, light_dis);

    palette.setColor(QPalette::Active, QPalette::Midlight, midlight_at);
    palette.setColor(QPalette::Inactive, QPalette::Midlight, midlight_iat);
    palette.setColor(QPalette::Disabled, QPalette::Midlight, midlight_dis);

    palette.setColor(QPalette::Active, QPalette::Dark, dark_at);
    palette.setColor(QPalette::Inactive, QPalette::Dark, dark_iat);
    palette.setColor(QPalette::Disabled, QPalette::Dark, dark_dis);

    palette.setColor(QPalette::Active, QPalette::Mid, mid_at);
    palette.setColor(QPalette::Inactive, QPalette::Mid, mid_iat);
    palette.setColor(QPalette::Disabled, QPalette::Mid, mid_dis);

    palette.setColor(QPalette::Active, QPalette::Text, text_at);
    palette.setColor(QPalette::Inactive, QPalette::Text, text_iat);
    palette.setColor(QPalette::Disabled, QPalette::Text, text_dis);

    palette.setColor(QPalette::Active, QPalette::BrightText, brightText_at);
    palette.setColor(QPalette::Inactive, QPalette::BrightText, brightText_iat);
    palette.setColor(QPalette::Disabled, QPalette::BrightText, brightText_dis);

    palette.setColor(QPalette::Active, QPalette::ButtonText, buttonText_at);
    palette.setColor(QPalette::Inactive, QPalette::ButtonText, buttonText_iat);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, buttonText_dis);

    palette.setColor(QPalette::Active, QPalette::Base, base_at);
    palette.setColor(QPalette::Inactive, QPalette::Base, base_iat);
    palette.setColor(QPalette::Disabled, QPalette::Base, base_dis);

    palette.setColor(QPalette::Active, QPalette::Window, window_at);
    palette.setColor(QPalette::Inactive, QPalette::Window, window_iat);
    palette.setColor(QPalette::Disabled, QPalette::Window, window_dis);

    palette.setColor(QPalette::Active, QPalette::Shadow, shadow_at);
    palette.setColor(QPalette::Inactive, QPalette::Shadow, shadow_iat);
    palette.setColor(QPalette::Disabled, QPalette::Shadow, shadow_dis);

    palette.setColor(QPalette::Active, QPalette::HighlightedText, highLightText_at);
    palette.setColor(QPalette::Inactive, QPalette::HighlightedText, highLightText_iat);
    palette.setColor(QPalette::Disabled, QPalette::HighlightedText, highLightText_dis);

    palette.setColor(QPalette::Active, QPalette::AlternateBase, alternateBase_at);
    palette.setColor(QPalette::Inactive, QPalette::AlternateBase, alternateBase_iat);
    palette.setColor(QPalette::Disabled, QPalette::AlternateBase, alternateBase_dis);

    palette.setColor(QPalette::Active, QPalette::NoRole, noRale_at);
    palette.setColor(QPalette::Inactive, QPalette::NoRole, noRole_iat);
    palette.setColor(QPalette::Disabled, QPalette::NoRole, noRole_dis);

    palette.setColor(QPalette::Active, QPalette::ToolTipBase, toolTipBase_at);
    palette.setColor(QPalette::Inactive, QPalette::ToolTipBase, toolTipBase_iat);
    palette.setColor(QPalette::Disabled, QPalette::ToolTipBase, toolTipBase_dis);

    palette.setColor(QPalette::Active, QPalette::ToolTipText, toolTipText_at);
    palette.setColor(QPalette::Inactive, QPalette::ToolTipText, toolTipText_iat);
    palette.setColor(QPalette::Disabled, QPalette::ToolTipText, toolTipText_dis);

#if (QT_VERSION >= QT_VERSION_CHECK(5,12,0))
    palette.setColor(QPalette::Active, QPalette::PlaceholderText, placeholderText_at);
    palette.setColor(QPalette::Inactive, QPalette::PlaceholderText, placeholderText_iat);
    palette.setColor(QPalette::Disabled, QPalette::PlaceholderText, placeholderText_dis);
#endif
    return palette;

}
void CSchceduleDlg::initTooltipStyle()
{
    QFont font;
    QFontMetrics fontMetrics(font);
    if(styleGsettings != nullptr)
    {
        QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
        if(currentTheme == "ukui-default" || currentTheme == "ukui-light" )
        {
            textColor = "#000000";
            backgroundColor = "#FFFFFA";
            TooltipBorderColor = "#D3D3D3";
        }
        if(currentTheme == "ukui-dark")
        {
            textColor = "#FFFFFF";
            backgroundColor = "#1A1A1A";
            TooltipBorderColor = "#A9A9A9";
        }
        TooltipStyle = QString("QToolTip {"
                               "color: %1;" // 文字颜色
                               "background-color: %2;" // 背景颜色
                               "border: 1px solid %3;"
                               "border-radius:10px;" // 可选：边框
                               "padding: 2px;" // 可选：内边距
                               "}"
                               "QLabel {"
                               "color: %4;"
                               "}").arg(textColor).arg(backgroundColor).arg(TooltipBorderColor).arg(textColor);
        lbl_beginTimeLabel->setStyleSheet(TooltipStyle);
        lbl_endTimeLabel->setStyleSheet(TooltipStyle);
        lbl_remindSetLabel->setStyleSheet(TooltipStyle);
        lbl_beginrepeatLabel->setStyleSheet(TooltipStyle);
        lbl_frequencyLabel->setStyleSheet(TooltipStyle);
        //        cmb_rmindComboxallday->setStyleSheet(TooltipStyle);

        QString elidedText_fre = fontMetrics.elidedText(tr("Frequency:"), Qt::ElideRight, 80);

        lbl_frequencyLabel->setToolTip(tr("Frequency:"));
        lbl_frequencyLabel->setText(elidedText_fre);


        QString elidedText_repeat = fontMetrics.elidedText(tr("Repeat:"), Qt::ElideRight, 70);

        lbl_beginrepeatLabel->setToolTip(tr("Repeat:"));
        lbl_beginrepeatLabel->setText(elidedText_repeat);


        QString elidedText_remind = fontMetrics.elidedText(tr("Remind Me:"), Qt::ElideRight, 70);

        lbl_remindSetLabel->setToolTip(tr("Remind Me:"));
        lbl_remindSetLabel->setText(elidedText_remind);

        for(int i = 0 ; i < cmb_rmindComboxallday->count(); i++)
        {
            qDebug() << "cmb_rmindComboxallday";
            QString text = cmb_rmindComboxallday->itemText(i);
            QString elideText_allday = fontMetrics.elidedText(text, Qt::ElideRight, 210);
            cmb_rmindComboxallday->setItemText(i, elideText_allday);
            cmb_rmindComboxallday->setToolTip(text);
        }


        connect(styleGsettings, &QGSettings::changed, [ = ](QString key)
        {
            qDebug() << "styleGsettingschan" << key;
            if ("styleName" == key)
            {
                QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
                if(currentTheme == "ukui-default" || currentTheme == "ukui-light" )
                {
                    textColor = "#000000";
                    backgroundColor = "#FFFFFA";
                    TooltipBorderColor = "#D3D3D3";
                }
                if(currentTheme == "ukui-dark")
                {
                    textColor = "#FFFFFF";
                    backgroundColor = "#1A1A1A";
                    TooltipBorderColor = "#A9A9A9";
                }
            }
            TooltipStyle = QString("QToolTip {"
                                   "color: %1;" // 文字颜色
                                   "background-color: %2;" // 背景颜色
                                   "border: 1px solid %3;"
                                   "border-radius:10px;" // 可选：边框
                                   "padding: 2px;" // 可选：内边距
                                   "}"
                                   "QLabel {"
                                   "color: %4;"
                                   "}").arg(textColor).arg(backgroundColor).arg(TooltipBorderColor).arg(textColor);
            lbl_beginTimeLabel->setStyleSheet(TooltipStyle);
            lbl_endTimeLabel->setStyleSheet(TooltipStyle);
            lbl_remindSetLabel->setStyleSheet(TooltipStyle);
            lbl_beginrepeatLabel->setStyleSheet(TooltipStyle);
            lbl_frequencyLabel->setStyleSheet(TooltipStyle);
            QFont font;
            QFontMetrics fontMetrics_fre1(font);
            QString elidedText_fre = fontMetrics_fre1.elidedText(tr("Frequency:"), Qt::ElideRight, 80);
            lbl_frequencyLabel->setToolTip(tr("Frequency:"));
            lbl_frequencyLabel->setText(elidedText_fre);
            QString elidedText_repeat = fontMetrics_fre1.elidedText(tr("Repeat:"), Qt::ElideRight, 70);
            lbl_beginrepeatLabel->setToolTip(tr("Repeat:"));
            lbl_beginrepeatLabel->setText(elidedText_repeat);
            QString elidedText_remind = fontMetrics_fre1.elidedText(tr("Remind Me:"), Qt::ElideRight, 70);
            lbl_remindSetLabel->setToolTip(tr("Remind Me:"));
            lbl_remindSetLabel->setText(elidedText_remind);
            //            qDebug() << "style change succussful" << TooltipStyle;
        });
    }
}
bool CSchceduleDlg::getShowLunar()
{
    //语言是简体或繁体中文且区域是中国且GSetting的值时，
    //这个时候才显示农历和黄历信息
    QString lang, format;
    getLocale(lang, format);
    if(calendar_gsettings != nullptr)
    {
        //qInfo() << lang << format << (calendar_gsettings->get("calendar").toString() == "lunar") ;
        if((lang.contains("zh_CN") || lang.contains("zh_HK")) &&
                (calendar_gsettings->get("calendar").toString() == "lunar"))
        {
            m_lunarcheckBox->setVisible(true);
            m_lunarLabel->setVisible(true);
            m_showLunar = true;
        }
        else
        {
            m_lunarcheckBox->setVisible(false);
            m_lunarLabel->setVisible(false);
            m_pBeginDateEdit->setShowLunar(false);
            m_pEndDateEdit->setShowLunar(false);
            m_pFrequencyTimeEdit->setShowLunar(false);
            m_showLunar = false;
        }
        connect(calendar_gsettings, &QGSettings::changed, this, [ = ] (const QString & key)
        {
            if(key == LUNAR_KEY)
            {
                if((lang.contains("zh_CN") || lang.contains("zh_HK")) &&
                        (calendar_gsettings->get("calendar").toString() == "lunar"))
                {
                    m_lunarcheckBox->setVisible(true);
                    m_lunarLabel->setVisible(true);
                    m_lunarcheckBox->setChecked(false);
                }
                else
                {
                    m_lunarcheckBox->setVisible(false);
                    m_lunarLabel->setVisible(false);
                    m_pBeginDateEdit->setShowLunar(false);
                    m_pEndDateEdit->setShowLunar(false);
                    m_pFrequencyTimeEdit->setShowLunar(false);
                }
            }
        });
    }
    return m_showLunar;
}
QStringList CSchceduleDlg::getLocale(QString &language, QString &locale)
{
    //判断区域(美国/中国)
    QString objpath;
    unsigned int uid = getuid();
    objpath = objpath + "/org/freedesktop/Accounts/User" + QString::number(uid);
    QString formats;
    QString lang;
    QStringList result;

    QDBusInterface localeInterface("org.freedesktop.Accounts",
                                   objpath,
                                   "org.freedesktop.DBus.Properties",
                                   QDBusConnection::systemBus(), this);
    QDBusReply<QMap<QString, QVariant>> reply = localeInterface.call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid())
    {
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        if (propertyMap.keys().contains("FormatsLocale"))
        {
            formats = propertyMap.find("FormatsLocale").value().toString();
            locale = formats;
        }
        if(lang.isEmpty() && propertyMap.keys().contains("Language"))
        {
            lang = propertyMap.find("Language").value().toString();
            language = lang;
        }
    }
    else
    {
        qDebug() << "reply failed";
    }
    result.append(formats);
    result.append(lang);
    return result;
}

QString CSchceduleDlg::getRealText(const TextEdit *textEdit)
{
    QString plaintext;
    QTextDocument* doc = textEdit->document();
    QTextCursor cursor(doc);

    // 遍历所有文本块
    for (QTextBlock block = doc->begin(); block.isValid(); block = block.next())
    {
        cursor.setPosition(block.position());
        for (int i = 0; i < block.length(); ++i)
        {
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
            QString charStr = cursor.selectedText();

            // 处理空格
            if (charStr == QString(QChar::Nbsp))
            {
                plaintext.append(' ');
            }
            else if (charStr == QChar::ParagraphSeparator)
            {
                continue;
            }
            else
            {
                plaintext.append(charStr);
            }
            cursor.clearSelection();
        }

        if (block.next().isValid())
        {
            plaintext.append('\n');
        }
    }
    return plaintext;
}

void CSchceduleDlg::setFormat(QString dateString)
{
    // 匹配不同日期格式的正则表达式
    QRegularExpression re_full_year(R"(^(20\d{2})[-./](\d{1,2})[-./](\d{1,2})$)");
    QRegularExpression re_short_year(R"(^(\d{2})[-./](\d{1,2})[-./](\d{1,2})$)");

    QDateTime dateTime;
    QString format;

    // 匹配四位年份格式 (如 "2024-10-08", "2024/10/08", "2024.10.08")
    QRegularExpressionMatch match_full = re_full_year.match(dateString);
    if (match_full.hasMatch())
    {
        int year = match_full.captured(1).toInt();
        int month = match_full.captured(2).toInt();
        int day = match_full.captured(3).toInt();
        dateTime = QDateTime(QDate(year, month, day));

        // 根据分隔符设置适当的显示格式
        QString separator = dateString.contains('-') ? "-" :
                                                       dateString.contains('/') ? "/" : ".";
        format = QString("yyyy%1MM%1dd").arg(separator); // "yyyy-MM-dd", "yyyy/MM/dd" 或 "yyyy.MM.dd"
    }

    // 匹配两位年份格式 (如 "24-10-8", "24/10/8", "24.10.8")
    QRegularExpressionMatch match_short = re_short_year.match(dateString);
    if (match_short.hasMatch())
    {
        int year = match_short.captured(1).toInt() + 2000;
        int month = match_short.captured(2).toInt();
        int day = match_short.captured(3).toInt();
        dateTime = QDateTime(QDate(year, month, day));

        // 根据分隔符设置适当的显示格式
        QString separator = dateString.contains('-') ? "-" :
                                                       dateString.contains('/') ? "/" : ".";
        format = QString("yy%1M%1d").arg(separator); // "yy-M-d", "yy/M/d" 或 "yy.M.d"
    }

    // 如果日期有效，设置到 QDateTimeEdit
    if (dateTime.isValid())
    {
        m_pBeginDateEdit->getDateEdit()->setDisplayFormat(format); // 设置显示格式
        m_pEndDateEdit->getDateEdit()->setDisplayFormat(format); // 设置显示格式
        m_pFrequencyTimeEdit->getDateEdit()->setDisplayFormat(format); // 设置显示格式
    }
    else
    {
        qDebug() << "无法识别日期格式：" << dateString;
    }
}
QString CSchceduleDlg::getFormat()
{
    return SDK2CPP(kdk_system_get_shortformat_date)();
}
