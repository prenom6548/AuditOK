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
#include "frmlunarcalendarwidget.h"
#include "ui_frmlunarcalendarwidget.h"
#include <QPainter>
#include <KWindowEffects>
#define TRANSPARENCY_SETTINGS       "org.ukui.control-center.personalise"
#define TRANSPARENCY_KEY            "transparency"
#define PANEL_CONTROL_IN_CALENDAR "org.ukui.control-center.panel.plugins"
#define LUNAR_KEY "calendar"
#define FIRST_DAY_KEY "firstday"
#include <QDBusInterface>
#include <QTranslator>
#include <QScreen>
#include <libkydatacollect.h>

const int CALENDAR_WIDTH = 452;
const int CALENDAR_HEIGHT = 780;
const int CALENDAR_LUNAR_HEIGHT = 75;
const int CALENDAR_LUNAR_HEIGHT_PLUSE = 77; //852
const int SCHEDULE_ITEM = 30;
const int WAYLAND_WIDGET_GAP = 22;

#define CALENDAR_BUTTON_WIDTH  300


#define SCROLLA 40
#define ORIGIN_CALENDAR_HEIGHT 600
#define LESSTWOSHCEDULE_CALENDAR_HEIGHT 740
#define ONE_CALENDAR_SCHEDULE 690
#define ORIGIN_CALENDAR_WIDTH 452
#define TWO_MORE_SCHEDULE_HEIGHT 835
#define THREE_SCHEDULE_HEIGHT 845
#define FOUR_MORE_SCHEDULE_HEIGHT 925
#define YIJI_HEIGHT 55
#define WIDGET_GAP 8
static const QString NotificationsDBusService = "org.ukui.calendar.Notifications";
static const QString NotificationsDBusPath = "/org/ukui/calendar/Notifications";
#define PANEL_SETTINGS      "org.ukui.panel.settings"
#define PANEL_SIZE_KEY      "panelsize"
#define PANEL_POSITION_KEY  "panelposition"
#include "notificationsadaptor.h"
frmLunarCalendarWidget::frmLunarCalendarWidget(QWidget *parent) : QWidget(parent), ui(new Ui::frmLunarCalendarWidget)
{
    installEventFilter(this);
    //加载翻译
    translator();
    //    ui->setupUi(this);
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.interface()->registerService(
        NotificationsDBusService,
        QDBusConnectionInterface::ReplaceExistingService,
        QDBusConnectionInterface::AllowReplacement);
    connection.registerObject(NotificationsDBusPath, this);
    NotificationsAdaptor *s = new NotificationsAdaptor(this);
    m_widget = new LunarCalendarWidget();
    KWindowSystem::setState(m_widget->winId(), NET::SkipPager | NET::SkipTaskbar | NET::SkipSwitcher);
    connect(s, &NotificationsAdaptor::dayWantToChange, this, &frmLunarCalendarWidget::slotdayWantToChange);
    connect(m_widget, &LunarCalendarWidget::NeedToHide, this, [ = ]()
            {
                isWidgetVisible = false;
            });
    connect(m_widget, &LunarCalendarWidget::updateSlideEffect, this, &frmLunarCalendarWidget::slotUpdateSlideEffect);
    connect(ui->lunarCalendarWidget, &LunarCalendarWidget::almanacChanged, this, &frmLunarCalendarWidget::showAlmanac);
    connect(this, &frmLunarCalendarWidget::onShowToday, ui->lunarCalendarWidget, &LunarCalendarWidget::showToday);

    if(QString(qgetenv("XDG_SESSION_TYPE")) == "wayland")
    {
        //        qputenv("QT_QPA_PLATFORM", "wayland"); //openKylin
        isWayland = true;
    }
    else
    {
        isWayland = false;
    }
    const QByteArray transparency_id(TRANSPARENCY_SETTINGS);
    if(QGSettings::isSchemaInstalled(transparency_id))
    {
        transparency_gsettings = new QGSettings(transparency_id);
    }
    const QByteArray calendar_id(PANEL_CONTROL_IN_CALENDAR);
    if(QGSettings::isSchemaInstalled(calendar_id))
    {
        calendar_gsettings = new QGSettings(calendar_id);
        //公历/农历切换
        connect(calendar_gsettings, &QGSettings::changed, this, [ = ] (const QString & key)
                {
                    if(key == LUNAR_KEY)
                    {
                        ckShowLunar_stateChanged(calendar_gsettings->get(LUNAR_KEY).toString() == "lunar");
                    }
                    if (key == FIRST_DAY_KEY)
                    {
                        cboxWeekNameFormat_currentIndexChanged(calendar_gsettings->get(FIRST_DAY_KEY).toString() == "sunday");
                    }
                });
    }
    else
    {
        ckShowLunar_stateChanged(false);
        cboxWeekNameFormat_currentIndexChanged(false);
    }
    setProperty("useStyleWindowManager", false);
    initPanelGSettings();

    QDBusConnection sessionBus = QDBusConnection::sessionBus();
    if (sessionBus.registerService("org.ukui.kylin_calendar"))
    {
        sessionBus.registerObject("/org/ukui/kylin_calendar", this, QDBusConnection::ExportAllContents);

        qDebug() << "初始化DBUS成功";
    }
    else
        qDebug() << "初始化DBUS失败";

    QDBusConnection::sessionBus().connect(
        "activate", // 发送者的服务名，留空表示任何服务
        "/org/ukui/kylin_calendar", // 发送者的对象路径
        "org.ukui.kylin_calendar", // 发送者的接口名
        "command_Control", // 方法名
        this, // 接收者
        SLOT(onActivateRequest()) // 槽函数
        );

    if (!m_argName.isEmpty())
    {
        qDebug() << "单例参数argName = " << m_argName;
        int num = m_argName.size();
        switch (num)
        {
        case 1:
            if(QFileInfo::exists(m_argName[0]))
            {
                QMessageBox::warning(this, tr("Alert"), tr("Recorder does not support selecting the opening method to open files!"));
                break;
            }
            else
            {
                command_Control(m_argName[0]);
                break;
            }
        default:
            break;
        }
    }
    m_isFirstObject = false; //可以接收外部命令
}

