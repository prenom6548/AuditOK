/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#ifndef INTERACTIVEDIALOG_H
#define INTERACTIVEDIALOG_H

#include "MacroFile.h"

#include <QWidget>
#include <QtWidgets>
#include <QLabel>
#include <QBoxLayout>
#include <QIcon>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QPainterPath>
#include <QTimer>
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QPushButton>
#include <qgsettings.h>
#include <windowmanager/windowmanager.h>
#include <kysdk/applications/kborderbutton.h>

QT_BEGIN_NAMESPACE
namespace Ui { class interactiveDialog; }
QT_END_NAMESPACE

class interactiveDialog: public QWidget
{
    Q_OBJECT
public:
    interactiveDialog(QString strDevId, QWidget *parent);
    ~interactiveDialog();
    void updateContentLable(QString strDevId);
    void moveChooseDialogRight();
    void showInteractivDialog();
private:
    QPushButton *chooseBtnCancle = nullptr;
    QPushButton *chooseBtnContinue = nullptr;
    QLabel *contentLable = nullptr;
    QHBoxLayout *content_H_BoxLayout = nullptr;
    QHBoxLayout *chooseBtn_H_BoxLayout = nullptr;
    QVBoxLayout *main_V_BoxLayout = nullptr;

    double m_transparency;
    int fontSize;
    QString currentThemeMode;
    QGSettings *m_themeSettings = nullptr;
    QGSettings *m_transparency_gsettings = nullptr;
    QColor m_backgroundColor;

private:
    void initWidgets(QString strDevId);
    void initTransparentState();
    void initThemeMode();
    void getTransparentData();
    void setBackgroundColor();

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
public Q_SLOTS:
    void convert();
Q_SIGNALS:
    void FORCESIG();

private:
    QString m_strDevId;
};

#endif
