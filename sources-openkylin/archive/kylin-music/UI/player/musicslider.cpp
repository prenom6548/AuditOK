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

#include "musicslider.h"
#include "UI/base/widgetstyle.h"
#include "UIControl/player/player.h"
#include <QApplication>

MusicSlider::MusicSlider(QWidget *parent):
    QSlider(parent),
    m_isPlaying(false)
{
//    this->installEventFilter(this);
    initStyle();//初始化样式
}


/**
 * @brief MusicSlider::initStyle
 * 如果不选择歌曲，进度条应该禁止操作
 * 所以将音乐播放进度条独立出来，方便协作和扩展
 */
void MusicSlider::initStyle()
{
    this->setFixedHeight(22);
    this->setOrientation(Qt::Horizontal);
//    this->setMinimum(0);
//    this->setMaximum(1000);

    changeStyleColor();
}

void MusicSlider::changeStyleColor()
{
    QColor highlight = QApplication::palette().highlight().color();
    QColor shadow = QApplication::palette().midlight().color();
    QString highlightStr = QString("#%1%2%3").arg(highlight.red(),2,16,QChar('0')).arg(highlight.green(),2,16,QChar('0')).arg(highlight.blue(),2,16,QChar('0'));
    QString shadowStr = QString("#%1%2%3").arg(shadow.red(),2,16,QChar('0')).arg(shadow.green(),2,16,QChar('0')).arg(shadow.blue(),2,16,QChar('0'));

//    qDebug() << "highlightStr: " << highlightStr
//             << "shadowStr: " << shadowStr;

    QString style;
    if(g_user_signal->isRTLLang){
        if (WidgetStyle::themeColor == 1) {
            style = QString("QSlider::handle:horizontal{height:6px;background-color:transparent;}"
                            "QSlider::groove:horizontal{left:0px;right:0px;height:2px;background-color:%2;}"
                            "QSlider::add-page:horizontal{background:%2;background-color:#333333;}" //设置滑块未滑过部分
                            "QSlider::sub-page:horizontal{background:%1;}").arg(shadowStr).arg(highlightStr); // //设置滑块已滑过部分
        } else {
            style = QString("QSlider::handle:horizontal{height:6px;background-color:transparent;}"
                            "QSlider::groove:horizontal{left:0px;right:0px;height:2px;background-color:%2;}"
                            "QSlider::add-page:horizontal{background:%2;}" //设置滑块未滑过部分
                            "QSlider::sub-page:horizontal{background:%1;}").arg(shadowStr).arg(highlightStr);
        }
    }else{
        if (WidgetStyle::themeColor == 1) {
            style = QString("QSlider::handle:horizontal{height:6px;background-color:transparent;}"
                            "QSlider::groove:horizontal{left:0px;right:0px;height:2px;background-color:%2;}"
                            "QSlider::add-page:horizontal{background:%2;background-color:#333333;}" //设置滑块未滑过部分
                            "QSlider::sub-page:horizontal{background:%1;}").arg(highlightStr).arg(shadowStr); // //设置滑块已滑过部分
        } else {
            style = QString("QSlider::handle:horizontal{height:6px;background-color:transparent;}"
                            "QSlider::groove:horizontal{left:0px;right:0px;height:2px;background-color:%2;}"
                            "QSlider::add-page:horizontal{background:%2;}" //设置滑块未滑过部分
                            "QSlider::sub-page:horizontal{background:%1;}").arg(highlightStr).arg(shadowStr);
        }
    }

    this->setStyleSheet(style);
}

void MusicSlider::setPosition(int pos)
{
    if (m_canSetPosition) {
        setValue(pos);
    }
}

void MusicSlider::isPlaying(bool isPlaying)
{
    m_isPlaying = isPlaying;
}
#ifndef WHITE_TEST
// 辅助函数：根据语言计算鼠标位置对应的进度值
int MusicSlider::calcValueFromMouseX(int mouseX)
{
    int min = this->minimum();
    int max = this->maximum();
    int width = this->width();
    if (width <= 0) return min;
    double x = g_user_signal->isRTLLang ? (width - mouseX) : mouseX;
    double ratio = x / width;
    int value = min + qRound((max - min) * ratio);

    // 确保值在合法范围内
    return qBound(min, value, max);
}

void MusicSlider::mousePressEvent(QMouseEvent *event)
{
    if(!m_isPlaying) {
        return QSlider::mousePressEvent(event);
    }

    this->blockSignals(true);

    if (event->button() == Qt::LeftButton) {
        int value = calcValueFromMouseX(event->x());
        setPosition(value);
    }
}

