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

#ifndef LUNARCALENDARWIDGET_H
#define LUNARCALENDARWIDGET_H

#include <QGSettings/QGSettings>
#include <QWidget>
#include <QDate>
#include <QTimer>
#include <QVBoxLayout>
#include <QWheelEvent>
#include "custommessagebox.h"
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QPalette>
#include "customscrollarea.h"
#include <QJsonParseError>
#include <QJsonObject>
#include <QRadioButton>
#include <QLabel>
#include <QObject>
#include <QCheckBox>
#include <QLocale>
#include <QProcess>
#include <QListWidget>
#include <QListWidgetItem>
#include <kysdk/applications/kborderlessbutton.h>
#include <QScrollArea>
#include <QMouseEvent>
#include "lunarcalendaritem.h"
#include "lunarcalendaryearitem.h"
#include "lunarcalendarmonthitem.h"
#include "kpushbutton.h"
#include "schedulewidget.h"
#include "schedule_item.h"
#include "schedulestruct.h"
#include "qdatetime.h"
#include "qlayout.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include "qfontdatabase.h"

using namespace kdk;

class QGestureEvent;
class QPanGesture;
class QPinchGesture;
class QSwipeGesture;
class CSchceduleDlg;
class QLabel;
class QComboBox;
class LunarCalendarYearItem;
class LunarCalendarMonthItem;
class LunarCalendarItem;
class CalendarDataBase;
class PartLineWidget;

class LunarCalendarWidget : public QWidget
{
    Q_OBJECT

public:
    enum CalendarStyle
    {
        CalendarStyle_Red = 0
    };

    enum WeekNameFormat
    {
        WeekNameFormat_Short = 0,   //短名称
        WeekNameFormat_Normal = 1,  //普通名称
        WeekNameFormat_Long = 2,    //长名称
        WeekNameFormat_En = 3       //英文名称
    };

    enum SelectType
    {
        SelectType_Rect = 0,        //矩形背景
        SelectType_Circle = 1,      //圆形背景
        SelectType_Triangle = 2,    //带三角标
        SelectType_Image = 3        //图片背景
    };
    explicit LunarCalendarWidget(QWidget *parent = 0);
    ~LunarCalendarWidget();
    QList<MarkInfo> twiceInfo;
    QList<MarkInfo> remainInfo;
    QVBoxLayout *scroll_layout = nullptr;
    QVBoxLayout *item_layout = nullptr;
    QVBoxLayout *m_verLayoutCalendar = nullptr;
    QHBoxLayout *m_horLayoutWidgetTop = nullptr;
    schedule_item *schedule_Widget = nullptr;
    QWidget *allButtom = nullptr;

    bool yijistate = false;
    bool isWayland;
    bool ScrollAreaNeedChange = false;
    bool isClicked  = false;
    bool isExceedScreenHeight = false;
    bool isMore = false;
    bool ScrollbarUnderTheResolution = false; //极限分辨率下的滑动区域
    bool ScrollbarOfMinimumResolution = false;

    QScreen *m_pScreen = nullptr;

    int panelPosition;
    int m_num = 0;
    int m_nPanelHeight; //屏幕到任务栏的高度
    int m_nPanelSize;  //任务栏高度
    QDate date;                         //当前日期
    static QDate s_clickDate;                    //保存点击日期
    QStringList m_markIdList;
    QGSettings *m_panelGsettings = nullptr;
    QList<MarkInfo> m_markInfoList;

    QString ScrollAreaStyleSheet;
    CustomScrollArea *scr_srcollAreaOfSchedule = nullptr; // 显示日程部分的滚动区域
    CustomScrollArea *scr_srcollAreaOfCalendar = nullptr;

    void updateLayout();
    void updateAllSchedule();
    void initGsettings();
    void populateScheduleItems();
    void initVerlayoutCalendar();
    void updateSchedule();
    void getFontInfo(QString &fontName, int &fontSize);
    void DisplayAtResolution();//极限分辨率下的显示
    bool getShowLunar();
    bool compare(MarkInfo info1, MarkInfo info2);
    bool getBool();
    void calculateWindowGeometry();
    void applyStyleSettings();
    void setScreen(QScreen* screen);

private:
    CustomMessageBox *msgBox = nullptr;
    CSchceduleDlg *m_ScheculeDlg = nullptr;
    QMessageBox *msg = nullptr;
    QDate m_date;      //当前日期

    QWidget *m_scheduleWidget = nullptr;
    QWidget *yijiWidget = nullptr;
    QWidget *widgetWeek = nullptr;
    QWidget *m_clickedWidget = nullptr;
    QWidget *m_widgetDayBody = nullptr;
    QWidget *widgetYearBody = nullptr;
    QWidget *m_widgetTop = nullptr;
    QWidget *labWidget = nullptr;
    QWidget *widgetmonthBody = nullptr;
    QWidget *widgetTime = nullptr;
    QWidget *scrollareawidget = nullptr;

