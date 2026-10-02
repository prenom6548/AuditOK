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

#ifndef LYRICSWIDGET_H
#define LYRICSWIDGET_H

#include <QDialog>
#include <QPainterPath>
#include <QRegion>
#include <xcb/xcb.h>
#include "UI/sidebar/sidebarwidget.h"
#include "UI/player/lrcparser.h"
#include "UIControl/player/coreplayer/mpvcore.h"
#include "kdialog.h"

class LrcParser;
struct m_TimestampedLyric {
    int minutes;
    int seconds;
    int milliseconds;
    QString lyric;
};

class LyricsWidget : public QWidget {
    Q_OBJECT

public:
    explicit LyricsWidget(QWidget *parent = nullptr);
    void changeLyricsPos(int posX, int posY, int width, int height);
    void PlayButtonClicked(qint64 m_nowtime);
    void StopButtonClicked();
    void setCurrentTime(int currentTimeMs);
    void widinitstyle();

private:
    void initSetModel();

    SideBarWidget *m_sideBarWid = nullptr;
    int lyricsWidPosX;
    int lyricsWidPosY;
    int lyricsWidPosWidth;
    int lyricsWidPosHeight;
    QWidget* containerWidget = nullptr;
    QTextEdit *textEdit = nullptr;
    QLabel *textlabel = nullptr;
    QList<m_TimestampedLyric> m_widtimestampedLyrics;
    QString lyrics;
    int m_currentTimeMs;
    int m_currentIndex;
    QTimer *m_timer;
    bool m_hasTimestamps = false;
    bool m_islrcfile = false;
    QString filePath;
    QLabel *imageLabel = nullptr;
    QGSettings *themeData = nullptr;
    QColor hightLightColor;
    QList<m_TimestampedLyric> parseLrc(const QString& lyrics);
    void scrollLyrics(const QList<m_TimestampedLyric>& timestampedLyrics, int currentTime);
    void setEdgeLinesGray(int linesAbove, int linesBelow, const QList<m_TimestampedLyric>& m_widtimestampedLyrics, int currentLineIndex);
    void ensureCurrentLineCentered(int currentLineIndex);
    QString rgbaToHex(const QString& rgbaStr);
Q_SIGNALS:
    void currentTimeChanged(int currentTimeMs);

public Q_SLOTS:
    void showLyricsWid();
    void toggleVisibility();
    void normalVisibility();
    void loadLyricsForSongWid(const QString &songFilePath);
    void slotSongInfo(QString path);
    QPixmap setCoverPhotoPixmapRadius(QPixmap pixmap);

protected:
    void resizeEvent(QResizeEvent *event);
};

#endif // LYRICSWIDGET_H
