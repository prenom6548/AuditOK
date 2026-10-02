/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
 *
 * Authors:
 *  YangChenBoHuang    yangchenbohuang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LOCKWIDGET_H
#define LOCKWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QStackedLayout>
#include <QDialog>

class QPushButton;
class QNetworkReply;

class LockWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LockWidget(QWidget *parent = 0);
    void checkkylinid();
    void finishedSlot(QNetworkReply *reply);
    void closelock();
    QLineEdit *m_line = nullptr;

private:
    QStackedLayout *m_stackedLayout = nullptr;
    QFrame *m_lockFrame = nullptr;
    QLabel *m_title = nullptr;
    QLabel *m_tiplabel = nullptr;
    QPushButton *m_Btn0 = nullptr;
    QPushButton *m_Btn1 = nullptr;
    QPushButton *m_Btn2 = nullptr;
    QPushButton *m_Btn3 = nullptr;
    QPushButton *m_Btn4 = nullptr;
    QPushButton *m_Btn5 = nullptr;
    QPushButton *m_Btn6 = nullptr;
    QPushButton *m_Btn7 = nullptr;
    QPushButton *m_Btn8 = nullptr;
    QPushButton *m_Btn9 = nullptr;
    QPushButton *m_resetBtn = nullptr;
    QPushButton *m_back = nullptr;
    QLabel *m_reset = nullptr;
    QLabel *m_kylinid = nullptr;
    QLabel *m_kylincode = nullptr;
    QLabel *m_newcode = nullptr;
    QLineEdit *m_idline = nullptr;
    QLineEdit *m_codeline = nullptr;
    QLineEdit *m_newcodeline = nullptr;
    QPushButton *m_backBtn = nullptr;
    QPushButton *m_okBtn = nullptr;
    QString code;
};

class SimpleLockWidget : public QDialog
{
public:
    SimpleLockWidget(QWidget *parent = nullptr);
    ~SimpleLockWidget() = default;
};

#endif // LOCKWIDGET_H
