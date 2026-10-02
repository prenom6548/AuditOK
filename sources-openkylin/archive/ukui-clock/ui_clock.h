/********************************************************************************
** Form generated from reading UI file 'clock.ui'
**
** Created by: Qt User Interface Compiler version 5.15.10
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CLOCK_H
#define UI_CLOCK_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <kpushbutton.h>
#include <kwidget.h>
#include "countdownAnimation.h"

QT_BEGIN_NAMESPACE

class Ui_Clock
{
public:
    QAction *actionOn;
    QVBoxLayout *verticalLayout;
    KWidget *titileWidget;
    QStackedWidget *mainWidget;
    QWidget *countdownPage;
    QStackedWidget *countdownStackedW;
    QWidget *countdownSetPage;
    Countdown_Animation *countdownRunPage;
    QWidget *tinyWidget;
    QHBoxLayout *horizontalLayout_6;
    QPushButton *tinyWindowBtn;
    QWidget *timeWidget;
    QVBoxLayout *verticalLayout_3;
    QLabel *remainTime;
    QWidget *alarmWidget;
    QHBoxLayout *horizontalLayout_7;
    QSpacerItem *horizontalSpacer_2;
    QLabel *countdownAlarmIcon;
    QLabel *countdownAlarmTime;
    QSpacerItem *horizontalSpacer;
    QWidget *btnWidget;
    QHBoxLayout *horizontalLayout_8;
    QPushButton *startCountdownBtn;
    QPushButton *suspendCountdownBtn;
    QWidget *alarmPage;
    QPushButton *addAlarmBtn;
    QListWidget *alarmListWidget;
    QLabel *noAlarmIcon;
    QLabel *noAlarm;
    QWidget *stopwatchPage;
    QPushButton *startStopwatch;
    QLabel *timeShowSmall;
    QLabel *timeShowBig;
    QPushButton *ringBtn;
    QListWidget *timeListWidget;
    QWidget *addAlarmPage;
    QWidget *editAlarmPage;
    QPushButton *cancelbtnOnEditAlarm;
    QPushButton *saveBtnOnEditAlarm;
    QLabel *timeFormatOnTimeWheel;
    QWidget *switchWidget;
    QHBoxLayout *horizontalLayout_9;
    QSpacerItem *horizontalSpacer_6;
    KPushButton *switchClock;
    QSpacerItem *horizontalSpacer_3;
    KPushButton *switchCountdown;
    QSpacerItem *horizontalSpacer_4;
    KPushButton *switchStopwatch;
    QSpacerItem *horizontalSpacer_5;

    void setupUi(QWidget *Clock)
    {
        if (Clock->objectName().isEmpty())
            Clock->setObjectName(QString::fromUtf8("Clock"));
        Clock->resize(400, 580);
        Clock->setStyleSheet(QString::fromUtf8(""));
        actionOn = new QAction(Clock);
        actionOn->setObjectName(QString::fromUtf8("actionOn"));
        actionOn->setCheckable(true);
        QIcon icon;
        icon.addFile(QString::fromUtf8(":/Images/Images/NO.png"), QSize(), QIcon::Normal, QIcon::Off);
        actionOn->setIcon(icon);
        verticalLayout = new QVBoxLayout(Clock);
        verticalLayout->setSpacing(0);
        verticalLayout->setContentsMargins(11, 11, 11, 11);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(1, 0, 1, -1);
        titileWidget = new KWidget(Clock);
        titileWidget->setObjectName(QString::fromUtf8("titileWidget"));
        titileWidget->setMinimumSize(QSize(0, 0));
        mainWidget = new QStackedWidget(titileWidget);
        mainWidget->setObjectName(QString::fromUtf8("mainWidget"));
        mainWidget->setGeometry(QRect(0, 106, 398, 465));
        mainWidget->setStyleSheet(QString::fromUtf8(""));
        countdownPage = new QWidget();
        countdownPage->setObjectName(QString::fromUtf8("countdownPage"));
        countdownStackedW = new QStackedWidget(countdownPage);
        countdownStackedW->setObjectName(QString::fromUtf8("countdownStackedW"));
        countdownStackedW->setGeometry(QRect(0, 0, 390, 477));
        countdownStackedW->setStyleSheet(QString::fromUtf8(""));
        countdownSetPage = new QWidget();
        countdownSetPage->setObjectName(QString::fromUtf8("countdownSetPage"));
        countdownStackedW->addWidget(countdownSetPage);
        countdownRunPage = new Countdown_Animation();
        countdownRunPage->setObjectName(QString::fromUtf8("countdownRunPage"));
        tinyWidget = new QWidget(countdownRunPage);
        tinyWidget->setObjectName(QString::fromUtf8("tinyWidget"));
        tinyWidget->setGeometry(QRect(9, 174, 371, 40));
        horizontalLayout_6 = new QHBoxLayout(tinyWidget);
        horizontalLayout_6->setSpacing(6);
        horizontalLayout_6->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        tinyWindowBtn = new QPushButton(tinyWidget);
        tinyWindowBtn->setObjectName(QString::fromUtf8("tinyWindowBtn"));
        tinyWindowBtn->setMinimumSize(QSize(24, 24));
        tinyWindowBtn->setMaximumSize(QSize(24, 24));
        tinyWindowBtn->setStyleSheet(QString::fromUtf8(""));
        tinyWindowBtn->setIconSize(QSize(24, 24));

        horizontalLayout_6->addWidget(tinyWindowBtn);

        timeWidget = new QWidget(countdownRunPage);
        timeWidget->setObjectName(QString::fromUtf8("timeWidget"));
        timeWidget->setGeometry(QRect(14, 100, 361, 77));
        verticalLayout_3 = new QVBoxLayout(timeWidget);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        verticalLayout_3->setContentsMargins(-1, 9, -1, -1);
        remainTime = new QLabel(timeWidget);
        remainTime->setObjectName(QString::fromUtf8("remainTime"));
        remainTime->setStyleSheet(QString::fromUtf8("font-size:40px;"));

        verticalLayout_3->addWidget(remainTime, 0, Qt::AlignHCenter|Qt::AlignVCenter);

        alarmWidget = new QWidget(countdownRunPage);
        alarmWidget->setObjectName(QString::fromUtf8("alarmWidget"));
        alarmWidget->setGeometry(QRect(16, 60, 361, 70));
        horizontalLayout_7 = new QHBoxLayout(alarmWidget);
        horizontalLayout_7->setSpacing(6);
        horizontalLayout_7->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_2);

        countdownAlarmIcon = new QLabel(alarmWidget);
        countdownAlarmIcon->setObjectName(QString::fromUtf8("countdownAlarmIcon"));
        countdownAlarmIcon->setMinimumSize(QSize(13, 16));
        countdownAlarmIcon->setMaximumSize(QSize(26, 26));

        horizontalLayout_7->addWidget(countdownAlarmIcon);

        countdownAlarmTime = new QLabel(alarmWidget);
        countdownAlarmTime->setObjectName(QString::fromUtf8("countdownAlarmTime"));
        QFont font;
        countdownAlarmTime->setFont(font);
        countdownAlarmTime->setStyleSheet(QString::fromUtf8("color: rgb(176, 176, 176);\n"
"font-size:18px;\n"
"\n"
"\n"
""));

        horizontalLayout_7->addWidget(countdownAlarmTime);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer);

        btnWidget = new QWidget(countdownRunPage);
        btnWidget->setObjectName(QString::fromUtf8("btnWidget"));
        btnWidget->setGeometry(QRect(9, 386, 361, 48));
        horizontalLayout_8 = new QHBoxLayout(btnWidget);
        horizontalLayout_8->setSpacing(6);
        horizontalLayout_8->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        startCountdownBtn = new QPushButton(btnWidget);
        startCountdownBtn->setObjectName(QString::fromUtf8("startCountdownBtn"));
        startCountdownBtn->setMinimumSize(QSize(120, 34));
        startCountdownBtn->setMaximumSize(QSize(120, 16777215));
        startCountdownBtn->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_8->addWidget(startCountdownBtn);

        suspendCountdownBtn = new QPushButton(btnWidget);
        suspendCountdownBtn->setObjectName(QString::fromUtf8("suspendCountdownBtn"));
        suspendCountdownBtn->setMinimumSize(QSize(120, 34));
        suspendCountdownBtn->setMaximumSize(QSize(120, 16777215));
        suspendCountdownBtn->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_8->addWidget(suspendCountdownBtn);

        countdownStackedW->addWidget(countdownRunPage);
        mainWidget->addWidget(countdownPage);
        alarmPage = new QWidget();
        alarmPage->setObjectName(QString::fromUtf8("alarmPage"));
        addAlarmBtn = new QPushButton(alarmPage);
        addAlarmBtn->setObjectName(QString::fromUtf8("addAlarmBtn"));
        addAlarmBtn->setGeometry(QRect(135, 401, 120, 34));
        addAlarmBtn->setMinimumSize(QSize(120, 0));
        addAlarmBtn->setStyleSheet(QString::fromUtf8(""));
        alarmListWidget = new QListWidget(alarmPage);
        alarmListWidget->setObjectName(QString::fromUtf8("alarmListWidget"));
        alarmListWidget->setGeometry(QRect(25, 0, 356, 354));
        alarmListWidget->setStyleSheet(QString::fromUtf8(""));
        noAlarmIcon = new QLabel(alarmPage);
        noAlarmIcon->setObjectName(QString::fromUtf8("noAlarmIcon"));
        noAlarmIcon->setGeometry(QRect(116, 94, 149, 130));
        noAlarmIcon->setAlignment(Qt::AlignCenter);
        noAlarm = new QLabel(alarmPage);
        noAlarm->setObjectName(QString::fromUtf8("noAlarm"));
        noAlarm->setGeometry(QRect(122, 232, 140, 31));
        noAlarm->setStyleSheet(QString::fromUtf8("color: rgba(49, 66, 89, 0.6);\n"
"font-size:16px;"));
        noAlarm->setAlignment(Qt::AlignCenter);
        mainWidget->addWidget(alarmPage);
        stopwatchPage = new QWidget();
        stopwatchPage->setObjectName(QString::fromUtf8("stopwatchPage"));
        startStopwatch = new QPushButton(stopwatchPage);
        startStopwatch->setObjectName(QString::fromUtf8("startStopwatch"));
        startStopwatch->setGeometry(QRect(210, 401, 120, 34));
        startStopwatch->setMinimumSize(QSize(120, 0));
        startStopwatch->setStyleSheet(QString::fromUtf8(""));
        timeShowSmall = new QLabel(stopwatchPage);
        timeShowSmall->setObjectName(QString::fromUtf8("timeShowSmall"));
        timeShowSmall->setGeometry(QRect(90, 55, 210, 41));
        timeShowSmall->setStyleSheet(QString::fromUtf8("color: rgb(176, 176, 176);\n"
"font: 19pt \n"
""));
        timeShowBig = new QLabel(stopwatchPage);
        timeShowBig->setObjectName(QString::fromUtf8("timeShowBig"));
        timeShowBig->setGeometry(QRect(90, 0, 210, 70));
        timeShowBig->setStyleSheet(QString::fromUtf8("font: 30pt "));
        ringBtn = new QPushButton(stopwatchPage);
        ringBtn->setObjectName(QString::fromUtf8("ringBtn"));
        ringBtn->setGeometry(QRect(65, 401, 120, 34));
        ringBtn->setMinimumSize(QSize(120, 0));
        ringBtn->setStyleSheet(QString::fromUtf8(""));
        timeListWidget = new QListWidget(stopwatchPage);
        timeListWidget->setObjectName(QString::fromUtf8("timeListWidget"));
        timeListWidget->setGeometry(QRect(25, 109, 356, 277));
        timeListWidget->setStyleSheet(QString::fromUtf8(""));
        mainWidget->addWidget(stopwatchPage);
        addAlarmPage = new QWidget();
        addAlarmPage->setObjectName(QString::fromUtf8("addAlarmPage"));
        editAlarmPage = new QWidget(addAlarmPage);
        editAlarmPage->setObjectName(QString::fromUtf8("editAlarmPage"));
        editAlarmPage->setGeometry(QRect(0, 0, 390, 477));
        editAlarmPage->setMaximumSize(QSize(16777215, 16777215));
        editAlarmPage->setStyleSheet(QString::fromUtf8(""));
        cancelbtnOnEditAlarm = new QPushButton(editAlarmPage);
        cancelbtnOnEditAlarm->setObjectName(QString::fromUtf8("cancelbtnOnEditAlarm"));
        cancelbtnOnEditAlarm->setGeometry(QRect(65, 425, 120, 34));
        cancelbtnOnEditAlarm->setMinimumSize(QSize(120, 0));
        cancelbtnOnEditAlarm->setStyleSheet(QString::fromUtf8(""));
        saveBtnOnEditAlarm = new QPushButton(editAlarmPage);
        saveBtnOnEditAlarm->setObjectName(QString::fromUtf8("saveBtnOnEditAlarm"));
        saveBtnOnEditAlarm->setGeometry(QRect(210, 425, 120, 34));
        saveBtnOnEditAlarm->setMinimumSize(QSize(120, 0));
        saveBtnOnEditAlarm->setStyleSheet(QString::fromUtf8(""));
        timeFormatOnTimeWheel = new QLabel(editAlarmPage);
        timeFormatOnTimeWheel->setObjectName(QString::fromUtf8("timeFormatOnTimeWheel"));
        timeFormatOnTimeWheel->setGeometry(QRect(60, 80, 41, 22));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(timeFormatOnTimeWheel->sizePolicy().hasHeightForWidth());
        timeFormatOnTimeWheel->setSizePolicy(sizePolicy);
        mainWidget->addWidget(addAlarmPage);
        switchWidget = new QWidget(titileWidget);
        switchWidget->setObjectName(QString::fromUtf8("switchWidget"));
        switchWidget->setGeometry(QRect(0, 48, 398, 58));
        horizontalLayout_9 = new QHBoxLayout(switchWidget);
        horizontalLayout_9->setSpacing(0);
        horizontalLayout_9->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_9->setObjectName(QString::fromUtf8("horizontalLayout_9"));
        horizontalLayout_9->setContentsMargins(0, -1, 9, -1);
        horizontalSpacer_6 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_9->addItem(horizontalSpacer_6);

        switchClock = new KPushButton(switchWidget);
        switchClock->setObjectName(QString::fromUtf8("switchClock"));
        switchClock->setMinimumSize(QSize(40, 40));
        switchClock->setMaximumSize(QSize(40, 40));

        horizontalLayout_9->addWidget(switchClock);

        horizontalSpacer_3 = new QSpacerItem(26, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_9->addItem(horizontalSpacer_3);

        switchCountdown = new KPushButton(switchWidget);
        switchCountdown->setObjectName(QString::fromUtf8("switchCountdown"));
        switchCountdown->setMinimumSize(QSize(40, 40));
        switchCountdown->setMaximumSize(QSize(40, 40));

        horizontalLayout_9->addWidget(switchCountdown);

        horizontalSpacer_4 = new QSpacerItem(26, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_9->addItem(horizontalSpacer_4);

        switchStopwatch = new KPushButton(switchWidget);
        switchStopwatch->setObjectName(QString::fromUtf8("switchStopwatch"));
        switchStopwatch->setMinimumSize(QSize(40, 40));
        switchStopwatch->setMaximumSize(QSize(40, 40));

        horizontalLayout_9->addWidget(switchStopwatch);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_9->addItem(horizontalSpacer_5);


        verticalLayout->addWidget(titileWidget);


        retranslateUi(Clock);

        mainWidget->setCurrentIndex(1);
        countdownStackedW->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(Clock);
    } // setupUi

    void retranslateUi(QWidget *Clock)
    {
        Clock->setWindowTitle(QCoreApplication::translate("Clock", "Alarm", nullptr));
        actionOn->setText(QCoreApplication::translate("Clock", "On", nullptr));
        tinyWindowBtn->setText(QString());
        remainTime->setText(QCoreApplication::translate("Clock", "00:00:00", nullptr));
        countdownAlarmIcon->setText(QCoreApplication::translate("Clock", "icon", nullptr));
        countdownAlarmTime->setText(QCoreApplication::translate("Clock", "PM", nullptr));
        startCountdownBtn->setText(QCoreApplication::translate("Clock", "start", nullptr));
        suspendCountdownBtn->setText(QCoreApplication::translate("Clock", "suspend", nullptr));
        addAlarmBtn->setText(QCoreApplication::translate("Clock", "add", nullptr));
        noAlarmIcon->setText(QString());
        noAlarm->setText(QCoreApplication::translate("Clock", "No alarm", nullptr));
#if QT_CONFIG(tooltip)
        startStopwatch->setToolTip(QCoreApplication::translate("Clock", "start", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        startStopwatch->setStatusTip(QString());
#endif // QT_CONFIG(statustip)
        startStopwatch->setText(QCoreApplication::translate("Clock", "start", nullptr));
        timeShowSmall->setText(QCoreApplication::translate("Clock", "00:00:00", nullptr));
        timeShowBig->setText(QCoreApplication::translate("Clock", "00:00:00", nullptr));
#if QT_CONFIG(tooltip)
        ringBtn->setToolTip(QCoreApplication::translate("Clock", "count", nullptr));
#endif // QT_CONFIG(tooltip)
#if QT_CONFIG(statustip)
        ringBtn->setStatusTip(QString());
#endif // QT_CONFIG(statustip)
        ringBtn->setText(QCoreApplication::translate("Clock", "count", nullptr));
        cancelbtnOnEditAlarm->setText(QCoreApplication::translate("Clock", "cancel", nullptr));
        saveBtnOnEditAlarm->setText(QCoreApplication::translate("Clock", "save", nullptr));
        timeFormatOnTimeWheel->setText(QString());
        switchClock->setText(QString());
        switchCountdown->setText(QString());
        switchStopwatch->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class Clock: public Ui_Clock {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CLOCK_H
