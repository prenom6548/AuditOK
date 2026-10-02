/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>
#include <unistd.h>
#include <QDebug>
#include <QScrollBar>
#include <QCoreApplication>
#include <QVBoxLayout>
#include <QJsonParseError>
#include <QJsonObject>
#include <QProcess>
#include <QContextMenuEvent>
#include <QGesture>
#include <KWindowEffects>
#include <KWindowSystem>
#include <kpushbutton.h>
#include <libkydate.h>
#include <QVector>
#include <QSettings>
#include <QScreen>
#include <QGuiApplication>
#include <QHeaderView>
#include <QDesktopWidget>
#include <ukuistylehelper/ukuistylehelper.h>
#include "schedule_item.h"
#include "lunarcalendarwidget.h"
#include "custommessagebox.h"
#include "customscrollarea.h"
#include "kysdk/applications/windowmanager/windowmanager.h"
#include <ukuiwindowhelper/ukuiwindowhelper.h>
#include "calendardatabase.h"
#include <libkydatacollect.h>
#define TwoSchedule  170
#define OneSchdule  100
#define afterClickMore 600
#define CALENDAR_GSETTINGS_SCHEMAS "org.kylin.calendar.plugin"
#define CALENDAR_LUNAR_DATE_KEY "lunar-date"
#define YIJISTATUS "yijistatus" //记录yiji的
#define PANEL_CONTROL_IN_CALENDAR "org.ukui.control-center.panel.plugins"
#define LUNAR_KEY                 "calendar"
#define FIRST_DAY_KEY "firstday"
#define SDK2CPP(func) []()->QString { char* p = func(); QString str(p); free(p); return str; }
#define TRANSPARENCY_SETTING "org.ukui.control-center.personalise"
#define TRANSPARENCY_KEY "transparency"

#define PANEL_SETTINGS      "org.ukui.panel.settings"
#define PANEL_POSITION_KEY  "panelposition"
#define ICON_SIZE_KEY       "iconsize"
#define PANEL_SIZE_KEY      "panelsize"

#define YIJI_HEIGHT 55

#define SCROLLA 40
#define LESSTWOSHCEDULE_CALENDAR_HEIGHT 740
#define ORIGIN_CALENDAR_WIDTH 452
#define TWO_MORE_SCHEDULE_HEIGHT 845
#define THREE_SCHEDULE_HEIGHT 890
#define FOUR_MORE_SCHEDULE_HEIGHT 925

#define ORG_UKUI_STYLE            "org.ukui.style"
#define SYSTEM_FONT_SIZE "systemFontSize"
#define SYSTEM_FONT      "systemFont"
#define STYLE_NAME                "styleName"
#define STYLE_NAME_KEY_DARK       "ukui-dark"
#define STYLE_NAME_KEY_DEFAULT    "ukui-default"
#define STYLE_NAME_KEY_BLACK       "ukui-black"
#define STYLE_NAME_KEY_LIGHT       "ukui-light"
#define STYLE_NAME_KEY_WHITE       "ukui-white"
#define PANEL_SIZE_LARGE  92
#define PANEL_SIZE_MEDIUM 70
#define PANEL_SIZE_SMALL  46
#define PANEL_SIZE_KEY    "panelsize"
#define ICON_COLOR_LOGHT      255
#define ICON_COLOR_DRAK       0
#define THEME_SCHAME "org.ukui.style"
#define COLOR_THEME "styleName"
#define KYSDK_DATESERVER "com.kylin.kysdk.DateServer"
#define KYSDK_DATEPATH "/com/kylin/kysdk/Date"
#define KYSDK_DATEINTERFACE "com.kylin.kysdk.DateInterface"
#define KYSDK_TIMERSERVER "com.kylin.kysdk.TimeServer"
#define KYSDK_TIMERPATH "/com/kylin/kysdk/Timer"
#define KYSDK_TIMERINTERFACE "com.kylin.kysdk.TimeInterface"

const int WIDGET_GAP = 8;

using namespace kdk;
class CalendarDataBase;
class schedule_item;
QDate LunarCalendarWidget::s_clickDate = QDate();
LunarCalendarWidget::LunarCalendarWidget(QWidget *parent) : QWidget(parent)
{
    setAccessibleName("kylin-calendar-plugin_lunarcalendar_widget");
    setAccessibleDescription("the calendar ui");
    setObjectName("LunarCalendarWidget");
    installEventFilter(this);
    setMouseTracking(true);
    analysisWorktimeJs();
    int res;
    res = DATABASE_INSTANCE.initDataBase();
    if (res != DB_OP_SUCC)
    {
        qDebug() << "数据库加载失败" << __FILE__ << "," << __FUNCTION__ << "," << __LINE__;
    }
    CalendarDataBase::getInstance().deleteschedule();//删除已经过期的日程
    CalendarDataBase::getInstance().searchDataSlot(date);
    updateAllSchedule();
    if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland")
    {
        //        qputenv("QT_QPA_PLATFORM", "wayland"); //openKylin
        isWayland = true;
    }
    else
    {
        isWayland = false;
    }
    btn_btnYear = new kdk::KPushButton(this);
    btn_btnYear->setAccessibleName("kylin-calendar-plugin_QPushButton_btn_year");
    btn_btnYear->setAccessibleDescription("a push button of year in calendar ui");
    btn_btnYear->setProperty("needTranslucent", true);
    btn_btnYear->setProperty("useButtonPalette", true);
    btn_btnYear->setTranslucent(true);

    btn_btnMonth = new kdk::KPushButton(this);
    btn_btnMonth->setAccessibleName("kylin-calendar-plugin_QPushButton_btn_Month");
    btn_btnMonth->setAccessibleDescription("a push button of Month in calendar ui");
    btn_btnMonth->setProperty("needTranslucent", true);
    btn_btnMonth->setProperty("useButtonPalette", true);
    btn_btnMonth->setTranslucent(true);

    btn_btnToday = new kdk::KPushButton(this);
    btn_btnToday->setAccessibleName("kylin-calendar-plugin_QPushButton_btn_Today");
    btn_btnToday->setAccessibleDescription("a push button of Today in calendar ui");
    btn_btnToday->setProperty("needTranslucent", true);
    btn_btnToday->setProperty("useButtonPalette", true);
    btn_btnToday->setTranslucent(true);
    dateStrFormat = SDK2CPP(kdk_system_get_longformat_date)();
    btnClick = false;
    calendarStyle = CalendarStyle_Red;
    date = QDate::currentDate();

    widgetTime = new QWidget();
    widgetTime->setAccessibleName("kylin-calendar-plugin_lunarcalendar_widget_time");
    widgetTime->setAccessibleDescription("top and left of the main calendar ui layout");

    // QDBusConnection::sessionBus().connect(KYSDK_DATESERVER,
    //                                       KYSDK_DATEPATH,
    //                                       KYSDK_DATEINTERFACE,
    //                                       "ShortDateSignal",
    //                                       this,
    //                                       SLOT(updateFormat(QString))
    //                                       );
    // QDBusConnection::systemBus().connect(KYSDK_TIMERSERVER,
    //                                      KYSDK_TIMERPATH,
    //                                      KYSDK_TIMERINTERFACE,
    //                                      "TimeChangeSignal",
    //                                      this,
    //                                      SLOT(updateFormat(QString))
    //                                      );//当日期改变的时候 改变对应的日期长格式
    m_verLayoutTopLeft = new QVBoxLayout(widgetTime);
    lbl_datelabel = new QLabel(this);
    lbl_datelabel->setFixedSize(290, 30);
    lbl_datelabel->setAccessibleName("kylin-calendar-plugin_QLabel_date_label");
    lbl_datelabel->setAccessibleDescription("this is date label of calendar layout");

    lbl_timelabel = new QLabel(this);
    lbl_timelabel->setFixedSize(290, 20);
    lbl_timelabel->setAccessibleName("kylin-calendar-plugin_QLabel_time_label");
    lbl_timelabel->setAccessibleDescription("this is time label of calendar layout");

    lbl_lunarlabel = new QLabel(this);
    lbl_lunarlabel->setAccessibleName("kylin-calendar-plugin_QLabel_lunar_label");
    lbl_lunarlabel->setAccessibleDescription("this is lunar label int the calendar layout");

    m_pScreen = QApplication::primaryScreen();
    initGsettings();
    QString vertical = "#666666";
    QString hover = "#ffffff";
    if (styleGsettings != nullptr)
    {
        QString currentTheme = styleGsettings->get(COLOR_THEME).toString();
        if(currentTheme == "ukui-light")
        {
            vertical = "#ffffff";
            hover = "#999999";
        }
    }
    ScrollAreaStyleSheet = QString("\
                                   QScrollBar:vertical {\
                                       background: transparent; /* 滚动条背景颜色 */\
                                       width: 5px; /* 滚动条宽度 */\
                                       margin: 0px 0px 0px 0px; /* 外边距 */\
                                       border-radius: 10px; /* 滚动条的整体圆角 */\
                                   }\
                                   QScrollBar::handle:vertical {\
                                       background: %1; /* 滚动条滑块背景颜色 */\
                                       min-height: 30px; /* 滑块最小高度 */\
                                       border-radius: 10px; /* 滑块圆角半径 */\
                                   }\
                                   QScrollBar::handle:vertical:hover {\
                                       background: %2; /* 滚动条滑块背景颜色（悬停时）*/\
                                       min-height: 30px; /* 滑块最小高度 */\
                                       border-radius: 10px; /* 悬停时的滑块圆角半径 */\
                                   }\
                                   QScrollBar::add-line:vertical {\
                                       background: transparent; /* 上方按钮背景颜色 */\
                                       border-radius: 10px; /* 上方按钮圆角半径 */\
                                   }\
                                   QScrollBar::sub-line:vertical {\
                                       background: transparent; /* 下方按钮背景颜色 */\
                                       border-radius: 10px; /* 下方按钮圆角半径 */\
                                   }\
                                   QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {\
                                       background: transparent; /* 滚动条轨道背景颜色 */\
                                       border-radius: 10px; /* 轨道圆角半径 */\
                                   }").arg(vertical).arg(hover);
}
//原样式
void LunarCalendarWidget::noSchedule()
{
    m_scheduleWidget->setContentsMargins(0, 0, 0, 0);
    m_scheduleWidget->setFixedSize(ORIGIN_CALENDAR_WIDTH, 80);
    scr_srcollAreaOfSchedule->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
    scr_srcollAreaOfSchedule->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    schedule_Widget = new schedule_item(this, false);
    schedule_Widget->setDescription("");
    schedule_Widget->setStartTime("");

    labWidget->setContentsMargins(16, 0, 16, 0);
    yijiWidget->setContentsMargins(16, 0, 16, 0);
    item_layout->addWidget(schedule_Widget);
    //item_layout->addSpacing(2);
    m_scheduleWidget->setLayout(item_layout);
    scr_srcollAreaOfSchedule->setWidget(m_scheduleWidget);
    if(m_nPanelHeight < this->height() || m_nPanelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT || isExceedScreenHeight)
    {
        qDebug() << "分辨率支持";
        QPoint currentPos = this->pos();
        scr_srcollAreaOfSchedule->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        this->setFixedSize(ORIGIN_CALENDAR_WIDTH, m_nPanelHeight - SCROLLA);
        m_scheduleWidget->setFixedHeight(100);
        scr_srcollAreaOfSchedule->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
        this->move(currentPos.x(), SCROLLA - WIDGET_GAP);
        isExceedScreenHeight = false;
    }
    else
    {
        QPoint currentPos = this->pos();
        if(yijistate)
        {
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
        }
        else
        {
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
        }
        if(ScrollAreaNeedChange)
        {
            if(yijistate)
            {
                scr_srcollAreaOfSchedule->setFixedSize(420, 80);
            }
            else
            {
                scr_srcollAreaOfSchedule->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
            }
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
            this->move(currentPos.x(), m_nPanelHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP);
        }
        else
        {
            calculateWindowGeometry();
        }

    }
    initVerlayoutCalendar();
}

void LunarCalendarWidget::updateFromItem(QString id)
{
    MarkInfo targetInfo;
    bool found = false;
    QList<MarkInfo> allInfos = CalendarDataBase::getInstance().getMarkInfoList(); // 获取完整日程列表

    foreach(const MarkInfo &info, allInfos) {
        if (info.m_markId == id) {
            targetInfo = info;
            found = true;
            break;
        }
    }

    if (!found) return;

    CustomMessageBox *msgBox = new CustomMessageBox(1, this);
    msgBox->SetText(tr("Are you sure you want to delete this event?"));
    msgBox->setWindowFlags(msgBox->windowFlags() | Qt::WindowStaysOnTopHint);
    // 获取屏幕中心点和尺寸
    QPoint screenCenter = QApplication::desktop()->availableGeometry().center();
    QSize dialogSize = msgBox->size();
    // 计算对话框应显示的中心点坐标，并将其移动到屏幕中心
    msgBox->move(screenCenter.x() - dialogSize.width() / 2, screenCenter.y() - dialogSize.height() / 2);
    connect(msgBox, &CustomMessageBox::yesClicked, [this, targetInfo, msgBox]() {
        emit DATABASE_INSTANCE.deleteDataSignal(targetInfo);
        CalendarDataBase::getInstance().del(targetInfo);  // 传递完整对象
        slotUpdateSchcedule();
        msgBox->deleteLater();
    });

    connect(msgBox, &CustomMessageBox::noClicked, [this, id, msgBox]()
    {
        msgBox->deleteLater();
    });
    KWindowSystem::setState(msgBox->winId(), NET::SkipPager | NET::SkipTaskbar | NET::SkipSwitcher);
    msgBox->show();
    msgBox->raise();
    msgBox->activateWindow();
    if(isWayland){
        UkuiWindowHelper *windowHelper = new UkuiWindowHelper(msgBox);
        windowHelper->setSkipTaskBar(true);
        windowHelper->setSkipSwitcher(true);
        windowHelper->setBlurEffect(QRegion(), 0, true);
    }else{
        kdk::WindowManager::setSkipSwitcher(msgBox->windowHandle(), true);
        kdk::WindowManager::setSkipTaskBar(msgBox->windowHandle(), true);
    }
}

/**
 * @brief LunarCalendarWidget::twoLessSchedule
 * 两个日程以下的部分，但是如果分辨率过低的情况下，无论几个日程都会按照这个处理
 */
