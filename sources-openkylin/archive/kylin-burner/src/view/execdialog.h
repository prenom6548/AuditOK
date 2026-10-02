/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef EXECDIALOG_H
#define EXECDIALOG_H

#include "inputchange.h"

#include <QDialog>
#include <QMutex>

namespace Ui {
class ExecDialog;
}

class UkuiWindowHelper;
class ExecDialog : public QDialog
{
    Q_OBJECT
Q_SIGNALS:
    void content(QString);
    void result(QString);
public:
    static ExecDialog *instance(QWidget *parent = nullptr);
public:
    ExecDialog(const ExecDialog*)=delete;
    ExecDialog *operator =(const ExecDialog*)=delete;
public:
    int information(QString, QString, bool next = false);
    void error(QString, QString, bool next = false);
    QPushButton* replayEnsure();
    QPushButton* replayCancel();
public slots:
    void close();
protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent* event);
private:
    explicit ExecDialog(QWidget *parent = nullptr);
    ~ExecDialog();
private slots:
    void slotsTipsAgain(bool b);
    void slotChangeUIFontSize(int fontSize);

private:
    QString           m_oTipTitle;
    QString           m_m_labelContent;
    QPushButton      *m_oCancelBtn;
    QPushButton      *m_oEnsureBtn;
    UkuiWindowHelper *m_windowHelper;

private:
    static ExecDialog *m_oInstance;
    static QMutex      m_oMutex;
private:
    Ui::ExecDialog *ui;
};
#define EXEC ExecDialog::instance()
#endif // EXECDIALOG_H
