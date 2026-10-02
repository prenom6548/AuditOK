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

#include "lyricsdialog.h"
#include <QFile>
#include <QStringList>
#include <QRegularExpression>
#include <QTextStream>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDebug>
#include <QScrollArea>
#include <QString>
#include <QTextCodec>
#include <ukuistylehelper/ukuistylehelper.h>
#include <ukuiwindowhelper/ukuiwindowhelper.h>
LyricsDialog::LyricsDialog(QWidget *parent) : QDialog(parent), m_closeButton(new QPushButton(this)), m_timer(nullptr), m_currentTimeMs(0)
{
    UkuiWindowHelper *windowHelper = new UkuiWindowHelper(this);
    windowHelper->removeTitleBar();
    windowHelper->setSkipTaskBar(true);
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_DeleteOnClose);

    m_closeButton->setToolTip(tr("Close"));
    m_closeButton->setIcon(QIcon(":/img/lightshow/close.svg"));
    m_closeButton->setCursor(Qt::PointingHandCursor);
    m_closeButton->setAccessibleName("kylin-music_lyricsdialog_QPushButton_m_closeButton");
    m_closeButton->setFixedSize(30, 30); // 可以调整大小

    connect(m_closeButton, &QPushButton::clicked, this, &LyricsDialog::closeDialog);

    QHBoxLayout *titleBarLayout = new QHBoxLayout;
    titleBarLayout->addStretch(); // 添加伸缩量来推远按钮
    titleBarLayout->addWidget(m_closeButton);
    titleBarLayout->setContentsMargins(0, 0, 0, 0); // 去除外边距

    m_lyricsLabel = new QLabel(tr("No lyrics"), this);
    QFont font;
    font.setPointSize(18);
    m_lyricsLabel->setFont(font);

    m_lyricsLabel->setAlignment(Qt::AlignCenter);
    m_lyricsLabel->setWordWrap(true); // 启用自动换行
    m_lyricsLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum); // 允许标签扩展
    m_lyricsLabel->setFixedSize(724,52);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(titleBarLayout);
    mainLayout->addWidget(m_lyricsLabel, 1, Qt::AlignCenter);
    mainLayout->setContentsMargins(0, 0, 0, 50);

    dlginitstyle();

    // 连接 currentTimeChanged 信号到 scrollLyrics 槽
    connect(this, &LyricsDialog::currentTimeChanged, this, [this](int currentTimeMs) {
        if(currentTimeMs != 0){
            scrollLyrics(m_timestampedLyrics, currentTimeMs);
        }
    });
    connect(this, &LyricsDialog::textChanged, this, &LyricsDialog::onLyricsTextChanged);

    //初始化定时器
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        m_currentTimeMs += 2000; // 每次增加100毫秒，模拟播放
        emit currentTimeChanged(m_currentTimeMs);
    });
}


LyricsDialog::~LyricsDialog() {
    delete m_timer;
}

void LyricsDialog::onLyricsTextChanged(QString text) {
    if (text == "No lyrics") {
        QFont font;
        font.setPointSize(18);
        m_lyricsLabel->setFont(font);
        m_lyricsLabel->setStyleSheet("QLabel {background-color: transparent; color: gray; border: none; border-radius: 5px;}");
        if (m_timer && m_timer->isActive()) {
              m_timer->stop();
              m_hasTimestamps = false;
        }
    } else {
        QFont font;
        font.setPointSize(18); // 恢复默认的字体大小
        m_lyricsLabel->setFont(font);
        m_lyricsLabel->setStyleSheet("QLabel {background-color: transparent; color: #3790FA; border: none; border-radius: 5px;}");
    }
}
void LyricsDialog::dlginitstyle()
{
    this->setStyleSheet(
                "QDialog {"
                "    border-radius: 12px;"
                "    opacity: 1;"
                "    background: rgba(31, 32, 34, 0.6);"
                "    box-sizing: border-box;"
                "    border: 1px solid rgba(0, 0, 0, 0.1);"
                "    box-shadow: 0px 4px 10px 0px rgba(0, 0, 0, 0.3);"
                "}"
                );
    QString text = m_lyricsLabel->text();
    onLyricsTextChanged(text);
    m_closeButton->setStyleSheet("QPushButton {background-color: transparent; color: white; border: none; border-radius: 5px;}");
}

void LyricsDialog::closeDialog() {
    Q_EMIT dlgCLoseSig();
}