void LunarCalendarWidget::twoLessSchedule()
{
    int num = m_markInfoList.count();
    m_scheduleWidget->setContentsMargins(0, 0, 0, 0);
    if(num == 1)
    {
        m_scheduleWidget->setFixedHeight(80);
        scr_srcollAreaOfSchedule->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
    }
    else if(num == 2)
    {
        m_scheduleWidget->setFixedHeight(124);
        scr_srcollAreaOfSchedule->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
    }
    scr_srcollAreaOfSchedule->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //panelHeight 是除了任务栏之外的高度
    QPoint currentPos = this->pos();
    //如果需要调整滑动区域大小了 如果当前只有一个日程 无需滑动条，2个日程就需要了
    if(ScrollAreaNeedChange)
    {
        this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT); //根据设计稿的高度，770
        this->move(currentPos.x(), m_nPanelHeight - LESSTWOSHCEDULE_CALENDAR_HEIGHT - WIDGET_GAP);
        if(yijistate)
        {
            scr_srcollAreaOfSchedule->setFixedSize(420, 80);
        }
        else
        {
            scr_srcollAreaOfSchedule->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
        }
        if(num == 2)
        {
            scr_srcollAreaOfSchedule->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);//滑动条样式还没处理
        }
    }
    else
    {
        if(yijistate)
        {
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
        }
        else
        {
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
        }
    }

    if(m_nPanelHeight < this->height() || m_nPanelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT || isExceedScreenHeight)
    {
        if(ScrollAreaNeedChange)
        {
            qDebug() << "不支持分辨率";
        }
        else
        {
            QPoint currentPos = this->pos();
            scr_srcollAreaOfSchedule->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, m_nPanelHeight - SCROLLA);
            m_scheduleWidget->setFixedHeight(100 * num);
            //scr_srcollAreaOfSchedule->setFixedSize(420, height*0.1);
            scr_srcollAreaOfSchedule->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
            this->move(currentPos.x(), SCROLLA - WIDGET_GAP);
            isExceedScreenHeight = false;
        }
    }
    calculateWindowGeometry();
    populateScheduleItems();
    labWidget->setContentsMargins(16, 0, 16, 0);
    yijiWidget->setContentsMargins(16, 0, 16, 0);
    m_scheduleWidget->setLayout(item_layout);
    scr_srcollAreaOfSchedule->setWidget(m_scheduleWidget);
    initVerlayoutCalendar();
}
void LunarCalendarWidget::updateSchedule()
{
    item_layout = new QVBoxLayout(this);
    int num = m_markInfoList.count();
    scr_srcollAreaOfSchedule->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_num = num;

    //正常分辨率
    if(!ScrollbarUnderTheResolution)
    {
        if(num == 0)
        {
            noSchedule();
        }
        else
        {
            if(num <= 2)
            {
                twoLessSchedule();
            }
            else if(CalendarDataBase::getInstance().m_markInfoList.count() > 2)
            {
                if(m_nPanelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT)
                {
                    isExceedScreenHeight = true;
                    twoLessSchedule();
                    isExceedScreenHeight = false;
                }
                else
                {
                    ThreeMoreSchedule();
                }
            }

        }
    }
    //极限分辨率
    else
    {
        qDebug() << "-------DisplayAtResolution";
        DisplayAtResolution();
    }
}

void LunarCalendarWidget::getFontInfo(QString &fontName, int &fontSize)
{
    qDebug() << __FILE__ << __FUNCTION__;
    fontName = m_fontName;
    fontSize = m_fontSize;
}

LunarCalendarWidget::~LunarCalendarWidget()
{
    if(m_panelGsettings)
    {
        delete m_panelGsettings;
        m_panelGsettings = nullptr;
    }
    if(calendar_gsettings)
    {
        delete calendar_gsettings;
        calendar_gsettings = nullptr;
    }
    if(style_settings)
    {
        delete style_settings;
        style_settings = nullptr;
    }
    if(yijiGsettings)
    {
        delete yijiGsettings;
        yijiGsettings = nullptr;
    }
    qDebug() << "LunarCalendarWidget destroyed";
}

/*
 * @brief 设置日历的背景及文字颜色
 * 参数：
 * weekColor 周六及周日文字颜色
*/
void LunarCalendarWidget::setColor(bool mdark_style)
{
    const QByteArray calendar_id(PANEL_CONTROL_IN_CALENDAR);
    if(mdark_style)
    {
        weekTextColor = QColor(0, 0, 0);
        weekBgColor = QColor(180, 180, 180);
        if(QGSettings::isSchemaInstalled(calendar_id))
        {
            m_showLunar = calendar_gsettings->get(LUNAR_KEY).toString() == "lunar";
        }
        bgImage = ":/image/bg_calendar.png";
        selectType = SelectType_Rect;
    }
    else
    {
        weekTextColor = QColor(255, 255, 255);
        weekBgColor = QColor(0, 0, 0);
        if(QGSettings::isSchemaInstalled(calendar_id))
        {
            m_showLunar = calendar_gsettings->get(LUNAR_KEY).toString() == "lunar";
        }
        bgImage = ":/image/bg_calendar.png";
        selectType = SelectType_Rect;
    }
    initStyle();
}

void LunarCalendarWidget::controlDownLine()
{
    QString lang;
    QString locale;
    getLocale(lang, locale);
    if((!lang.contains("zh", Qt::CaseInsensitive)) ||
            (!locale.contains("zh", Qt::CaseInsensitive)))
    {
        if(lineDown != nullptr)
        {
            lineDown->hide();
        }
    }
    if(calendar_gsettings->get("calendar").toString() == "solarlunar")
    {
        if(lineDown != nullptr)
        {
            lineDown->hide();
        }
    }
}

void LunarCalendarWidget::_timeUpdate()
{
    //qDebug()<<"oemccd300m===_timeUpdate()";
    QDateTime time = QDateTime::currentDateTime();
    QLocale locale = (QLocale::system().name() == "zh_CN" ? (QLocale::Chinese) : (QLocale::English));

    QDateTime currentDateTime = QDateTime::currentDateTime();
    if(timemodel == "12")
    {
        QString lang, format;
        getLocale(lang, format);
        //qInfo() << lang << format << (calendar_gsettings->get("calendar").toString() == "lunar") ;
        if((lang.contains("zh_CN") || lang.contains("zh_HK")))
        {
            timeString = currentDateTime.toString("AP hh:mm:ss");
        }
        else
        {
            const auto locale = QLocale::system();
            if (locale.name().startsWith(QLatin1String("mn")))
            {
                QLocale m_locale(QLocale::Mongolian);
                timeString = m_locale.toString(QDateTime::currentDateTime(), "hh:mm:ss AP");
            }
            else if(locale.name().startsWith(QLatin1String("en")))
            {
                QLocale m_locale(QLocale::English);

                timeString = m_locale.toString(QDateTime::currentDateTime(), "hh:mm:ss AP");
            }
            else if(locale.name().startsWith(QLatin1String("bo")))
            {
                QLocale m_locale(QLocale::Tibetan);
                timeString = m_locale.toString(QDateTime::currentDateTime(), "hh:mm:ss AP");
            }

        }
    }
    else
    {
        timeString = currentDateTime.toString("HH:mm:ss");
    }

    QFont font(m_fontName);
    lbl_datelabel->setText(timeString);
    lbl_datelabel->setFont(font);
    QString strHoliday;
    QString strSolarTerms;
    QString strLunarFestival;
    QString strLunarYear;
    QString strLunarMonth;
    QString strLunarDay;

    LunarCalendarInfo::Instance()->getLunarCalendarInfo(locale.toString(time, "yyyy").toInt(),
                                                        locale.toString(time, "MM").toInt(),
                                                        locale.toString(time, "dd").toInt(),
                                                        strHoliday,
                                                        strSolarTerms,
                                                        strLunarFestival,
                                                        strLunarYear,
                                                        strLunarMonth,
                                                        strLunarDay);

    if (QGSettings::isSchemaInstalled(QByteArray(CALENDAR_GSETTINGS_SCHEMAS)))
    {
        QGSettings(QByteArray(CALENDAR_GSETTINGS_SCHEMAS)).set(CALENDAR_LUNAR_DATE_KEY, QVariant(QString(strLunarMonth + strLunarDay)));
    }

    // 修复点：实时获取日期格式字符串（不再使用缓存）
    //qDebug()<<"oemccd300m===kdk_system_get_longformat_date:"<<SDK2CPP(kdk_system_get_longformat_date)();
   // qDebug()<<"oemccd300m===kdk_system_longweek:"<<SDK2CPP(kdk_system_longweek)();
    // 修复点：使用实时获取的日期字符串
    QString _date = SDK2CPP(kdk_system_get_longformat_date)() + " " + SDK2CPP(kdk_system_longweek)();
    //qDebug()<<"oemccd300m===_date:"<<_date;
    if (lunarstate)
    {
        _date = _date + " " + strLunarMonth + strLunarDay;
    }

    //    lbl_timelabel->setToolTip(_date);
    //    font.setPixelSize(14);
    if (style_settings != nullptr)
    {
        // 获取系统字体大小
        float systemFontSize = style_settings->get(SYSTEM_FONT_SIZE).toFloat();

        float systemFontSizeFloat = static_cast<float>(systemFontSize);
        int datelabelFontSize, timelabelFontSize;

        if (systemFontSizeFloat <= 10.0)
        {
            datelabelFontSize = 24;
            timelabelFontSize = 14;
        }
        else if (systemFontSizeFloat <= 12.0)
        {
            datelabelFontSize = 26;
            timelabelFontSize = 16;
        }
        else if (systemFontSizeFloat <= 13.5)
        {
            datelabelFontSize = 28;
            timelabelFontSize = 18;
        }
        else if (systemFontSizeFloat <= 15.0)
        {
            datelabelFontSize = 30;
            timelabelFontSize = 20;
        }
        else
        {
            datelabelFontSize = 24;
            timelabelFontSize = 14;
        }
        font.setPixelSize(datelabelFontSize);
        lbl_datelabel->setFont(font);
        font.setPixelSize(timelabelFontSize);
        lbl_timelabel->setFont(font);
        // 监听系统字体变化的信号，并在变化时调整字体大小
        connect(style_settings, &QGSettings::changed, this, [this](const QString & key)
        {
            if (key == SYSTEM_FONT_SIZE)
            {
                // 获取系统字体大小
                float systemFontSize = style_settings->get(SYSTEM_FONT_SIZE).toFloat();
                float systemFontSizeFloat = static_cast<float>(systemFontSize);
                int datelabelFontSize, timelabelFontSize;
                if (systemFontSizeFloat <= 10.0)
                {
                    datelabelFontSize = 24;
                    timelabelFontSize = 14;
                }
                else if (systemFontSizeFloat <= 12.0)
                {
                    datelabelFontSize = 26;
                    timelabelFontSize = 16;
                }
                else if (systemFontSizeFloat <= 13.5)
                {
                    datelabelFontSize = 28;
                    timelabelFontSize = 18;
                }
                else if (systemFontSizeFloat <= 15.0)
                {
                    datelabelFontSize = 30;
                    timelabelFontSize = 20;
                }
                else
                {
                    // 默认或更大字体大小
                    datelabelFontSize = 24;
                    timelabelFontSize = 14;
                }
                QFont newDateLabelFont = lbl_datelabel->font();
                QFont newTimeLabelFont = lbl_timelabel->font();
                newDateLabelFont.setPixelSize(datelabelFontSize);
                lbl_datelabel->setFont(newDateLabelFont);
                newTimeLabelFont.setPixelSize(timelabelFontSize);
                lbl_timelabel->setFont(newTimeLabelFont);
            }
        });
    }
    QFont timelabel_font;
    QFontMetrics metrics(timelabel_font);
    QString elided = metrics.elidedText(_date, Qt::ElideRight, 250);//200后用...省略
    lbl_timelabel->setText(elided);
    lbl_timelabel->setToolTip(_date);
    QFont font1(m_font);
    font1.setPixelSize(22);
    QFont font2(m_font);
    font2.setPixelSize(12);
    QFontMetrics fm(font1);
    QFontMetrics fm2(font2);
    int h = fm.boundingRect(lbl_datelabel->text()).height();
    int h2 = fm2.boundingRect(lbl_timelabel->text()).height();
    lbl_datelabel->setAlignment(Qt::AlignLeft);
    lbl_timelabel->setAlignment(Qt::AlignLeft);
    lbl_datelabel->setFixedHeight(h + 5);
    lbl_timelabel->setFixedHeight(h2 + 9);
    lbl_datelabel->setContentsMargins(0, 0, 0, 0);
    lbl_timelabel->setContentsMargins(0, 0, 0, 0);
}

void LunarCalendarWidget::timerUpdate()
{
    //qDebug()<<"===============:"<<QTime::currentTime();
    _timeUpdate();
}

