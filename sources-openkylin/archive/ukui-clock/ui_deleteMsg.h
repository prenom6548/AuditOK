/********************************************************************************
** Form generated from reading UI file 'deleteMsg.ui'
**
** Created by: Qt User Interface Compiler version 5.15.10
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DELETEMSG_H
#define UI_DELETEMSG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_delete_msg
{
public:
    QWidget *widget;
    QWidget *widget_2;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QLabel *questionIcon;
    QLabel *msgInfo;
    QSpacerItem *horizontalSpacer_2;
    QWidget *verticalLayoutWidget;
    QHBoxLayout *horizontalLayout_2;
    QPushButton *closebtn;
    QWidget *widget_3;
    QHBoxLayout *horizontalLayout_3;
    QSpacerItem *horizontalSpacer_3;
    QPushButton *cancelbtn;
    QSpacerItem *horizontalSpacer_5;
    QPushButton *surebtn;
    QSpacerItem *horizontalSpacer_4;

    void setupUi(QDialog *delete_msg)
    {
        if (delete_msg->objectName().isEmpty())
            delete_msg->setObjectName(QString::fromUtf8("delete_msg"));
        delete_msg->resize(350, 174);
        delete_msg->setStyleSheet(QString::fromUtf8(""));
        widget = new QWidget(delete_msg);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(0, 0, 350, 174));
        widget->setStyleSheet(QString::fromUtf8(""));
        widget_2 = new QWidget(widget);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        widget_2->setGeometry(QRect(0, 50, 350, 46));
        horizontalLayout = new QHBoxLayout(widget_2);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        horizontalSpacer = new QSpacerItem(19, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);

        questionIcon = new QLabel(widget_2);
        questionIcon->setObjectName(QString::fromUtf8("questionIcon"));
        questionIcon->setMinimumSize(QSize(24, 24));
        questionIcon->setMaximumSize(QSize(24, 24));
        questionIcon->setStyleSheet(QString::fromUtf8("border-image: url(:/image/warning-24x24.png);"));

        horizontalLayout->addWidget(questionIcon);

        msgInfo = new QLabel(widget_2);
        msgInfo->setObjectName(QString::fromUtf8("msgInfo"));
        msgInfo->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout->addWidget(msgInfo);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer_2);

        verticalLayoutWidget = new QWidget(widget);
        verticalLayoutWidget->setObjectName(QString::fromUtf8("verticalLayoutWidget"));
        verticalLayoutWidget->setGeometry(QRect(310, 0, 40, 40));
        horizontalLayout_2 = new QHBoxLayout(verticalLayoutWidget);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalLayout_2->setContentsMargins(5, 5, 5, 5);
        closebtn = new QPushButton(verticalLayoutWidget);
        closebtn->setObjectName(QString::fromUtf8("closebtn"));
        closebtn->setMinimumSize(QSize(30, 30));
        closebtn->setMaximumSize(QSize(30, 30));
        closebtn->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_2->addWidget(closebtn);

        widget_3 = new QWidget(widget);
        widget_3->setObjectName(QString::fromUtf8("widget_3"));
        widget_3->setGeometry(QRect(0, 100, 350, 61));
        horizontalLayout_3 = new QHBoxLayout(widget_3);
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        horizontalSpacer_3 = new QSpacerItem(156, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);

        cancelbtn = new QPushButton(widget_3);
        cancelbtn->setObjectName(QString::fromUtf8("cancelbtn"));
        cancelbtn->setMinimumSize(QSize(80, 30));
        cancelbtn->setMaximumSize(QSize(80, 30));
        cancelbtn->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_3->addWidget(cancelbtn);

        horizontalSpacer_5 = new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_5);

        surebtn = new QPushButton(widget_3);
        surebtn->setObjectName(QString::fromUtf8("surebtn"));
        surebtn->setMinimumSize(QSize(80, 30));
        surebtn->setMaximumSize(QSize(80, 30));
        surebtn->setStyleSheet(QString::fromUtf8(""));

        horizontalLayout_3->addWidget(surebtn);

        horizontalSpacer_4 = new QSpacerItem(24, 20, QSizePolicy::Fixed, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_4);


        retranslateUi(delete_msg);

        QMetaObject::connectSlotsByName(delete_msg);
    } // setupUi

    void retranslateUi(QDialog *delete_msg)
    {
        delete_msg->setWindowTitle(QCoreApplication::translate("delete_msg", "Dialog", nullptr));
        questionIcon->setText(QString());
        msgInfo->setText(QCoreApplication::translate("delete_msg", "are you sure ?", nullptr));
        closebtn->setText(QString());
        cancelbtn->setText(QCoreApplication::translate("delete_msg", "cancel", nullptr));
        surebtn->setText(QCoreApplication::translate("delete_msg", "sure", nullptr));
    } // retranslateUi

};

namespace Ui {
    class delete_msg: public Ui_delete_msg {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DELETEMSG_H
