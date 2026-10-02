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

#ifndef LYRICS_DIALOG_H
#define LYRICS_DIALOG_H

#include <QDialog>
#include <QTextEdit>
#include <QTimer>
#include <QList>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include "lrcparser.h"
#include "UIControl/player/coreplayer/mpvcore.h"
#include "UIControl/player/player.h"

class LrcParser;

struct TimestampedLyric {
    int minutes;
    int seconds;
    int milliseconds;
    QString lyric;
};

class LyricsDialog : public QDialog {
    Q_OBJECT
public:
    explicit LyricsDialog(QWidget *parent = nullptr);
    ~LyricsDialog();

    void onPlayButtonClicked(qint64 m_nowtime);
    void onStopButtonClicked();
    void loadLyricsForSongDlg(const QString &songFilePath);
    void dlginitstyle();

Q_SIGNALS:
    void currentTimeChanged(int currentTimeMs);
    void dlgCLoseSig();
    void textChanged(QString text);

public Q_SLOTS:
    void closeDialog();
    void onLyricsTextChanged(QString text);

private:
    QList<LrcLine> m_lines;
    QTextEdit *m_textEdit;
    QTimer *m_timer;
    int m_currentTimeMs;
    int m_currentIndex;
    LrcParser m_parser;
    QPushButton *m_closeButton = nullptr;
    QLabel *m_lyricsLabel = nullptr;
    QList<TimestampedLyric> m_timestampedLyrics;
    QString lyrics = nullptr;
    MpvCore *m_player; // 播放器对象
    bool m_hasTimestamps = false;

    QList<TimestampedLyric> parseLrc(const QString& lyrics) const;
    void scrollLyrics(const QList<TimestampedLyric>& timestampedLyrics, int currentTime);
    void setCurrentTime(int currentTimeMs);

};

#endif // LYRICS_DIALOG_H
