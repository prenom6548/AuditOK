/*
 * Ukui-shutdown-timer
 *
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
 *
 * Authors:  liushanwen <liushanwen@kylinos.cn>
 *
 */

#include "widget.h"
#include <QPainterPath>
#include <QWindow>

/* widget主类 */
Widget::Widget(KWidget *parent)
    : KWidget(parent)
{
    // 国际化
    initTranslation();

    // 初始化窗口设置
    initWindowSettings();

    // 初始化成员变量
    initMemberVariable();

    // 初始化界面布局
    initLayout();

    // 初始化所有成员变量的信号槽
    initSignalSlots();

    // 初始化gsetting值
    initGSettings();

    // 创建当前动作
    createActions();

    // 初始化当前的托盘栏图标
    createTrayIcon();

    // 获取保存在gsetting中的定时关机设置状态
    getTimedShutdownState();

    // 获取保存在gsetting中的设置的时间
    getShutDownTime();

    // 获取当前关机频率
    getComBoxShutdownFrequency();

    // 初始化定时关机显示界面
    initTimeShutdownWidget();

    // 初始化下拉框状态
    initDropDownBoxLabelStatus();
    QDesktopWidget *pDesk = QApplication::desktop();
    move((pDesk->width() - this->width()) / 2, (pDesk->height() - this->height()) / 2);
}

Widget::~Widget()
{

}

void Widget::initTranslation()
{
    m_ptranslator = new QTranslator;
    QTranslator *translatorQt = new QTranslator;
    //获取系统语言环境 选择翻译文件
    if (m_ptranslator->load(QLocale(), QLatin1String("time-shutdown"),
                            QLatin1String("_"), QM_FILES_INSTALL_PATH))
        QApplication::installTranslator(m_ptranslator);
    else
        qDebug() << "cannot load translator ukui-time-shutdown" << QLocale::system().name() << ".qm!";

    if (translatorQt->load("/usr/share/qt5/translations/qt_" + QLocale::system().name()))
        QApplication::installTranslator(translatorQt);
    else
        qDebug() << "cannot load translator ukui-time-shutdown" << QLocale::system().name() << ".qm!";
}

void Widget::initWindowSettings()
{
    this->setIcon(QIcon::fromTheme("time-shutdown", QIcon(":/data/time-shutdown/time-shutdown.png")));
    this->setWidgetName(QObject::tr("Time-shutdown"));
    this->windowButtonBar()->setWindowTitle(QObject::tr("Time-shutdown"));
    this->windowButtonBar()->maximumButton()->hide();

    this->windowButtonBar()->menuButton()->setToolTip(QObject::tr("option"));
    this->windowButtonBar()->minimumButton()->setToolTip(QObject::tr("minimize"));
    this->windowButtonBar()->closeButton()->setToolTip(QObject::tr("close"));

    KMenuButton* menuButton = this->windowButtonBar()->menuButton();
    menuButton->menu()->removeAction(this->windowButtonBar()->menuButton()->settingAction());
    menuButton->menu()->removeAction(this->windowButtonBar()->menuButton()->themeAction());

    connect(menuButton->assistAction(), &QAction::triggered, this, [=](){
        kdk::UserManual userManualTest;
        if (!userManualTest.callUserManual("time-shutdown")) {
            qCritical() << "user manual call fail!";
        }
    });

    connect(menuButton->aboutAction(), &QAction::triggered, this, [=](){
        kdk::KAboutDialog aboutWindow(this, QIcon::fromTheme("time-shutdown", QIcon(":/data/time-shutdown/time-shutdown.png")), QObject::tr("Time-shutdown"), tr("Version: ") + getCurrentVersion());
        aboutWindow.exec();
    });
    connect(menuButton->quitAction(), &QAction::triggered, this, [=](){exit(0);});
}

