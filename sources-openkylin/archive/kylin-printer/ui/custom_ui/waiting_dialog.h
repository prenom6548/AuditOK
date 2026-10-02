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
 */
#ifndef WAITING_DIALOG_H
#define WAITING_DIALOG_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class WaitingDialog;
}

class WaitingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WaitingDialog(const QString &waitingInfo, int msec, QWidget *parent = nullptr);
    explicit WaitingDialog(QWidget *parent = nullptr);
    ~WaitingDialog();
    void setAutoCloseTime(int msec);
    void labelSetText(QString text);

private:
    QWidget *m_parentWindow;
    Ui::WaitingDialog *m_ui;
    QTimer *m_autoCloseTimer{nullptr};
    int m_interval{0};
    void init();
public Q_SLOTS:
    int exec();
    void show();
};

#endif // WAITING_DIALOG_H