void LunarCalendarWidget::initWidget()
{
    setObjectName("lunarCalendarWidget");
    //顶部widget
    m_widgetTop = new QWidget;
    m_widgetTop->setAccessibleName("kylin-calendar-plugin_lunarcalendar_widget_top");
    m_widgetTop->setAccessibleDescription("top layout of calendar year month today line");
    m_widgetTop->setObjectName("widgetTop");
    m_widgetTop->setMinimumHeight(35);

    //上个月的按钮
    btnPrevYear = new kdk::KBorderlessButton;
    btnPrevYear->setObjectName("kylin-calendar-plugin_KBorderlessButton_btnPrevYear");
    btnPrevYear->setAccessibleName("kylin-calendar-plugin-KBorderlessButton_btn_pre_year");
    btnPrevYear->setAccessibleDescription("pre month kdk button");
    btnPrevYear->setFixedWidth(36);
    btnPrevYear->setFixedHeight(36);
    btnPrevYear->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //btnPrevYear->setProperty("useIconHighlightEffect", 0x2);
    btnPrevYear->setIcon(QIcon::fromTheme("ukui-start-symbolic"));


    //下个月按钮
    btnNextYear = new kdk::KBorderlessButton;
    btnNextYear->setObjectName("kylin-calendar-plugin_KBorderlessButton_btnNextYear");
    btnNextYear->setAccessibleName("kylin-calendar-plugin_KBorderlessButton_btn_next_year");
    btnNextYear->setAccessibleDescription("next month kdk button");
    btnNextYear->setFixedWidth(36);
    btnNextYear->setFixedHeight(36);
    //btnNextYear->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    btnNextYear->setProperty("useIconHighlightEffect", 0x2);
    btnNextYear->setIcon(QIcon::fromTheme("ukui-end-symbolic"));

    m_font.setFamily(m_fontName);
    m_font.setPixelSize(14);
    QFont smallfont = m_font;
    if(QLocale::system().name() != "zh_CN")
    {
        smallfont.setPointSize(8);
    }
    //转到年显示
    btn_btnYear->setObjectName("btn_btnYear");
    btn_btnYear->setFocusPolicy(Qt::NoFocus);
    btn_btnYear->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //    btn_btnYear->setFont(smallfont);
    btn_btnYear->setText(tr("Year"));
    btn_btnYear->setToolTip(tr("Year"));
    btn_btnYear->setFixedSize(56, 36);
    connect(btn_btnYear, &QPushButton::clicked, this, &LunarCalendarWidget::yearWidgetChange);

    //转到月显示
    btn_btnMonth->setObjectName("btn_btnMonth");
    btn_btnMonth->setFocusPolicy(Qt::NoFocus);
    //    btn_btnMonth->setFont(smallfont);
    btn_btnMonth->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btn_btnMonth->setText(tr("Month"));
    btn_btnMonth->setToolTip(tr("Month"));
    btn_btnMonth->setFixedSize(56, 36);
    connect(btn_btnMonth, &QPushButton::clicked, this, &LunarCalendarWidget::monthWidgetChange);

    //转到今天
    btn_btnToday->setObjectName("btn_btnToday");
    btn_btnToday->setFocusPolicy(Qt::NoFocus);
    //    btn_btnToday->setFont(smallfont);
    btn_btnToday->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    btn_btnToday->setText(tr("Today"));
    btn_btnToday->setToolTip(tr("Today"));
    btn_btnToday->setFixedSize(56, 36);

    btn_btnYear->setProperty("needTranslucent", true);
    btn_btnMonth->setProperty("needTranslucent", true);
    btn_btnToday->setProperty("needTranslucent", true);
    btn_btnYear->setProperty("useButtonPalette", true);
    btn_btnMonth->setProperty("useButtonPalette", true);
    btn_btnToday->setProperty("useButtonPalette", true);
    //年份与月份下拉框 暂不用此
    //    cboxYearandMonth = new QComboBox;
    lbl_cboxYearandMonthLabel = new QLabel();
    lbl_cboxYearandMonthLabel->setAccessibleName("kylin-calendar-plugin_QLabel_year_month_label");
    lbl_cboxYearandMonthLabel->setAccessibleDescription("this is xxxx_xx label in calendar ui");
    lbl_cboxYearandMonthLabel->setFixedWidth(80);
    lbl_cboxYearandMonthLabel->setFixedHeight(36);
    //    lbl_cboxYearandMonthLabel->setFont(m_font);

    //顶部横向布局
    QHBoxLayout *layoutTop = new QHBoxLayout(m_widgetTop);
    layoutTop->setContentsMargins(8, 0, 24, 0);
    layoutTop->addSpacing(8);
    layoutTop->addWidget(btnPrevYear);
    layoutTop->addWidget(lbl_cboxYearandMonthLabel);
    layoutTop->addWidget(btnNextYear);
    layoutTop->addStretch();
    layoutTop->addWidget(btn_btnYear);
    layoutTop->addWidget(btn_btnMonth);
    layoutTop->addWidget(btn_btnToday);
    layoutTop->addSpacing(8);

    //时间
    widgetTime->setFixedHeight(89);
    m_verLayoutTopLeft->setContentsMargins(0, 0, 0, 0);
    m_verLayoutTopLeft->setSpacing(0);
    m_verLayoutTopLeft->addStretch();
    m_verLayoutTopLeft->addWidget(lbl_datelabel);
    m_verLayoutTopLeft->setSpacing(5);
    m_verLayoutTopLeft->addWidget(lbl_timelabel);
    m_verLayoutTopLeft->addStretch();
    //星期widget
    widgetWeek = new QWidget();
    widgetWeek->setAccessibleName("kylin-calendar-plugin_lunarcalendar_widget_week");
    widgetWeek->setAccessibleDescription("this is monday - sunday label widget");
    widgetWeek->setObjectName("widgetWeek");
    widgetWeek->setMinimumHeight(30);
    widgetWeek->installEventFilter(this);

    //星期布局
    QHBoxLayout *layoutWeek = new QHBoxLayout(widgetWeek);
    layoutWeek->setMargin(0);
    layoutWeek->setSpacing(0);

    for (int i = 0; i < 7; i++)
    {
        QLabel *lab = new QLabel;
        lab->setObjectName(QString("WeekLabel_%1").arg(i));
        lab->setAccessibleName(QString("kylin-calendar-plugin_QLabel_WeekLabel_item_%1").arg(i));
        QFont font(m_font);
        font.setPixelSize(12);
        lab->setFont(font);
        lab->setAlignment(Qt::AlignCenter);
        layoutWeek->addWidget(lab);
        lbl_labWeeks.append(lab);
    }

    //日期标签widget
    m_widgetDayBody = new QWidget;
    m_widgetDayBody->setAccessibleName("kylin-calendar-plugin_lunarcalendar_widget_day_body");
    m_widgetDayBody->setAccessibleDescription("this is main calendar day part body");
    m_widgetDayBody->setObjectName("widgetDayBody");
    m_widgetDayBody->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_widgetDayBody->setMinimumHeight(350);
    //日期标签布局
    QGridLayout *layoutBodyDay = new QGridLayout(m_widgetDayBody);
    layoutBodyDay->setMargin(1);
    layoutBodyDay->setHorizontalSpacing(0);
    layoutBodyDay->setVerticalSpacing(0);

    //逐个添加日标签
    for (int i = 0; i < 42; i++)
    {
        LunarCalendarItem *lab = new LunarCalendarItem(m_fontName, false);
        lab->setObjectName(QString("kylin-calendar-plugin_DayLabel_item_widget_%1").arg(i));
        lab->setAccessibleName(QString("kylin-calendar-plugin_QLabel_Day_item_%1").arg(i));
        lab->setFixedSize(60, 56);
        lab->installEventFilter(this);
        lab->worktime = worktime;
        connect(lab, SIGNAL(clicked(QDate, LunarCalendarItem::DayType)), this, SLOT(labClicked(QDate, LunarCalendarItem::DayType)));
        layoutBodyDay->addWidget(lab, i / 7, i % 7);
        dayItems.append(lab);
    }

    //年份标签widget
    widgetYearBody = new QWidget;
    widgetYearBody->setAccessibleName("kylin-calendar-plugin_lunarcalendar_widget_year_body");
    widgetYearBody->setAccessibleDescription("this is main calendar year part body");
    widgetYearBody->setObjectName("widgetYearBody");
    widgetYearBody->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    widgetYearBody->setMinimumHeight(350);
    //年份标签布局
    QGridLayout *layoutBodyYear = new QGridLayout(widgetYearBody);
    layoutBodyYear->setMargin(1);
    layoutBodyYear->setHorizontalSpacing(0);
    layoutBodyYear->setVerticalSpacing(0);

    //逐个添加年标签
    for (int i = 0; i < 12; i++)
    {
        LunarCalendarYearItem *labYear = new LunarCalendarYearItem(m_fontName);
        labYear->setObjectName(QString("YEARLabel_%1").arg(i));
        labYear->setAccessibleName(QString("kylin-calendar-plugin_QLabel_Year_item_%1").arg(i));
        labYear->installEventFilter(this);
        connect(labYear, SIGNAL(yearMessage(QDate, LunarCalendarYearItem::DayType)), this, SLOT(updateYearClicked(QDate, LunarCalendarYearItem::DayType)));
        layoutBodyYear->addWidget(labYear, i / 3, i % 3);
        yearItems.append(labYear);
    }
    widgetYearBody->hide();

    //月份标签widget
    widgetmonthBody = new QWidget;
    widgetmonthBody->setAccessibleName("kylin-calendar-plugin_lunarcalendar_widget_month_body");
    widgetmonthBody->setAccessibleDescription("this is main calendar month part body");
    widgetmonthBody->setObjectName("widgetmonthBody");
    widgetmonthBody->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    widgetmonthBody->setMinimumHeight(350);
    //月份标签布局
    QGridLayout *layoutBodyMonth = new QGridLayout(widgetmonthBody);
    layoutBodyMonth->setMargin(1);
    layoutBodyMonth->setHorizontalSpacing(0);
    layoutBodyMonth->setVerticalSpacing(0);

    //逐个添加月标签
    for (int i = 0; i < 12; i++)
    {
        LunarCalendarMonthItem *labMonth = new LunarCalendarMonthItem(m_fontName);
        labMonth->setObjectName(QString("MONTHLabel_%1").arg(i));
        labMonth->setAccessibleName(QString("kylin-calendar-plugin_QLabel_Month_item_widget_%1").arg(i));
        labMonth->installEventFilter(this);
        connect(labMonth, SIGNAL(monthMessage(QDate, LunarCalendarMonthItem::DayType)), this, SLOT(updateMonthClicked(QDate, LunarCalendarMonthItem::DayType)));
        layoutBodyMonth->addWidget(labMonth, i / 3, i % 3);
        monthItems.append(labMonth);
    }
    widgetmonthBody->hide();

    labWidget = new QWidget();
    labWidget->setAccessibleName("kylin-calendar-plugin_lunarcalendar_lab_widget");
    labWidget->setAccessibleDescription("yiji part in calendar ui");
    labWidget->installEventFilter(this);

    lbl_labBottom = new QLabel();
    lbl_labBottom->setAccessibleName("kylin-calendar-plugin_QLabel_lbl_labButtom");
    lbl_labBottom->setAccessibleDescription("lunar date in yiji part");
    lbl_labBottom->installEventFilter(this);

    lbl_yijichooseLabel = new QLabel();
    lbl_yijichooseLabel->setAccessibleName("kylin-calendar-plugin_QLabel_lbl_yijichooselabel");
    lbl_yijichooseLabel->setAccessibleDescription("yiji label in calendar ui");
    lbl_yijichooseLabel->setText("宜忌");
    lbl_yijichooseLabel->installEventFilter(this);

    lbl_labBottom->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    lbl_labBottom->setFixedHeight(25);
    chk_yijichoose = new QCheckBox();
    chk_yijichoose->setAccessibleName("kylin-calendar-plugin_QCheckBox_chk_yijichoose");
    chk_yijichoose->setAccessibleDescription("check box of yiji ");
    chk_yijichoose->installEventFilter(this);
    chk_yijichoose->setFixedHeight(18);

    labLayout = new QHBoxLayout();
    labLayout->addWidget(lbl_labBottom);
    labLayout->addStretch();
    labLayout->addWidget(lbl_yijichooseLabel);
    labLayout->addWidget(chk_yijichoose);
    labWidget->setLayout(labLayout);

    yijiLayout = new QVBoxLayout;
    yijiWidget = new QWidget(this);
    yijiWidget->setAccessibleName("kylin-calendar-plugin_lunarcalendar_yiji_widget");
    yijiWidget->setAccessibleDescription("yi and ji part");
    lbl_yiLabel = new QLabel();
    lbl_yiLabel->setAccessibleName("kylin-calendar-plugin_QLabel_yi_label");
    lbl_yiLabel->setAccessibleDescription("yi label part");

    lbl_jiLabel = new QLabel();

    lbl_jiLabel->setAccessibleName("kylin-calendar-plugin_QLabel_ji_label");
    lbl_jiLabel->setAccessibleDescription("ji label part");
    lbl_yiLabel->setFixedHeight(22);
    lbl_jiLabel->setFixedHeight(22);
    lbl_yiLabel->update();
    lbl_jiLabel->update();

    yijiLayout->addWidget(lbl_yiLabel);
    yijiLayout->addWidget(lbl_jiLabel);
    yijiWidget->setLayout(yijiLayout);
    lbl_yiLabel->setVisible(false);
    lbl_jiLabel->setVisible(false);

    //添加日程按钮
    m_createScheduleButton = new kdk::KPushButton(this); //创建日程按钮
    m_createScheduleButton->setAccessibleName("kylin-calendar-plugin_KPushButton_createScheduleBtn");
    m_createScheduleButton->setAccessibleDescription("button to create schedule in calendar ui");
    m_createScheduleButton->setText(tr("Add Schedule"));
    m_createScheduleButton->setTranslucent(true);

    m_createScheduleButton->setFixedSize(96, 36);


    m_createScheduleButton->setProperty("needTranslucent", true);
    m_createScheduleButton->setProperty("useButtonPalette", true);

    //主布局
    lineUp = new PartLineWidget();
    lineUp->setAccessibleName("kylin-calendar-plugin_lunarcalendar_PartLineWidget_line_up");
    lineUp->setAccessibleDescription("the top line");

    lineDown = new PartLineWidget();
    lineDown->setAccessibleName("kylin-calendar-plugin_lunarcalendar_PartLineWidget_line_down");
    lineDown->setAccessibleDescription("the middle line");

    lineSchedule = new PartLineWidget();
    lineSchedule->setAccessibleName("kylin-calendar-plugin_lunarcalendar_PartLineWidget_line_schedule");
    lineSchedule->setAccessibleDescription("the down line");

    lineUp->setFixedSize(ORIGIN_CALENDAR_WIDTH, 1);
    lineDown->setFixedSize(ORIGIN_CALENDAR_WIDTH, 1);
    lineSchedule->setFixedSize(ORIGIN_CALENDAR_WIDTH, 1);

    CheckResolution();
    updateLayout();

    connect(chk_yijichoose, &QRadioButton::clicked, this, &LunarCalendarWidget::customButtonsClicked);
    const QByteArray style_id(CALENDAR_GSETTINGS_SCHEMAS);
    if (QGSettings::isSchemaInstalled(style_id))
    {
        int num = CalendarDataBase::getInstance().m_markInfoList.count();
        yijiGsettings = new QGSettings(style_id);
        if(yijiGsettings != nullptr)
        {
            bool currentTheme = yijiGsettings->get(YIJISTATUS).toBool();
            chk_yijichoose->setChecked(currentTheme);
            if(currentTheme)
            {
                lbl_yiLabel->setVisible(true);
                lbl_jiLabel->setVisible(true);
                yijistate = true;
            }
            else
            {
                lbl_yiLabel->setVisible(false);
                lbl_jiLabel->setVisible(false);
                yijistate = false;
                Q_EMIT yijiChangeDown(num);
            }
            connect(yijiGsettings, &QGSettings::changed, this, [ = ](const QString & keys)
            {
                if (keys == YIJISTATUS)
                {
                    bool currentTheme = yijiGsettings->get(YIJISTATUS).toBool();
                    chk_yijichoose->setChecked(currentTheme);
                    if(currentTheme)
                    {
                        lbl_yiLabel->setVisible(true);
                        lbl_jiLabel->setVisible(true);
                        yijistate = true;
                    }
                    else
                    {
                        lbl_yiLabel->setVisible(false);
                        lbl_jiLabel->setVisible(false);
                        yijistate = false;
                        Q_EMIT yijiChangeDown(num);
                    }

                }
            });
        }
    }

    widgetWeek->setContentsMargins(16, 0, 16, 0);
    m_widgetDayBody->setContentsMargins(16, 0, 16, 0);
    widgetYearBody->setContentsMargins(16, 0, 16, 0);
    widgetmonthBody->setContentsMargins(16, 0, 16, 0);
    labWidget->setContentsMargins(16, 0, 16, 0);
    yijiWidget->setContentsMargins(16, 0, 16, 0);
    //绑定按钮和下拉框信号
    connect(btnPrevYear, &kdk::KBorderlessButton::clicked, this, [ = ]()
    {
        switch (m_currentType)
        {
        case 0:
            showPreviousMonth(true);
            break;
        case 2:
            changeYear(false);
            break;
        case 1:
        default:
            break;
        }
    });
    connect(btnNextYear, &kdk::KBorderlessButton::clicked, this, [ = ]()
    {
        switch (m_currentType)
        {
        case 0:
            showNextMonth(true);
            break;
        case 2:
            changeYear(true);
            break;
        case 1:
        default:
            break;
        }
    });
    connect(btn_btnToday, SIGNAL(clicked(bool)), this, SLOT(showToday()));
    //connect(this,&LunarCalendarWidget::DayClick,this,&LunarCalendarWidget::CreateSchedule);
    connect(m_createScheduleButton, &QPushButton::clicked, this, &LunarCalendarWidget::slotAddSchedule);
    chk_yijichoose->setProperty("needTranslucent", true);
}

void LunarCalendarWidget::slotShowMore()
{

}