QString Widget::getCurrentVersion()
{
    QString version = "3.25.0.0";
    QProcess p;
    QStringList arguments;
    arguments << "-l" << "time-shutdown";
    p.start("dpkg", arguments);
    p.waitForStarted();
    p.waitForFinished();

    QString str = QString::fromUtf8(p.readAllStandardOutput());
    QStringList strList = str.split(" ");
    strList.removeAll("");
    for (int var = 1; var < strList.size(); ++var) {
        if (strList.at(var - 1) == "time-shutdown") {
            version = strList.at(var);
            break;
        }
    }
    p.close();

    return version;
}

void Widget::initMemberVariable()
{
    this->setContentsMargins(0, 0, 0, 0);
    this->setFixedSize(372, 377);
    this->setLayoutType(VerticalType);

    // 提示关机时间Label
    //固定大小字体，代码或许可以简写
    m_pTimeRemainLabel = new QLabel();
    QFont font = m_pTimeRemainLabel->font();
    font.setPixelSize(12);
    m_pTimeRemainLabel->setFont(font);
    m_pTimeRemainLabel->setAlignment(Qt::AlignHCenter);
    m_pTimeRemainLabel->setFixedHeight(18);

    m_traslate                      = QObject::tr("next shutdown");
    m_traslateHours                 = QObject::tr("hour");
    m_traslateMinute                = QObject::tr("minute");

    // 初始化定时器
    m_pMonitorTime       = new QTimer();

    // 初始化透明窗口,存在问题
    m_pTransparentWidget = new QWidget();
    m_pTransparentWidget->move(116, 120);
    m_pTransparentWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_pTransparentWidget->setFixedSize(380, 155);

    m_pComBoxWidget      = new comBoxWidget(this); //关机频率显示栏
    m_pDropDownBox = new DropDownMenu();
    m_pConfirmAreaWidget = new confirmAreaWidget(); //取消，确认显示区
    m_pTimeShowWidget    = new timeShowWidget(); //时间显示区
    m_pBlankShadowWidget = new BlankShadowWidget(this);

    m_pWindowHelper = new UkuiWindowHelper(this);
    m_pWindowHelper->setBlurEffect(QRegion());
}

void Widget::initLayout()
{
    m_pBlankShadowWidget->move(30, 182);
    m_pComBoxWidget->move(30, 60);

    this->layout()->addItem(new QSpacerItem(10, 81));
    this->layout()->addWidget(m_pTimeShowWidget);
    this->layout()->addItem(new QSpacerItem(10, 16));
    this->layout()->addWidget(m_pTimeRemainLabel);
    this->layout()->addItem(new QSpacerItem(10, 24));
    this->layout()->addWidget(m_pConfirmAreaWidget);
    this->layout()->addItem(new QSpacerItem(10, 32, QSizePolicy::Fixed));

    m_pTransparentWidget->setParent(this);
    m_pDropDownBox->setVisible(false);
    return;
}

void Widget::initSignalSlots()
{
    connect(m_pComBoxWidget, &comBoxWidget::comBoxWidgetClicked, this, &Widget::dropDownBoxShowHideSlots);          // 下拉框点击槽函数
    connect(m_pDropDownBox, &DropDownMenu::aboutToHide, this, &Widget::updatedropDownBoxSelectSlots);     // ListWidget条目点击槽函数
    connect(m_pMonitorTime, &QTimer::timeout, this, &Widget::threadSlots);                                          // 定时器
    connect(m_pConfirmAreaWidget->m_pConfirmButton, &QPushButton::clicked, this, &Widget::confirmButtonSlots);      // 确认按钮槽函数
    connect(m_pConfirmAreaWidget->m_pCancelButton, &QPushButton::clicked, this, &Widget::canceButtonSlots);         // 取消按钮槽函数
}

//初始化下拉框选项选中状态
void Widget::initDropDownBoxLabelStatus()
{
    m_pDropDownBox->initLabelStatus(m_weekSelect);
}

void Widget::initGSettings()
{
    if (!QGSettings::isSchemaInstalled(UKUITIMESHUTDOWN)) {
        qCritical() << "Time shutdown GSettings schema not installed:" << UKUITIMESHUTDOWN;
        QCoreApplication::quit();
    }
    m_pTimeShutdownGSettings = new QGSettings(UKUITIMESHUTDOWN);

    initStyleGSettings();
}