void MusicSlider::mouseMoveEvent(QMouseEvent *event)
{
    if(!m_isPlaying) {
        return QSlider::mousePressEvent(event);
    }

    auto rang = this->width();
    if(rang == 0) {
        return;
    }

    int value = calcValueFromMouseX(event->x());
    setPosition(value);
}

void MusicSlider::mouseReleaseEvent(QMouseEvent *event)
{
    if(!m_isPlaying) {
        return QSlider::mousePressEvent(event);
    }

    this->blockSignals(false);
    QSlider::mouseReleaseEvent(event);

    int position = calcValueFromMouseX(event->x());
    m_canSetPosition = false;
    // 当点击进度条实现跳转，由于我们在PressEvent的时候设置了进度条位置，但是mpv此时还有可能发送一些更新pos的消息（发送给mpv的命令是异步的，可能有延时）
    // 可能导致进度条回跳到之前播放的位置，（当mpv修改了新的播放位置后），再跳回鼠标的位置。通过延时100ms，来规避mpv的遗留影响
    QTimer::singleShot(100, [this]() {
        m_canSetPosition = true;
    });

    playController::getInstance().getMpvCorePointer()->setPosition(position);

}

void MusicSlider::enterEvent(QEvent *event)
{
    QColor highlight = QApplication::palette().highlight().color();
    QColor shadow = QApplication::palette().midlight().color();
    QString highlightStr = QString("#%1%2%3").arg(highlight.red(),2,16,QChar('0')).arg(highlight.green(),2,16,QChar('0')).arg(highlight.blue(),2,16,QChar('0'));
    QString shadowStr = QString("#%1%2%3").arg(shadow.red(),2,16,QChar('0')).arg(shadow.green(),2,16,QChar('0')).arg(shadow.blue(),2,16,QChar('0'));
    QString style;
    if(g_user_signal->isRTLLang){
        if(WidgetStyle::themeColor == 1) {
            style = QString("QSlider::handle:horizontal{"
                    "width:12px; height:12px; /* 宽高相等 */"
                    "margin: -5px 0px; /* 仅上下居中，移除左右边距 */"
                    "border-radius:6px; /* 宽高的一半，完美正圆 */"
                    "background-color:%1; /* 主题强调色填充 */"
                    "border:none; /* 移除边框避免变形，如需边框用1px solid %2 */"
                    "padding:0px; /* 清空内边距 */"
                    "}"
                    "QSlider::groove:horizontal{left:0px;right:0px;height:2px;background-color:transparent;}"
                    "QSlider::add-page:horizontal{background:%2;}"
                    "QSlider::sub-page:horizontal{background:%1;}").arg(shadowStr).arg(highlightStr);
        } else if (WidgetStyle::themeColor == 0){
            style = QString("QSlider::handle:horizontal{"
                    "width:12px; height:12px;"
                    "margin: -5px 0px;"
                    "border-radius:6px;"
                    "background-color:%1;"
                    "border:none;"
                    "padding:0px;"
                    "}"
                    "QSlider::groove:horizontal{left:0px;right:0px;height:2px;background-color:transparent;}"
                    "QSlider::add-page:horizontal{background:%2;}"
                    "QSlider::sub-page:horizontal{background:%1;}").arg(shadowStr).arg(highlightStr);
        }
    }else{
        if(WidgetStyle::themeColor == 1) {
            style = QString("QSlider::handle:horizontal{"
                    "width:12px; height:12px;"
                    "margin: -5px 0px;"
                    "border-radius:6px;"
                    "background-color:%1;"
                    "border:none;"
                    "padding:0px;"
                    "}"
                    "QSlider::groove:horizontal{left:0px;right:0px;height:2px;background-color:transparent;}"
                    "QSlider::add-page:horizontal{background:%2;}"
                    "QSlider::sub-page:horizontal{background:%1;}").arg(highlightStr).arg(shadowStr);

        } else if (WidgetStyle::themeColor == 0){
            style = QString("QSlider::handle:horizontal{"
                    "width:12px; height:12px;"
                    "margin: -5px 0px;"
                    "border-radius:6px;"
                    "background-color:%1;"
                    "border:none;"
                    "padding:0px;"
                    "}"
                    "QSlider::groove:horizontal{left:0px;right:0px;height:2px;background-color:transparent;}"
                    "QSlider::add-page:horizontal{background:%2;}"
                    "QSlider::sub-page:horizontal{background:%1;}").arg(highlightStr).arg(shadowStr);

        }
    }

    this->setStyleSheet(style);

//    QSlider::enterEvent(event);
}

void MusicSlider::leaveEvent(QEvent *event)
{
    initStyle();
    QSlider::leaveEvent(event);
}

void MusicSlider::wheelEvent(QWheelEvent *e)
{
    QWidget::wheelEvent(e);
}
#endif