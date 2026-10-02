/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef URLEDITWIDGET_H
#define URLEDITWIDGET_H

#include <QLabel>
#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>

class URLEditWidget : public QDialog
{
    Q_OBJECT
public:
    explicit URLEditWidget(QWidget *parent = nullptr);
    ~URLEditWidget();

    QString getUrl(){return m_url;}

private:
    void initLayout();

    QString m_url;

    QVBoxLayout *m_layout_widget;
    QHBoxLayout *m_layout_title;
    QHBoxLayout *m_layout_button;
    QLineEdit *m_line_edit_url;
    QLabel *m_label_title;
    QPushButton *m_button_icon;
    QPushButton *m_button_close;
    QPushButton *m_button_ok;
    QPushButton *m_button_cancel;
};

#endif // URLEDITWIDGET_H
