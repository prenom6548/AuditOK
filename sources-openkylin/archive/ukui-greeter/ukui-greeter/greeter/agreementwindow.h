/*
 * Copyright (C) 2018 KylinSoftCo., Ltd.
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
#ifndef AGREEMENTWINDOW_H
#define AGREEMENTWINDOW_H

#include <QWidget>
#include <QTextBrowser>
#include <QPushButton>
#include <QLabel>
#include <QFile>

class AgreementWindow : public QWidget
{
    Q_OBJECT
public:
    explicit AgreementWindow(QWidget *parent = nullptr);
    bool getShowLoginPrompt();
    void initUI();

signals:
    void switchToGreeter();

protected:
    void resizeEvent(QResizeEvent *event);

protected Q_SLOTS:
    void switchPage();

private:
    void init();
    bool loadText();

    QTextBrowser *browser = nullptr;
    QLabel  *titleLbl = nullptr;
    QPushButton  *ensureBtn = nullptr;

    QFile *file = nullptr;

    bool  showLoginPrompt = false;
    bool  hideTitle = true;

    QString promptTitle = "";
    QString promptText = "";
    QString promptTextFilePath = "";

    QWidget *centerWidget = nullptr;

};

#endif // AGREEMENTWINDOW_H