frmLunarCalendarWidget::~frmLunarCalendarWidget()
{
    qDebug() << "~frmLunarCalendarWidget()";
    delete ui;
}

void frmLunarCalendarWidget::initPanelGSettings()
{
    const QByteArray id(PANEL_SETTINGS);
    if (QGSettings::isSchemaInstalled(id))
    {
        if (m_panelGSettings == nullptr)
        {
            m_panelGSettings = new QGSettings(id);
        }
        if (m_panelGSettings->keys().contains(PANEL_POSITION_KEY))
        {
            m_panelPosition = m_panelGSettings->get(PANEL_POSITION_KEY).toInt();
        }
        connect(m_panelGSettings, &QGSettings::changed, this, [&] (const QString & key)
                {
                    if(key == PANEL_POSITION_KEY)
                    {
                        m_panelPosition = m_panelGSettings->get(PANEL_POSITION_KEY).toInt();
                    }
                });
    }
}

void frmLunarCalendarWidget::onActivateRequest()
{
    qDebug() << "onActivateRequest";
    if(m_widget->isVisible())
    {
        UkuiWindowHelper *windowHelper = new UkuiWindowHelper(m_widget);
        QPair<UkuiWindowHelper::Position, int> pair = getSlideEffectPos();
        windowHelper->setSlideEffect(pair.first,pair.second);
        m_widget->hide();
    }
    else
    {
        if(m_widget->isWayland)
        {
            UkuiWindowHelper *windowHelper = new UkuiWindowHelper(m_widget);
            windowHelper->removeTitleBar();
            QPair<UkuiWindowHelper::Position, int> pair = getSlideEffectPos();
            windowHelper->setSlideEffect(pair.first,pair.second);
            windowHelper->setSkipTaskBar(true);
            windowHelper->setSkipSwitcher(true);
            windowHelper->setBlurEffect(QRegion(), 0, true);

            QScreen* pScreen = windowHelper->getCurrentScreenBlocking();
            if(!pScreen)
            {
                pScreen = qApp->primaryScreen();
            }
            m_widget->setScreen(pScreen);
            m_widget->show();
            m_widget->raise();
            m_widget->activateWindow();
        }
        else
        {
            kdk::WindowManager::setSkipSwitcher(this->windowHandle(), true);
            kdk::WindowManager::setSkipTaskBar(this->windowHandle(), true);

            m_widget->show();
            m_widget->raise();
            m_widget->activateWindow();
        }
    }
}

