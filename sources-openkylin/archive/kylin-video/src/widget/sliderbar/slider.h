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
#ifndef SLIDER_H
#define SLIDER_H

#include <QWidget>

class FixedSlider : public QWidget
{
    Q_OBJECT
    
    friend class unitConstruct;  // 可能存在这样的声明
public:
    explicit FixedSlider(QWidget *parent = nullptr);
    ~FixedSlider();

    void setDescribe(QString desc) {m_describe = desc;setToolTip(m_describe);}
    int value;

signals:
    void clicked();

private:
    QString m_describe;
    QWidget *m_widgetSlider;

protected:
    void mouseReleaseEvent(QMouseEvent *e) override;
};

class Slider : public QWidget
{
    Q_OBJECT
    friend class unitConstruct;
public:
    explicit Slider(QWidget *parent = nullptr);
    ~Slider();

    void setHide();
    void setXRange(int min, int max) {m_min_x = min; m_max_x = max;}

public slots:

signals:
    void lockProgress(bool);
    void posXUpdate(double);
    void posXUpdateWithoutSeek(double);

private:
    void updatePos(bool needSeek = false);

    QWidget *m_widgetSlider;
    bool m_canHide;
    int m_min_x,
        m_max_x,
        m_mouse_x;

    bool m_canChange {false};

private:
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;
    void resizeEvent(QResizeEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
};

#endif // SLIDER_H