void LunarCalendarWidget::slotAddSchedule()
{
    // 创建新对话框
    CSchceduleDlg *dlg = new CSchceduleDlg(nullptr); // 注意：父对象设为nullptr
    dlg->setAccessibleName("kylin-calendar-plugin_lunarcalendar_Schedule_dialog");
    dlg->setProperty(1, true); // 新增日程
    dlg->setAttribute(Qt::WA_DeleteOnClose); // 关键：自动删除

    // 添加对话框到管理列表
    m_scheduleDialogs.append(dlg);
    connect(dlg, &QObject::destroyed, [this, dlg]() {
        m_scheduleDialogs.removeAll(dlg);
    });

    // 确保布局完成
    dlg->layout()->activate();
    dlg->adjustSize();

    // 获取父窗口所在屏幕
    QWindow* parentWindow = this->window()->windowHandle();
    QScreen* targetScreen = parentWindow ? parentWindow->screen() : QGuiApplication::primaryScreen();
    QRect screenRect = targetScreen->availableGeometry();
    QSize dialogSize = dlg->size();

    // 计算中心位置
    int x = screenRect.left() + (screenRect.width() - dialogSize.width()) / 2;
    int y = screenRect.top() + (screenRect.height() - dialogSize.height()) / 2;
    dlg->move(x, y);

    if(isWayland)
    {
        UkuiWindowHelper *windowHelper = new UkuiWindowHelper(dlg);
        windowHelper->setSkipTaskBar(true);
        windowHelper->setBlurEffect(QRegion(), 0, true);
    }
    else
    {
        kdk::WindowManager::setSkipTaskBar(dlg->windowHandle(), true);
        KWindowSystem::setState(dlg->winId(), NET::SkipTaskbar);
    }

    dlg->show();
    dlg->raise();
    dlg->activateWindow();

    if (isWayland)
    {
        QApplication::processEvents();
        dlg->move(x, y); // 二次定位确保生效
    }
}

void LunarCalendarWidget::slotEditSchedulefromMenu(QString id)
{
    slotEditSchedule(id);
}

void LunarCalendarWidget::slotEditSchedule(QString id)
{
    // 创建新对话框
    CSchceduleDlg *dlg = new CSchceduleDlg(nullptr);
    dlg->setAccessibleName("kylin-calendar-plugin_lunarcalendar_Schedule_dialog");
    dlg->setProperty(0, true); // 编辑日程
    dlg->setId(id);
    dlg->setStatusFromId(id);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    // 添加到管理列表
    m_scheduleDialogs.append(dlg);
    connect(dlg, &QObject::destroyed, [this, dlg]() {
        m_scheduleDialogs.removeAll(dlg);
    });

    // 确保布局完成
    dlg->layout()->activate();
    dlg->adjustSize();

    // 获取父窗口所在屏幕
    QWindow* parentWindow = this->window()->windowHandle();
    QScreen* targetScreen = parentWindow ? parentWindow->screen() : QGuiApplication::primaryScreen();
    QRect screenRect = targetScreen->availableGeometry();
    QSize dialogSize = dlg->size();

    // 计算中心位置
    int x = screenRect.left() + (screenRect.width() - dialogSize.width()) / 2;
    int y = screenRect.top() + (screenRect.height() - dialogSize.height()) / 2;
    dlg->move(x, y);

    if(isWayland)
    {
        UkuiWindowHelper *windowHelper = new UkuiWindowHelper(dlg);
        windowHelper->setSkipTaskBar(true);
        windowHelper->setBlurEffect(QRegion(), 0, true);
    }
    else
    {
        kdk::WindowManager::setSkipTaskBar(dlg->windowHandle(), true);
        KWindowSystem::setState(dlg->winId(), NET::SkipTaskbar);
    }

    dlg->show();
    dlg->raise();
    dlg->activateWindow();

    if (isWayland) {
        QApplication::processEvents();
        dlg->move(x, y); // 二次定位确保生效
    }
}

//设置日历的地区
void LunarCalendarWidget::setLocaleCalendar()
{
#if 0
    QStringList res = getLocale();
    qDebug() << "设置区域:" << res.at(0);
    int isCN = res.at(0) == "zh_CN.UTF-8" ? 1 : 0;
#else
    QString language;
    QString locale;
    QStringList res = getLocale(language, locale);
    qDebug() << language << locale;
    qDebug() << "设置区域:" << res.at(0);
    //int isCN = res.at(0) == "zh_CN.UTF-8" ? 1 : 0;
    bool isCN = language.contains("zh_CN");
#endif
    if (FirstdayisSun)
    {
        lbl_labWeeks.at(0)->setText(tr("Sun"));
        lbl_labWeeks.at(1)->setText(tr("Mon"));
        lbl_labWeeks.at(2)->setText(tr("Tue"));
        lbl_labWeeks.at(3)->setText(tr("Wed"));
        lbl_labWeeks.at(4)->setText(tr("Thur"));
        lbl_labWeeks.at(5)->setText(tr("Fri"));
        lbl_labWeeks.at(6)->setText(tr("Sat"));
    }
    else
    {
        lbl_labWeeks.at(0)->setText(tr("Mon"));
        lbl_labWeeks.at(1)->setText(tr("Tue"));
        lbl_labWeeks.at(2)->setText(tr("Wed"));
        lbl_labWeeks.at(3)->setText(tr("Thur"));
        lbl_labWeeks.at(4)->setText(tr("Fri"));
        lbl_labWeeks.at(5)->setText(tr("Sat"));
        lbl_labWeeks.at(6)->setText(tr("Sun"));
    }
}

//获取指定地区的编号代码
QStringList LunarCalendarWidget::getLocale(QString &language, QString &locale)
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

void LunarCalendarWidget::initStyle()
{
}

void LunarCalendarWidget::analysisWorktimeJs()
{
    /*解析json文件*/
    QFile file("/usr/share/calendar/plugin-calendar/html/jiejiari.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString value = file.readAll();
    file.close();

    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(), &parseJsonErr);
    if(!(parseJsonErr.error == QJsonParseError::NoError))
    {
        qDebug() << tr("解析json文件错误！");
        return;
    }
    QJsonObject jsonObject = document.object();
    QStringList args = jsonObject.keys();

    for (int i = 0; i < args.count(); i++)
    {
        if(jsonObject.contains(args.at(i)))
        {
            QJsonValue jsonValueList = jsonObject.value(args.at(i));
            QJsonObject item = jsonValueList.toObject();
            QStringList arg2 = item.keys();
            for (int j = 0; j < arg2.count(); j++)
            {
                worktimeinside.insert(arg2.at(j), item[arg2.at(j)].toString());
            }
        }
        worktime.insert(args.at(i), worktimeinside);
        worktimeinside.clear();
    }
}

void LunarCalendarWidget::yearWidgetChange()
{
    if (widgetYearBody->isHidden())
    {
        widgetYearBody->show();
        widgetWeek->hide();
        m_widgetDayBody->hide();
        widgetmonthBody->hide();
        changeMode(2);
        m_savedYear = date.year();
        qDebug () << m_savedYear;
    }
    else
    {
        widgetYearBody->hide();
        widgetWeek->show();
        m_widgetDayBody->show();
        widgetmonthBody->hide();
        date.setDate(m_savedYear, date.month(), date.day());
        changeMode(0);
        initDate();
    }

}

void LunarCalendarWidget::monthWidgetChange()
{
    if(widgetmonthBody->isHidden())
    {
        widgetYearBody->hide();
        widgetWeek->hide();
        m_widgetDayBody->hide();
        widgetmonthBody->show();
        changeMode(1);
    }
    else
    {
        widgetYearBody->hide();
        widgetWeek->show();
        m_widgetDayBody->show();
        widgetmonthBody->hide();
        changeMode(0);
    }
}
void LunarCalendarWidget::changeMode(int mode)
{
    m_currentType = mode;
    if(mode == 1 || mode == 2)
    {
        btnNextYear->setEnabled(false);
        btnPrevYear->setEnabled(false);
    }
    else
    {
        btnNextYear->setEnabled(true);
        btnPrevYear->setEnabled(true);
    }
}

//初始化日期面板
void LunarCalendarWidget::initDate()
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if(oneRun)
    {
        downLabelHandle(date);
        yijihandle(date);
        oneRun = false;
    }
    //设置为今天,设置变量防止重复触发
    btnClick = true;
    //cboxYearandMonth->setCurrentIndex(cboxYearandMonth->findText(QString("%1.%2").arg(year).arg(month)));
    btnClick = false;

    lbl_cboxYearandMonthLabel->setAlignment(Qt::AlignCenter);
    if(m_currentType != 2)
    {
        updateYearMonthString(year, month);
    }
    else
    {
        updateYearMonthString(m_savedYear, month);
    }

    //首先判断当前月的第一天是星期几
    int week = LunarCalendarInfo::Instance()->getFirstDayOfWeek(year, month, FirstdayisSun);
    //当前月天数
    int countDay = LunarCalendarInfo::Instance()->getMonthDays(year, month);
    //上月天数
    int countDayPre = LunarCalendarInfo::Instance()->getMonthDays(1 == month ? year - 1 : year, 1 == month ? 12 : month - 1);
    pro = countDayPre;
    //如果上月天数上月刚好一周则另外处理
    int startPre, endPre, startNext, endNext, index, tempYear, tempMonth, tempDay;
    if (0 == week)
    {
        startPre = 0;
        endPre = 7;
        startNext = 0;
        endNext = 42 - (countDay + 7);
    }
    else
    {
        startPre = 0;
        endPre = week;
        startNext = week + countDay;
        endNext = 42;
    }

    //纠正1月份前面部分偏差,1月份前面部分是上一年12月份
    tempYear = year;
    tempMonth = month - 1;
    if (tempMonth < 1)
    {
        tempYear--;
        tempMonth = 12;
    }

    //显示上月天数
    for (int i = startPre; i < endPre; i++)
    {
        index = i;
        tempDay = countDayPre - endPre + i + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = LunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        dayItems.at(index)->setDate(date, lunar, LunarCalendarItem::DayType_MonthPre);
        int x = CalendarDataBase::getInstance().searchDataSlot(dayItems.at(index)->date);
        if(x == DB_OP_SUCC)
        {
            dayItems.at(index)->hasSchedule = true;
        }
        else
        {
            dayItems.at(index)->hasSchedule = false;
        }
        if(date == QDate::currentDate())
        {
            dayItems.at(index)->isToday = true;
        }
        else
        {
            dayItems.at(index)->isToday = false;
        }
        dayItems.at(index)->setFont(m_fontName);

    }

    //纠正12月份后面部分偏差,12月份后面部分是下一年1月份
    tempYear = year;
    tempMonth = month + 1;
    if (tempMonth > 12)
    {
        tempYear++;
        tempMonth = 1;
    }

    //显示下月天数
    for (int i = startNext; i < endNext; i++)
    {
        index = 42 - endNext + i;
        tempDay = i - startNext + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = LunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        dayItems.at(index)->setDate(date, lunar, LunarCalendarItem::DayType_MonthNext);
        int x = CalendarDataBase::getInstance().searchDataSlot(dayItems.at(index)->date);
        if(x == DB_OP_SUCC)
        {
            dayItems.at(index)->hasSchedule = true;
        }
        else
        {
            dayItems.at(index)->hasSchedule = false;
        }
        if(date == QDate::currentDate())
        {
            dayItems.at(index)->isToday = true;
        }
        else
        {
            dayItems.at(index)->isToday = false;
        }
    }

    //重新置为当前年月
    tempYear = year;
    tempMonth = month;
    QFont font(m_fontName);

    //显示当前月
    for (int i = week; i < (countDay + week); i++)
    {
        index = (0 == week ? (i + 7) : i);
        tempDay = i - week + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = LunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        dayItems.at(index)->setDate(date, lunar, LunarCalendarItem::DayType_MonthCurrent);
        int x = CalendarDataBase::getInstance().searchDataSlot(dayItems.at(index)->date);
        if(x == DB_OP_SUCC)
        {
            dayItems.at(index)->hasSchedule = true;
        }
        else
        {
            dayItems.at(index)->hasSchedule = false;
        }
        if(date == QDate::currentDate())
        {
            dayItems.at(index)->isToday = true;
        }
        else
        {
            dayItems.at(index)->isToday = false;
        }
    }

    for (int i = 0; i < 12; i++)
    {
        monthItems.at(i)->setDate(QDate(date.year(), i + 1, 1));
        yearItems.at(i)->setDate(date.addYears(i));
        yearItems.at(i)->setFont(font);
        monthItems.at(i)->setFont(font);
    }
    auto it = dayItems.begin();
    for(; it != dayItems.end(); it++)
    {
        (*it)->setFont(font);
    }

    auto itw = lbl_labWeeks.begin();
    QFont f(m_fontName);
    f.setPixelSize(14);
    for(; itw != lbl_labWeeks.end(); itw++)
    {
        (*itw)->setFont(f);
    }

    lbl_labBottom->repaint();

    lbl_yijichooseLabel->repaint();

    yijiWidget->repaint();
    QFont smallfont = m_font;
    if(QLocale::system().name() != "zh_CN")
    {
        smallfont.setPointSize(8);
    }
    btn_btnYear->repaint();
    btn_btnToday->repaint();
    btn_btnMonth->repaint();
    m_widgetTop->repaint();
    lbl_cboxYearandMonthLabel->repaint();
}

void LunarCalendarWidget::customButtonsClicked(bool checked)
{
    int num = CalendarDataBase::getInstance().m_markInfoList.count();
    lbl_yiLabel->setVisible(checked);
    lbl_jiLabel->setVisible(checked);
    yijistate = checked;
    if (yijiGsettings)
    {
        yijiGsettings->set(YIJISTATUS, QVariant(checked));
    }
    if(checked)
    {
        QPoint currentPos = this->pos();
        //2种情况1.目前屏幕大小 740 <x< 740+75 2.屏幕高度 x <740
        if(LESSTWOSHCEDULE_CALENDAR_HEIGHT < m_nPanelHeight && m_nPanelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT)
        {
            ScrollAreaNeedChange = true;
            updateLayout();
            return;
        }
        else if(m_nPanelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT)
        {
            ScrollbarUnderTheResolution = true;
            if(m_nPanelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT - SCROLLA)
            {
                ScrollbarOfMinimumResolution = true;
            }
            else
            {
                ScrollbarOfMinimumResolution = false;
            }
            updateLayout();
            return;
        }
        //当位置在上方的时候 不需要move，只需要改变高度即可
        if(panelPosition != 1)
        {
            this->move(currentPos.x(), currentPos.y() - YIJI_HEIGHT);
        }
        QTimer::singleShot(5, [&]()
        {
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, this->height() + YIJI_HEIGHT);
        });
    }
    else
    {
        if(LESSTWOSHCEDULE_CALENDAR_HEIGHT < m_nPanelHeight && m_nPanelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT)
        {
            ScrollAreaNeedChange = false;
            updateLayout();
            return;
        }
        else if(m_nPanelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT)
        {
            ScrollbarUnderTheResolution = true;
            if(m_nPanelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT - SCROLLA)
            {
                ScrollbarOfMinimumResolution = true;
            }
            else
            {
                ScrollbarOfMinimumResolution = false;
            }
            updateLayout();
            return;
        }
        QPoint currentPos = this->pos();

        //        this->setFixedSize(ORIGIN_CALENDAR_WIDTH,this->height()-YIJI_HEIGHT);
        //当位置在上方的时候 不需要move，只需要改变高度即可
        if(panelPosition != 1)
        {
            this->move(currentPos.x(), currentPos.y() + YIJI_HEIGHT);
        }
        QTimer::singleShot(5, [&]()
        {
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, this->height() - YIJI_HEIGHT);
        });
    }
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    //传入事件描述“打开文件”，所在界面“主界面”后，上传事件
    kdk_dia_upload_default(node, "kylin_calendar_yiji_changed", "MainWindow");
    //释放内存
    kdk_dia_data_free(node);
}

