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

#ifndef FAILEDPAGEWIDGET_H
#define FAILEDPAGEWIDGET_H

#include "saneobject.h"

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QPalette>
#include <QColor>
#include <QFont>
#include <QPalette>
#include <QThread>
#include<QSize>

#define FailedPageIconSize  QSize(96, 96)
#define FailedPageButtonSize  QSize(102, 36)
#define FailedPageButtonSizePADMODE  QSize(110, 48)


class FailedPageWidget : public QWidget
{
    Q_OBJECT
public:
    explicit FailedPageWidget(QWidget *parent = nullptr);

    void setupGui();
    void initConnect();

signals:
    void scanButtonClicked();

private:
    QLabel *m_failedPageIcon = nullptr;
    QLabel *m_failedPageText = nullptr;
    QPushButton *m_failedPageButton = nullptr;
    QVBoxLayout *m_failedPageVLayout = nullptr;
    QString themeColor = "white";

    bool isDarkTheme();

public slots:
    void fontSizeChanged();
    void clickfiledPageButton();
    void initTheme();
    void rotateChangedSlot(bool isPCMode);
};

#endif // FAILEDPAGEWIDGET_H
