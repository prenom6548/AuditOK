/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "kmessagedialog.h"

KMessageDialog::KMessageDialog(int mod, QWidget *parent) : mode(mod), QWidget(parent)
{
    init();
    initConnections();
}

KMessageDialog::~KMessageDialog() {}

void KMessageDialog::init()
{
    checkParent(parentWidget());
    if (!m_dialog) {
        m_dialog = new BindDialog(mode, this);
        connect(m_dialog, SIGNAL(accepted(QString)), this, SIGNAL(accepted(QString)));
        connect(m_dialog, SIGNAL(rejected()), this, SIGNAL(rejected()));
        connect(m_dialog, &BindDialog::finished, this, [=]() {
            this->hide();
            Q_EMIT finished();
        });
    }
}

void KMessageDialog::initConnections() {}

void KMessageDialog::resizeEvent(QResizeEvent *event)
{
    if (parentWidget()) {
        setFixedSize(parentWidget()->size());
        move(0, 0);
    }
    QWidget::resizeEvent(event);
}

void KMessageDialog::showEvent(QShowEvent *event)
{
    this->raise();
    QWidget::showEvent(event);
}

void KMessageDialog::checkParent(QWidget *parent)
{
    if (!parent) {
        return;
    }
    QWidget *tempParent = parent;
    bool isFoundSuitableParent = false;
    do {
        QString widgetClassName = tempParent->metaObject()->className();
        if (widgetClassName == "GreeterWindow" || widgetClassName == "LockWidget") {
            isFoundSuitableParent = true;
            break;
        }
        tempParent = tempParent->parentWidget();
    } while (tempParent);

    if (tempParent && isFoundSuitableParent) {
        setParent(tempParent);
    }
}

void KMessageDialog::setOtpPromptText(QString text)
{
    if (m_dialog)
        m_dialog->setOtpPromptText(text);
}

void KMessageDialog::setOtpQrcodeText(QString text)
{
    if (m_dialog)
        m_dialog->setOtpQrcodeText(text);
}
void KMessageDialog::setOtpPrpmptPromptText(QString text)
{
    if (m_dialog)
        m_dialog->setOtpPrpmptPromptText(text);
}
void KMessageDialog::setOtpMessageText(QString text)
{
    if (m_dialog)
        m_dialog->setOtpMessageText(text);
}

void KMessageDialog::setOtpEmergMsg(QString text)
{
    if (m_dialog)
        m_dialog->setOtpEmergMsg(text);
}
