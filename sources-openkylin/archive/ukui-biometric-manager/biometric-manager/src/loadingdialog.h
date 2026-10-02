/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef LOADINGDIALOG_H
#define LOADINGDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QHBoxLayout>
#include <QLabel>

class QTimer;

class LoadingDialog : public QWidget
{
    Q_OBJECT
public:
    LoadingDialog(QWidget *parent = nullptr ,float opacity = 0);
    virtual ~LoadingDialog();

    /**
     * 设置为加载状态
     *
     */
    void setLoading();

public Q_SLOTS:
    void onLoading();

protected:
    void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    QHBoxLayout *m_layoutMain = nullptr;
    QLabel *m_labelLoading = nullptr;
    QTimer *m_timerLoading = nullptr;
    int m_nCurTick = 0;
    unsigned long long m_lluInitTick = 0;
    float transparency = 0;
};

#endif // LOADINGDIALOG_H