void Widget::initStyleGSettings()
{
    if (!QGSettings::isSchemaInstalled(ORG_UKUI_STYLE)) {
        qWarning() << "Style GSettings schema not installed:" << ORG_UKUI_STYLE;
        return;
    }

    m_pStyleGSettings = new QGSettings(ORG_UKUI_STYLE);
    if (m_pStyleGSettings == nullptr) {
        qWarning() << "Style GSettings is nullptr";
        return;
    }

    connect(m_pStyleGSettings, &QGSettings::changed, this, [this](const QString& keyName) {
        if (keyName == "styleName") {
            m_pDropDownBox->repaint();
        }
    });
}

void Widget::createActions()
{
    restoreAction = new QAction(QIcon(":/data/time-shutdown/time-shutdown.png"), tr("&Restore"), this);
    connect(restoreAction, &QAction::triggered, this, &Widget::showScreen);

    quitAction = new QAction(QIcon::fromTheme("application-exit-symbolic"), tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
}

void Widget::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setToolTip(QObject::tr("Time-shutdown"));
    connect(trayIcon, &QSystemTrayIcon::activated, this, &Widget::iconActivated);
    trayIcon->setIcon(QIcon::fromTheme("time-shutdown", QIcon(":/data/time-shutdown/time-shutdown.png")));
    trayIcon->show();
}

/* 获取关机频率字符串 */
void Widget::getComBoxShutdownFrequency()
{
    if (m_timeShutdown && m_pTimeShutdownGSettings != nullptr) {
        QStringList keyList = m_pTimeShutdownGSettings->keys();
        if (keyList.contains("shutdownfrequency")) {
            m_selectIndexGsetting = m_pTimeShutdownGSettings->get("shutdownfrequency").toString();
            m_weekSelect = m_pDropDownBox->getWeekDateList(m_selectIndexGsetting);
        }
        disconnect(m_pComBoxWidget, &comBoxWidget::comBoxWidgetClicked, this, &Widget::dropDownBoxShowHideSlots);
    } else {
        m_weekSelect.clear();
        m_weekSelect.append(NEVER_SHUTDON);
    }
    return;
}

/* 获取关机时间 */
void Widget::getShutDownTime()
{
    if (m_timeShutdown && m_pTimeShutdownGSettings != nullptr) {
        QStringList keyList = m_pTimeShutdownGSettings->keys();
        if (keyList.contains("shutdowntime")) {
            m_pShowDownTime = m_pTimeShutdownGSettings->get("shutdowntime").toString();
        }
    } else {
        m_pShowDownTime.clear();
        QTime time = QTime::currentTime();
        int timeH = time.hour();
        int timeM = time.minute();
        m_hours = timeH;
        m_minute = timeM;
        return;
    }
    QStringList timeList = m_pShowDownTime.split(":");
    qDebug() << "timeList---->" << timeList.count();
    if (timeList.count() > 1) {
        m_hours  = QString(timeList.at(0)).toInt();
        m_minute = QString(timeList.at(1)).toInt();
    }
    return;
}

/*  获取定时关机状态 */
bool Widget::getTimedShutdownState()
{
    if (m_pTimeShutdownGSettings != nullptr) {
        QStringList keyList = m_pTimeShutdownGSettings->keys();
        if (keyList.contains("timeshutdown")) {
            m_timeShutdown = m_pTimeShutdownGSettings->get("timeshutdown").toBool();
        }
    }
    if (!m_timeShutdown) {
        m_pConfirmAreaWidget->m_pCancelButton->setEnabled(false);
    }
    return m_timeShutdown;
}

