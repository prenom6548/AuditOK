/********************************************************************************
** Form generated from reading UI file 'tinycountdown.ui'
**
** Created by: Qt User Interface Compiler version 5.15.10
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TINYCOUNTDOWN_H
#define UI_TINYCOUNTDOWN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>
#include <kpushbutton.h>
QT_BEGIN_NAMESPACE

class Ui_tinyCountdown
{
public:
    QHBoxLayout *horizontalLayout_2;
    QLabel *clockIcon;
    QHBoxLayout *horizontalLayout;
    QLabel *timeLabel;
    kdk::KPushButton *suspendRunBtn;
    kdk::KPushButton *finishBtn;
    QSpacerItem *horizontalSpacer;
    QPushButton *mainBtn;
    QPushButton *closeBtn;

    void setupUi(QWidget *tinyCountdown)
    {
        if (tinyCountdown->objectName().isEmpty())
            tinyCountdown->setObjectName(QString::fromUtf8("tinyCountdown"));
        tinyCountdown->resize(320, 50);
        tinyCountdown->setMinimumSize(QSize(320, 46));
        tinyCountdown->setMaximumSize(QSize(320, 50));
        horizontalLayout_2 = new QHBoxLayout(tinyCountdown);
        horizontalLayout_2->setSpacing(8);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(20, -1, 16, -1);
        clockIcon = new QLabel(tinyCountdown);
        clockIcon->setObjectName(QString::fromUtf8("clockIcon"));
        clockIcon->setMinimumSize(QSize(24, 24));
        clockIcon->setMaximumSize(QSize(24, 24));
        clockIcon->setMargin(0);
        horizontalLayout = new QHBoxLayout(clockIcon);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));

        horizontalLayout_2->addWidget(clockIcon);

        timeLabel = new QLabel(tinyCountdown);
        timeLabel->setObjectName(QString::fromUtf8("timeLabel"));
        timeLabel->setMinimumSize(QSize(102, 32));
        timeLabel->setMaximumSize(QSize(102, 32));
        QFont font;
        font.setPointSize(18);
        timeLabel->setFont(font);
        timeLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignVCenter);

        horizontalLayout_2->addWidget(timeLabel);

        suspendRunBtn = new kdk::KPushButton(tinyCountdown);
        suspendRunBtn->setObjectName(QString::fromUtf8("suspendRunBtn"));
        suspendRunBtn->setMinimumSize(QSize(32, 32));
        suspendRunBtn->setMaximumSize(QSize(32, 32));

        horizontalLayout_2->addWidget(suspendRunBtn);

        finishBtn = new kdk::KPushButton(tinyCountdown);
        finishBtn->setObjectName(QString::fromUtf8("finishBtn"));
        finishBtn->setEnabled(true);
        finishBtn->setMinimumSize(QSize(32, 32));
        finishBtn->setMaximumSize(QSize(32, 32));

        horizontalLayout_2->addWidget(finishBtn);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer);

        mainBtn = new QPushButton(tinyCountdown);
        mainBtn->setObjectName(QString::fromUtf8("mainBtn"));
        mainBtn->setMinimumSize(QSize(24, 24));
        mainBtn->setMaximumSize(QSize(24, 24));

        horizontalLayout_2->addWidget(mainBtn);

        closeBtn = new QPushButton(tinyCountdown);
        closeBtn->setObjectName(QString::fromUtf8("closeBtn"));
        closeBtn->setMinimumSize(QSize(24, 24));
        closeBtn->setMaximumSize(QSize(24, 24));

        horizontalLayout_2->addWidget(closeBtn);


        retranslateUi(tinyCountdown);

        QMetaObject::connectSlotsByName(tinyCountdown);
    } // setupUi

    void retranslateUi(QWidget *tinyCountdown)
    {
        tinyCountdown->setWindowTitle(QCoreApplication::translate("tinyCountdown", "Count down", nullptr));
        timeLabel->setText(QCoreApplication::translate("tinyCountdown", "01:29:58", nullptr));
        suspendRunBtn->setText(QString());
        finishBtn->setText(QString());
        mainBtn->setText(QString());
        closeBtn->setText(QString());
    } // retranslateUi

};

namespace Ui {
    class tinyCountdown: public Ui_tinyCountdown {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TINYCOUNTDOWN_H
