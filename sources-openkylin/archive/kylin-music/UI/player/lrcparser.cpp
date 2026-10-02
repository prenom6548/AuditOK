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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "lrcparser.h"
#include <QFile>
#include <QStringList>
#include <QRegularExpression>
#include <QTextStream>
#include <QDebug>
#include <QTextCodec>

QList<LrcLine> LrcParser::parseLrc(const QString &filePath) {
    QList<LrcLine> lines;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件:" << filePath;
        return lines;
    }

    // 使用QTextCodec处理UTF-8编码
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QByteArray fileContent = file.readAll();
    file.close();
    
    QString content = codec->toUnicode(fileContent);
    QStringList contentLines = content.split('\n');
    
    for (const QString &line : contentLines) {
        QString trimmedLine = line.trimmed();
        if (trimmedLine.isEmpty()) continue;
        
        // 修正正则表达式，正确转义点号
        QRegularExpression re("\\[(\\d{2}:\\d{2}\\.\\d{2})\\](.*)");
        QRegularExpressionMatch match = re.match(trimmedLine);
        if (match.hasMatch()) {
            QString timeStr = match.captured(1);
            int timeMs = timeStrToMs(timeStr);
            QString text = match.captured(2).trimmed();  // 去除前后空格
            lines.append({timeMs, text});
        }
    }
    return lines;
}

int LrcParser::timeStrToMs(const QString &timeStr) {
    QRegularExpression re("(\\d{2}):(\\d{2})\\.(\\d{2})");
    QRegularExpressionMatch match = re.match(timeStr);
    if (match.hasMatch()) {
        int minutes = match.captured(1).toInt();
        int seconds = match.captured(2).toInt();
        int milliseconds = match.captured(3).toInt() * 100;
        return (minutes * 60 + seconds) * 1000 + milliseconds;
    }
    return -1;
}
