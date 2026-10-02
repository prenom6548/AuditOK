/********************************************************************************
** Form generated from reading UI file 'deletebackupdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.15.19
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DELETEBACKUPDIALOG_H
#define UI_DELETEBACKUPDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>

QT_BEGIN_NAMESPACE

class Ui_DeleteBackupDialog
{
public:

    void setupUi(QDialog *DeleteBackupDialog)
    {
        if (DeleteBackupDialog->objectName().isEmpty())
            DeleteBackupDialog->setObjectName(QString::fromUtf8("DeleteBackupDialog"));
        DeleteBackupDialog->resize(400, 138);

        retranslateUi(DeleteBackupDialog);

        QMetaObject::connectSlotsByName(DeleteBackupDialog);
    } // setupUi

    void retranslateUi(QDialog *DeleteBackupDialog)
    {
        DeleteBackupDialog->setWindowTitle(QCoreApplication::translate("DeleteBackupDialog", "Dialog", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DeleteBackupDialog: public Ui_DeleteBackupDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DELETEBACKUPDIALOG_H
