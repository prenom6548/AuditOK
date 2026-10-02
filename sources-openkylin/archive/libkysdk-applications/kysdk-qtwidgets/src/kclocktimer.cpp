/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "kclocktimer.h"
#include "themeController.h"
#include <QBoxLayout>
#include <QDebug>
#include <QLabel>
#include <QStringList>
#include <QTimer>

namespace kdk
{

class Q_DECL_HIDDEN KClockTimerPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KClockTimer)
public:
    KClockTimerPrivate(KClockTimer *parent);
    void updateTime();

private:
    KClockTimer *q_ptr;
    QLabel *m_pMainLabel;
    QLabel *m_pSubLabel;
    QVBoxLayout *m_vLayout;

    QTimer *m_pTimer;
    qint64 m_runningTime;
    bool isRunning;
};

KClockTimer::KClockTimer(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KClockTimerPrivate(this))
{
    Q_D(KClockTimer);
}

QString KClockTimer::time()
{
    Q_D(KClockTimer);
    return d->m_pMainLabel->text();
}

void KClockTimer::startStopwatch()
{
    Q_D(KClockTimer);
    if (!d->isRunning) {
        d->m_pTimer->start();
        d->isRunning = true;
    }
}

void KClockTimer::stopStopwatch()
{
    Q_D(KClockTimer);
    if (d->isRunning) {
        d->m_pTimer->stop();
        d->isRunning = false;
    }
}

void KClockTimer::resetStopwatch()
{
    Q_D(KClockTimer);
    if (d->isRunning) {
        d->m_pTimer->stop();
        d->isRunning = false;
    }
    d->m_runningTime = 0;
    d->m_pMainLabel->setText("00:00.00");
    d->m_pSubLabel->setText("00:00.00");
}

KClockTimerPrivate::KClockTimerPrivate(KClockTimer *parent)
    : q_ptr(parent)
    , isRunning(false)
{
    Q_Q(KClockTimer);
    setParent(parent);
    m_vLayout = new QVBoxLayout(q);

    m_pMainLabel = new QLabel();
    m_pMainLabel->setFixedSize(210, 70);
    m_pMainLabel->setAlignment(Qt::AlignHCenter);
    m_pMainLabel->setText("00:00.00");

    m_pSubLabel = new QLabel();
    m_pSubLabel->setFixedSize(210, 41);
    m_pSubLabel->setAlignment(Qt::AlignHCenter);
    m_pSubLabel->setText("00:00.00");

    QPalette pale;
    pale.setColor(QPalette::WindowText, Qt::gray);
    m_pSubLabel->setPalette(pale);
    m_vLayout->setSpacing(0);
    m_vLayout->addWidget(m_pMainLabel);
    m_vLayout->addWidget(m_pSubLabel);
    m_runningTime = 0;

    m_pTimer = new QTimer();
    m_pTimer->setInterval(10);

    QPalette mainPalette;
    mainPalette.setColor(QPalette::WindowText, ThemeController::getPaletteColorFromDT("brighttext-active"));
    m_pMainLabel->setPalette(mainPalette);

    QFont f = q->font();
    f.setPointSize(f.pointSize() + 15);
    m_pMainLabel->setFont(f);
    connect(m_gsetting, &QGSettings::changed, q, [=](QString key){
        if(key == "styleName" || key == "iconThemeName" || key == "widgetThemeName")
        {
            QPalette mainPalette;
            mainPalette.setColor(QPalette::WindowText, ThemeController::getPaletteColorFromDT("brighttext-active"));
            m_pMainLabel->setPalette(mainPalette);
        }
        if(key == "systemFontSize" || key == "systemFont")
        {
            QFont f = q->font();
            f.setPointSize(f.pointSize() + 15);
            m_pMainLabel->setFont(f);
        }

    });
    connect(m_pTimer, &QTimer::timeout, this, [=] {
        updateTime();
    });
}

void KClockTimerPrivate::updateTime()
{
    Q_Q(KClockTimer);
    m_runningTime += 10; // 增加10毫秒

    int minutes = m_runningTime / (1000 * 60);
    int seconds = (m_runningTime / 1000) % 60;
    int milliseconds = m_runningTime % 1000;

    // 取毫秒的前两位
    milliseconds = milliseconds / 10;

    // 使用QString的arg方法格式化输出
    m_pMainLabel->setText(QString("%1:%2.%3")
                              .arg(minutes, 2, 10, QChar('0'))
                              .arg(seconds, 2, 10, QChar('0'))
                              .arg(milliseconds, 2, 10, QChar('0')));
    m_pSubLabel->setText(QString("%1:%2.%3")
                             .arg(minutes, 2, 10, QChar('0'))
                             .arg(seconds, 2, 10, QChar('0'))
                             .arg(milliseconds, 2, 10, QChar('0')));
}

}

#include "kclocktimer.moc"
#include "moc_kclocktimer.cpp"
