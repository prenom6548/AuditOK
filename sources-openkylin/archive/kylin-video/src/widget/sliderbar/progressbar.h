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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#include <QWidget>

#include "slider.h"

#define DEFAULT_HEIGHT      4
#define SLIDER_SIDE_SIZE    12
#define SLIDER_MARK_HEIGHT  10
#define SLIDER_MARK_WIDTH   6

class ProgressBar : public QWidget
{
    Q_OBJECT
    
    friend class unitConstruct;  // 可能存在这样的声明
public:
    explicit ProgressBar(QWidget *parent = nullptr);

    void setRange(qint64 min, qint64 max);
    void setValue(double value);
    void setSlider(Slider *sl);
    void updateSliderPos();

    void addMark(int value, QString desc);
    void deleteMark(int value);
    void clearMark();

    int getValueBarWidth(){return m_valueBar->width();}

    void setBlackTheme();
    void setLightTheme();

signals:
    void valueChange(int);
    void sliderMoved(double);
    void mousePosChange(qint64);
    void mouseLeave();
    void mousePress();
    void mouseRelease();

private:
    QString m_color;
    Slider *m_slider;
    QWidget *m_valueBar,
            *m_widgetDock;
    qint64  m_minValue,
            m_maxValue;
    double  m_currentValue;
    double  m_mouse_value;
    bool m_can_change_value,
         m_mouse_move_seek_flag;
    QList<FixedSlider*> m_markList;

    double w2v(qint64 width);
    qint64 v2w(qint64 value);
    void updateMouseValue(int vb_x);

protected:
    void enterEvent(QEvent *e) override;
#ifndef WHITE_TEST
    void leaveEvent(QEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
#endif
};

#endif // PROGRESSBAR_H
