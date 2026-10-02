/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "itemwidget.h"
#include "ui_itemwidget.h"
#include <QDebug>
#include <QTextCodec>
#include <QMouseEvent>
#include <QSettings>
#include <QMovie>
#include <QGSettings/QGSettings>

#define ORG_UKUI_STYLE            "org.ukui.style"
#define STYLE_NAME                "styleName"
#define STYLE_NAME_KEY_DARK       "ukui-dark"
#define STYLE_NAME_KEY_DEFAULT    "ukui-default"
#define STYLE_NAME_KEY_LIGHT      "ukui-light"
#define STYLE_NAME_KEY_WHITE      "ukui-white"

ItemWidget::ItemWidget(QString itemname,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ItemWidget)
{
    ui->setupUi(this);
    setProperty("useStyleWindowManager", false);
    QString path = ":/intelGif/resources/intelRes/gesture.ini";
    gesturesetting = new QSettings(path,QSettings::IniFormat);
    gesturesetting->beginGroup(itemname);

    QLocale local;
    QString language = local.languageToString(local.language());
    QString describe;
    QString name;
    QString gifpath;
    QString pixmappath;
    if (QString::compare(language, "Chinese") == 0)
    {
        gesturesetting->setIniCodec("UTF-8");
        describe = gesturesetting->value("describe").toString();
        name = gesturesetting->value("name").toString();
    } else {

        describe = gesturesetting->value("describe_en").toString();
        name = gesturesetting->value("name_en").toString();
    }

    gifpath = gesturesetting->value("gif").toString();
    pixmappath = gesturesetting->value("pixmap").toString();
    ui->label->setText(name);
    ui->label_2->setText(describe);
    movie = new QMovie;
    movie->setFileName(gifpath);
    currentpix = QPixmap(pixmappath);
    ui->label_3->setPixmap(currentpix);
    ui->label_3->setScaledContents(true);
    ui->label_2->setEnabled(false);
    ui->label_2->setWordWrap(true);


    movie->start();//开始播放
    movie->stop();
    status = -1;
    gesturesetting->sync();
    gesturesetting->endGroup();
    ui->pushButton->setIconSize(QSize(30,30));
    ui->pushButton->setIcon(QIcon(":/intelGif/resources/intelRes/play.png"));
    ui->pushButton->setText(QString::fromLocal8Bit(""));
    ui->pushButton->setStyleSheet("QPushButton"
                                  "{/*border:0px solid blue;"
                                  "background:white;"
                                  "*/border-radius: 15px;}"
                                      /*"QPushButton:hover{border:0px;background:blue;border-radius: 15px;}"
                                      "QPushButton:pressed{border:0px;background:red;border-radius: 15px;}"*/);

    color1 = ui->label->palette().windowText().color();
    color2 = ui->label_2->palette().windowText().color();

    const QByteArray id(ORG_UKUI_STYLE);
    if (QGSettings::isSchemaInstalled(id)) {
        stylegsetting = new QGSettings(id);
    }
    connect(stylegsetting, &QGSettings::changed, this, [=] (const QString &key) {
        if (key == STYLE_NAME) {
            if (stylegsetting->get(STYLE_NAME).toString() == STYLE_NAME_KEY_LIGHT)
                color1 = QColor(38, 38, 38, 255);
            else
                color1 = QColor(255, 255, 255, 255);

        }
    });
}

ItemWidget::~ItemWidget()
{
    delete ui;
    delete movie;
    delete gesturesetting;

}

void ItemWidget::startMovieSlot()
{
    ui->label_3->setMovie(movie);
    movie->start();
    ui->pushButton->setIcon(QIcon(":/intelGif/resources/intelRes/pause.png"));
    color1_pre = ui->label->palette().windowText().color();
    color2_pre = ui->label_2->palette().windowText().color();

    color1 = QColor(47, 179, 232,255);
    color2 = QColor(63, 184, 233,150);
}

void ItemWidget::stopMovieSlot()
{
    movie->stop();
    ui->pushButton->setIcon(QIcon(":/intelGif/resources/intelRes/play.png"));
    color1 = color1_pre;
    color2 = color2_pre;
    ui->label_3->setPixmap(currentpix);
}

void ItemWidget::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << Q_FUNC_INFO;
    if (event->button() == Qt::LeftButton) {
        Q_EMIT picClicked();
    }
}

void ItemWidget::paintEvent(QPaintEvent*)
{
    QPalette overPalette;   //走过的步骤
    overPalette.setColor(QPalette::WindowText, color1);
    ui->label->setPalette(overPalette);
    QPalette normalPalette; //正常的步骤
    normalPalette.setColor(QPalette::WindowText, color2);
    ui->label_2->setPalette(normalPalette);

}