/* 初始化当前的启动界面 */
void Widget::initTimeShutdownWidget()
{
    // 设置保存在gsetting中已设置好的时:分
    m_pTimeShowWidget->m_pHourRollWidget->setCurrentValue(m_hours);
    m_pTimeShowWidget->m_pMinuteRollWidget->setCurrentValue(m_minute);
    m_pComBoxWidget->setLabelWeekSelect(m_pDropDownBox->getChosenList(m_weekSelect));

    if (m_timeShutdown) {
        m_pMonitorTime->start(1000);                                            // 开始轮训，每秒轮训一次
        m_pTimeShowWidget->m_pHourRollWidget->setColor(false);                  // 修改时间界面透明度
        m_pTimeShowWidget->m_pMinuteRollWidget->setColor(false);
        m_pTimeShowWidget->m_pHourRollWidget->update();
        m_pTimeShowWidget->m_pMinuteRollWidget->update();
        m_pTransparentWidget->setVisible(true);                                 // 置时间调试界面为不可滚动
        m_pBlankShadowWidget->setVisible(true);
        m_pShowDownTime = QStringLiteral("%1:%2").arg(m_hours).arg(m_minute);
        m_pConfirmAreaWidget->m_pConfirmButton->setEnabled(false);
    } else {
        m_pMonitorTime->stop();
        m_pTimeShowWidget->m_pHourRollWidget->setColor(true);
        m_pTimeShowWidget->m_pMinuteRollWidget->setColor(true);
        m_pTimeShowWidget->m_pHourRollWidget->update();
        m_pTimeShowWidget->m_pMinuteRollWidget->update();
        m_pTransparentWidget->setVisible(false);
        m_pBlankShadowWidget->setVisible(false);
    }
}

/* 设置定时关机时状态 */
void Widget::setTimeShutdownValue(bool value)
{
    m_pTimeShutdownGSettings->set("timeshutdown", value);
    m_timeShutdown = value;
    return;
}

/* 设置定时关机时间 */
void Widget::setShutDownTimeValue(QString value)
{
    m_pTimeShutdownGSettings->set("shutdowntime", value);
    m_pShowDownTime = value;
    return;
}

/* 设置定时关机频率 */
void Widget::setFrequencyValue(QString value)
{
    m_pTimeShutdownGSettings->set("shutdownfrequency", value);
    return;
}

void Widget::setWeekSelectShutdownFrequency()
{
    if (m_weekSelect[0] == NEVER_SHUTDON) {
        setTimeShutdownValue(false);
    } else if (m_weekSelect[0] == ONLY_ONCE) {
        setTimeShutdownValue(false);//存在问题，需要复查,当仅一次关机的情况下，是否应该当在此时将状态设置为false
        canceButtonSlots();
    } else {
        setTimeShutdownValue(true);
    }
}

void Widget::setShutdownFrequency(QList<int> list)
{
    if (list[0] == NEVER_SHUTDON) {
        setTimeShutdownValue(false);
    } else {
        setTimeShutdownValue(true);
    }
}

void Widget::setNextShutDownTime()
{
    QString shutDownTime;
    if (m_weekSelect[0] == NEVER_SHUTDON) {
        m_pTimeRemainLabel->setVisible(false);
    } else {
        if (!m_pTimeRemainLabel->isVisible()) {
            m_pTimeRemainLabel->setVisible(true);
        }
        shutDownTime = getCalculateShutDownTime(m_weekSelect);
    }
    qDebug() << shutDownTime;
    QStringList timeList = shutDownTime.split(":");
    m_pTimeRemainLabel->setText(QStringLiteral("%1 %2 %3 %4 %5").arg(m_traslate).arg(timeList.at(0)).arg(m_traslateHours).arg(timeList.at(1)).arg(m_traslateMinute));
}

//获取当前系统时间的星期所对应的选项，返回int型数字？
int Widget::getCurrentShutDownNum()
{
    QDate date = QDateTime::currentDateTime().date();
    return date.dayOfWeek() + 2;
}

