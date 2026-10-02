/********************************************************************************
** Form generated from reading UI file 'noticeAlarm.ui'
**
** Created by: Qt User Interface Compiler version 5.15.10
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_NOTICEALARM_H
#define UI_NOTICEALARM_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Natice_alarm
{
public:
    QVBoxLayout *verticalLayout;
    QWidget *widget;
    QVBoxLayout *verticalLayout_2;
    QWidget *widget_4;
    QHBoxLayout *horizontalLayout;
    QLabel *titleIcon;
    QLabel *titleLabel;
    QSpacerItem *horizontalSpacer_5;
    QPushButton *closeTitleBtn;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *timeEndInfo;
    QLabel *autoCloseTime;
    QWidget *widget_3;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer_4;
    QPushButton *remindLateBtn;

    void setupUi(QWidget *Natice_alarm)
    {
        if (Natice_alarm->objectName().isEmpty())
            Natice_alarm->setObjectName(QString::fromUtf8("Natice_alarm"));
        Natice_alarm->resize(288, 120);
        verticalLayout = new QVBoxLayout(Natice_alarm);
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        widget = new QWidget(Natice_alarm);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setMinimumSize(QSize(0, 0));
        widget->setMaximumSize(QSize(16800, 16800));
        widget->setStyleSheet(QString::fromUtf8(""));
        verticalLayout_2 = new QVBoxLayout(widget);
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        verticalLayout_2->setContentsMargins(-1, 0, 8, -1);
        widget_4 = new QWidget(widget);
        widget_4->setObjectName(QString::fromUtf8("widget_4"));
        widget_4->setMinimumSize(QSize(0, 0));
        widget_4->setMaximumSize(QSize(16777215, 16777215));
        horizontalLayout = new QHBoxLayout(widget_4);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalLayout->setContentsMargins(5, 5, 5, -1);
        titleIcon = new QLabel(widget_4);
        titleIcon->setObjectName(QString::fromUtf8("titleIcon"));
        titleIcon->setMinimumSize(QSize(24, 24));
        titleIcon->setMaximumSize(QSize(24, 24));

        horizontalLayout->addWidget(titleIcon);

        titleLabel = new QLabel(widget_4);
        titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
        titleLabel->setMinimumSize(QSize(100, 22));
        titleLabel->setMaximumSize(QSize(100, 22));
        titleLabel->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout->addWidget(titleLabel);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_5);

        closeTitleBtn = new QPushButton(widget_4);
        closeTitleBtn->setObjectName(QString::fromUtf8("closeTitleBtn"));
        closeTitleBtn->setMinimumSize(QSize(24, 24));
        closeTitleBtn->setMaximumSize(QSize(24, 24));
        closeTitleBtn->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout->addWidget(closeTitleBtn);


        verticalLayout_2->addWidget(widget_4);

        widget_2 = new QWidget(widget);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(widget_2->sizePolicy().hasHeightForWidth());
        widget_2->setSizePolicy(sizePolicy);
        widget_2->setMinimumSize(QSize(0, 35));
        widget_2->setMaximumSize(QSize(16777215, 16777215));
        horizontalLayout_2 = new QHBoxLayout(widget_2);
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(11, 0, 11, 0);
        timeEndInfo = new QLabel(widget_2);
        timeEndInfo->setObjectName(QString::fromUtf8("timeEndInfo"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(timeEndInfo->sizePolicy().hasHeightForWidth());
        timeEndInfo->setSizePolicy(sizePolicy1);
        timeEndInfo->setMinimumSize(QSize(90, 30));
        timeEndInfo->setMaximumSize(QSize(110, 30));
        QFont font;
        timeEndInfo->setFont(font);
        timeEndInfo->setStyleSheet(QString::fromUtf8("font-size:11px"));
        timeEndInfo->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        horizontalLayout_2->addWidget(timeEndInfo);

        autoCloseTime = new QLabel(widget_2);
        autoCloseTime->setObjectName(QString::fromUtf8("autoCloseTime"));
        sizePolicy1.setHeightForWidth(autoCloseTime->sizePolicy().hasHeightForWidth());
        autoCloseTime->setSizePolicy(sizePolicy1);
        autoCloseTime->setMinimumSize(QSize(90, 30));
        autoCloseTime->setMaximumSize(QSize(16777215, 30));
        QFont font1;
        font1.setPointSize(9);
        autoCloseTime->setFont(font1);
        autoCloseTime->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_2->addWidget(autoCloseTime);


        verticalLayout_2->addWidget(widget_2);

        widget_3 = new QWidget(widget);
        widget_3->setObjectName(QString::fromUtf8("widget_3"));
        sizePolicy.setHeightForWidth(widget_3->sizePolicy().hasHeightForWidth());
        widget_3->setSizePolicy(sizePolicy);
        widget_3->setMinimumSize(QSize(0, 35));
        horizontalLayout_3 = new QHBoxLayout(widget_3);
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 11, 0);
        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_4);

        remindLateBtn = new QPushButton(widget_3);
        remindLateBtn->setObjectName(QString::fromUtf8("remindLateBtn"));
        sizePolicy.setHeightForWidth(remindLateBtn->sizePolicy().hasHeightForWidth());
        remindLateBtn->setSizePolicy(sizePolicy);
        remindLateBtn->setMinimumSize(QSize(85, 34));
        remindLateBtn->setMaximumSize(QSize(120, 34));
        remindLateBtn->setLayoutDirection(Qt::LeftToRight);

        horizontalLayout_3->addWidget(remindLateBtn);


        verticalLayout_2->addWidget(widget_3);


        verticalLayout->addWidget(widget);


        retranslateUi(Natice_alarm);

        QMetaObject::connectSlotsByName(Natice_alarm);
    } // setupUi

    void retranslateUi(QWidget *Natice_alarm)
    {
        Natice_alarm->setWindowTitle(QCoreApplication::translate("Natice_alarm", "Form", nullptr));
        titleIcon->setText(QString());
        titleLabel->setText(QCoreApplication::translate("Natice_alarm", "Alarm clock", nullptr));
#if QT_CONFIG(tooltip)
        closeTitleBtn->setToolTip(QCoreApplication::translate("Natice_alarm", "Close", nullptr));
#endif // QT_CONFIG(tooltip)
        closeTitleBtn->setText(QString());
        timeEndInfo->setText(QCoreApplication::translate("Natice_alarm", "11:20 \350\256\276\350\256\241\344\276\213\344\274\232...", nullptr));
        remindLateBtn->setText(QCoreApplication::translate("Natice_alarm", "Remind later", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Natice_alarm: public Ui_Natice_alarm {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_NOTICEALARM_H
