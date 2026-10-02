/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "messageboxutils.h"
#include <QIcon>
#include <QScopedPointer>
#include "gsettingswrapper.h"
#include "../common/mydefine.h"
#include "globalsignals.h"
#include "globalbackupinfo.h"
#include <QPushButton>


MyMessageBox::MyMessageBox(QWidget *parent) :
    QMessageBox(parent)
{
    setThemeIcon(THEME_YHKYLIN_BACKUP_TOOLS);
}

MyMessageBox::~MyMessageBox()
{}

void MyMessageBox::setThemeIcon(const QString& themeIcon)
{
    m_themeIcon = themeIcon;
    on_themeIconChanged();
    disconnect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::themeIconChanged, this, &MyMessageBox::on_themeIconChanged);
    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::themeIconChanged, this, &MyMessageBox::on_themeIconChanged);
}

void MyMessageBox::setMsgIcon(QMessageBox::Icon icon) {
    m_icon = icon;
    on_iconChanged();
    disconnect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::themeIconChanged, this, &MyMessageBox::on_iconChanged);
    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::themeIconChanged, this, &MyMessageBox::on_iconChanged);
}

void MyMessageBox::on_themeIconChanged()
{
    QIcon titleIcon = QIcon::fromTheme(m_themeIcon, QIcon(":/images/yhkylin-backup-tools.png"));
    setWindowIcon(titleIcon);
}

void MyMessageBox::on_iconChanged()
{
    setIcon(m_icon);
}

void MessageBoxUtils::QMESSAGE_BOX_INFORMATION(QWidget* q_parent, const QString& typelabel, const QString& message, const QString& label)
{
    QScopedPointer<MyMessageBox> box(new MyMessageBox(q_parent));
    box->setMsgIcon(QMessageBox::Information);
    box->setWindowTitle(typelabel);
    box->setText(message);
    QPushButton * okbtn = box->addButton(label, QMessageBox::AcceptRole);
    box->setDefaultButton(okbtn);
    box->exec();
}

void MessageBoxUtils::QMESSAGE_BOX_WARNING(QWidget* q_parent, const QString& typelabel, const QString& message, const QString& label)
{
    QScopedPointer<MyMessageBox> box(new MyMessageBox(q_parent));
    box->setMsgIcon(QMessageBox::Warning);
    box->setWindowTitle(typelabel);
    box->setText(message);
    QPushButton * okbtn = box->addButton(label, QMessageBox::AcceptRole);
    box->setDefaultButton(okbtn);
    box->exec();
}

bool MessageBoxUtils::QMESSAGE_BOX_WARNING_CANCEL(QWidget *q_parent, const QString &typelabel, const QString &message, const QString &label_yes, const QString &label_no)
{
    QScopedPointer<MyMessageBox> box(new MyMessageBox(q_parent));
    box->setMsgIcon(QMessageBox::Question);
    box->setWindowTitle(typelabel);
    box->setText(message);
    QPushButton * okbtn = box->addButton(label_yes, QMessageBox::AcceptRole);
    box->addButton(label_no, QMessageBox::RejectRole);
    box->setDefaultButton(okbtn);
    box->exec();
    if (box->clickedButton() != okbtn) {
        return false;
    }

    return true;
}

void MessageBoxUtils::QMESSAGE_BOX_CRITICAL(QWidget* q_parent, const QString& typelabel, const QString& message, const QString& label)
{
    QScopedPointer<MyMessageBox> box(new MyMessageBox(q_parent));
    box->setMsgIcon(QMessageBox::Critical);
    box->setWindowTitle(typelabel);
    box->setText(message);
    QPushButton * okbtn = box->addButton(label, QMessageBox::AcceptRole);
    box->setDefaultButton(okbtn);
    box->exec();
}