void frmLunarCalendarWidget::checkSingle(QStringList path) //检查单例模式
{
    QString str;
    if (path.size() > 1)
    {
        str = path[1];
    }

    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    //兼容VNC的单例模式
    QString lockPath = homePath.at(0) + "/.config/kylin-calendar-lock";
    int fd = open(lockPath.toUtf8().data(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0)
    {
        exit(1);
    }
    if (lockf(fd, F_TLOCK, 0))
    {
        QDBusInterface interface("org.ukui.kylin_calendar", "/org/ukui/kylin_calendar", "org.ukui.kylin_calendar",
                                 QDBusConnection::sessionBus());
        if (path.size() == 1)
            interface.call("command_Control", str);
        if (path.size() == 2)
            interface.call("command_Control", str);
        qDebug() << "Can't lock single file, kylin-calendar is already running!";
        exit(0);
    }
    m_isFirstObject = true; //我是首个对象
    m_argName << str;
}


void frmLunarCalendarWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange)
    {
        auto windowState = window()->windowState();
        if (windowState & Qt::WindowMinimized)
        {
            // 隐藏窗口至托盘
            showMinimized();
        }
    }
    QWidget::changeEvent(event);
}

QPair<UkuiWindowHelper::Position, int> frmLunarCalendarWidget::getSlideEffectPos()
{
    QPair<UkuiWindowHelper::Position, int> pair;
    UkuiWindowHelper::Position position = UkuiWindowHelper::Position::Bottom;
    m_nPanelSize = m_panelGSettings->get(PANEL_SIZE_KEY).toInt();
    int offset = m_nPanelSize;
    switch (m_panelPosition) {
    case 0: // bottom
        position = UkuiWindowHelper::Position::Bottom;;
        break;
    case 1: // 顶部任务栏
        position = UkuiWindowHelper::Position::Top;
        break;
    case 2: // 左侧任务栏
        position = UkuiWindowHelper::Position::Left;
        break;
    case 3: // right
        position = UkuiWindowHelper::Position::Right;
        break;
    default:
        break;
    }

    pair.first = position;
    pair.second = offset;
    return pair;
}

int frmLunarCalendarWidget::command_Control(QString cmd1)
{
    qDebug() << "命令:" << cmd1;
    if (cmd1 == "")   // 无参数，切换显示/隐藏状态
    {
        qDebug() << "是空";
        if (isWidgetVisible)
        {
            UkuiWindowHelper *windowHelper = new UkuiWindowHelper(m_widget);
            QPair<UkuiWindowHelper::Position, int> pair = getSlideEffectPos();
            windowHelper->setSlideEffect(pair.first,pair.second);
            // 如果当前是显示的，则隐藏
            m_widget->hide();
            qDebug() << "窗口隐藏";
            isWidgetVisible = false; // 更新状态
            kdk_dia_upload_program_exit();
        }
        else
        {
            // 如果当前是隐藏的，则显示
            changeWidowposFromClickButton(true); // 假设这个函数是为了某种布局或位置调整
            qDebug() << "窗口置顶并显示";
            isWidgetVisible = true; // 更新状态
            kdk_dia_upload_program_startup();
        }
        return 0;
    }
    else if (cmd1 == "-s" || cmd1 == "-show") //打开日历
    {
        if(m_widget->isWayland)
        {
            UkuiWindowHelper *windowHelper = new UkuiWindowHelper(m_widget);
            windowHelper->removeTitleBar();
            QPair<UkuiWindowHelper::Position, int> pair = getSlideEffectPos();
            windowHelper->setSlideEffect(pair.first,pair.second);
            windowHelper->setSkipTaskBar(true);
            windowHelper->setSkipSwitcher(true);
            windowHelper->setBlurEffect(QRegion(), 0, true);

            QScreen* pScreen = windowHelper->getCurrentScreenBlocking();
            if(!pScreen)
            {
                pScreen = qApp->primaryScreen();
            }
            m_widget->setScreen(pScreen);
            m_widget->show();
            m_widget->raise();
            m_widget->activateWindow();
        }
        else
        {
            kdk::WindowManager::setSkipSwitcher(this->windowHandle(), true);
            kdk::WindowManager::setSkipTaskBar(this->windowHandle(), true);
            m_widget->show();
            m_widget->raise();
            m_widget->activateWindow();
        }
        return 0;
    }
    else if (cmd1 == "-h" || cmd1 == "-hide") //隐藏
    {
        qDebug() << "---h";
        showMinimized();
        return 0;
    }
    else
    {
        QMessageBox::warning(this, tr("Alert"), tr("Calendar does not support opening file in open mode."));
    }
    return 0; //重要:int类型的函数一定要加返回值
}

