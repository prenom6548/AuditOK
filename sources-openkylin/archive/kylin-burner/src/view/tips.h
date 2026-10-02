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


#ifndef TIPS_H
#define TIPS_H

#include <QWidget>
#include <QLabel>
#include <QPainter>

namespace Ui {
class tips;
}

class CustomLabel : public QLabel {
    Q_OBJECT
public:
    explicit CustomLabel(QWidget *parent = nullptr) : QLabel(parent) {}
#ifndef UNIT_TEST
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

class tips : public QWidget
{
    Q_OBJECT
    friend class MainWindowTest;
Q_SIGNALS:
    void success(QString);
    void fail(QString);
    void checked(bool);
public:
    explicit tips(QWidget *parent = nullptr);
    ~tips();
public slots:
    void information(QString, bool next = false);
    void error(QString, bool next = false);
    void question(QString, bool next = false);
    void slotChangeUIFontSize(int fontSize);

private:
    Ui::tips *ui;
};

#endif // TIPS_H
