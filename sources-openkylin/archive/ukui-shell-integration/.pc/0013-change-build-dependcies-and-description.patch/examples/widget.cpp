// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include "widget.h"

#include <QComboBox>
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>

Widget::Widget(QWidget *parent)
{
    setWindowTitle("Dialog Example");
    auto layout = new QVBoxLayout(this);
    auto button1 = new QPushButton("Modal Dialog", this);
    layout->addWidget(button1);
    connect(button1, &QPushButton::clicked, [this] {
        QDialog dialog(this);
        dialog.setWindowTitle("Modal Dialog");
        dialog.resize(400, 300);
        dialog.exec();
    });

    auto button2 = new QPushButton("Normal Dialog", this);
    layout->addWidget(button2);
    connect(button2, &QPushButton::clicked, [this] {
        QDialog *dialog = new QDialog(this);
        dialog->setWindowTitle("Normal Dialog");
        dialog->resize(400, 300);
        dialog->show();
    });

    layout->setStretch(0, 0);
    layout->setAlignment(Qt::AlignTop);
    setLayout(layout);

    auto button3 = new QPushButton("No TitleBar Window", this);
    layout->addWidget(button3);
    connect(button3, &QPushButton::clicked, [this] {
        QWidget *w = new QWidget();
        w->setAttribute(Qt::WA_NativeWindow);
        w->windowHandle()->setProperty("no_titlebar", true);
        w->show();

        // show and hide the window to test remove titlebar feature
        // just for test, no need to do this in real application
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, [w] {
            if (w->isVisible()) {
                w->hide();
            } else {
                w->show();
            }
        });
        timer->start(1000);
    });

    // Window Role Example
    QHBoxLayout *hlayout = new QHBoxLayout();
    layout->addLayout(hlayout);
    QComboBox *comboBox = new QComboBox(this);
    auto button4 = new QPushButton("Window Role", this);
    hlayout->addWidget(comboBox);
    hlayout->addWidget(button4);
    // supported roles
    QStringList roles = {"normal",
                         "desktop",
                         "panel",
                         "onscreendisplay",
                         "notification",
                         "tooltip",
                         "criticalnotification",
                         "appletpopup",
                         "screenlock",
                         "watermark",
                         "systemwindow",
                         "inputpanel",
                         "logout",
                         "screenlocknotification",
                         "switcher"};
    comboBox->addItems(roles);
    QWidget *win = new QWidget();
    win->setAttribute(Qt::WA_NativeWindow);
    connect(button4, &QPushButton::clicked, [comboBox, win] {
        // set window role property which is case insensitive
        win->windowHandle()->setProperty("window_role", comboBox->currentText());
        // re-show the window to apply the role
        win->hide();
        win->show();
    });

    // Window Icon Example
    QHBoxLayout *hlayout1 = new QHBoxLayout();
    layout->addLayout(hlayout1);
    QLineEdit *lineEdit1 = new QLineEdit("kylin-video", this);
    auto button5 = new QPushButton("Window Icon", this);
    hlayout1->addWidget(lineEdit1);
    hlayout1->addWidget(button5);
    QWidget *win2 = new QWidget();
    win2->setAttribute(Qt::WA_NativeWindow);
    connect(button5, &QPushButton::clicked, [lineEdit1, win2] {
        // set window icon property, the icon name should be the same as the icon name in the theme
        win2->windowHandle()->setProperty("icon", lineEdit1->text());
        // re-show the window to apply the icon
        if (win2->isVisible())
            win2->hide();
        win2->show();
    });

    // Watermark Example
    QWidget *win_watermark = new QWidget();
    win_watermark->setAttribute(Qt::WA_NativeWindow);
    win_watermark->setWindowFlags(Qt::WindowTransparentForInput | Qt::FramelessWindowHint);
    // Mask is the region of the window that is not transparent
    win_watermark->setMask(QRegion());
    auto button6 = new QPushButton("Watermark", this);
    layout->addWidget(button6);
    connect(button6, &QPushButton::clicked, [this, win_watermark] {
        if (win_watermark->isVisible())
            win_watermark->hide();
        win_watermark->show();
    });

    // Show current window geometry in line edit x,y,w,h
    m_geoLineEdit = new QLineEdit(this);
    QHBoxLayout *hlayout2 = new QHBoxLayout();
    layout->addLayout(hlayout2);
    hlayout2->addWidget(new QLabel("Window Geometry: ", this));
    hlayout2->addWidget(m_geoLineEdit);
    auto button7 = new QPushButton("Set Window Geometry", this);
    hlayout2->addWidget(button7);
    connect(button7, &QPushButton::clicked, [this] {
        QStringList geo = m_geoLineEdit->text().split(",");
        if (geo.size() == 4) {
            setGeometry(geo[0].toInt(), geo[1].toInt(), geo[2].toInt(), geo[3].toInt());
        }
    });
    resize(800, 600);
}

Widget::~Widget()
{
    exit(0);
}

void Widget::moveEvent(QMoveEvent *event)
{
    m_geoLineEdit->setText(QString("%1,%2,%3,%4").arg(x()).arg(y()).arg(width()).arg(height()));
}

void Widget::resizeEvent(QResizeEvent *event)
{
    m_geoLineEdit->setText(QString("%1,%2,%3,%4").arg(x()).arg(y()).arg(width()).arg(height()));
}