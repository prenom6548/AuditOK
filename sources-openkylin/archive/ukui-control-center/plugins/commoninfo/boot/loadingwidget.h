/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef LOADINGWIDGET_H
#define LOADINGWIDGET_H

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QLabel>
#include <kballontip.h>
using namespace kdk;

class QTimer;

class LoadingWidget : public QWidget
{
    Q_OBJECT
public:
    LoadingWidget(QWidget *parent = nullptr, float opacity = 0);
    virtual ~LoadingWidget();

    /**
     * 设置为加载状态
     *
     */
    void setLoading();

    void setLoadingText(QString text);
    QSize labelsize();

public Q_SLOTS:
    void onLoading();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QVBoxLayout *m_pLayoutMain = nullptr;
    KBallonTip *m_pLoadingLabel = nullptr;
    QTimer *m_pTimerLoading = nullptr;
    int m_nCurTick = 0;
    unsigned long long m_lluInitTick = 0;
    float m_transparency = 0;
};

#endif // LOADINGWIDGET_H
