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
#ifndef KMESSAGEDIALOG_H
#define KMESSAGEDIALOG_H

#include <QWidget>
#include <QDialog>
#include "binddialog.h"

class KMessageDialog : public QWidget
{
    Q_OBJECT
public:
    explicit KMessageDialog(int mode,QWidget *parent = nullptr);
    virtual ~KMessageDialog();

    void setOtpPromptText(QString text);
    void setOtpQrcodeText(QString text);
    void setOtpPrpmptPromptText(QString text);
    void setOtpMessageText(QString text);
    void setOtpEmergMsg(QString text);

protected:
    void resizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *);

Q_SIGNALS:
    void accepted(QString text);
    void rejected();
    void finished();

private:
    void init();
    void initConnections();

    void checkParent(QWidget *parent);

private:
    BindDialog *m_dialog = nullptr;
    int mode = -1;
};

#endif // KMESSAGEDIALOG_H
