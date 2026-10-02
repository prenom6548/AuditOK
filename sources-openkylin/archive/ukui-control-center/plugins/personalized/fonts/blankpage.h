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
#ifndef BLANKPAGE_H
#define BLANKPAGE_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

class BlankPage : public QWidget
{
    Q_OBJECT

public:
    BlankPage(QWidget *parent = nullptr);
    ~BlankPage();

    void setWidgetUi();
    void initLayout();
    void initGsetting();
    void changeSearchText();
    void changeBlankText();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void updateFontSize();

private:
    QLabel *m_blankLabel;
    QLabel *m_blankTextLabel;
    QVBoxLayout *m_vLayout = nullptr;
    QHBoxLayout *m_hLayout = nullptr;

};
#endif // FONTWIDGET_H