    //    QListWidget *m_tableWidget = nullptr;
    QList<MarkInfo> *allinfo = nullptr;

    QLabel *lbl_labBottom = nullptr;
    QLabel *lbl_datelabel = nullptr;
    QLabel *lbl_timelabel = nullptr;
    QLabel *lbl_lunarlabel = nullptr;
    QLabel *lbl_yiLabel = nullptr;
    QLabel *lbl_jiLabel = nullptr;
    QLabel *lbl_yijichooseLabel = nullptr;

    QTimer *timer = nullptr;

    QVBoxLayout *m_verLayoutTopLeft = nullptr;
    QVBoxLayout *yijiLayout = nullptr;
    QHBoxLayout *labLayout = nullptr;

    KPushButton *btn_btnYear = nullptr;
    KPushButton *btn_btnMonth = nullptr;
    KPushButton *btn_btnToday = nullptr;
    QPushButton *btn_yesButton = nullptr;
    QPushButton *btn_noButton = nullptr;

    int m_fontSize = 0;
    int m_start_y = -1;
    int pro;
    int m_savedYear = 0;
    int m_nScreenHeight;
    int m_trans = 75;
    int m_currentType = 0;//0:日，1:月, 2:年

    PartLineWidget *lineUp = nullptr;
    PartLineWidget *lineDown = nullptr;
    PartLineWidget *lineSchedule = nullptr;

    QGSettings * yijiGsettings = nullptr;
    QGSettings *calendar_gsettings = nullptr;
    QGSettings *style_settings = nullptr;
    QGSettings *styleGsettings = nullptr;
    QGSettings *m_transparencySetting = nullptr;

    kdk::KBorderlessButton *btnPrevYear;
    kdk::KBorderlessButton *btnNextYear;

    KPushButton *m_createScheduleButton = nullptr ;
    QCheckBox *chk_yijichoose = nullptr;
    QProcess *myProcess = nullptr;
    QString timemodel = 0;
    QString m_fontName;

    bool FirstdayisSun;                 //首日期为周日
    bool m_showLunar = false;
    bool lunarstate = false;
    bool oneRun = true;
    bool btnClick;
    bool dark_style;

    //按钮单击,避开下拉选择重复触发
    QMap<QString, QString> worktimeinside;
    QMap<QString, QMap<QString, QString>> worktime;
    QFont iconFont;                     //图形字体
    //    QComboBox *cboxYearandMonth = nullptr;        //年份下拉框
    QLabel *lbl_cboxYearandMonthLabel = nullptr;
    QList<QLabel *> lbl_labWeeks;           //顶部星期名称
    QList<LunarCalendarItem *> dayItems;            //日期元素
    QList<LunarCalendarYearItem *> yearItems;       //年份元素
    QList<LunarCalendarMonthItem *> monthItems;     //月份元素
    QFont m_font;
    CalendarStyle calendarStyle;        //整体样式
    MarkInfo info;
    QMap<QString, QString> m_zh_HK_dict; //繁中宜忌翻译字典
    QList<CSchceduleDlg*> m_scheduleDialogs; // 新增：存储所有打开的对话框

    void setColor(bool mdark_style);
    void _timeUpdate();
    void setBool(bool more);
    void yijihandle(const QDate &date);
    void analysisWorktimeJs();       //解析js文件
    void downLabelHandle(const QDate &date);
    void setSettings(QString arg);
    void setLocaleCalendar();
    void changeMonth(bool forward);
    void changeYear(bool forward);
    void noSchedule();
    void twoLessSchedule();
    //设置子控件是否显示农历信息
    void setShowLunar(bool showLunar);
    void twoMoreSchedule();
    void ThreeMoreSchedule();
    void changeMode(int mode);
    void initTransparency();
    void controlDownLine();
    void updateLayout1();
    void MinimumResolution();//最低分辨率
    void CheckResolution();

    QString getSettings();
    QString timeString ;
    QString dateStrFormat;
    QString toHK(QString str);
    QStringList getLocale(QString &language, QString &locale);
    QColor weekTextColor;               //星期名称文字颜色
    QColor weekBgColor;                 //星期名称背景色
    QString bgImage;                    //背景图片
    SelectType selectType;              //选中模式



    QColor borderColor;                 //边框颜色
    QColor weekColor;                   //周末颜色
    QColor superColor;                  //角标颜色
    QColor lunarColor;                  //农历节日颜色

    QColor currentTextColor;            //当前月文字颜色
    QColor otherTextColor;              //其他月文字颜色
    QColor selectTextColor;             //选中日期文字颜色
    QColor hoverTextColor;              //悬停日期文字颜色

    QColor currentLunarColor;           //当前月农历文字颜色
    QColor otherLunarColor;             //其他月农历文字颜色
    QColor selectLunarColor;            //选中日期农历文字颜色
    QColor hoverLunarColor;             //悬停日期农历文字颜色

