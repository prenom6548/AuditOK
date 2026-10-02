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
#ifndef BASE_NOTIFY_DIALOG_H
#define BASE_NOTIFY_DIALOG_H

#include <QDialog>
#include <QMovie>
#include <QIcon>
#include <QTimer>
#include "custom_label.h"
namespace Ui {
class BaseNotifyDialog;
}

enum class BaseNotifyDialogType : int{
    WAITTING = 0,
    WARNING,
    ERROR,
    SUCCEED,
};

class BaseNotifyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BaseNotifyDialog(QWidget *parent = nullptr);
    ~BaseNotifyDialog();
    void dialogSetType(BaseNotifyDialogType);
    void labelSetText(QString text);
    void showThis();

signals:
    void closeThis();
private slots:
    void gifPlay();
private:
    Ui::BaseNotifyDialog *m_ui;

    void setMovieDialog();
    int m_count = 0;
    QTimer *m_timer;
    QIcon *m_loadingIcon;
};

#endif // BASE_NOTIFY_DIALOG_H