QString LunarCalendarWidget::getSettings()
{
    QString arg = "配置文件";
    return  arg;

}

void LunarCalendarWidget::setSettings(QString arg)
{

}

void LunarCalendarWidget::downLabelHandle(const QDate &date)
{

    QString strHoliday;
    QString strSolarTerms;
    QString strLunarFestival;
    QString strLunarYear;
    QString strLunarMonth;
    QString strLunarDay;

    LunarCalendarInfo::Instance()->getLunarCalendarInfo(date.year(),
                                                        date.month(),
                                                        date.day(),
                                                        strHoliday,
                                                        strSolarTerms,
                                                        strLunarFestival,
                                                        strLunarYear,
                                                        strLunarMonth,
                                                        strLunarDay);

    QString lbl_labBottomarg = strLunarYear + "  " + strLunarMonth + strLunarDay;
    lbl_labBottom->setText(lbl_labBottomarg);

}

void LunarCalendarWidget::yijihandle(const QDate &date)
{
    QString yi = "宜：";
    QString ji = "忌：";
    /*解析json文件*/
    QFile file(QString("/usr/share/calendar/plugin-calendar/html/hlnew/hl%1.json").arg(date.year()));
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString value = file.readAll();
    file.close();

    QJsonParseError parseJsonErr;
    QJsonDocument document = QJsonDocument::fromJson(value.toUtf8(), &parseJsonErr);
    if(!(parseJsonErr.error == QJsonParseError::NoError))
    {
        qDebug() << tr("解析json文件错误！");
        lbl_yiLabel->setText(yi);
        lbl_jiLabel->setText(ji);
        return;
    }
    QJsonObject jsonObject = document.object();

    if(jsonObject.contains(QString("d%1").arg(date.toString("MMdd"))))
    {
        QJsonValue jsonValueList = jsonObject.value(QString("d%1").arg(date.toString("MMdd")));
        QJsonObject item = jsonValueList.toObject();
        QString yiString;
        QString jiString;
        if (item["y"].toString() == "." || item["y"].isNull())
        {
            yiString = yi;
        }
        else
        {
            yiString = yi + item["y"].toString();
        }
        if (item["j"].toString() == "." || item["j"].isNull())
        {
            jiString = ji;
        }
        else
        {
            jiString = ji + item["j"].toString();
        }

        lbl_yiLabel->setText(toHK(yiString));
        lbl_jiLabel->setText(toHK(jiString));
    }
}

QString LunarCalendarWidget::toHK(QString str)
{
    QString result;
    for (int i = 0; i < str.length(); i++)
    {
        QString tmp = str.at(i);
        if (m_zh_HK_dict.keys().contains(tmp))
        {
            tmp = m_zh_HK_dict.value(tmp);
        }
        result.append(tmp);
    }
    return result;
}

void LunarCalendarWidget::yearChanged(const QString &arg1)
{
    //如果是单击按钮切换的日期变动则不需要触发
    if (btnClick)
    {
        return;
    }
    int nIndex = arg1.indexOf(".");
    if(-1 == nIndex)
    {
        return;
    }
    int year = arg1.mid(0, nIndex).toInt();
    int month = arg1.mid(nIndex + 1).toInt();
    int day = date.day();
    dateChanged(year, month, day);
}

void LunarCalendarWidget::monthChanged(const QString &arg1)
{
    //如果是单击按钮切换的日期变动则不需要触发
    if (btnClick)
    {
        return;
    }

    int year = date.year();
    int month = arg1.mid(0, arg1.length()).toInt();
    int day = date.day();
    dateChanged(year, month, day);
}

void LunarCalendarWidget::labClicked(const QDate &date, const LunarCalendarItem::DayType &dayType)
{
    this->date = date;

    s_clickDate = date;
    if (LunarCalendarItem::DayType_MonthPre == dayType)
    {
        showPreviousMonth(false);
    }
    else if (LunarCalendarItem::DayType_MonthNext == dayType)
    {
        showNextMonth(false);
    }
    else
    {
        slotUpdateSchcedule();
        dayChanged(this->date, s_clickDate);

    }

}

void LunarCalendarWidget::updateYearClicked(const QDate &date, const LunarCalendarYearItem::DayType &dayType)
{
    //通过传来的日期，设置当前年月份
    this->date = date;
    widgetYearBody->hide();
    widgetWeek->show();
    m_widgetDayBody->show();
    widgetmonthBody->hide();
    s_clickDate = date;
    changeDate(date);
    dayChanged(date, QDate());
    changeMode(0);
    initDate();
    slotUpdateSchcedule();
}

void LunarCalendarWidget::updateMonthClicked(const QDate &date, const LunarCalendarMonthItem::DayType &dayType)
{
    //通过传来的日期，设置当前年月份
    this->date = date;
    widgetYearBody->hide();
    widgetWeek->show();
    m_widgetDayBody->show();
    widgetmonthBody->hide();
    s_clickDate = date;
    changeDate(date);
    dayChanged(date, QDate());
    changeMode(0);
    initDate();
    slotUpdateSchcedule();
}

void LunarCalendarWidget::changeDate(const QDate &date)
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if(oneRun)
    {
        downLabelHandle(date);
        yijihandle(date);
        oneRun = false;
    }

    updateYearMonthString(year, month);
    //首先判断当前月的第一天是星期几
    int week = LunarCalendarInfo::Instance()->getFirstDayOfWeek(year, month, FirstdayisSun);
    //当前月天数
    int countDay = LunarCalendarInfo::Instance()->getMonthDays(year, month);
    //上月天数
    int countDayPre = LunarCalendarInfo::Instance()->getMonthDays(1 == month ? year - 1 : year, 1 == month ? 12 : month - 1);

    //如果上月天数上月刚好一周则另外处理
    int startPre, endPre, startNext, endNext, index, tempYear, tempMonth, tempDay;
    if (0 == week)
    {
        startPre = 0;
        endPre = 7;
        startNext = 0;
        endNext = 42 - (countDay + 7);
    }
    else
    {
        startPre = 0;
        endPre = week;
        startNext = week + countDay;
        endNext = 42;
    }

    //纠正1月份前面部分偏差,1月份前面部分是上一年12月份
    tempYear = year;
    tempMonth = month - 1;
    if (tempMonth < 1)
    {
        tempYear--;
        tempMonth = 12;
    }

    //显示上月天数
    for (int i = startPre; i < endPre; i++)
    {
        index = i;
        tempDay = countDayPre - endPre + i + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = LunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        dayItems.at(index)->setDate(date, lunar, LunarCalendarItem::DayType_MonthPre);
    }

    //纠正12月份后面部分偏差,12月份后面部分是下一年1月份
    tempYear = year;
    tempMonth = month + 1;
    if (tempMonth > 12)
    {
        tempYear++;
        tempMonth = 1;
    }

    //显示下月天数
    for (int i = startNext; i < endNext; i++)
    {
        index = 42 - endNext + i;
        tempDay = i - startNext + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = LunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        dayItems.at(index)->setDate(date, lunar, LunarCalendarItem::DayType_MonthNext);
    }

    //重新置为当前年月
    tempYear = year;
    tempMonth = month;

    //显示当前月
    for (int i = week; i < (countDay + week); i++)
    {
        index = (0 == week ? (i + 7) : i);
        tempDay = i - week + 1;

        QDate date(tempYear, tempMonth, tempDay);
        QString lunar = LunarCalendarInfo::Instance()->getLunarDay(tempYear, tempMonth, tempDay);
        dayItems.at(index)->setDate(date, lunar, LunarCalendarItem::DayType_MonthCurrent);
    }

    for (int i = 0; i < 12; i++)
    {
        yearItems.at(i)->setDate(s_clickDate.addYears(i));
        monthItems.at(i)->setDate(QDate(s_clickDate.year(), i + 1, 1));
    }
}

void LunarCalendarWidget::dayChanged(const QDate &date, const QDate &m_date)
{
    //计算星期几,当前天对应标签索引=日期+星期几-1
    int year = date.year();
    int month = date.month();
    int day = date.day();
    int week = LunarCalendarInfo::Instance()->getFirstDayOfWeek(year, month, FirstdayisSun);
    //选中当前日期,其他日期恢复,这里还有优化空间,比方说类似单选框机制
    for (int i = 0; i < 42; i++)
    {
        //当月第一天是星期天要另外计算
        int index = day + week - 1;
        if (week == 0)
        {
            index = day + 6;
        }
        dayItems.at(i)->setSelect(false);
        if(dayItems.at(i)->getDate() == m_date)
        {
            dayItems.at(i)->setSelect(i == index);
        }
        if (i == index)
        {
            downLabelHandle(dayItems.at(i)->getDate());
            yijihandle(dayItems.at(i)->getDate());
        }
    }

    //发送日期单击信号
    Q_EMIT clicked(date);
    //发送日期更新信号
    Q_EMIT selectionChanged();
}

void LunarCalendarWidget::dateChanged(int year, int month, int day)
{
    if(year <= 1970 || year >= 2099)
    {
        return ;
    }
    if(month < 1 || month > 12)
    {
        return;
    }

    QDate m_date(year, month, 1);
    int maxDay = m_date.daysInMonth();
    if(day > maxDay)
    {
        date.setDate(year, month, 1);
    }
    else
    {
        date.setDate(year, month, day);
    }
    initDate();
}

bool LunarCalendarWidget::getShowLunar()
{
    //语言是简体或繁体中文且区域是中国且GSetting的值时，
    //这个时候才显示农历和黄历信息
    QString lang, format;
    getLocale(lang, format);
    //qInfo() << lang << format << (calendar_gsettings->get("calendar").toString() == "lunar") ;
    if((lang.contains("zh_CN") || lang.contains("zh_HK")) &&
            (calendar_gsettings->get("calendar").toString() == "lunar"))
    {
        m_showLunar = true;
    }
    else
    {
        m_showLunar = false;
    }
    return m_showLunar;
}

void LunarCalendarWidget::setShowLunar(bool showLunar)
{
    bool realShow = showLunar & yijistate;
    //日界面需要重绘
    for(auto i = 0 ; i < dayItems.size(); i++)
    {
        dayItems.at(i)->setShowLunar(showLunar);
    }
    //日期字符串要重新写
    lunarstate = realShow;
    //黄历信息不能再显示了
    lbl_yiLabel->setVisible(realShow);
    lbl_jiLabel->setVisible(realShow);
    //yijistate = realShow;
    labWidget->setVisible(showLunar);
    yijiWidget->setVisible(showLunar);

    //    m_ScheculeDlg->m_lunarcheckBox->setVisible(showLunar);
    //    m_ScheculeDlg->m_lunarLabel->setVisible(showLunar);
    //    chk_yijichoose->setChecked(yijistate);
    //重新布局当前窗口
    Q_EMIT almanacChanged(realShow);
}

//显示上一年
void LunarCalendarWidget::showPreviousYear()
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if (year <= 1901)
    {
        return;
    }

    year--;
    dateChanged(year, month, day);
}

//显示下一年
void LunarCalendarWidget::showNextYear()
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if (year >= 2099)
    {
        return;
    }

    year++;
    dateChanged(year, month, day);
}

//显示上月日期
void LunarCalendarWidget::showPreviousMonth(bool date_clicked)
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if (year <= 1901 && month == 1)
    {
        return;
    }

    //extra:
    if (date_clicked) month--;
    if (month < 1)
    {
        month = 12;
        year--;
    }
    dateChanged(year, month, day);
    //    QDate change_To_Date(year, month, day);
    //    downLabelHandle(change_To_Date);
    //    yijihandle(change_To_Date);
}

//显示下月日期
void LunarCalendarWidget::showNextMonth(bool date_clicked)
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if (year >= 2099 )
    {
        return;
    }

    //extra
    if (date_clicked)month++;
    if (month > 12)
    {
        month = 1;
        year++;
    }

    dateChanged(year, month, day);
}

void LunarCalendarWidget::setBool(bool more)
{
    isMore = more;
}

bool LunarCalendarWidget::getBool()
{
    return isMore;
}

void LunarCalendarWidget::calculateWindowGeometry()
{
    QScreen *targetScreen = m_pScreen ? m_pScreen : QGuiApplication::primaryScreen();
    if (!targetScreen) return;

    // 获取面板设置
    if (!m_panelGsettings)
    {
        m_panelGsettings = new QGSettings(PANEL_SETTINGS);
    }
    panelPosition = m_panelGsettings->get(PANEL_POSITION_KEY).toInt();
    m_nPanelSize = m_panelGsettings->get(PANEL_SIZE_KEY).toInt();

    // 获取屏幕几何信息
    QRect screenGeometry = targetScreen->geometry();

    UkuiWindowHelper *windowHelper = new UkuiWindowHelper(this);
    windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::SystemWindow);
    QScreen* pScreen = windowHelper->getCurrentScreenBlocking();
    if(!pScreen)
    {
        pScreen = QApplication::primaryScreen();
    }
    m_nScreenHeight = pScreen->size().height() + pScreen->geometry().y();
    m_nPanelHeight = m_nScreenHeight - m_nPanelSize; //这个值就是屏幕到任务栏的高度

    int windowWidth = this->width();
    int windowHeight = this->height();

    // 调整窗口高度（如果需要）
    if (screenGeometry.height() < windowHeight || isExceedScreenHeight) {
        windowHeight = screenGeometry.height() - SCROLLA;
    }

    // 计算窗口位置（使用相对坐标）
    QPoint targetPoint;
    switch (panelPosition) {
    case 0: // 底部任务栏
        targetPoint = QPoint(screenGeometry.right() - windowWidth - WIDGET_GAP, screenGeometry.bottom() - m_nPanelSize - windowHeight - WIDGET_GAP);
        break;
    case 1: // 顶部任务栏
        targetPoint = QPoint(screenGeometry.right() - windowWidth - WIDGET_GAP, screenGeometry.top() + m_nPanelSize + WIDGET_GAP);
        break;
    case 2: // 左侧任务栏
        targetPoint = QPoint(screenGeometry.left() + m_nPanelSize + WIDGET_GAP, screenGeometry.bottom() - windowHeight - WIDGET_GAP);
        break;
    case 3: // 右侧任务栏
        targetPoint = QPoint(screenGeometry.right() - windowWidth - m_nPanelSize - WIDGET_GAP, screenGeometry.bottom() - windowHeight - WIDGET_GAP);
        break;
    default:
        targetPoint = this->pos();
        break;
    }

    // 确保位置在屏幕范围内
    targetPoint.setX(qBound(screenGeometry.left(), targetPoint.x(), screenGeometry.right() - windowWidth));
    targetPoint.setY(qBound(screenGeometry.top(), targetPoint.y(), screenGeometry.bottom() - windowHeight));

    // 应用样式
    applyStyleSettings();

    // 设置窗口位置
    if (isWayland)
    {
        this->move(targetPoint);
    }
    else
    {
        kdk::WindowManager::setGeometry(windowHandle(), QRect(targetPoint, QSize(windowWidth, windowHeight)));
    }
}