// 计算下一次需要关机的星期
int Widget::caculateNextShutDownNum(int currentTmp, QList<int> weekSelect)
{
    if(weekSelect[0] == EVERYDAY) {
        weekSelect.clear();
        if (currentTmp == SUNDAY) {
            weekSelect = {MONDAY, SUNDAY};
        } else {
            weekSelect = {currentTmp, currentTmp + 1};
        }
    }

    QTime time = QTime::currentTime();
    int timeHours = time.hour();
    int minite = time.minute();
    for (int i = 0; i < weekSelect.count(); i++) {
        if ((weekSelect[i] == currentTmp
                && ((timeHours == m_pTimeShowWidget->m_pHourRollWidget->readValue()
                && minite < m_pTimeShowWidget->m_pMinuteRollWidget->readValue())
                    || timeHours < m_pTimeShowWidget->m_pHourRollWidget->readValue()))
                || weekSelect[i] > currentTmp) {
            return weekSelect[i];
        }
    }
    return weekSelect[0];
}

//计算出距离下次关机所需时间
QString Widget::getCalculateShutDownTime(QList<int> weekSelect)
{
    m_currentTmp = getCurrentShutDownNum();
    if (weekSelect[0] == ONLY_ONCE) {
        m_nextCurrenttmp = m_currentTmp;
    } else {
        m_nextCurrenttmp = caculateNextShutDownNum(m_currentTmp, weekSelect);
    }

    QTime time = QTime::currentTime();
    int timeMinute = time.minute();
    int timeHours = time.hour();
    int hour, minute = 0;
    qDebug() << "当前关机序号" << m_currentTmp << "下一次关机序号" << m_nextCurrenttmp;

    if (m_nextCurrenttmp > m_currentTmp) {
        hour = 24 * (m_nextCurrenttmp - m_currentTmp)- (timeHours - m_pTimeShowWidget->m_pHourRollWidget->readValue());
    } else if (m_nextCurrenttmp < m_currentTmp) {
        hour = 24 * (7 - (m_currentTmp - m_nextCurrenttmp)) - (timeHours - m_pTimeShowWidget->m_pHourRollWidget->readValue());
        // 今天之前
    } else if (m_nextCurrenttmp == m_currentTmp &&
               timeHours > m_pTimeShowWidget->m_pHourRollWidget->readValue()) {
            hour = 24 * (7 - (m_currentTmp - m_nextCurrenttmp)) -
                     (timeHours - m_pTimeShowWidget->m_pHourRollWidget->readValue());
    } else if (m_nextCurrenttmp == m_currentTmp &&
               timeHours == m_pTimeShowWidget->m_pHourRollWidget->readValue() &&
               timeMinute >= m_pTimeShowWidget->m_pMinuteRollWidget->readValue()) {
        hour = 24 * 7;
    }else if (m_nextCurrenttmp == m_currentTmp &&
               timeHours <= m_pTimeShowWidget->m_pHourRollWidget->readValue()) {
        hour = m_pTimeShowWidget->m_pHourRollWidget->readValue() - timeHours;
    }

    if (timeMinute > m_pTimeShowWidget->m_pMinuteRollWidget->readValue()) {
        minute = 60 - (timeMinute - m_pTimeShowWidget->m_pMinuteRollWidget->readValue());
        hour = hour - 1;
    } else {
        minute = m_pTimeShowWidget->m_pMinuteRollWidget->readValue() - timeMinute;
    }
    return QStringLiteral("%1:%2").arg(hour).arg(minute);
}

void Widget::closeEvent(QCloseEvent *event)
{
#ifdef Q_OS_OSX
    if (!event->spontaneous() || !isVisible()) {
        return;
    }
#endif
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
    QWidget::closeEvent(event);
}

void Widget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    QPainterPath path;
    opt.rect.adjust(0,0,0,0);
    QColor color = palette().color(QPalette::Window);
    color.setAlpha(m_transparency * 255 / 100);
    p.setBrush(color);

    p.setOpacity(1);
    p.setPen(Qt::NoPen);
    p.setRenderHint(QPainter::Antialiasing); //反锯齿
    p.drawRoundedRect(opt.rect, 0, 0);
    path.addRoundedRect(opt.rect, 0, 0);
}

