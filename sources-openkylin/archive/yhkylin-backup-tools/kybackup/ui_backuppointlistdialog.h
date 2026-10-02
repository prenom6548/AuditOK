/********************************************************************************
** Form generated from reading UI file 'backuppointlistdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_BACKUPPOINTLISTDIALOG_H
#define UI_BACKUPPOINTLISTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE

class Ui_BackupPointListDialog
{
public:

    void setupUi(QDialog *BackupPointListDialog)
    {
        if (BackupPointListDialog->objectName().isEmpty())
            BackupPointListDialog->setObjectName(QString::fromUtf8("BackupPointListDialog"));
        BackupPointListDialog->resize(686, 371);

        retranslateUi(BackupPointListDialog);

        QMetaObject::connectSlotsByName(BackupPointListDialog);
    } // setupUi

    void retranslateUi(QDialog *BackupPointListDialog)
    {
        BackupPointListDialog->setWindowTitle(QCoreApplication::translate("BackupPointListDialog", "Dialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class BackupPointListDialog: public Ui_BackupPointListDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_BACKUPPOINTLISTDIALOG_H
