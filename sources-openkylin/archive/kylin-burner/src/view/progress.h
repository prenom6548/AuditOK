/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/


#ifndef PROGRESS_H
#define PROGRESS_H

#include <QWidget>
#include <QIcon>
#include <QLabel>
#include <QEvent>
#include <QToolTip>
#include <QHelpEvent>
#include <QDebug>
#include <QPainter>

namespace Ui {
class Progress;
}

class MyTooltipLabel : public QLabel{
    Q_OBJECT
public:
    MyTooltipLabel(QWidget* parent = nullptr) : QLabel(parent){
    }
#ifndef UNIT_TEST
    bool event(QEvent *e) override {
        if(e->type() == QEvent::Enter){
            QFontMetrics metrics(font());
            QString fullText = text();
            // 如果文本宽度超过控件宽度，显示tooltip
            if (metrics.width(fullText) > width()) {
                // 计算控件左下角在屏幕坐标系中的位置
                QPoint bottomLeft = mapToGlobal(QPoint(0, height()));
                // 在左下角显示tooltip
                QToolTip::showText(bottomLeft, fullText, this, QRect(), toolTipDuration());
            }
        }else if(e->type() == QEvent::Leave){
            // 鼠标离开时隐藏tooltip
            QToolTip::hideText();
        }
        return QLabel::event(e);
    }
    
protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::TextAntialiasing); // 抗锯齿

        QFontMetrics fm = painter.fontMetrics();
        int lineHeight = fm.height();       // 行高（包含行距）
        int contentWidth = width() - 4;     // 内容宽度（减去左右边距）
        int maxHeight = height() - 4;       // 最大可显示高度（减去上下边距）
        if (contentWidth <= 0 || maxHeight <= 0) return; // 避免无效尺寸

        QString text = this->text();
        if (text.isEmpty())
            return;

        // 将文本按行分割（考虑自动换行）
        QStringList lines;
        QString currentLine;
        int currentX = 0;

        for (int i = 0; i < text.size(); ++i) {
            QChar c = text[i];
            int charWidth = fm.width(c);
            if (currentX + charWidth > contentWidth && !currentLine.isEmpty()) {
                lines.append(currentLine);
                currentLine = QString(c);
                currentX = charWidth;
            } else {
                currentLine += c;
                currentX += charWidth;
            }
        }
        if (!currentLine.isEmpty()) {
            lines.append(currentLine);
        }

        int maxLines = maxHeight / lineHeight; // 最大可显示行数
        if (maxLines <= 0) return;

        // 如果行数超出最大显示行数，截断并添加省略号
        bool needEllipsis = lines.size() > maxLines;
        if (needEllipsis) {
            lines = lines.mid(0, maxLines);
            QString lastLine = lines.last();
            QString ellipsis = "...";

            // 确保最后一行加上省略号后不超出宽度
            while (!lastLine.isEmpty() && fm.width(lastLine + ellipsis) > contentWidth) {
                lastLine.chop(1);
            }
            lines[lines.size() - 1] = lastLine + ellipsis;
        }

        // 垂直居中计算
        int totalLines = lines.size();
        int totalTextHeight = totalLines * lineHeight; // 文本总高度

        // 计算垂直居中位置：使用控件完整高度，确保文本垂直居中
        int availableHeight = height();
        int startY = (availableHeight - totalTextHeight) / 2 + fm.ascent();

        // 逐行绘制，左对齐显示
        for (int i = 0; i < lines.size(); ++i) {
            QString line = lines[i];
            int startX = 2; // 左对齐起始位置（左边距）
            int y = startY + i * lineHeight;
            painter.drawText(startX, y, line);
        }
    }   
#endif
};

class Progress : public QWidget
{
    Q_OBJECT
Q_SIGNALS:
    void setRange(int, int);
    void setValue(int, QIcon, QString);
    void addValue(int, QIcon, QString);
    void title(QString);
public:
    explicit Progress(QWidget *parent = nullptr);
    ~Progress();

public slots:
     void slotChangeUIFontSize(int fontSize);
     void resetProgressBar();

public slots:
    void setsSetRange(int, int);
    void slotsSetValue(int, QIcon icon = QIcon(), QString str = QString());
    void slotsAddValue(int, QIcon icon = QIcon(), QString str = QString());
    void slotsSetValueWithoutClosing(int, QString str = QString());

private:
    Ui::Progress *ui;
    int m_RangeBegin;
    int m_RangeEnd;
    int m_currnet;
};

#endif // PROGRESS_H