bool Widget::event(QEvent *event)
{
    if (m_pStyleGSettings == nullptr || !m_pStyleGSettings->keys().contains("menuTransparency")) {
        return QWidget::event(event);
    }

    if (event->type() == QEvent::WindowActivate) {
        m_transparency = m_pStyleGSettings->get("menuTransparency").toInt();
        m_pComBoxWidget->setTransparency(m_transparency);
    } else if (event->type() == QEvent::WindowDeactivate) {
        m_transparency = 100;
        m_pComBoxWidget->setTransparency(m_transparency);
    }

    return QWidget::event(event);
}

void Widget::dropDownBoxShowHideSlots()
{
    if (!m_pDropDownBox->isVisible()) {
        m_pDropDownBox->exec(this->mapToGlobal(QPoint(24, 37)));
    } else {
        m_pDropDownBox->hide();
    }
    return;
}

void Widget::updatedropDownBoxSelectSlots()
{
    m_weekSelect.clear();
    m_selectIndexGsetting.clear();
    QStringList WeekStringList = m_pDropDownBox->getWeekStringList();
    for (int i = 0; i < WeekStringList.count(); i++) {
        if (i == 0)
            m_selectIndexGsetting = WeekStringList.at(i);
        else
            m_selectIndexGsetting = m_selectIndexGsetting + " " + WeekStringList.at(i);
    }

    if (m_selectIndexGsetting == "") {
        m_selectIndexGsetting = QString().number(0);
    }
    m_weekSelect = m_pDropDownBox->getWeekDateList(m_selectIndexGsetting);
    m_pComBoxWidget->setLabelWeekSelect(m_pDropDownBox->getChosenList(m_weekSelect));
    return;
}

void Widget::confirmButtonSlots()
{
    if (m_weekSelect[0] == NEVER_SHUTDON) {
        KMessageBox *msg = new KMessageBox(this);
        msg->setIcon(KMessageBox::Warning);
        msg->setText(tr("no set shutdown"));
        QPushButton *button = new QPushButton(QObject::tr("OK"));
        button->setProperty("useButtonPalette", true);
        msg->addButton(button, KMessageBox::RejectRole);
        msg->move(this->x() - (msg->width() - this->width()) / 8, this->y() + 60);
        msg->show();
//        QMessageBox::warning(NULL, tr("warning"), tr("no set shutdown"), QMessageBox::Ok);
        return;
    }

    if (determineShutdownTime()) {
        KMessageBox *msg = new KMessageBox(this);
        msg->setIcon(KMessageBox::Warning);
        msg->setText(tr("The shutdown time is shorter than the current time"));
        QPushButton *button = new QPushButton(QObject::tr("OK"));
        button->setProperty("useButtonPalette", true);
        msg->addButton(button, KMessageBox::RejectRole);
        msg->move(this->x() - (msg->width() - this->width()) / 8, this->y() + 60);
        msg->show();
//        QMessageBox::warning(NULL, tr("warning"), \
//                             tr("The shutdown time is shorter than the current time"), \
//                             QMessageBox::Ok);
        return;
    }

    setShutdownFrequency(m_weekSelect);
    if (!m_timeShutdown) {
        return;
    }
    m_pMonitorTime->start(1000);                           // 开始轮训，每秒轮训一次
    m_pTimeShowWidget->m_pHourRollWidget->setColor(false); // 修改时间界面透明度
    m_pTimeShowWidget->m_pMinuteRollWidget->setColor(false);
    m_pTimeShowWidget->m_pHourRollWidget->update();
    m_pTimeShowWidget->m_pMinuteRollWidget->update();
    m_pTransparentWidget->setVisible(true);                // 置时间调试界面为不可滚动
    m_pBlankShadowWidget->setVisible(true);
    // 设置关机频率
    setTimeShutdownValue(true);
    setShutDownTimeValue(m_pShowDownTime);
    setFrequencyValue(m_selectIndexGsetting);
    m_pConfirmAreaWidget->m_pConfirmButton->setEnabled(false);
    m_pConfirmAreaWidget->m_pCancelButton->setEnabled(true);
    setNextShutDownTime();
    m_pComBoxWidget->setEnabled(false);

    QProcess p(0);
    p.startDetached("time-shutdown-daemon");
    p.waitForStarted();

    // 断开点击Combox展示下拉框的 信号 槽
    disconnect(m_pComBoxWidget, &comBoxWidget::comBoxWidgetClicked, this, &Widget::dropDownBoxShowHideSlots);
    return;
}