void LyricsDialog::loadLyricsForSongDlg(const QString &songFilePath) {
    qInfo() << "[LyricsDialog::loadLyricsForSongDlg] 开始加载歌词，歌曲路径:" << songFilePath;

    // 检查控制栏是否有有效的歌曲信息
    musicDataStruct musicStruct;
    QString checkPath = songFilePath;
    if (!checkPath.isEmpty()) {
        checkPath = checkPath.remove("file://");
        g_db->getSongInfoFromDB(checkPath, musicStruct);
        qInfo() << "[LyricsDialog::loadLyricsForSongDlg] 查询数据库结果，title:" << musicStruct.title << ",filepath:" << checkPath;
    } else {
        qInfo() << "[LyricsDialog::loadLyricsForSongDlg] 歌曲路径为空";
    }

    // 如果没有有效的歌曲信息（title为空），不显示歌词
    if (musicStruct.title.isEmpty()) {
        qInfo() << "[LyricsDialog::loadLyricsForSongDlg] 数据库中没有歌曲信息，不显示歌词";
        m_timestampedLyrics.clear();
        m_hasTimestamps = false;
        if (m_timer && m_timer->isActive()) {
            m_timer->stop();
        }
        m_lyricsLabel->setText(tr("No lyrics"));
        m_lyricsLabel->setVisible(true);
        Q_EMIT textChanged("No lyrics");
        return;
    }

    qInfo() << "[LyricsDialog::loadLyricsForSongDlg] 数据库中有歌曲信息，继续加载歌词，歌曲标题:" << musicStruct.title;

    // 有有效歌曲信息，继续加载歌词
    QFileInfo fileInfo(songFilePath);

    // 定义要检查的文件扩展名
    QString lrcExt = ".lrc";
    bool fileFound = false;

    // 构建.lrc文件路径
    QString lrcFilePath = fileInfo.absolutePath() + "/" + fileInfo.baseName() + lrcExt;
    QFile file(lrcFilePath);
    if (!fileFound) {
        m_timer->stop();
        // 如果没有找到任何歌词文件，则显示默认的"暂无歌词"信息
        qDebug() << "No lyrics file found.";
        m_lyricsLabel->setText(tr("No lyrics"));
        m_lyricsLabel->setVisible(true); // 确保标签可见
        Q_EMIT textChanged("No lyrics");
    }
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Failed to open file:" << file.errorString();
        } else {
            QByteArray content = file.readAll();
            // 检查是否可以通过一下常见格式进行解码
            QStringList encodings = {"UTF-8", "GBK", "GB18030", "BIG5", "ISO-8859-1"};
            bool hasGetLyrics = false;
            for (const auto &enc : encodings) {
                QTextCodec *codec = QTextCodec::codecForName(enc.toLatin1());
                lyrics = codec->toUnicode(content);
                if (!lyrics.contains(QChar::ReplacementCharacter)) {
                    qInfo() << "成功解码:" <<  enc;
                    hasGetLyrics = true;
                    break;
                }
            }
            // 检查是否有时间戳
            bool hasTimestamps = lyrics.contains(QRegularExpression("\\[\\d{2}:\\d{2}(\\.\\d{2,3})?\\]"));
            if (hasTimestamps) {
                m_timestampedLyrics = parseLrc(lyrics);
                m_hasTimestamps = true;
            } else {
                QStringList lines = lyrics.split('\n');
                if (!lines.isEmpty()) {
                    m_lyricsLabel->setText(lines.first()); // 显示第一行歌词
                    Q_EMIT textChanged(lines.first());
                } else {
                    m_lyricsLabel->setText(tr("No lyrics"));
                    Q_EMIT textChanged("No lyrics");
                }
                m_hasTimestamps = false; // 标记为无时戳
                m_timer->stop();
            }

            // 确保 m_lyricsLabel 可见
            m_lyricsLabel->setVisible(true);
            if (m_hasTimestamps) {
                scrollLyrics(m_timestampedLyrics, 0); // 显示第一行歌词
            }

            file.close();
            fileFound = true;
        }
    }
}

void LyricsDialog::setCurrentTime(int currentTimeMs) {
    m_currentTimeMs = currentTimeMs;
    emit currentTimeChanged(currentTimeMs);
}

void LyricsDialog::onPlayButtonClicked(qint64 m_nowtime) {
    if(m_nowtime == 0){
        scrollLyrics(m_timestampedLyrics, 0);
    }
    if (m_hasTimestamps) { // 只有有时戳时才更新时间和启动计时器
        setCurrentTime(m_nowtime); // 更新歌词显示
        m_timer->start(2000);
    }
}
void LyricsDialog::onStopButtonClicked() {
    m_timer->stop();
}
// 解析 .lrc 文件
QList<TimestampedLyric> LyricsDialog::parseLrc(const QString& lyrics) const {
    QList<TimestampedLyric> timestampedLyrics;
    QRegularExpression re("\\[(\\d{2}):(\\d{2})(?:\\.(\\d{2,3}))?\\]");
    QStringList lines = lyrics.split('\n');

    for (const auto& line : lines) {
        QRegularExpressionMatch match = re.match(line);
        if (match.hasMatch()) {
            TimestampedLyric tl;
            bool ok;

            tl.minutes = match.captured(1).toInt(&ok);
            if (!ok) {
                qWarning() << "Failed to parse minutes in line: " << line;
                continue;
            }

            tl.seconds = match.captured(2).toInt(&ok);
            if (!ok) {
                qWarning() << "Failed to parse seconds in line: " << line;
                continue;
            }

            tl.milliseconds = 0;
            if (match.captured(3).length() > 0) {
                tl.milliseconds = match.captured(3).toInt(&ok);
                if (!ok) {
                    qWarning() << "Failed to parse milliseconds in line: " << line;
                    continue;
                }
            }

            int timeEndPos = match.capturedEnd();
            tl.lyric = line.mid(timeEndPos).trimmed();

            timestampedLyrics.append(tl);
        }
    }

    return timestampedLyrics;
}

void LyricsDialog::scrollLyrics(const QList<TimestampedLyric>& timestampedLyrics, int currentTime) {
    if (!m_hasTimestamps) return; // 如果没有时间戳则直接返回
    QString currentLyric;

    for (const auto& tl : timestampedLyrics) {
        int totalMilliseconds = tl.minutes * 60 * 1000 + tl.seconds * 1000 + tl.milliseconds;
        if (totalMilliseconds <= currentTime) {
            currentLyric = tl.lyric;
        } else {
            break; // 找到了下一个时间戳，停止循环
        }
    }
    m_lyricsLabel->setText(currentLyric);
    Q_EMIT textChanged(currentLyric);
    // 强制刷新界面
    m_lyricsLabel->update();
    m_lyricsLabel->repaint();

    // 确保父窗口也刷新
    this->update();
    this->repaint();
}
