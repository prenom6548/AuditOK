/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "enrollbtn.h"
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QVariant>

#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

EnrollBtn::EnrollBtn(QWidget *parent):
    QPushButton(parent)
{
    this->setObjectName("this");
    this->setMinimumSize(QSize(580, 60));
    this->setMaximumSize(QSize(16777215, 60));
    this->setProperty("useButtonPalette", true);
    this->setFlat(true);

    this->setStyleSheet("AddBtn:!checked:!pressed:!hover{background-color: palette(base); border-radius: 6px;}"
                        "AddBtn:!checked:!pressed:hover{background-color: palette(button); border-radius: 6px;}");

    QHBoxLayout *addLyt = new QHBoxLayout;

    iconLabel = new QLabel();
    QIcon mAddIcon = QIcon::fromTheme("list-add-symbolic");
    iconLabel->setPixmap(mAddIcon.pixmap(mAddIcon.actualSize(QSize(24, 24))));
    iconLabel->setProperty("iconHighlightEffectMode", 1);
    KDK_EXTEND_ALL_INFO_FORMAT(
        iconLabel, "LoginOptions", "BiometricsWidget", "add biometric feature button icon");
    textLabel = new QLabel();
    KDK_EXTEND_ALL_INFO_FORMAT(
        iconLabel, "LoginOptions", "BiometricsWidget", "add biometric feature button text");
    const QByteArray idd(THEME_QT_SCHEMA);
    QGSettings *qtSettings  = new QGSettings(idd, QByteArray(), this);
    QString currentThemeMode = qtSettings->get(MODE_QT_KEY).toString();
    if ("ukui-dark" == currentThemeMode || "ukui-black" == currentThemeMode){
        iconLabel->setProperty("useIconHighlightEffect", true);
    }
    connect(qtSettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "styleName") {
            QString currentThemeMode = qtSettings->get(key).toString();
            if ("ukui-black" == currentThemeMode || "ukui-dark" == currentThemeMode) {
                iconLabel->setProperty("useIconHighlightEffect", true);
            } else if("ukui-white" == currentThemeMode || "ukui-default" == currentThemeMode) {
                iconLabel->setProperty("useIconHighlightEffect", false);
            }
        }
    });

    iconLabel->setProperty("iconHighlightEffectMode", 1);

    addLyt->addStretch();
    addLyt->addWidget(iconLabel);
    addLyt->addWidget(textLabel);
    addLyt->addStretch();
    this->setLayout(addLyt);

}

EnrollBtn::~EnrollBtn()
{

}

void EnrollBtn::enterEvent(QEvent *event){
    Q_EMIT enterWidget();

    QPushButton::enterEvent(event);
}

void EnrollBtn::leaveEvent(QEvent *event){
    Q_EMIT leaveWidget();

    QPushButton::leaveEvent(event);
}

void EnrollBtn::setName(const QString text)
{
    textLabel->setText(text);
}
