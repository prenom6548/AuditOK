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
#ifndef SLIDERBAR_H
#define SLIDERBAR_H

#include <QWidget>

#define DEFAULT_LR_MARGIN   8
#define MIN_HEIGHT          20

QT_FORWARD_DECLARE_CLASS(ProgressBar)
QT_FORWARD_DECLARE_CLASS(Slider)

class SliderBar : public QWidget
{
    Q_OBJECT
    
    friend class unitConstruct;  // 可能存在这样的声明
public:
    explicit SliderBar(QWidget *parent = nullptr);
    ~SliderBar();

    void addMark(int value, QString desc);
    void deleteMark(int value);
    void clearMark();
    void setTimeTip(QString tip);

    void setBlackTheme();
    void setLightTheme();


public slots:
    void setValue(double val);
    void setRange(qint64 min, qint64 max);

signals:
    void valueChange(int);
    void sliderMoved(double);
    void mousePosChange(qint64);
    void mouseLeave();

private:
    void initUI();

private:
    ProgressBar *m_progressBar;
    Slider *m_slider;
    int m_value;
    bool m_canSetValue;
    QTimer* m_ignoreMpvTimer { nullptr };

    qint64 m_lastTime;
    int m_count {0};


protected:
    void resizeEvent(QResizeEvent *e) override;
    void enterEvent(QEvent *e) override;
    void leaveEvent(QEvent *e) override;
};

#endif // SLIDERBAR_H
