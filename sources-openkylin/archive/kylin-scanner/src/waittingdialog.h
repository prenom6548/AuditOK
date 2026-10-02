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
#ifndef WAITTINGDIALOG_H
#define WAITTINGDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QHBoxLayout>

class WaittingDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WaittingDialog(QWidget *parent = nullptr);

private:
    QVBoxLayout *m_layout = nullptr;
    QHBoxLayout *m_titleHBoxLayout = nullptr;
    QVBoxLayout *m_mainVlayout = nullptr;
    QHBoxLayout *m_mainHLayout = nullptr;
    QWidget *m_mainWidget = nullptr;
    QPushButton *m_closeButton = nullptr;
    QLabel *m_text = nullptr;
    QProgressBar *m_progress = nullptr;

    void init();
    void initConnect();

public:
    void setText(QString text);

};

#endif // WAITTINGDIALOG_H
