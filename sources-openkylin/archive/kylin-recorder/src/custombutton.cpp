/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 *
 *  Authors: baijincheng <baijincheng@kylinos.cn>
 */
#include "custombutton.h"
#include <QPixmap>
#include <QBitmap>
#include <QApplication>

CustomButton::CustomButton(QString normalPicPath, QString hoverPicPath, QString selectedPicPath, QWidget *parent)
    : QPushButton(parent)

{
    QPixmap pixmap(normalPicPath);
    resize(pixmap.size());           //自适应图片的大小
    //setMask(QBitmap(pixmap.mask())); //设置按钮的有效区域
    setMask(pixmap.rect());
    m_curState = BUTTONSTATE::NORMAL;
    m_NormalPicPath = normalPicPath;
    m_HoverPicPath = hoverPicPath;
    m_SelectedPicPath = selectedPicPath;
    m_grayPicPath = ":/svg/svg/rec-gray.svg";
    m_miniGrayPicPath = ":/svg/svg/mini-rec-gray.svg";
    normalPix.load(normalPicPath);
}

void CustomButton::paintEvent(QPaintEvent *)
{
#ifndef WHITE_TEST
    QPixmap drawPix;
    if (m_curState == BUTTONSTATE::NORMAL) {
        //        qDebug()<<"BUTTONSTATE::NORMAL";
        drawPix = normalPix;
    } else if (m_curState == BUTTONSTATE::HOVER) {
        //        qDebug()<<"BUTTONSTATE::HOVER";
        drawPix = hoverPix;
    } else if (m_curState == BUTTONSTATE::SELECTED) {
        //        qDebug()<<"BUTTONSTATE::SELECTED";
        drawPix = selectedPix;
    } else {
        drawPix = grayPix;
    }
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.drawPixmap(0, 0, drawPix);
#endif
}

void CustomButton::setNormalPixmap(QString strImagePath)
{
    normalPix.load(strImagePath);
}

void CustomButton::setEnterPixmap(QString strImagePath)
{
    hoverPix.load(strImagePath);
}

void CustomButton::setLeavePixmap(QString strImagePath)
{
    normalPix.load(strImagePath);
}

void CustomButton::setPressPixmap(QString strImagePath)
{
    selectedPix.load(strImagePath);
}

void CustomButton::setReleasePixmap(QString strImagePath)
{
    normalPix.load(strImagePath);
}

void CustomButton::setButtonEnabled(bool isAvailable, bool isMini)
{
    setEnabled(isAvailable);
    if (isEnabled() == false) {
        SetBtnState(BUTTONSTATE::GRAY);
        if (!isMini) {
            grayPix.load(m_grayPicPath);
        } else {
            grayPix.load(m_miniGrayPicPath);
        }
    } else {
        SetBtnState(BUTTONSTATE::NORMAL);
        grayPix.load(m_NormalPicPath);
    }
}

//设置每组3种状态，可以设置多种按钮：继续、暂停，也可以用一种
void CustomButton::setImagePath(QString normalImgPath, QString hoverImgPath, QString selectedImgPath)
{
    m_NormalPicPath = normalImgPath;
    m_HoverPicPath = hoverImgPath;
    m_SelectedPicPath = selectedImgPath;
    normalPix.load(m_NormalPicPath); //对于切换主题颜色时需要加载不同主题模式的Normal图标
    hoverPix.load(m_HoverPicPath);
    selectedPix.load(m_SelectedPicPath);
}

//设置按钮当前状态
void CustomButton::SetBtnState(BUTTONSTATE state)
{
    m_curState = state;
}

void CustomButton::enterEvent(QEvent *event)
{
    if (isEnabled()) {
        SetBtnState(BUTTONSTATE::HOVER);
        setEnterPixmap(m_HoverPicPath);
    }
    QPushButton::enterEvent(event);
}
void CustomButton::leaveEvent(QEvent *event)
{

    if (isEnabled()) {
        SetBtnState(BUTTONSTATE::NORMAL);
        setLeavePixmap(m_NormalPicPath);
    }
    QPushButton::enterEvent(event);
}

void CustomButton::mousePressEvent(QMouseEvent *mouseEvent)
{

#ifndef WHITE_TEST
    if (isEnabled()) {
        SetBtnState(BUTTONSTATE::SELECTED);
        setPressPixmap(m_SelectedPicPath);
    }
    QPushButton::mousePressEvent(mouseEvent);
#endif
}

void CustomButton::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
#ifndef WHITE_TEST

    if (isEnabled()) {
        SetBtnState(BUTTONSTATE::NORMAL);
        setReleasePixmap(m_NormalPicPath);
    }
    QPushButton::mouseReleaseEvent(mouseEvent);
#endif
}

CustomButton::~CustomButton()
{
    this->deleteLater();
}
