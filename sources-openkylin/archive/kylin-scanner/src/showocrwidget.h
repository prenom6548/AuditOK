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
#ifndef SHOWOCNWIDGET_H
#define SHOWOCRWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QLinearGradient>
#include <QLabel>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "globalsignal.h"
#include "toolbarwidget.h"
#include "saneobject.h"
#include "include/common.h"
#include "include/theme.h"

class showOcrWidget : public QLabel
{
    Q_OBJECT
public:
    explicit showOcrWidget(QWidget *parent = nullptr);

    void setupGui();
    void initConnect();

protected:
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;


public slots:
    void showScanLine();
    void loadScanningPicture();

    void startScanSlot(QString pic_path);
    void stopScanStop();

    void updateOcrTextEdit();
    void style_changed();
    void setOcrText();
signals:
    void updateTextEdit();

private:
    QLabel *m_ocrImageLabel = nullptr;
    QHBoxLayout *m_ocrImageHLayout = nullptr;

    QTextEdit *m_ocrTextEdit = nullptr;
    QHBoxLayout *m_ocrTextEditHLayout = nullptr;

    QWidget *m_ocrTextEditContainer = nullptr;

    ToolBarWidget *m_toolbarWidget = nullptr;

    QVBoxLayout *m_ocrVLayout = nullptr;

    QTimer *timer = nullptr;
    int scanHeight = 0;
    QString pixFileName;

    bool isDarkTheme();

};

#endif // SHOWOCRWIDGET_H
