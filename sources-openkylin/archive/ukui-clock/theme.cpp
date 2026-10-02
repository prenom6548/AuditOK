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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "theme.h"
#include <QByteArray>
#include <QVariant>
#include <QPainter>
/**
 * QT主题
 */
//#define THEME_QT_SCHEMA "org.ukui.style"
//#define MODE_QT_KEY "style-name"

#define WHITE_BACK_COLOR 255, 255, 255,255
#define BLACK_BACK_COLOR 18, 18, 18,255
#define DIALOG_BACK_COLOR_WHITE_STYLE 255, 255, 255,255
#define DIALOG_BACK_COLOR_BLACK_STYLE 26, 26, 26,255
#define SELECT_DIALOG_BACK_COLOR_WHITE_STYLE 255, 255, 255,255
#define SELECT_DIALOG_BACK_COLOR_BLACK_STYLE 51, 51, 51,255
#define ITEM_BACK_COLOR_WHITE_STYLE 247, 247, 247,255
#define ITEM_BACK_COLOR_BLACK_STYLE 41, 41, 41,255
#define BTN_BACK_COLOR_WHITE_STYLE 240, 240, 240,255
#define BTN_BACK_COLOR_BLACK_STYLE 55, 55, 59,255
#define SWITCH_BTN_BACK_COLOR_WHITE_STYLE 233,233,233,255
#define SWITCH_BTN_BACK_COLOR_BLACK_STYLE 72,72,76,255
#define COUNTDOWN_RING_BACKCOLOR_WHITE_STYLE 247, 247, 247,255
#define COUNTDOWN_RING_BACKCOLOR_BLACK_STYLE 37,37,37,255
#define TINY_BTN_BACK_COLOR_WHITE_STYLE 233,233,233,255
#define TINY_BTN_BACK_COLOR_BLACK_STYLE 65,65,65,255
#define BODER_COLOR_WHITE_STYLE 0,0,0,40
#define BODER_COLOR_BLACK_STYLE 0,0,0,40
QColor theme::backcolcr=QColor(WHITE_BACK_COLOR);
QColor theme::dialogBackcolcr=QColor(DIALOG_BACK_COLOR_WHITE_STYLE);
QColor theme::selectDialogBackcolcr=QColor(SELECT_DIALOG_BACK_COLOR_WHITE_STYLE);
QColor theme::itemBackColor=QColor(ITEM_BACK_COLOR_WHITE_STYLE);
QColor theme::selectBtnBackColor=QColor(BTN_BACK_COLOR_WHITE_STYLE);
QColor theme::switchBtnBackColor=QColor(SWITCH_BTN_BACK_COLOR_WHITE_STYLE);
QColor theme::timeScrollBackColor=QColor(BTN_BACK_COLOR_WHITE_STYLE);
QColor theme::countdownRingBackColor=QColor(COUNTDOWN_RING_BACKCOLOR_WHITE_STYLE);
QColor theme::tinyBtnBackColor=QColor(TINY_BTN_BACK_COLOR_WHITE_STYLE);
QColor theme::boderColor=QColor(BODER_COLOR_WHITE_STYLE);

//主题
int theme::themetype=0;
theme::theme(QObject *parent) : QObject(parent)
{

    //const QByteArray idd(THEME_QT_SCHEMA);
    if (QGSettings::isSchemaInstalled(ORG_UKUI_STYLE)){
        qtSettings = new QGSettings(ORG_UKUI_STYLE);

    } else {
        //qCritical() <<THEME_QT_SCHEMA << "not installed\n";
    }
    //qtSettings->get(STYLE_NAME)
    QString currentThemeMode =qtSettings->get(STYLE_NAME).toString();
    setColorByTheme(currentThemeMode);
    connect(qtSettings, &QGSettings::changed, this, [=]{
        QString currentThemeMode = qtSettings->get(STYLE_NAME).toString();
        setColorByTheme(currentThemeMode);
    });
}
theme::~theme()
{
    delete qtSettings;
}