void LunarCalendarWidget::applyStyleSettings()
{
    if (!style_settings) {
        style_settings = new QGSettings(ORG_UKUI_STYLE);
    }

    QString strCurTheme = style_settings->get("styleName").toString();
    QString styleSheet = "color: %1;";

    if (strCurTheme == "ukui-default" || strCurTheme == "ukui-light") {
        QString lightColor = "rgb(29, 29, 29)";
        lbl_labBottom->setStyleSheet(styleSheet.arg(lightColor));
        lbl_jiLabel->setStyleSheet(styleSheet.arg(lightColor));
        lbl_yiLabel->setStyleSheet(styleSheet.arg(lightColor));
        lbl_yijichooseLabel->setStyleSheet(styleSheet.arg(lightColor));
        chk_yijichoose->setStyleSheet(styleSheet.arg(lightColor));
        m_scheduleWidget->setStyleSheet(styleSheet.arg(lightColor));
    } else if (strCurTheme == "ukui-dark") {
        QString darkColor = "rgb(255, 255, 255)";
        lbl_labBottom->setStyleSheet(styleSheet.arg(darkColor));
        lbl_jiLabel->setStyleSheet(styleSheet.arg(darkColor));
        lbl_yiLabel->setStyleSheet(styleSheet.arg(darkColor));
        lbl_yijichooseLabel->setStyleSheet(styleSheet.arg(darkColor));
        chk_yijichoose->setStyleSheet(styleSheet.arg(darkColor));
        m_scheduleWidget->setStyleSheet(styleSheet.arg(darkColor));
    }
}

void LunarCalendarWidget::setScreen(QScreen *screen)
{
    m_pScreen = screen;
}

void LunarCalendarWidget::updateAllSchedule()
{
    CalendarDataBase::getInstance().searchDataSlot(date);
    m_markInfoList = CalendarDataBase::getInstance().getInfoList();
    m_markIdList = CalendarDataBase::getInstance().getIdList();
    m_num   = m_markInfoList.count();

    Q_EMIT scheduleNum(m_num); ;
    // 遍历所有 MarkInfo，提取开始小时和分钟
    QList<int> startTimes;
    for (MarkInfo info : m_markInfoList)
    {
        int startTime = info.hour * 60 + info.minute; // 将小时转换为分钟并加上分钟数
        startTimes.append(startTime);
    }

    // 使用提取的开始时间对 m_markInfoList 进行排序
    QList<int> sortedIndices;
    for (int i = 0; i < startTimes.size(); ++i)
        sortedIndices.append(i);

    std::sort(sortedIndices.begin(), sortedIndices.end(), [&](int a, int b)
    {
        return startTimes[a] < startTimes[b];
    });

    QList<MarkInfo> sortedMarkInfoList;
    for (int index : sortedIndices)
        sortedMarkInfoList.append(m_markInfoList[index]);

    m_markInfoList = sortedMarkInfoList;
    twiceInfo = m_markInfoList.mid(0, 2);
    remainInfo = m_markInfoList.mid(2);

}
bool LunarCalendarWidget::compare(MarkInfo info1, MarkInfo info2)
{
    if(info1.hour != info2.hour)
    {
        return info1.hour  < info2.hour;
    }
    return info1.minute < info2.minute;
}
//转到今天
void LunarCalendarWidget::showToday()
{
    widgetYearBody->hide();
    widgetmonthBody->hide();
    m_widgetDayBody->show();
    widgetWeek->show();
    date = QDate::currentDate();
    changeMode(0);
    initDate();
    s_clickDate = date;
    dayChanged(this->date, QDate());
    CalendarDataBase::getInstance().searchDataSlot(date);
    slotUpdateSchcedule();
}

void LunarCalendarWidget::setCalendarStyle(const LunarCalendarWidget::CalendarStyle &calendarStyle)
{
    if (this->calendarStyle != calendarStyle)
    {
        this->calendarStyle = calendarStyle;
    }
}

void LunarCalendarWidget::setWeekNameFormat(bool FirstDayisSun)
{
    FirstdayisSun = FirstDayisSun;
    setLocaleCalendar();//在设置是从某星期作为起点时,再判断某区域下日历的显示
    initDate();
}

void LunarCalendarWidget::setDate(const QDate &date)
{
    if (this->date != date)
    {
        this->date = date;
        initDate();
    }
}

void LunarCalendarWidget::setWeekTextColor(const QColor &weekTextColor)
{
    if (this->weekTextColor != weekTextColor)
    {
        this->weekTextColor = weekTextColor;
        initStyle();
    }
}

void LunarCalendarWidget::setWeekBgColor(const QColor &weekBgColor)
{
    if (this->weekBgColor != weekBgColor)
    {
        this->weekBgColor = weekBgColor;
        initStyle();
    }
}

void LunarCalendarWidget::setBgImage(const QString &bgImage)
{
    if (this->bgImage != bgImage)
    {
        this->bgImage = bgImage;
        initStyle();
    }
}

void LunarCalendarWidget::setSelectType(const LunarCalendarWidget::SelectType &selectType)
{
    if (this->selectType != selectType)
    {
        this->selectType = selectType;
        initStyle();
    }
}

void LunarCalendarWidget::setBorderColor(const QColor &borderColor)
{
    if (this->borderColor != borderColor)
    {
        this->borderColor = borderColor;
        initStyle();
    }
}

void LunarCalendarWidget::setWeekColor(const QColor &weekColor)
{
    if (this->weekColor != weekColor)
    {
        this->weekColor = weekColor;
        initStyle();
    }
}

void LunarCalendarWidget::setSuperColor(const QColor &superColor)
{
    if (this->superColor != superColor)
    {
        this->superColor = superColor;
        initStyle();
    }
}

void LunarCalendarWidget::setLunarColor(const QColor &lunarColor)
{
    if (this->lunarColor != lunarColor)
    {
        this->lunarColor = lunarColor;
        initStyle();
    }
}

void LunarCalendarWidget::setCurrentTextColor(const QColor &currentTextColor)
{
    if (this->currentTextColor != currentTextColor)
    {
        this->currentTextColor = currentTextColor;
        initStyle();
    }
}

void LunarCalendarWidget::setOtherTextColor(const QColor &otherTextColor)
{
    if (this->otherTextColor != otherTextColor)
    {
        this->otherTextColor = otherTextColor;
        initStyle();
    }
}

void LunarCalendarWidget::setSelectTextColor(const QColor &selectTextColor)
{
    if (this->selectTextColor != selectTextColor)
    {
        this->selectTextColor = selectTextColor;
        initStyle();
    }
}

void LunarCalendarWidget::setHoverTextColor(const QColor &hoverTextColor)
{
    if (this->hoverTextColor != hoverTextColor)
    {
        this->hoverTextColor = hoverTextColor;
        initStyle();
    }
}

void LunarCalendarWidget::setCurrentLunarColor(const QColor &currentLunarColor)
{
    if (this->currentLunarColor != currentLunarColor)
    {
        this->currentLunarColor = currentLunarColor;
        initStyle();
    }
}

void LunarCalendarWidget::setOtherLunarColor(const QColor &otherLunarColor)
{
    if (this->otherLunarColor != otherLunarColor)
    {
        this->otherLunarColor = otherLunarColor;
        initStyle();
    }
}

void LunarCalendarWidget::setSelectLunarColor(const QColor &selectLunarColor)
{
    if (this->selectLunarColor != selectLunarColor)
    {
        this->selectLunarColor = selectLunarColor;
        initStyle();
    }
}

void LunarCalendarWidget::setHoverLunarColor(const QColor &hoverLunarColor)
{
    if (this->hoverLunarColor != hoverLunarColor)
    {
        this->hoverLunarColor = hoverLunarColor;
        initStyle();
    }
}

void LunarCalendarWidget::setCurrentBgColor(const QColor &currentBgColor)
{
    if (this->currentBgColor != currentBgColor)
    {
        this->currentBgColor = currentBgColor;
        initStyle();
    }
}

void LunarCalendarWidget::setOtherBgColor(const QColor &otherBgColor)
{
    if (this->otherBgColor != otherBgColor)
    {
        this->otherBgColor = otherBgColor;
        initStyle();
    }
}

void LunarCalendarWidget::setSelectBgColor(const QColor &selectBgColor)
{
    if (this->selectBgColor != selectBgColor)
    {
        this->selectBgColor = selectBgColor;
        initStyle();
    }
}

void LunarCalendarWidget::setHoverBgColor(const QColor &hoverBgColor)
{
    if (this->hoverBgColor != hoverBgColor)
    {
        this->hoverBgColor = hoverBgColor;
        initStyle();
    }
}
void LunarCalendarWidget::changeMonth(bool forward)
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if (year <= 1901 && month == 1)
    {
        return;
    }
    if(forward)
    {
        month += 3;
        if(month > 12)
        {
            year += 1;
            month -= 12;
        }
    }
    else
    {
        month -= 3;
        if(month <= 0)
        {
            year -= 1;
            month += 12;
        }
    }

    dateChanged(year, month, day);
}
void LunarCalendarWidget::changeYear(bool forward)
{
    int year = date.year();
    int month = date.month();
    int day = date.day();
    if (year <= 1901 || year >= 2099)
    {
        return;
    }

    if(forward)
        year += 3;
    else
        year -= 3;

    dateChanged(year, month, day);
}
void LunarCalendarWidget::wheelEvent(QWheelEvent *event)
{

    if (scr_srcollAreaOfSchedule->underMouse())
    {
        if (scr_srcollAreaOfSchedule->TopOrButtom)
        {
            event->ignore();
            return;
        }
    }

    switch (m_currentType)
    {
    case 0:
        if (event->delta() > 100)
        {
            showPreviousMonth();
        }
        else if (event->delta() < -100)
        {
            showNextMonth();
        }
        break;

    case 2:
        if (event->delta() > 100)
        {
            //向前三年
            changeYear(false);
        }
        else if (event->delta() < -100)
        {
            //向后三年
            changeYear(true);
        }
        break;
    default:

    case 1:
        break;
    }
    event->accept();
    //    return QWidget::wheelEvent(event);

}

bool  LunarCalendarWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouse = static_cast<QMouseEvent*>(event);
        if(mouse != nullptr)
        {
            if(watched == this)
            {
                if(mouse->source() == Qt::MouseEventSynthesizedByQt)
                {
                    m_start_y = mouse->pos().y();
                }
                if(m_currentType == 0)
                {
                    QPoint pt = m_widgetDayBody->mapFromGlobal(QCursor::pos());
                    for(auto w : dayItems)
                    {
                        if(w->geometry().contains(pt))
                            m_clickedWidget = w;

                    }
                }
                else if(m_currentType == 2)
                {
                    QPoint pt = widgetYearBody->mapFromGlobal(QCursor::pos());
                    for(auto w : yearItems)
                    {
                        if(w->geometry().contains(pt))
                            m_clickedWidget = w;

                    }
                }
                else if(m_currentType == 1)
                {
                    QPoint pt = widgetmonthBody->mapFromGlobal(QCursor::pos());
                    for(auto w : monthItems)
                    {
                        if(w->geometry().contains(pt))
                            m_clickedWidget = w;
                    }
                }
                return false;
            }
        }
    }
    else if(event->type() == QEvent::MouseButtonRelease)
    {
        //return -1;
        QMouseEvent *mouse = static_cast<QMouseEvent*>(event);
        if(mouse != nullptr)
        {
            if(watched == this)
            {
                if(mouse->source() == Qt::MouseEventSynthesizedByQt)
                {
                    //触摸的释放
                    if(m_start_y == -1)
                        return false;

                    if( mouse->pos().y() - m_start_y > 100)
                    {
                        switch(m_currentType)
                        {
                        case 0:
                            showNextMonth();
                            break;
                        case 2:
                            changeYear(true);
                            break;
                        }
                        return false;
                    }
                    else if(mouse->pos().y() - m_start_y < -100)
                    {
                        switch(m_currentType)
                        {
                        case 0:
                            showPreviousMonth();
                            break;
                        case 2:
                            changeYear(false);
                            break;
                        }
                        return false;
                    }
                }
                //鼠标抬起
                if(m_clickedWidget == nullptr)
                {
                    return false;
                }
                if(mouse->button() == Qt::LeftButton && m_clickedWidget->geometry().contains(m_widgetDayBody->mapFromGlobal(QCursor::pos())))
                {
                    LunarCalendarItem *item = qobject_cast<LunarCalendarItem*>(m_clickedWidget);
                    if(dayItems.contains(item))
                        item->onSelected();

                    LunarCalendarYearItem *yItem = qobject_cast<LunarCalendarYearItem*>(m_clickedWidget);
                    if(yearItems.contains(yItem))
                        yItem->onSelected();

                    LunarCalendarMonthItem *mItem = qobject_cast<LunarCalendarMonthItem*>(m_clickedWidget);
                    if(monthItems.contains(mItem))
                        mItem->onSelected();

                }
                return false;
            }
        }
    }
    else if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouse = static_cast<QMouseEvent*>(event);
        if(mouse != nullptr)
        {
            if(mouse->source() == Qt::MouseEventSynthesizedByQt)
            {
                return -1;
            }
        }
        if(m_currentType == 0)
        {
            for(auto w : dayItems)
            {
                if(w->geometry().contains(m_widgetDayBody->mapFromGlobal(QCursor::pos())))
                {
                    w->onEnter();
                }
                else
                {
                    w->onLeave();
                }
            }
        }
        else if(m_currentType == 2)
        {
            for(auto w : yearItems)
            {
                if(w->geometry().contains(widgetYearBody->mapFromGlobal(QCursor::pos())))
                {
                    w->onEnter();
                }
                else
                {
                    w->onLeave();
                }
            }
        }
        else if(m_currentType == 1)
        {
            for(auto w : monthItems)
            {
                if(w->geometry().contains(widgetmonthBody->mapFromGlobal(QCursor::pos())))
                {
                    w->onEnter();
                }
                else
                {
                    w->onLeave();
                }
            }
        }
        return false;
    }
    else if(event->type() == QEvent::Enter)
    {
        LunarCalendarItem *item = qobject_cast<LunarCalendarItem*>(watched);
        if(dayItems.contains(item) == true)
        {
            item->onEnter();
        }
        LunarCalendarYearItem *yItem = qobject_cast<LunarCalendarYearItem*>(watched);
        if(yearItems.contains(yItem))
        {
            yItem->onEnter();
        }
    }
    else if(event->type() == QEvent::Leave)
    {
        for(auto day : dayItems)
            day->onLeave();
        for (auto month : monthItems)
            month->onLeave();
        for(auto year : yearItems)
            year->onLeave();

    }
    else if(event->type() == QEvent::WindowDeactivate)
    {
        Q_EMIT NeedToHide();
        Q_EMIT updateSlideEffect();
        timer->stop();
        this->hide();
        return false;
    }

    return false;
}

