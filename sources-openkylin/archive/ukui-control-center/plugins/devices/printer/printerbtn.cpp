/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#include "printerbtn.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QVariant>
#include <QGSettings>
#include <QProcess>

#define THEME_QT_SCHEMA  "org.ukui.style"

#define ICON_QT_KEY      "icon-theme-name"

PrinterBtn::PrinterBtn(const PrinterInfo &printer, QWidget *parent):
    printerinfo(printer),
    QPushButton(parent)
{
    this->setStyleSheet("PrinterBtn:!pressed:hover{background-color: palette(button); border-radius: 6px;}"
                        "PrinterBtn:!pressed:!hover{background-color: palette(base); border-radius: 6px;}");
    this->setProperty("useButtonPalette", true);
    this->setFlat(true);
    setMinimumSize(QSize(550, 60));
    setMaximumSize(QSize(16777215, 60));
    QHBoxLayout *mPrinterLyt = new QHBoxLayout(this);
    mPrinterLyt->setSpacing(16);

    QLabel *iconLabel = new QLabel(this);
    QIcon icon;
    if (printerinfo.uri.contains("usb://",Qt::CaseInsensitive) || printerinfo.uri.contains("/usb",Qt::CaseInsensitive)) {
        icon = QIcon::fromTheme("cupsprinter");
    } else {
        icon = QIcon::fromTheme("printer-remote");
    }
    iconLabel->setPixmap(icon.pixmap(icon.actualSize(QSize(24, 24))));

    const QByteArray id(THEME_QT_SCHEMA);
    QGSettings *icongsettings = new QGSettings(id, QByteArray(), this);

    connect(icongsettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "iconThemeName") {
            QIcon icon;
            if (printerinfo.uri.contains("usb://",Qt::CaseInsensitive) || printerinfo.uri.contains("/usb",Qt::CaseInsensitive)) {
                icon = QIcon::fromTheme("cupsprinter");
            } else {
                icon = QIcon::fromTheme("printer-remote");
            }
            iconLabel->setPixmap(icon.pixmap(icon.actualSize(QSize(24, 24))));
        }
    });

    KLabel *textLabel = new KLabel(this);
    textLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    textLabel->setText(printerinfo.name);
    this->setAccessibleName(printerinfo.name);

    mPrinterLyt->addWidget(iconLabel);
    mPrinterLyt->addWidget(textLabel,Qt::AlignLeft);
}