void theme::setColorByTheme(QString currentThemeMode)
{
    if ("ukui-white" == currentThemeMode) {
        backcolcr=QColor(WHITE_BACK_COLOR);
        itemBackColor=QColor(ITEM_BACK_COLOR_WHITE_STYLE);
        selectBtnBackColor=QColor(BTN_BACK_COLOR_WHITE_STYLE);
        switchBtnBackColor=QColor(SWITCH_BTN_BACK_COLOR_WHITE_STYLE);
        timeScrollBackColor=QColor(BTN_BACK_COLOR_WHITE_STYLE);
        dialogBackcolcr=QColor(DIALOG_BACK_COLOR_WHITE_STYLE);
        selectDialogBackcolcr=QColor(SELECT_DIALOG_BACK_COLOR_WHITE_STYLE);
        countdownRingBackColor=QColor(COUNTDOWN_RING_BACKCOLOR_WHITE_STYLE);
        tinyBtnBackColor=QColor(TINY_BTN_BACK_COLOR_WHITE_STYLE);
        boderColor=QColor(BODER_COLOR_WHITE_STYLE);
        themetype=0;
    }else if ("ukui-light" == currentThemeMode) {
        backcolcr=QColor(WHITE_BACK_COLOR);
        itemBackColor=QColor(ITEM_BACK_COLOR_WHITE_STYLE);
        selectBtnBackColor=QColor(BTN_BACK_COLOR_WHITE_STYLE);
        switchBtnBackColor=QColor(SWITCH_BTN_BACK_COLOR_WHITE_STYLE);
        timeScrollBackColor=QColor(BTN_BACK_COLOR_WHITE_STYLE);
        dialogBackcolcr=QColor(DIALOG_BACK_COLOR_WHITE_STYLE);
        selectDialogBackcolcr=QColor(SELECT_DIALOG_BACK_COLOR_WHITE_STYLE);
        countdownRingBackColor=QColor(COUNTDOWN_RING_BACKCOLOR_WHITE_STYLE);
        tinyBtnBackColor=QColor(TINY_BTN_BACK_COLOR_WHITE_STYLE);
        boderColor=QColor(BODER_COLOR_WHITE_STYLE);
        themetype=0;
    }else if ("ukui-dark" == currentThemeMode) {
        backcolcr=QColor(BLACK_BACK_COLOR);
        itemBackColor=QColor(ITEM_BACK_COLOR_BLACK_STYLE);
        selectBtnBackColor=QColor(BTN_BACK_COLOR_BLACK_STYLE);
        switchBtnBackColor=QColor(SWITCH_BTN_BACK_COLOR_BLACK_STYLE);
        timeScrollBackColor=QColor(BTN_BACK_COLOR_BLACK_STYLE);
        dialogBackcolcr=QColor(DIALOG_BACK_COLOR_BLACK_STYLE);
        selectDialogBackcolcr=QColor(SELECT_DIALOG_BACK_COLOR_BLACK_STYLE);
        countdownRingBackColor=QColor(COUNTDOWN_RING_BACKCOLOR_BLACK_STYLE);
        tinyBtnBackColor=QColor(TINY_BTN_BACK_COLOR_BLACK_STYLE);
        boderColor=QColor(BODER_COLOR_BLACK_STYLE);
        themetype=1;
    }else if ("ukui-black" == currentThemeMode) {
        backcolcr=QColor(BLACK_BACK_COLOR);
        itemBackColor=QColor(ITEM_BACK_COLOR_BLACK_STYLE);
        selectBtnBackColor=QColor(BTN_BACK_COLOR_BLACK_STYLE);
        switchBtnBackColor=QColor(SWITCH_BTN_BACK_COLOR_BLACK_STYLE);
        timeScrollBackColor=QColor(BTN_BACK_COLOR_BLACK_STYLE);
        dialogBackcolcr=QColor(DIALOG_BACK_COLOR_BLACK_STYLE);
        selectDialogBackcolcr=QColor(SELECT_DIALOG_BACK_COLOR_BLACK_STYLE);
        countdownRingBackColor=QColor(COUNTDOWN_RING_BACKCOLOR_BLACK_STYLE);
        tinyBtnBackColor=QColor(TINY_BTN_BACK_COLOR_BLACK_STYLE);
        boderColor=QColor(BODER_COLOR_BLACK_STYLE);
        themetype=1;
    }else if ("" == currentThemeMode) {
        backcolcr=QColor(WHITE_BACK_COLOR);
        itemBackColor=QColor(ITEM_BACK_COLOR_WHITE_STYLE);
        selectBtnBackColor=QColor(BTN_BACK_COLOR_WHITE_STYLE);
        switchBtnBackColor=QColor(SWITCH_BTN_BACK_COLOR_WHITE_STYLE);
        timeScrollBackColor=QColor(BTN_BACK_COLOR_WHITE_STYLE);
        dialogBackcolcr=QColor(DIALOG_BACK_COLOR_WHITE_STYLE);
        selectDialogBackcolcr=QColor(SELECT_DIALOG_BACK_COLOR_WHITE_STYLE);
        countdownRingBackColor=QColor(COUNTDOWN_RING_BACKCOLOR_WHITE_STYLE);
        tinyBtnBackColor=QColor(TINY_BTN_BACK_COLOR_WHITE_STYLE);
        boderColor=QColor(BODER_COLOR_WHITE_STYLE);
        themetype=0;
    } else{
        backcolcr=QColor(WHITE_BACK_COLOR);
        itemBackColor=QColor(ITEM_BACK_COLOR_WHITE_STYLE);
        selectBtnBackColor=QColor(BTN_BACK_COLOR_WHITE_STYLE);
        switchBtnBackColor=QColor(SWITCH_BTN_BACK_COLOR_WHITE_STYLE);
        timeScrollBackColor=QColor(BTN_BACK_COLOR_WHITE_STYLE);
        dialogBackcolcr=QColor(DIALOG_BACK_COLOR_WHITE_STYLE);
        selectDialogBackcolcr=QColor(SELECT_DIALOG_BACK_COLOR_WHITE_STYLE);
        countdownRingBackColor=QColor(COUNTDOWN_RING_BACKCOLOR_WHITE_STYLE);
        tinyBtnBackColor=QColor(TINY_BTN_BACK_COLOR_WHITE_STYLE);
        boderColor=QColor(BODER_COLOR_WHITE_STYLE);
        themetype=0;
    }
}
QPixmap theme::changeIconColor(QPixmap pixmap, QColor color)
{
    QPixmap target = pixmap;
    QPainter p(&target);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(target.rect(), color);
    return target;
}
QString theme::getColorStr(QColor color)
{
    QString colorInfo=""+QString::number(color.red())+","+QString::number(color.green())
            +","+QString::number(color.blue())+","+QString::number(color.alpha());
    return "rgba("+colorInfo+")";
}
QColor theme::highLight_Click()
{
    return QColor(41, 108, 217);
}
QColor theme::highLight_Hover()
{
    return QColor(64, 169, 251);
}
QColor theme::button_Click()
{
    return QColor(217, 217, 217);
}

QColor theme::button_Hover()
{
    return QColor(235, 235, 235);
}
