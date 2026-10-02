/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#ifndef UKMEDIACUSTOMCLASS_H
#define UKMEDIACUSTOMCLASS_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QCoreApplication>
#include <QSlider>
#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QProxyStyle>
#include <QPainter>
#include <QDebug>
#include <QTabWidget>
#include <QFrame>
#include "kslider.h"

#define MIDDLE_COLOR 178
#define BACKGROUND_COLOR QColor(0,0,0,0)

using namespace kdk;

typedef struct UkuiThemeIcon
{
    QImage image;
    QColor color;
}UkuiThemeIcon;

enum PushButtonState{
    PUSH_BUTTON_NORMAL,
    PUSH_BUTTON_CLICK,
    PUSH_BUTTON_PRESS
};
static QColor symbolic_color = Qt::gray;

class UkuiButtonDrawSvg:public QPushButton
{
    Q_OBJECT
public:
    UkuiButtonDrawSvg(QWidget *parent = nullptr);
    ~UkuiButtonDrawSvg();
    QPixmap filledSymbolicColoredPixmap(QImage &source, QColor &baseColor);
    QRect IconGeometry();
    void draw(QPaintEvent* e);
    void init(QImage image ,QColor color);
    friend class DeviceSwitchWidget;
    friend class UkmediaMainWidget;
    friend class UkmediaOsdDisplayWidget;
    friend class UkmediaSystemVolumeWidget;

protected:
    void paintEvent(QPaintEvent *event);
    bool event(QEvent *e);
private:
    int buttonState = PUSH_BUTTON_NORMAL;
    UkuiThemeIcon themeIcon;
};

class UkuiApplicationWidget:public QWidget
{
    Q_OBJECT
public:
    UkuiApplicationWidget(QWidget *parent = nullptr);
    ~UkuiApplicationWidget();
protected:
    void paintEvent(QPaintEvent*);
};

class UkuiSettingButton:public QLabel
{
    Q_OBJECT
public:
    UkuiSettingButton(QWidget *parent = nullptr);
    ~UkuiSettingButton();
    friend class UkmediaMiniMasterVolumeWidget;
    friend class UkmediaMainWidget;
    friend class UkmediaSystemVolumeWidget;
    friend class ApplicationVolumeWidget;

protected:

    void paintEvent(QPaintEvent *event);
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:

    QColor m_foregroundColor;

    void setPressColor();
    void setHoverColor();
    void setNormalColor();

Q_SIGNALS:
    void clicked(void);

private Q_SLOTS:
    void onPaletteChanged();
};

class AudioSlider : public KSlider
{
    Q_OBJECT
public:
    AudioSlider(QWidget *parent = nullptr);
    ~AudioSlider();
    bool isMouseWheel = false;

private:
    int blueValue = 0;

Q_SIGNALS:
    void blueValueChanged(int value);   //针对蓝牙a2dp模式下滑动条跳动，以10为间隔设置音量

protected:
    void wheelEvent(QWheelEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
};

//文本长自动省略并添加悬浮
class FixLabel : public QLabel
{

    Q_OBJECT
public:
    explicit FixLabel(QWidget *parent = nullptr);
    explicit FixLabel(QString text , QWidget *parent = nullptr);
    ~FixLabel();
    void setText(const QString &text, bool saveTextFlag = true);
    QString fullText;
private:
    void paintEvent(QPaintEvent *event);

private:
};

class Divider : public QFrame
{
public:
    Divider(QWidget * parent = nullptr);
    ~Divider() = default;

protected:
    void paintEvent(QPaintEvent *event);
};

#endif
