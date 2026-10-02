// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#pragma once

#include <QGridLayout>
#include <QLineEdit>
#include <QWidget>

class Widget : public QWidget
{
    Q_OBJECT
public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

protected:
    void moveEvent(QMoveEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    QLineEdit *m_geoLineEdit = nullptr;
};