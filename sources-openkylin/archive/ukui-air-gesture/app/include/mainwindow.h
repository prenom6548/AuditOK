/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QApplication>
#include <QScreen>
#include <QGSettings>
#include <QPainter>
#include <ukuiwindowhelper.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

    void setIcon(const QString &palmDirection);
private:
    void initGSettings();
    void initWindow();
    void initUI();
    QPixmap invertPixmapColors(const QPixmap& pixmap);
protected:
    void paintEvent(QPaintEvent *event) override;
    
private:
    int m_transparency = 100;
    QGSettings *m_pStyleGSettings = nullptr;
    QLabel *m_pIconLabel = nullptr;
    QString m_currentDirection = "";
};

#endif // MAINWINDOW_H