void frmLunarCalendarWidget::showAlmanac(bool big)
{
    qDebug() << __FILE__ << __LINE__ << __FUNCTION__ << big;
    if(big)
    {
        m_widget->setFixedHeight(CALENDAR_HEIGHT + CALENDAR_LUNAR_HEIGHT + CALENDAR_LUNAR_HEIGHT_PLUSE);
    }
    else
    {
        m_widget->setFixedHeight(CALENDAR_HEIGHT);
    }

    // 尺寸变化后重新计算位置
    if (m_widget->isVisible()) {
        QTimer::singleShot(0, m_widget, [this]() {
            m_widget->calculateWindowGeometry();
        });
    }
}

void frmLunarCalendarWidget::cboxCalendarStyle_currentIndexChanged(int index)
{
    m_widget->setCalendarStyle((LunarCalendarWidget::CalendarStyle)index);
}

void frmLunarCalendarWidget::cboxSelectType_currentIndexChanged(int index)
{
    m_widget->setSelectType((LunarCalendarWidget::SelectType)index);
}

void frmLunarCalendarWidget::cboxWeekNameFormat_currentIndexChanged(bool FirstDayisSun)
{
    m_widget->setWeekNameFormat(FirstDayisSun);
}

void frmLunarCalendarWidget::ckShowLunar_stateChanged(bool arg1)
{
    if (arg1)
    {
        if (yijiIsUp)
        {
            this->setFixedSize(CALENDAR_WIDTH, CALENDAR_HEIGHT + CALENDAR_LUNAR_HEIGHT + CALENDAR_LUNAR_HEIGHT_PLUSE);
        }
        else
        {
            this->setFixedSize(CALENDAR_WIDTH, CALENDAR_HEIGHT + CALENDAR_LUNAR_HEIGHT);
        }
    }
    else
    {
        this->setFixedSize(CALENDAR_WIDTH, CALENDAR_HEIGHT);
    }
}

void frmLunarCalendarWidget::paintEvent(QPaintEvent *)
{
    QStyleOption opt;
    opt.init(this);
    QRect rect = this->rect();
    QPainter p(this);
    double tran = 1;
    const QByteArray transparency_id(TRANSPARENCY_SETTINGS);
    if(transparency_gsettings != nullptr)
    {
        tran = transparency_gsettings->get(TRANSPARENCY_KEY).toDouble() * 255;
    }

    QColor color = palette().color(QPalette::Base);
    color.setAlpha(tran);
    QBrush brush = QBrush(color);
    p.setBrush(brush);
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawRect(rect);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

/*
 * 事件过滤，检测鼠标点击外部活动区域则收回收纳栏
*/
bool frmLunarCalendarWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this)
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                return true;
            }
            else if(mouseEvent->button() == Qt::RightButton)
            {
                return true;
            }
        }
        else if(event->type() == QEvent::ContextMenu)
        {
            return false;
        }
        else if (event->type() == QEvent::WindowDeactivate)
        {
            isWidgetVisible = false;
            return true;
        }
        else if (event->type() == QEvent::StyleChange)
        {
        }
    }

    return false;
}

/**
 * @brief V11任务栏点击响应
 * @param restore
 */
void frmLunarCalendarWidget::changeWidowposFromClickButton(bool restore)
{
    qDebug() << "changeWidowposFromClickButton";
    CalendarDataBase::getInstance().searchDataSlot(QDate::currentDate());
    m_num = CalendarDataBase::getInstance().getInfoList().count();

    if(m_widget->isWayland)
    {
        UkuiWindowHelper *windowHelper = new UkuiWindowHelper(m_widget);
        windowHelper->removeTitleBar();
        QPair<UkuiWindowHelper::Position, int> pair = getSlideEffectPos();
        windowHelper->setSlideEffect(pair.first,pair.second);
        windowHelper->setSkipTaskBar(true);
        windowHelper->setSkipSwitcher(true);
        windowHelper->setBlurEffect(QRegion(), 0, true);

        QScreen* pScreen = windowHelper->getCurrentScreenBlocking();
        if(!pScreen)
        {
            pScreen = qApp->primaryScreen();
        }
        m_widget->setScreen(pScreen);
        m_widget->showCalendar(restore);
        m_widget->raise();
        m_widget->activateWindow();
    }
    else
    {
        kdk::WindowManager::setSkipSwitcher(this->windowHandle(), true);
        kdk::WindowManager::setSkipTaskBar(this->windowHandle(), true);
        KWindowSystem::setState(m_widget->winId(), NET::SkipPager | NET::SkipTaskbar | NET::SkipSwitcher);
        m_widget->showCalendar(restore);
    }
}

