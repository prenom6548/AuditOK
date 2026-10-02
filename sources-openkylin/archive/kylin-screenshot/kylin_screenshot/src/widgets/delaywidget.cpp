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
#include "delaywidget.h"
#include  <QPainter>
#include  <QFont>

#include <QGSettings>
#define UKUI_STYLE_SCHEMA          "org.ukui.style"
#define STYLE_NAME                 "styleName"
#define STYLE_NAME_KEY_DARK        "ukui-dark"
#define STYLE_NAME_KEY_DEFAULT     "ukui-default"
#define STYLE_NAME_KEY_BLACK       "ukui-black"
#define STYLE_NAME_KEY_LIGHT       "ukui-light"
#define STYLE_NAME_KEY_WHITE       "ukui-white"

DelayWidget::DelayWidget(QWidget *parent) :
    QWidget(parent)
{
    setWindowFlags(Qt::WindowStaysOnTopHint
                   | Qt::FramelessWindowHint
                   | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setGeometry(100, 100, 118, 118);
    label = new  QLabel(this);

    QFont font;
    font.setPixelSize(62);
    font.setFamily("Arial");
    label->setFont(font);
    label->setAlignment(Qt::AlignCenter);
    label->setFixedSize(118, 118);
    label->setProperty("useSystemStyleBlur", true);
    listenToGsettings();
}

DelayWidget::~DelayWidget()
{
}

void DelayWidget::updateText(int time)
{
    QString num = QString::number(time);
    label->setText(num);
}

void DelayWidget::listenToGsettings()
{
    const QByteArray styleID(UKUI_STYLE_SCHEMA);
    QStringList stylelist;
    if (QGSettings::isSchemaInstalled(styleID)) {
        QGSettings *styleUKUI = new QGSettings(styleID, QByteArray(), this);
        stylelist << STYLE_NAME_KEY_DARK << STYLE_NAME_KEY_BLACK;
        // <<STYLE_NAME_KEY_DEFAULT;
        if (stylelist.contains(styleUKUI->get(STYLE_NAME).toString())) {
            label->setStyleSheet("QLabel {"
                                 "border-width: 0px; "
                                 "border-radius: 18px; "
                                 "background-color:rgb(25,25,25,235);}");
        } else {
            label->setStyleSheet("QLabel {"
                                 "border-width: 0px; "
                                 "border-radius: 18px; "
                                 "background-color:rgb(225,225,225,235);}");
        }
        connect(styleUKUI, &QGSettings::changed, this, [=](const QString &key)
        {
            if (key == STYLE_NAME) {
                if (stylelist.contains(styleUKUI->get(
                                           STYLE_NAME).toString())) {
                    label->setStyleSheet("QLabel {"
                                         "border-width: 0px; "
                                         "border-radius: 18px; "
                                         "background-color:rgb(25,25,25,235);}");
                } else {
                    label->setStyleSheet("QLabel {"
                                         "border-width: 0px; "
                                         "border-radius: 18px; "
                                         "background-color:rgb(225,225,225,235);}");
                }
            }
        });
    }
}
