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
#ifndef WATERMARKDIALOG_H
#define WATERMARKDIALOG_H

#include <QDialog>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QStyleOption>
#include <QColor>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QProcess>
#include <QPixmap>
#include <QFileInfo>
#include <QFileIconProvider>

#include <QDebug>
#include <kpushbutton.h>
#include "globalsignal.h"
#include "include/common.h"
#include "include/theme.h"
#define UKUI_THEME_GSETTING_PATH "org.ukui.style"

class WatermarkDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WatermarkDialog(QWidget *parent = nullptr);

    void initWindow();
    void initLayout();
    void initConnect();

    QString getLineEditText();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
public slots:
    void confirmButtonClickedSlot();
    void fontSizeChangedSlot();
private:
    QPushButton *m_closeButton = nullptr;
    QHBoxLayout *m_titleHBoxLayout = nullptr;

    QLabel *m_infoTextLabel = nullptr;
    QLineEdit *m_watermarkLineEdit = nullptr;
    QHBoxLayout *m_watermarkHBoxLayout = nullptr;

    kdk::KPushButton *m_cancelButton = nullptr;
    kdk::KPushButton *m_confirmButton = nullptr;
    QHBoxLayout *m_buttonsHBoxLayout = nullptr;

    QVBoxLayout *m_mainVBoxLayout = nullptr;
    QGSettings *m_themeData = nullptr;

};

#endif // WATERMARKDIALOG_H
