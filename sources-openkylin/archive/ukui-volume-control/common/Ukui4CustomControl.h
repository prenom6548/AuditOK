/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef UKUI4CUSTOMCONTROL_H
#define UKUI4CUSTOMCONTROL_H

#include <QSlider>
#include <QLabel>
#include <kslider.h>

//文本长自动省略并添加悬浮
class AutoOmitLabel : public QLabel
{
    Q_OBJECT
public:
    explicit AutoOmitLabel(QWidget* = nullptr);
    explicit AutoOmitLabel(const QString&, QWidget* = nullptr);
    ~AutoOmitLabel() = default;

public:
    void setText(const QString&, bool = true);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QString m_fullText;
};

class UkmediaVolumeSlider : public kdk::KSlider
{
    Q_OBJECT
public:
    UkmediaVolumeSlider(QWidget* = nullptr);
    ~UkmediaVolumeSlider();

Q_SIGNALS:
    void silderPressSignal();

    void silderReleaseSignal();

protected:
    void mousePressEvent(QMouseEvent*) override;

    void mouseReleaseEvent(QMouseEvent*) override;

    void mouseMoveEvent(QMouseEvent*) override;

    void wheelEvent(QWheelEvent*) override;

    void keyReleaseEvent(QKeyEvent*) override;

    void enterEvent(QEvent*) override;

    void leaveEvent(QEvent*) override;

private:
    void initStyleOption(QStyleOptionSlider*);

    void updateValue(QMouseEvent*);

private:
    bool m_bMousePress = false;
};

class UkBalanceVolumeSlider : public kdk::KSlider
{
    Q_OBJECT
public:
    UkBalanceVolumeSlider(QWidget* = nullptr);
    ~UkBalanceVolumeSlider();

protected:
    virtual bool eventFilter(QObject*, QEvent*) override;

private:
    void initStyleOption(QStyleOptionSlider*);

    void showTooltip();

};

#endif // UKUI4CUSTOMCONTROL_H