void LunarCalendarWidget::showCalendar(bool needShowToday)
{
    if(needShowToday)
    {
        showToday();
    }
    this->show();
    this->raise();
    this->activateWindow();
}

void LunarCalendarWidget::initTransparency()
{
    const QByteArray tranparency(TRANSPARENCY_SETTING);
    if (QGSettings::isSchemaInstalled(tranparency))
    {
        m_transparencySetting = new QGSettings(tranparency);
        if(m_transparencySetting != nullptr)
        {
            m_trans = m_transparencySetting->get(TRANSPARENCY_KEY).toDouble() * 255;
            update();
            connect(m_transparencySetting, &QGSettings::changed, this, [ = ](const QString & key)
            {
                if(key == TRANSPARENCY_KEY)
                {
                    m_trans = m_transparencySetting->get(TRANSPARENCY_KEY).toDouble() * 255;
                    update();
                }
            });
        }
    }
    else
    {
        m_trans = 0.75;
        update();
    }
}
void LunarCalendarWidget::paintEvent(QPaintEvent*e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    QColor color = palette().color(QPalette::Base);
    color.setAlpha(m_trans);
    QBrush brush = QBrush(color);
    p.setBrush(brush);
    p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    p.setPen(Qt::NoPen);
    //    p.drawRoundedRect(opt.rect,6,6);
    p.drawRect(opt.rect);
    return QWidget::paintEvent(e);
}
PartLineWidget::PartLineWidget(QWidget *parent) : QWidget(parent)
{

}

void PartLineWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);

    QRect rect = this->rect();

    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QColor color = qApp->palette().color(QPalette::BrightText);
    p.setBrush(color);
    p.setOpacity(0.08);
    p.setPen(Qt::NoPen);
    p.drawRoundedRect(rect, 0, 0);

    QWidget::paintEvent(event);
}

//快捷键
void LunarCalendarWidget::keyPressEvent(QKeyEvent *ev)
{
    if(ev->key() == Qt::Key_Escape)
    {
        Q_EMIT updateSlideEffect();
        this->hide();
    }
    QWidget::keyPressEvent(ev);
}

void LunarCalendarWidget::showEvent(QShowEvent *event)
{
    updateFormat("");

    // 确保窗口尺寸已稳定
    QTimer::singleShot(0, this, [this]() {
        calculateWindowGeometry();
    });

    if(timer)
    {
        timerUpdate();
        timer->start();
    }

    // 激活所有存在的对话框
    for (CSchceduleDlg* dlg : m_scheduleDialogs)
    {
        if (dlg->isMinimized())
        {
            dlg->showNormal(); // 恢复最小化窗口
        }
        dlg->raise();
        dlg->activateWindow();
    }

    QWidget::showEvent(event);
}

void LunarCalendarWidget::hideEvent(QHideEvent *event)
{
    if(timer && timer->isActive())
    {
        timer->stop();
    }
}

void LunarCalendarWidget::closeEvent(QCloseEvent *event)
{
    if(timer && timer->isActive())
    {
        timer->stop();
    }
}

void LunarCalendarWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (isVisible()) {
        calculateWindowGeometry();
    }
}

void LunarCalendarWidget::updateYearMonthString(int year, int month)
{
    if(lbl_cboxYearandMonthLabel == nullptr)
    {
        qInfo() << "widget is not initialized...";
        return ;
    }

    QDate date(year, month, 1);
    QString shortFmt = SDK2CPP(kdk_system_get_shortformat)();
    shortFmt.replace("d", "");
    if(shortFmt.lastIndexOf(".") != -1)
    {
        shortFmt.replace("..", ".");
    }
    else if(shortFmt.lastIndexOf("-") != -1)
    {
        shortFmt.replace("--", "-");
    }
    else if(shortFmt.lastIndexOf("/") != -1)
    {
        shortFmt.replace("//", "/");
    }
    else
    {
        qInfo() << "Wrong date format...";
        return ;
    }
    if(shortFmt.endsWith(".") || shortFmt.endsWith("/") || shortFmt.endsWith("-"))
    {
        shortFmt.chop(1);
    }
    lbl_cboxYearandMonthLabel->setText(date.toString(shortFmt));

}
void LunarCalendarWidget::updateLayout()
{
    updateLayout1();
    updateSchedule();
    calculateWindowGeometry();
}
void LunarCalendarWidget::updateLayout1()
{
    if(scroll_layout)
    {
        delete scroll_layout;
    }
    if(scr_srcollAreaOfCalendar)
    {
        scr_srcollAreaOfCalendar->deleteLater();
        scr_srcollAreaOfCalendar = nullptr;
    }
    if(scr_srcollAreaOfSchedule)
    {
        scr_srcollAreaOfSchedule->deleteLater();
        scr_srcollAreaOfSchedule = nullptr; // 将指针设置为nullptr
    }
    qDebug() << "no have such scr_srcollAreaOfSchedule";
    scr_srcollAreaOfSchedule = new CustomScrollArea(this);
    scr_srcollAreaOfSchedule->setAccessibleName("kylin-calendar-plugin_lunarcalendar_scrollarea_of_schedule");
    scr_srcollAreaOfSchedule->viewport()->setAccessibleName("kylin-calendar-plugin_lunarcalendar_scrollarea_of_schedule");
    scr_srcollAreaOfSchedule->setAccessibleDescription("scrollarea of schedule in schedule part in calendar ui");
    scr_srcollAreaOfSchedule->setWidgetResizable(true);
    if(m_verLayoutCalendar)
    {
        delete m_verLayoutCalendar;
        m_verLayoutCalendar = new QVBoxLayout(this);
    }
    else if(!m_verLayoutCalendar)
    {
        m_verLayoutCalendar = new QVBoxLayout(this);
    }
    if(scrollareawidget)
    {
        scrollareawidget->deleteLater();
        scrollareawidget = nullptr;
    }
    if(schedule_Widget)
    {
        schedule_Widget->deleteLater();
        schedule_Widget = nullptr;
    }
    if(m_scheduleWidget)
    {
        m_scheduleWidget->deleteLater();
        m_scheduleWidget = nullptr;
    }
    m_scheduleWidget = new QWidget(this);
    m_scheduleWidget->setProperty("needTranslucent", true);
    m_scheduleWidget->setPalette(QPalette(Qt::transparent));
    scr_srcollAreaOfSchedule->setPalette(QPalette(Qt::transparent));
    QPalette pal = scr_srcollAreaOfSchedule->verticalScrollBar()->palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    scr_srcollAreaOfSchedule->verticalScrollBar()->setPalette(pal);
    m_scheduleWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_scheduleWidget->setAutoFillBackground(false);


    // 设置 scroll area 透明
    scr_srcollAreaOfSchedule->setAttribute(Qt::WA_TranslucentBackground);
    scr_srcollAreaOfSchedule->setAutoFillBackground(false);
    m_scheduleWidget->setAccessibleName("kylin-calendar-plugin_lunarcalendar_schedule_widget");
    m_scheduleWidget->setAccessibleDescription("the part of schedule in calendar ui");

}

void LunarCalendarWidget::slotUpdateSchcedule()
{
    updateAllSchedule();
    updateLayout();
}

void LunarCalendarWidget::ThreeMoreSchedule()
{
    int num = m_markInfoList.count();
    m_scheduleWidget->setContentsMargins(0, 0, 0, 0);
    m_scheduleWidget->setFixedHeight(59 * num);
    labWidget->setContentsMargins(16, 0, 16, 0);
    yijiWidget->setContentsMargins(16, 0, 16, 0);
    scr_srcollAreaOfSchedule->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scr_srcollAreaOfSchedule->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
    scr_srcollAreaOfSchedule->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //把前两个日程先显示出来 这部分不用修改
    populateScheduleItems();
    m_scheduleWidget->setLayout(item_layout);
    scr_srcollAreaOfSchedule->setWidget(m_scheduleWidget);
    initVerlayoutCalendar();
    if(ScrollAreaNeedChange)
    {
        if(yijistate)
        {
            scr_srcollAreaOfSchedule->setFixedSize(420, 80);
        }
        else
        {
            scr_srcollAreaOfSchedule->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
        }
    }
    else
    {
        if(yijistate)
        {
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT);
        }
        else
        {
            this->setFixedSize(ORIGIN_CALENDAR_WIDTH, LESSTWOSHCEDULE_CALENDAR_HEIGHT);
        }
    }

    calculateWindowGeometry();
}

void LunarCalendarWidget::initGsettings()
{
    qDebug() << "运行了一次initGsettings";
    const QByteArray id(PANEL_SETTINGS);
    if (QGSettings::isSchemaInstalled(id))
    {
        m_panelGsettings = new QGSettings(id);
        if (m_panelGsettings != nullptr)
        {
            UkuiWindowHelper *windowHelper = new UkuiWindowHelper(this);
            windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::SystemWindow);
            QScreen* p_screen = windowHelper->getCurrentScreenBlocking();
            if(!p_screen)
            {
                p_screen = QApplication::primaryScreen();
            }
            if(p_screen)
            {
                QSize screenSize = p_screen->size();
                if(screenSize.isValid())
                {
                    m_nScreenHeight = screenSize.height() + p_screen->geometry().y();
                }
            }

            m_nPanelSize = m_panelGsettings->get(PANEL_SIZE_KEY).toInt();
            m_nPanelHeight = m_nScreenHeight - m_nPanelSize; //这个值就是屏幕到任务栏的高度
            panelPosition = m_panelGsettings->get(PANEL_POSITION_KEY).toInt();
            connect(m_panelGsettings, &QGSettings::changed, this, &LunarCalendarWidget::onGSettingsChanged);
        }
    }
    widgetTime->setObjectName("widgetTime");
    connect(m_pScreen, &QScreen::geometryChanged, this, &LunarCalendarWidget::onPrimaryScreenChanged);
    const QByteArray calendar_id(PANEL_CONTROL_IN_CALENDAR);
    if(QGSettings::isSchemaInstalled(calendar_id))
    {
        calendar_gsettings = new QGSettings(calendar_id);
    }
    initWidget();
    if(calendar_gsettings != nullptr)
    {
        //农历切换监听与日期显示格式
        connect(calendar_gsettings, &QGSettings::changed, this, [ = ] (const QString & key)
        {
            if(key == LUNAR_KEY)
            {
                m_showLunar = getShowLunar();
                setShowLunar(m_showLunar);
                controlDownLine();
                _timeUpdate();
            }
            else if(key == "date")
            {
            }
            else if (key == FIRST_DAY_KEY)
            {
                setWeekNameFormat(calendar_gsettings->get(FIRST_DAY_KEY).toString() == "sunday");
            }
        });
        m_showLunar = getShowLunar();
        QDateTime currentDateTime = QDateTime::currentDateTime();
        setShowLunar(m_showLunar);
        setWeekNameFormat(calendar_gsettings->get(FIRST_DAY_KEY).toString() == "sunday");
        //监听12/24小时制
        connect(calendar_gsettings, &QGSettings::changed, this, [ = ] (const QString & keys)
        {
            timemodel = calendar_gsettings->get("hoursystem").toString();
            if(timemodel == "12")
            {
                //        timeString = currentDateTime.toString("AP hh:mm:ss");
                QString lang, format;
                getLocale(lang, format);
                //qInfo() << lang << format << (calendar_gsettings->get("calendar").toString() == "lunar") ;
                if((lang.contains("zh_CN") || lang.contains("zh_HK")))
                {
                    timeString = currentDateTime.toString("AP hh:mm:ss");
                }
                else
                {
                    const auto locale = QLocale::system();
                    if (locale.name().startsWith(QLatin1String("mn")))
                    {
                        QLocale m_locale(QLocale::Mongolian);
                        timeString = m_locale.toString(QDateTime::currentDateTime(), "hh:mm:ss AP");
                    }
                    else if(locale.name().startsWith(QLatin1String("en")))
                    {
                        QLocale m_locale(QLocale::English);

                        timeString = m_locale.toString(QDateTime::currentDateTime(), "hh:mm:ss AP");
                    }
                    else if(locale.name().startsWith(QLatin1String("bo")))
                    {
                        QLocale m_locale(QLocale::Tibetan);
                        timeString = m_locale.toString(QDateTime::currentDateTime(), "hh:mm:ss AP");
                    }

                }

            }
            else
            {
                timeString = currentDateTime.toString("HH:mm:ss");
            }
            //更新crontab
            CalendarDataBase::getInstance().refreshCronTab(timemodel);
            //更新视觉
            _timeUpdate();
        });
        timemodel = calendar_gsettings->get("hoursystem").toString();
        if(timemodel == "12")
        {
            timeString = currentDateTime.toString("AP hh:mm:ss");
        }
        else
        {
            timeString = currentDateTime.toString("HH:mm:ss");
        }

    }
    else
    {
        //无设置默认公历
        lunarstate = true;
    }

    //使用系统提供的sdk刷新时间显示
    QDBusConnection::sessionBus().connect(KYSDK_DATESERVER,
                                          KYSDK_DATEPATH,
                                          KYSDK_DATEINTERFACE,
                                          "ShortDateSignal",
                                          this,
                                          SLOT([](QString)
                                          {
                                              updateYearMonthString(QDateTime::currentDateTime().date().year(), QDateTime::currentDateTime().date().month());
                                          }
                                          )
            );

    //切换主题
    const QByteArray style_id(ORG_UKUI_STYLE);
    QStringList stylelist;
    stylelist << STYLE_NAME_KEY_DARK << STYLE_NAME_KEY_BLACK;
    if(QGSettings::isSchemaInstalled(style_id))
    {
        style_settings = new QGSettings(style_id);
        if (style_settings != nullptr)
        {
            dark_style = stylelist.contains(style_settings->get(STYLE_NAME).toString());
            setColor(dark_style);
            m_fontName = style_settings->get(SYSTEM_FONT).toString();
            m_font.setFamily(m_fontName);
        }
    }
    //    lbl_cboxYearandMonthLabel->setFont(m_font);
    if (style_settings != nullptr)
    {
        connect(style_settings, &QGSettings::changed, this, [ = ] (const QString & key)
        {
            if(key == STYLE_NAME)
            {
                dark_style = stylelist.contains(style_settings->get(STYLE_NAME).toString());
                _timeUpdate();
                setColor(dark_style);
            }
            else if(key == SYSTEM_FONT)
            {
                m_fontName = style_settings->get(SYSTEM_FONT).toString();
                m_font.setFamily(m_fontName);
            }
            else
            {
                qDebug() << "key!=STYLE_NAME";
            }
        });
    }
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(timerUpdate()));
    timer->setInterval(1000);

    locale = QLocale::system().name();

    setLocaleCalendar();//设置某区域下日历的显示
    initTransparency();
    setWindowOpacity(1);
    setAttribute(Qt::WA_TranslucentBackground, true); //设置窗口背景透明
    setProperty("useStyleWindowManager", false);

    updateYearMonthString(QDateTime::currentDateTime().date().year(), QDateTime::currentDateTime().date().month());
    _timeUpdate();

    //加载繁中宜忌字典
    if (locale != "zh_HK")
    {
        return;
    }
    QFile dict("/usr/share/ukui-panel/plugin-calendar/html/zh_HK.html");
    if (!dict.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "open field : /usr/share/ukui-panel/plugin-calendar/html/zh_HK.html";
        return;
    }
    QString orig = dict.readLine();
    QString trans = dict.readLine();
    orig.remove("\n");
    for (int i = 0; i < orig.length(); i++)
    {
        if (i >= trans.length())
        {
            break;
        }
        m_zh_HK_dict.insert(orig.at(i), trans.at(i));
    }
}