    QColor currentBgColor;              //当前月背景颜色
    QColor otherBgColor;                //其他月背景颜色
    QColor selectBgColor;               //选中日期背景颜色
    QColor hoverBgColor;                //悬停日期背景颜色


protected :
    void wheelEvent(QWheelEvent *event)override;
    bool eventFilter(QObject *watched, QEvent *event)override;
    void paintEvent(QPaintEvent*)override;
    void keyPressEvent(QKeyEvent *ev)override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private Q_SLOTS:
    void initWidget();

    void initStyle();
    void initDate();
    void changeDate(const QDate &date);
    void yearChanged(const QString &arg1);
    void monthChanged(const QString &arg1);
    void labClicked(const QDate &date, const LunarCalendarItem::DayType &dayType);

    void timerUpdate();
    void customButtonsClicked(bool checked);
    void yearWidgetChange();
    void monthWidgetChange();
    void updateYearMonthString(int year, int month);
    void slotUpdateSchcedule();
    void updateFormat(QString timerStr);
public:
    QDate getCurrentDate();
    static QDate getClickedDate();
    QString locale                     ;
    void showCalendar(bool showToday);

public Q_SLOTS:
    void updateFromItem(QString id);
    void updateYearClicked(const QDate &date, const LunarCalendarYearItem::DayType &dayType);
    void updateMonthClicked(const QDate &date, const LunarCalendarMonthItem::DayType &dayType);
    //上一年,下一年
    void showPreviousYear();
    void showNextYear();
    void slotShowMore();

    void dayChanged(const QDate &date, const QDate &m_date);
    void dateChanged(int year, int month, int day);

    void onGSettingsChanged(const QString &key);

    //上一月,下一月
    void showPreviousMonth(bool date_clicked = true);
    void showNextMonth(bool date_clicked = true);

    //转到今天
    void showToday();

    //设置整体样式
    void setCalendarStyle(const CalendarStyle &calendarStyle);
    //设置星期名称格式
    void setWeekNameFormat(bool FirstDayisSun);

    //设置日期
    void setDate(const QDate &date);

    //设置顶部星期名称文字颜色+背景色
    void setWeekTextColor(const QColor &weekTextColor);
    void setWeekBgColor(const QColor &weekBgColor);


    //设置背景图片
    void setBgImage(const QString &bgImage);
    //设置选中背景样式
    void setSelectType(const SelectType &selectType);

    //设置边框颜色
    void setBorderColor(const QColor &borderColor);
    //设置周末颜色
    void setWeekColor(const QColor &weekColor);
    //设置角标颜色
    void setSuperColor(const QColor &superColor);
    //设置农历节日颜色
    void setLunarColor(const QColor &lunarColor);

    //设置当前月文字颜色
    void setCurrentTextColor(const QColor &currentTextColor);
    //设置其他月文字颜色
    void setOtherTextColor(const QColor &otherTextColor);
    //设置选中日期文字颜色
    void setSelectTextColor(const QColor &selectTextColor);
    //设置悬停日期文字颜色
    void setHoverTextColor(const QColor &hoverTextColor);

    //设置当前月农历文字颜色
    void setCurrentLunarColor(const QColor &currentLunarColor);
    //设置其他月农历文字颜色
    void setOtherLunarColor(const QColor &otherLunarColor);
    //设置选中日期农历文字颜色
    void setSelectLunarColor(const QColor &selectLunarColor);
    //设置悬停日期农历文字颜色
    void setHoverLunarColor(const QColor &hoverLunarColor);

    //设置当前月背景颜色
    void setCurrentBgColor(const QColor &currentBgColor);
    //设置其他月背景颜色
    void setOtherBgColor(const QColor &otherBgColor);
    //设置选中日期背景颜色
    void setSelectBgColor(const QColor &selectBgColor);
    //设置悬停日期背景颜色
    void setHoverBgColor(const QColor &hoverBgColor);
    //新建日程弹窗
    void slotAddSchedule();
    void slotEditSchedule(QString id);
    void slotEditSchedulefromMenu(QString id);
    void onPrimaryScreenChanged(const QRect &newGeometry);
    //    void onDialogAccepted();

Q_SIGNALS:
    void NeedToHide();
    void scheduleNum(int num);
    //    void signalLabel
    void clicked(const QDate &date);
    void showMore(bool isshow);
    void selectionChanged();
    void Up(int n);
    void allschedule();
    void yijiChangeDown(int num);
    //  true：显示农历、黄历 , false：不显示农历、黄历
    void almanacChanged(bool);
    void DayClick(QDate date);//月视图内每一个日小框双击信号
    void newschdule(int n);
    void updateSlideEffect();
};


class PartLineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PartLineWidget(QWidget *parent = nullptr);
    void paintEvent(QPaintEvent *event);

};


#endif // LUNARCALENDARWIDGET_H