void Widget::canceButtonSlots()
{
    m_pMonitorTime->stop();                                 // 开始轮训，每秒轮训一次
    m_pTimeShowWidget->m_pHourRollWidget->setColor(true);   // 修改时间界面透明度
    m_pTimeShowWidget->m_pMinuteRollWidget->setColor(true);
    m_pTimeShowWidget->m_pHourRollWidget->update();
    m_pTimeShowWidget->m_pMinuteRollWidget->update();
    m_pTransparentWidget->setVisible(false);                // 置时间调试界面为可滚动
    m_pBlankShadowWidget->setVisible(false);
    setShutDownTimeValue(m_pShowDownTime);
    setFrequencyValue(m_selectIndexGsetting);
    setTimeShutdownValue(false);
    m_pConfirmAreaWidget->m_pConfirmButton->setEnabled(true);
    m_pTimeRemainLabel->setVisible(false);
    // 重新连接点击Combox展示下拉框的 信号 槽
    m_pComBoxWidget->setEnabled(true);
    connect(m_pComBoxWidget, &comBoxWidget::comBoxWidgetClicked, this, &Widget::dropDownBoxShowHideSlots);
    m_pConfirmAreaWidget->m_pCancelButton->setEnabled(false);
    return;
}

//只要时间变化就执行此槽函数，获取倒计时状态。
void Widget::threadSlots()
{
    QTime time = QTime::currentTime();
    int timeS = time.second();
    QTime current_time = QTime::currentTime();
    int currentHours = current_time.hour();
    int currentMinute = current_time.minute();
    QString s_currentTime = QStringLiteral("%1:%2").arg(currentHours).arg(currentMinute);
    m_timeShutdown = getTimedShutdownState();

    if (m_timeShutdown && m_currentTmp == m_nextCurrenttmp && s_currentTime == m_pShowDownTime && timeS <= 2) {
        qDebug() << "-------------------------------------->进入关机状态";
        setWeekSelectShutdownFrequency();
        if (m_weekSelect[0] != ONLY_ONCE) {
            setNextShutDownTime();
        }
        QStringList argvList;
        argvList.clear();
        argvList.append("--shutdown");
        QProcess p(0);
        p.startDetached("ukui-session-tools", argvList);
        p.waitForStarted();
        return;
    }
    setNextShutDownTime();
}

void Widget::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
        if (!this->isVisible() || this->isMinimized()) {
            this->hide();
            this->show();
            this->raise();
            this->activateWindow();
        } else {
            this->hide();
        }
    case QSystemTrayIcon::Context:
        break;
    default:
        ;
    }
}

void Widget::showScreen()
{
    this->hide();
    this->show();
    this->raise();
    this->activateWindow();
}

void Widget::bootOptionsFilter(const QString&)
{
    this->show();
    this->raise();
    this->activateWindow();
}

//判断仅一次关机时是否可执行关机操作
bool Widget::determineShutdownTime()
{
    QTime current_time = QTime::currentTime();
    int currentHours = current_time.hour();
    int currentMinute = current_time.minute();
    m_pShowDownTime.clear();
    m_hours  = m_pTimeShowWidget->m_pHourRollWidget->readValue();
    m_minute = m_pTimeShowWidget->m_pMinuteRollWidget->readValue();
    m_pShowDownTime = QStringLiteral("%1:%2").arg(m_hours).arg(m_minute);
    qDebug() << m_hours << m_minute << "currentHours:" << currentHours << "currentMinute:" << currentMinute;

    if (m_weekSelect[0] == ONLY_ONCE) {
        if (m_hours < currentHours || (currentHours == m_hours && m_minute <= currentMinute)) {
            return true;
        }
    }
    return false;
}