void LunarCalendarWidget::onGSettingsChanged(const QString &key)
{
    if (key == PANEL_POSITION_KEY || key == PANEL_SIZE_KEY)
    {
        // 使用统一的位置计算函数
        calculateWindowGeometry();
    }
    UkuiWindowHelper *windowHelper = new UkuiWindowHelper(this);
    windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::SystemWindow);
    QScreen* p_screen = windowHelper->getCurrentScreenBlocking();
    if(!p_screen)
    {
        p_screen = QApplication::primaryScreen();
    }
    if(p_screen)
    {
        QSize screenSize = p_screen->size();
        if(screenSize.isValid())
        {
            m_nScreenHeight = screenSize.height() + p_screen->geometry().y();
        }
    }
}

void LunarCalendarWidget::onPrimaryScreenChanged(const QRect &newGeometry)
{
    qDebug() << "分辨率变化";
    m_pScreen = qobject_cast<QScreen *>(sender());
    // 当主屏幕变化时，更新主屏幕和高度
    qDebug() << "Screen geometry changed:" << m_pScreen->name() << newGeometry;
    UkuiWindowHelper *windowHelper = new UkuiWindowHelper(this);
    windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::SystemWindow);
    QScreen* pScreen = windowHelper->getCurrentScreenBlocking();
    if(!pScreen)
    {
        pScreen = QApplication::primaryScreen();
    }
    m_nScreenHeight = pScreen->size().height() + pScreen->geometry().y();
    m_nPanelHeight = m_nScreenHeight - m_nPanelSize;
    CheckResolution();
}

void LunarCalendarWidget::CheckResolution()
{
    if(LESSTWOSHCEDULE_CALENDAR_HEIGHT < m_nPanelHeight && m_nPanelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT + YIJI_HEIGHT)
    {
        ScrollAreaNeedChange = true;
    }
    else
    {
        ScrollAreaNeedChange = false;
    }
    if(m_nPanelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT)
    {
        ScrollbarUnderTheResolution = true;
    }
    else
    {
        ScrollbarUnderTheResolution = false;
    }
    if(m_nPanelHeight < LESSTWOSHCEDULE_CALENDAR_HEIGHT - SCROLLA)
    {
        ScrollbarOfMinimumResolution = true;
    }
    else
    {
        ScrollbarOfMinimumResolution = false;
    }
}

void LunarCalendarWidget::MinimumResolution()
{
    scr_srcollAreaOfCalendar = new CustomScrollArea(this);
    scr_srcollAreaOfCalendar->setAccessibleName("kylin-calendar-plugin_lunarcalendar_scrollarea_of_calendar");
    scr_srcollAreaOfCalendar->viewport()->setAccessibleName("kylin-calendar-plugin_lunarcalendar_scrollarea_of_calendar");
    scr_srcollAreaOfCalendar->setAccessibleDescription("scrollarea of calendar in calendar ui");
    scr_srcollAreaOfCalendar->setWidgetResizable(true);
    scrollareawidget = new QWidget(this);
    scrollareawidget->setAccessibleName("kylin-calendar-plugin_lunarcalendar_scrollareawidget");
    scrollareawidget->setAccessibleDescription("make the whole calendar a scrollarea");
    scrollareawidget->setProperty("needTranslucent", true);
    scrollareawidget->setPalette(QPalette(Qt::transparent));
    scr_srcollAreaOfCalendar->setPalette(QPalette(Qt::transparent));
    QPalette pal = scr_srcollAreaOfCalendar->verticalScrollBar()->palette();
    pal.setColor(QPalette::Base, Qt::transparent);
    scr_srcollAreaOfCalendar->verticalScrollBar()->setPalette(pal);
    scrollareawidget->setAttribute(Qt::WA_TranslucentBackground);
    scrollareawidget->setAutoFillBackground(false);
    scr_srcollAreaOfCalendar->setAttribute(Qt::WA_TranslucentBackground);
    scr_srcollAreaOfCalendar->setAutoFillBackground(false);
    scrollareawidget->setStyleSheet("color: rgb(29, 29, 29);");
    calculateWindowGeometry();
    scrollareawidget->setFixedSize(ORIGIN_CALENDAR_WIDTH,LESSTWOSHCEDULE_CALENDAR_HEIGHT - SCROLLA);
    this->setFixedSize(ORIGIN_CALENDAR_WIDTH, m_nPanelHeight - SCROLLA);
    scr_srcollAreaOfCalendar->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
    scr_srcollAreaOfCalendar->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scr_srcollAreaOfCalendar->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    initVerlayoutCalendar();

    scrollareawidget->setLayout(m_verLayoutCalendar);
    scr_srcollAreaOfCalendar->setWidget(scrollareawidget);

    scroll_layout = new QVBoxLayout(this);
    scroll_layout->setContentsMargins(0,0,0,0);
    scroll_layout->setSpacing(0);
    scroll_layout->addWidget(scr_srcollAreaOfCalendar);
}

void LunarCalendarWidget::DisplayAtResolution()
{
    this->setFixedSize(ORIGIN_CALENDAR_WIDTH, m_nPanelHeight - SCROLLA);
    int num = m_markInfoList.count();
    if(num == 0)
    {
        m_scheduleWidget->setContentsMargins(0, 0, 0, 0);
        m_scheduleWidget->setFixedSize(ORIGIN_CALENDAR_WIDTH, 220);
        scr_srcollAreaOfSchedule->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
        scr_srcollAreaOfSchedule->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        labWidget->setContentsMargins(5, 0, 10, 0);
        yijiWidget->setContentsMargins(5, 0, 0, 0);

        item_layout->addWidget(labWidget);
        item_layout->addStretch(0);
        item_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item_layout->addWidget(yijiWidget);

        lbl_labBottom->setPalette(QApplication::palette(lbl_labBottom));
        lbl_jiLabel->setPalette(QApplication::palette(lbl_jiLabel));
        lbl_yiLabel->setPalette(QApplication::palette(lbl_yiLabel));
        lbl_yijichooseLabel->setPalette(QApplication::palette(lbl_yijichooseLabel));
        chk_yijichoose->setPalette(QApplication::palette(chk_yijichoose));

        item_layout->addWidget(schedule_Widget);
        schedule_Widget = new schedule_item(this, false);
        schedule_Widget->lbl_dateTimeLabel->setPalette(QApplication::palette(lbl_labBottom));
        schedule_Widget->setStartTime("");
        schedule_Widget->setDescription("");

        if(style_settings != nullptr)
        {
            QString strCurTheme = style_settings->get("styleName").toString();
            if(strCurTheme == "ukui-default" || strCurTheme == "ukui-light" )
            {
                lbl_labBottom->setStyleSheet("color: rgb(29, 29, 29);");
                lbl_jiLabel->setStyleSheet("color: rgb(29, 29, 29);");
                lbl_yiLabel->setStyleSheet("color: rgb(29, 29, 29);");
                lbl_yijichooseLabel->setStyleSheet("color: rgb(29, 29, 29);");
                chk_yijichoose->setStyleSheet("color: rgb(29, 29, 29);");
                m_scheduleWidget->setStyleSheet("color: rgb(29, 29, 29);");
                schedule_Widget->setStyleSheet("color: rgb(29, 29, 29);");
            }
            else if(strCurTheme == "ukui-dark")
            {
                lbl_labBottom->setStyleSheet("color: rgb(255, 255, 255);");
                lbl_jiLabel->setStyleSheet("color: rgb(255, 255, 255);");
                lbl_yiLabel->setStyleSheet("color: rgb(255, 255, 255);");
                lbl_yijichooseLabel->setStyleSheet("color: rgb(255, 255, 255);");
                chk_yijichoose->setStyleSheet("color: rgb(255, 255, 255);");
                m_scheduleWidget->setStyleSheet("color: rgb(255, 255, 255);");
                schedule_Widget->setStyleSheet("color: rgb(255, 255, 255);");
            }
        }

        item_layout->addWidget(schedule_Widget);
        item_layout->addSpacing(2);
        m_scheduleWidget->setLayout(item_layout);
        scr_srcollAreaOfSchedule->setWidget(m_scheduleWidget);
        if(ScrollbarOfMinimumResolution)
        {
            MinimumResolution();
        }
        else
        {
            initVerlayoutCalendar();
        }
    }
    else
    {
        m_scheduleWidget->setContentsMargins(0, 0, 0, 0);
        m_scheduleWidget->setFixedSize(ORIGIN_CALENDAR_WIDTH, 140 + 70 * num); //确保日程越多，可滑动区域越大
        scr_srcollAreaOfSchedule->setFixedWidth(ORIGIN_CALENDAR_WIDTH);
        scr_srcollAreaOfSchedule->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        labWidget->setContentsMargins(5, 0, 10, 0);
        yijiWidget->setContentsMargins(5, 0, 0, 0);
        item_layout->addWidget(labWidget);
        item_layout->addStretch(0);
        item_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        item_layout->addWidget(yijiWidget);
        item_layout->addWidget(schedule_Widget);
        populateScheduleItems();
        m_scheduleWidget->setLayout(item_layout);
        scr_srcollAreaOfSchedule->setWidget(m_scheduleWidget);
        if(ScrollbarOfMinimumResolution)
        {
            MinimumResolution();
        }
        else
        {
            initVerlayoutCalendar();
        }
    }

}

QDate LunarCalendarWidget::getCurrentDate()
{
    return date;
}

QDate LunarCalendarWidget::getClickedDate()
{
    return s_clickDate;
}

void LunarCalendarWidget::populateScheduleItems()
{
    foreach(info, m_markInfoList)
    {
        schedule_Widget = new schedule_item(this, true);
        QString time1;
        QString time2;
        if(timemodel == "12")
        {
            schedule_Widget->timemode = "12";
            if(info.hour <= 12)
            {
                time1 = tr("morning");
            }
            else
            {
                time1 = tr("afternoon");
                info.hour = info.hour - 12;
            }
            if(info.end_hour <= 12)
            {
                time2 = tr("morning");
            }
            else
            {
                time2 = tr("afternoon");
                info.end_hour = info.end_hour - 12;
            }
        }
        else if(timemodel == "24")
        {
            time1 = "";
            time2 = "";
        }
        QString starttimeString = QString::number(info.hour) + ":" + QString::number(info.minute); // 确保这是有效的格式
        QTime m_startTime = QTime::fromString(starttimeString, "h:m");
        QString start = m_startTime.toString("hh:mm");
        QString timeString = QString::number(info.end_hour) + ":" + QString::number(info.end_minute); // 确保这是有效的格式
        QTime m_endTime = QTime::fromString(timeString, "h:m");
        QString end = m_endTime.toString("hh:mm");
        QString  lineOne = QString(" %1%2-%3%4 ").arg(time1).arg(start).arg(time2).arg(end)
                + info.timeLong;
        if(info.m_isAllDay)
        {
            lineOne = tr("   allday");
        }
        schedule_Widget->setDescription(info.m_descript);
        schedule_Widget->setStartTime(lineOne);
        if(info.isKuaDay)
        {
            schedule_Widget->adjustDate(info.m_markEndDate);
            schedule_Widget->setEndTime(info.end_hour, info.end_minute, info.m_markEndDate, time2);
        }
        else
        {
            schedule_Widget->adjustDate(this->date);
            schedule_Widget->setEndTime(info.end_hour, info.end_minute, this->date, time2);
        }


        schedule_Widget->setId(info.m_markId);
        if(info.isKuaDay)
        {

            QString  lineOne = QString("%1 %2%3 - %4 %5%6").arg(info.m_markStartDate.toString("yyyy-MM-dd")).arg(time1).arg(start).arg(info.m_markEndDate.toString("yyyy-MM-dd")).arg(time2).arg(end);
            schedule_Widget->setStartTime(lineOne);
        }
        connect(schedule_Widget, &schedule_item::ScheduleClick, this, &LunarCalendarWidget::slotEditSchedule);
        connect(schedule_Widget, &schedule_item::EditSchedule, this, &LunarCalendarWidget::slotEditSchedule);
        connect(schedule_Widget, &schedule_item::updateWindow, this, &LunarCalendarWidget::updateFromItem);

        if(style_settings != nullptr)
        {
            QString strCurTheme = style_settings->get("styleName").toString();
            if(strCurTheme == "ukui-default" || strCurTheme == "ukui-light" )
            {
                schedule_Widget->setStyleSheet("color: rgb(29, 29, 29);");
            }
            else if(strCurTheme == "ukui-dark")
            {
                schedule_Widget->setStyleSheet("color: rgb(255, 255, 255);");
            }
        }

        item_layout->addWidget(schedule_Widget);
        item_layout->setSpacing(0);
        //item_layout->addSpacing(2);
    }
}
void LunarCalendarWidget::initVerlayoutCalendar()
{
    m_horLayoutWidgetTop = new QHBoxLayout(this);
    m_horLayoutWidgetTop->setContentsMargins(26, 0, 24, 0);
    m_horLayoutWidgetTop->addWidget(widgetTime);
    m_horLayoutWidgetTop->addWidget(m_createScheduleButton);
    m_verLayoutCalendar->setMargin(0);
    m_verLayoutCalendar->setSpacing(0);
    m_verLayoutCalendar->addLayout(m_horLayoutWidgetTop);
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    if(!ScrollbarUnderTheResolution)
    {
        m_verLayoutCalendar->addWidget(lineUp);
    }
    else if(lineUp)
    {
        lineUp->deleteLater();
        lineUp = nullptr;
    }
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(m_widgetTop);
    m_verLayoutCalendar->addSpacing(18);
    m_verLayoutCalendar->addWidget(widgetWeek);
    m_verLayoutCalendar->addWidget(m_widgetDayBody, 1);
    m_verLayoutCalendar->addWidget(widgetYearBody, 1);
    m_verLayoutCalendar->addWidget(widgetmonthBody, 1);
    m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(lineDown);
    controlDownLine();
    m_verLayoutCalendar->addItem(new QSpacerItem(3, 3));
    if(!ScrollbarUnderTheResolution)  {
        m_verLayoutCalendar->addWidget(labWidget);
        m_verLayoutCalendar->addWidget(yijiWidget);
        m_verLayoutCalendar->addItem(new QSpacerItem(3, 3));
    }
    if(!ScrollbarUnderTheResolution)
    {
        lineSchedule->setFixedHeight(1);
    }
    else
    {

        lineSchedule->setFixedHeight(0.1);
    }

    m_verLayoutCalendar->addWidget(lineSchedule);
    //m_verLayoutCalendar->addItem(new QSpacerItem(6, 6));
    m_verLayoutCalendar->addWidget(scr_srcollAreaOfSchedule);
}

void LunarCalendarWidget::updateFormat(QString timerStr)
{
    Q_UNUSED(timerStr);
    dateStrFormat = SDK2CPP(kdk_system_get_longformat_date)();
}
