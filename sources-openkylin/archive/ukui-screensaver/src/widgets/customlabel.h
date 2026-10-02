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
#ifndef CUSTOMKLABEL_H
#define CUSTOMKLABEL_H

#include <QWidget>
#include <QLabel>
#include <QTimer>

class CustomLabel : public QLabel
{
    Q_OBJECT
public:
    CustomLabel(QWidget *parent = nullptr);
    void setFontSize(int fontSize);
    QString getElidedText(QFont font, int width, QString strInfo);
    static QWidget *m_parentWidget;
    void setTipText(const QString &);
    void clear();
    QString text() const;
    void setFontFamily(QString &family);
    void setFontWeight(int weight);
    void setMarqueueMode(bool isMarqueue);

public Q_SLOTS:
    void setText(const QString &);

protected:
    void paintEvent(QPaintEvent *event);

private:
    void startMarqueueTimer();
    void stopMarqueueTimer();

private Q_SLOTS:
    void onMarqueueTimer();

private:
    QFont sysFont;
    double m_ptToPx = 1.0;

    QString m_strText;

    bool m_isMarqueeMode = false;
    int m_speed; // 滚动速度，可以根据需要调整
    int m_scrollOffset; // 文本滚动的偏移量
    QTimer *m_timerMarqueue = nullptr;
};

#endif // CUSTOMKLABEL_H
