/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
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
#ifndef BINDDIALOG_H
#define BINDDIALOG_H

#include <QDialog>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>

enum DIALOGTYPE{
    ENSUREDIALOG,
    BINDOTPDIALOG
};

class BindDialog : public QWidget
{
    Q_OBJECT

public:
    explicit BindDialog(int mode,QWidget *parent = 0);
    ~BindDialog();

    void setOtpPromptText(QString text);
    void setOtpQrcodeText(QString text);
    void setOtpPrpmptPromptText(QString text);
    void setOtpMessageText(QString text);
    void setOtpEmergMsg(QString text);
protected:
    void keyPressEvent(QKeyEvent *);
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

public Q_SLOTS:
    void on_btnClose_clicked();
    void on_btnOk_clicked();
    void on_btnCancel_clicked();
    void on_otpPromptEdit_textChanged(QString text);
    //void on_lineEdit_returnPressed();

Q_SIGNALS:
    void accepted(QString text);
    void rejected();
    void finished();

private:
    void updateSize();
    void initUI();
    void showFinishedPrompt(QString text = "");

    QLabel *otpPromptLabel = nullptr;
    QLabel *otpQrcodeLabel = nullptr;
    QLineEdit *otpPromptEdit = nullptr;
    QLabel *otpMessageLabel = nullptr;

    QPushButton *btnOk = nullptr;
    QPushButton *btnCancel = nullptr;
    QPushButton *btnClose = nullptr;

    QWidget *finishWidget = nullptr;
    QLabel *finishLabel = nullptr;
    QLabel *finishMsg = nullptr;
    QLabel *finishTip = nullptr;

    int mode = -1;
};

#endif // BINDDIALOG_H
