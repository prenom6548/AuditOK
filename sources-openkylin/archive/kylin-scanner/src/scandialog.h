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
#ifndef SCANDIALOG_H
#define SCANDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QSize>
#include <QPushButton>
#include <QLabel>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QVariant>

#define ScanDialogWindowSize QSize(380, 164)
#define ScanDialogIconSize QSize(26, 26)
#define ScanDialogLabelHeight 34
#define ScanDialogButtonSize QSize(96, 36)

#include "saneobject.h"
#include "custom_push_button.h"

class ScanDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ScanDialog(QWidget *parent = nullptr);

    void setupGui();
    void initConnect();

    int pageNumber() const;
    void setPageNumber(int pageNumber);

    QString scanMsg() const;
    void setScanMsg(const QString &scanMsg);

Q_SIGNALS:


public slots:
    void updatePageNumberWhileScanning();
    void updatePageNumberWhileStopScanning();


private:
    int m_pageNumber = 1;
    QString m_scanMsg;

    QLabel *m_titleLabel = nullptr;
    QPushButton *m_closeButton = nullptr;
    QLabel *m_iconLabel = nullptr;
    QLabel *m_msgLabel = nullptr;
    CustomPushButton *m_cancelButton = nullptr;

    QWidget *m_titleWidget = nullptr;
    QHBoxLayout *m_titleHBoxLayout = nullptr;
    QWidget *m_msgWidget = nullptr;
    QHBoxLayout *m_msgHBoxLayout = nullptr;
    QWidget *m_cancelWidget = nullptr;
    QHBoxLayout *m_cancelHBoxLayout = nullptr;
    QVBoxLayout *m_mainVBoxLayout = nullptr;

    bool isDarkTheme();
    void cancelOpration();


};

#endif // SCANDIALOG_H
