/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 *
 * Authors: baijincheng <baijincheng@kylinos.cn>
 */
#ifndef SAVE_H
#define SAVE_H

#include <QMainWindow>
#include <QLabel>
#include <QToolButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QGridLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <QMouseEvent>
#include <QGSettings>
class Save : public QMainWindow
{
    Q_OBJECT
public:
    explicit Save(QWidget *parent = nullptr);
    QGSettings *saveData = nullptr;
    QGridLayout *pd = nullptr;
    QLabel *label = nullptr;
    QLabel *label1 = nullptr;
    QLabel *label2 = nullptr;
    QLabel *label3 = nullptr;

    QLineEdit *lineEdit1 = nullptr;
    QLineEdit *lineEdit2 = nullptr;

    QComboBox *combobox = nullptr;

    QToolButton *closeButton = nullptr;
    QToolButton *cancelButton = nullptr;
    QToolButton *saveButton = nullptr;

    QHBoxLayout *titleLayout = nullptr;
    QVBoxLayout *mainLayout = nullptr;

    QWidget *titleWid = nullptr;
    QWidget *mainWid = nullptr;
    QWidget *Wid = nullptr;
    QString desktop_path;

signals:
    void clicked();
public slots:
    void savefile();
    void saveToRecorder();
};

#endif // SAVE_H
