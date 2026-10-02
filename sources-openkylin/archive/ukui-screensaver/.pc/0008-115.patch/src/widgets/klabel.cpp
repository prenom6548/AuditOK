/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "klabel.h"
#include <QFont>
#include <QApplication>
#include <QToolTip>
#include <QDesktopWidget>
#include <QPainter>
#include <QDebug>

KLabel::KLabel(QWidget *parent) : QLabel(parent), m_speed(2), m_scrollOffset(0)
{
    setWindowFlags(Qt::FramelessWindowHint);
    // m_strText = "";
    sysFont = qApp->font();

    QPalette pe;
    pe.setColor(QPalette::WindowText, Qt::white);
    this->setPalette(pe);
}

void KLabel::setFontSize(int fontSize)
{
    sysFont.setPointSize(fontSize);
    this->setFont(sysFont);
}

void KLabel::setFontFamily(QString &family)
{
    sysFont.setFamily(family);
    this->setFont(sysFont);
}

void KLabel::setFontWeight(int weight)
{
    sysFont.setWeight(weight);
    this->setFont(sysFont);
}

void KLabel::setTipText(const QString &strText)
{
    m_scrollOffset = 0;
    m_strText = strText;
    QLabel::setText(strText);
}

void KLabel::setText(const QString &strText)
{
    setTipText(strText);
}

void KLabel::clear()
{
    m_strText.clear();
    QLabel::clear();
}

QString KLabel::text() const
{
    if (m_strText.isEmpty())
        return QLabel::text();
    else
        return m_strText;
}

QString KLabel::getElidedText(QFont font, int width, QString strInfo)
{
    QFontMetrics fontMetrics(font);
    // 如果当前字体下，字符串长度大于指定宽度
    if (fontMetrics.width(strInfo) > width) {
        strInfo = QFontMetrics(font).elidedText(strInfo, Qt::ElideRight, width);
    }
    return strInfo;
}

void KLabel::paintEvent(QPaintEvent *event)
{
    if (m_strText.isEmpty() && !QLabel::text().isEmpty())
        m_strText = QLabel::text();
    QString strEText = getElidedText(font(), width() - contentsMargins().left() - contentsMargins().right(), m_strText);
    if (strEText == m_strText) {
        QLabel::setText(m_strText);
    } else if (strEText != m_strText) {
        if (m_isMarqueeMode) {
            QLabel::setText("");
            QPainter painter(this);
            painter.drawText(
                m_scrollOffset + contentsMargins().left(),
                rect().top(),
                width() - contentsMargins().left() - contentsMargins().right() - m_scrollOffset,
                height(),
                Qt::AlignLeft | Qt::TextSingleLine,
                m_strText);
            // qDebug()<<"MarqueueText:"<<m_scrollOffset<<rect().top()<<width() - contentsMargins().left() -
            // contentsMargins().right() - m_scrollOffset<<height()<<","<<m_strText;
            startMarqueueTimer();
        } else {
            QLabel::setText(strEText);
            QLabel::setToolTip(m_strText);
        }
    }
    QLabel::paintEvent(event);
}

void KLabel::setMarqueueMode(bool isMarqueue)
{
    m_isMarqueeMode = isMarqueue;
    if (isMarqueue && !m_strText.isEmpty()) {
        QString strEText
            = getElidedText(font(), width() - contentsMargins().left() - contentsMargins().right(), m_strText);
        if (strEText != m_strText) {
            startMarqueueTimer();
        }
    } else if (!isMarqueue) {
        stopMarqueueTimer();
    }
}

void KLabel::startMarqueueTimer()
{
    if (!m_timerMarqueue) {
        m_timerMarqueue = new QTimer(this);
        connect(m_timerMarqueue, &QTimer::timeout, this, &KLabel::onMarqueueTimer);
    }
    if (m_timerMarqueue->isActive()) {
        m_timerMarqueue->stop();
    }
    m_timerMarqueue->start(100);
}

void KLabel::stopMarqueueTimer()
{
    if (m_timerMarqueue->isActive()) {
        m_timerMarqueue->stop();
    }
}

void KLabel::onMarqueueTimer()
{
    if (!isVisible())
        return;
    int textWidth = fontMetrics().width(text());
    if (m_scrollOffset - m_speed < -textWidth) {
        m_scrollOffset = width() - contentsMargins().left() - contentsMargins().right(); // 重置位置
    } else {
        m_scrollOffset -= m_speed; // 更新位置
    }
    update(); // 更新显示
}
