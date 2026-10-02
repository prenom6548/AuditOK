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


#ifndef TIPSCHANGE_H
#define TIPSCHANGE_H

#include <QDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QHBoxLayout>

namespace Ui {
class TipsChange;
}

class UkuiWindowHelper;
class TipsChange : public QDialog
{
    Q_OBJECT

public:
    explicit TipsChange(QWidget *parent = nullptr);
    ~TipsChange();

public:
    int information(QString, QString, bool next = false);
    void error(QString, QString, bool next = false);
    bool question(QString, QString, bool next = false);
    QString result();
    QString content();
    void setResult(QString);
    void setcontext(QString);
    bool input(QString&, QString, QString, QString result);
    QPushButton* replayEnsure();
    QPushButton* replayCancel();
protected:
    void keyPressEvent(QKeyEvent *event);
public slots:
    void slotsClose();
    void slotChangeUIFontSize(int fontSize);
private slots:
     void slotsTipsAgain(bool b);

 private:
     Ui::TipsChange *ui;
     bool               m_bIsWayland;
     QString            m_oTipTitle;

     QString m_labelContent;
     QPushButton *m_oEnsureBtn;
     QPushButton *m_oCancelBtn;
     QString m_result;
     QHBoxLayout *m_hlay;
     UkuiWindowHelper *m_windowHelper;
};

#endif // TIPSCHANGE_H