void frmLunarCalendarWidget::translator()
{
    QTranslator *translator = new QTranslator(this);
    translator->load(QLocale(), "calendar", "_", CALENDAR_TRANSLATION_DIR);
    QCoreApplication::installTranslator(translator);
}

void frmLunarCalendarWidget::slotdayWantToChange(int day, int month, int year)
{
    if((year <= 1970) || (year >= 2099))
    {
        return;
    }
    if((month < 1) || (month > 12))
    {
        return;
    }

    if((day < 1) || (day > 31))
    {
        return;
    }

    QDate date(year, month, day);
    if (isWidgetVisible)
    {
        m_widget->date = m_widget->s_clickDate = date;
        m_widget->dateChanged(year, month, day);
        m_widget->dayChanged(date, date);
        m_widget->updateAllSchedule();
        m_widget->updateLayout();
        CalendarDataBase::getInstance().searchDataSlot(date);
        m_widget->activateWindow();  // 确保窗口获得焦点
        m_widget->raise();           // 确保窗口置顶
        isWidgetVisible = true; // 更新状态
    }
    else
    {
        // 如果当前是隐藏的，则显示
        changeWidowposFromClickButton(false);
        m_widget->date = m_widget->s_clickDate = date;
        m_widget->dateChanged(year, month, day);
        m_widget->dayChanged(date, date);
        m_widget->updateAllSchedule();
        m_widget->updateLayout();
        CalendarDataBase::getInstance().searchDataSlot(date);
        if(m_widget->isWayland)
        {
            UkuiWindowHelper *windowHelper = new UkuiWindowHelper(m_widget);
            windowHelper->removeTitleBar();
            QPair<UkuiWindowHelper::Position, int> pair = getSlideEffectPos();
            windowHelper->setSlideEffect(pair.first,pair.second);
            windowHelper->setSkipTaskBar(true);
            windowHelper->setSkipSwitcher(true);
            windowHelper->setBlurEffect(QRegion(), 0, true);

            QScreen* pScreen = windowHelper->getCurrentScreenBlocking();
            if(!pScreen)
            {
                pScreen = qApp->primaryScreen();
            }

            m_widget->setScreen(pScreen);
            m_widget->show();
            m_widget->raise();
            m_widget->activateWindow();
        }
        else
        {
            kdk::WindowManager::setSkipSwitcher(this->windowHandle(), true);
            kdk::WindowManager::setSkipTaskBar(this->windowHandle(), true);

            m_widget->show();
            m_widget->activateWindow();
            m_widget->raise();
        }

        isWidgetVisible = true; // 更新状态
    }
}

void frmLunarCalendarWidget::slotUpdateSlideEffect()
{
    UkuiWindowHelper *windowHelper = new UkuiWindowHelper(m_widget);
    QPair<UkuiWindowHelper::Position, int> pair = getSlideEffectPos();
    windowHelper->setSlideEffect(pair.first,pair.second);
}

void frmLunarCalendarWidget::setRect(QRect &rect, int x, int y, int width, int height)
{
    rect.setRect(x, y, width, height);
}

bool frmLunarCalendarWidget::isRTLLanguage()
{
    // 获取当前系统的语言代码
    QLocale locale = QLocale::system();
    QString languageCode = locale.name();
    QString languageName;
    qDebug() << "languageCode" << languageCode;
    int separatorIndex = languageCode.indexOf('_');
    if (separatorIndex == -1)
    {
        separatorIndex = languageCode.indexOf('-');
    }
    // 如果找到了分隔符，就剥离出语言代码部分
    if (separatorIndex != -1)
    {
        languageName =  languageCode.left(separatorIndex);
    }
    QStringList rtlLanguages = {"ar", "fa", "he", "ps", "ur", "kk", "ky", "ug"};
    // 检查当前语言代码是否在RTL语言列表中
    return rtlLanguages.contains(languageName);
}
