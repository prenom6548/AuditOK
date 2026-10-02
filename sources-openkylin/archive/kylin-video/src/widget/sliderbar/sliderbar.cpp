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
#include "sliderbar.h"

#include <QTimer>
#include <QDebug>
#include <QDateTime>

#include "progressbar.h"
#include "slider.h"

SliderBar::SliderBar(QWidget *parent) : QWidget(parent)
{
    initUI();
    m_canSetValue = true;
    m_ignoreMpvTimer = new QTimer(this);

    // 当鼠标松开后，才去接收mpv发送的时间戳。为了避免收到旧的时间戳，延迟一段时间开启。
    // 如果在延迟的时间内，触发了按压事件，会导致m_camSetValue失效（先false，再true，再true，导致失效），所以按压的时候需要取消之前的定时任务
    connect(m_ignoreMpvTimer, &QTimer::timeout, [this](){
        m_canSetValue = true;
        m_ignoreMpvTimer->stop();
    });
    m_lastTime = QDateTime::currentMSecsSinceEpoch();

}

SliderBar::~SliderBar()
{
    if (m_slider) {
        delete m_slider;
        m_slider = nullptr;
    }

    if (m_progressBar) {
        delete m_progressBar;
        m_progressBar = nullptr;
    }
}

void SliderBar::addMark(int value, QString desc)
{
    m_progressBar->addMark(value, desc);
}

void SliderBar::deleteMark(int value)
{
    m_progressBar->deleteMark(value);
}

void SliderBar::clearMark()
{
    m_progressBar->clearMark();
}

void SliderBar::setTimeTip(QString tip)
{
    m_progressBar->setToolTip(tip);
}

void SliderBar::setBlackTheme()
{
    m_progressBar->setBlackTheme();
}

void SliderBar::setLightTheme()
{
    m_progressBar->setLightTheme();
}

void SliderBar::setValue(double val)
{
    if (!m_canSetValue) {
        return;
    }

       m_value = val;

    uint64_t current = QDateTime::currentMSecsSinceEpoch();
    if (current - m_lastTime > 1000){
        m_progressBar->setValue(val);

        m_slider->move(DEFAULT_LR_MARGIN + m_progressBar->getValueBarWidth() -
                       (SLIDER_SIDE_SIZE / 2), (height() - SLIDER_SIDE_SIZE) / 2);
        m_lastTime = current;
    }
}

void SliderBar::setRange(qint64 min, qint64 max)
{
    m_progressBar->setRange(min, max);
}

void SliderBar::initUI()
{
    setAttribute(Qt::WA_TranslucentBackground, true);

    m_progressBar = new ProgressBar(this);
    m_progressBar->setAccessibleName("kylin-video_sliderbar_ProgressBar_m_progressBar");
    connect(m_progressBar, &ProgressBar::valueChange, [this](int value) {
        emit valueChange(value);
    });
    connect(m_progressBar, &ProgressBar::sliderMoved, [this](double value) {
        emit sliderMoved(value);
    });

    // 当鼠标点击时（进度条或者slider被点击时，都会触发），不允许mpv的值更新进度条
    connect(m_progressBar, &ProgressBar::mousePress, [this]() {
        qInfo() << "mouse press";
        m_canSetValue = false;
        if (m_ignoreMpvTimer->isActive()) {
            m_ignoreMpvTimer->stop();
        }
    });

    // 当鼠标释放时（进度条或者slider被释放时，都会触发），允许mpv的值更新进度条,但延迟一下
    connect(m_progressBar, &ProgressBar::mouseRelease, [this]() {
        // 鼠标释放后，跳转的时间位置才会发送给mpv，在此期间，mpv依然会将播放时刻返回（旧的时刻），由于滑块位置已经改变，就会导致跳变
        qInfo() << "mouse Release";
        m_ignoreMpvTimer->start(500);
    });
    connect(m_progressBar, &ProgressBar::mousePosChange, [this](double value) {
        emit mousePosChange(value);
    });
    connect(m_progressBar, &ProgressBar::mouseLeave, [this]() {
        emit mouseLeave();
    });

    m_slider = new Slider(this);
    m_slider->setAccessibleName("kylin-video_sliderbar_Slider_m_slider");
    m_slider->setHide();
    m_slider->resize(12, 12);

    m_progressBar->setSlider(m_slider);
}

void SliderBar::resizeEvent(QResizeEvent *e)
{
    m_progressBar->setGeometry(DEFAULT_LR_MARGIN, (height()-DEFAULT_HEIGHT) / 2,
                               width() - (2 * DEFAULT_LR_MARGIN), DEFAULT_HEIGHT);
    // 设置滑块 x 轴滑动范围
    m_slider->setXRange(DEFAULT_LR_MARGIN - (SLIDER_SIDE_SIZE / 2), width() - DEFAULT_LR_MARGIN - (SLIDER_SIDE_SIZE / 2));
    m_progressBar->updateSliderPos();
}

void SliderBar::enterEvent(QEvent *e)
{
    m_progressBar->setGeometry(DEFAULT_LR_MARGIN, (height()-DEFAULT_HEIGHT - 4) / 2,
                               width() - (2 * DEFAULT_LR_MARGIN), DEFAULT_HEIGHT + 4);
}

void SliderBar::leaveEvent(QEvent *e)
{
#ifndef WHITE_TEST
    QTimer::singleShot(200, [this](){m_progressBar->setGeometry(DEFAULT_LR_MARGIN, (height()-DEFAULT_HEIGHT) / 2,
                                                                width() - (2 * DEFAULT_LR_MARGIN), DEFAULT_HEIGHT);});
#endif
}
