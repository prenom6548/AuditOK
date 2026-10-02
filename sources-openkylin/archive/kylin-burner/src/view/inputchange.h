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


#ifndef INPUTCHANGE_H
#define INPUTCHANGE_H

#include <QDialog>
#include <QMessageBox>
#include <QKeyEvent>

namespace Ui {
class InputChange;
}

class UkuiWindowHelper;
class InputChange : public QDialog
{
    Q_OBJECT
    friend class MainWindowTest;
public:
    explicit InputChange(QWidget *parent = nullptr);
    ~InputChange();

    QString result();
    QString content();

    bool input(QString&, QString, QString, QString fileName);

public slots:
    void slotTextChanged(QString);
    void slotClose();

    void slotChangeUIFontSize(int fontSize);

protected:
    void keyPressEvent(QKeyEvent *event);
    void setLineEditSelect(QString fileName);
    int indexSuffix(QString fileName);

private:
    Ui::InputChange *ui;
    bool             m_bIsWayland;
    QString          m_result;
    QString          m_labelContent;
    QPushButton     *m_oCancelBtn;
    QPushButton     *m_oEnsureBtn;
    UkuiWindowHelper *m_windowHelper;
};

#endif // INPUTCHANGE_H
