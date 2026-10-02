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

#include "lyricswidget.h"
#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <QRect>
#include <QHBoxLayout>
#include <QDebug>
#include <QPushButton>
#include "UI/globalsignal.h"
#include "UI/mainwidget.h"

LyricsWidget::LyricsWidget(QWidget *parent) : QWidget(parent)
{
    initSetModel();
    widinitstyle();
}

void LyricsWidget::widinitstyle()
{
    if (WidgetStyle::themeColor == 1) {
        this->setStyleSheet(
                    "QWidget {"
                    "    border-radius: 12px;"
                    "    opacity: 0;"
                    "    background: #1d1d1d;"
                    "}"
                    "QTextEdit {"
                    "    background-color: #1d1d1d;" // 明确指定背景颜色为白色
                    "    border: none;"
                    "    color: #8F9399;" // 确保文字颜色是黑色
                    "    text-align: center;" // 文本居中
                    "}"
                    "QTextEdit:focus {"
                    "    background-color: #1d1d1d;" // 确保获取焦点时背景颜色不变
                    "    border: none;" // 确保获取焦点时没有边框
                    "    color: #8F9399;" // 确保获取焦点时文字颜色是黑色
                    "}"
                    "QScrollArea {"
                    "    border: none;"
                    "    background: #1d1d1d;"
                    "}"
                    );

        containerWidget->setStyleSheet(
                    "QWidget {"
                    "    border-radius: 12px;"
                    "    opacity: 0;"
                    "    background: #1d1d1d;"
                    "}"
                    "QTextEdit {"
                    "    background-color: #1d1d1d;" // 明确指定背景颜色为白色
                    "    border: none;"
                    "    color: #8F9399;" // 确保文字颜色是黑色
                    "    text-align: center;" // 文本居中
                    "}"
                    "QTextEdit:focus {"
                    "    background-color: #1d1d1d;" // 确保获取焦点时背景颜色不变
                    "    border: none;" // 确保获取焦点时没有边框
                    "    color: #8F9399;" // 确保获取焦点时文字颜色是黑色
                    "}"

                    "QWidget {"
                    "    background: #1d1d1d;" // 确保 containerWidget 背景透明
                    "    border: none;" // 确保 containerWidget 没有边框
                    "}"
                    );
        containerWidget->setStyleSheet("QScrollBar:vertical{width:16px; border-radius:4px;}" // 可设置可区分背景颜色进行调节，整个滑动条控件的宽度width = 滑块宽度 + margin-left + margin-right
                                                 "QScrollBar::handle:vertical{background:rgba(255,255,255, 10%); width: 4px;border-radius: 2px; margin-left:6px;margin-right:6px;}" // 滑动条滑块handle两端变椭圆,和滑轨之间的间隙
                                                 "QScrollBar::handle:vertical:hover{background:rgba(255,255,255, 15%); width: 8px; border-radius:4px;margin-left:4px;margin-right:4px;}" // 鼠标放到滑块上颜色加深
                                                 "QScrollBar::sub-line:vertical{border: none; background: none; color:none;}" // 设置上箭头不可见
                                                 "QScrollBar::add-line:vertical{border: none; background: none; color:none;}" // 设置下箭头不可见
                                                 "QScrollBar::sub-page:vertical{background:none;}" // 滑块还没经过的滑轨区域的颜色
                                                 "QScrollBar::add-page:vertical{background:none;}" // 滑块已经经过的滑轨区域的颜色
                                                 "QScrollBar::up-arrow:vertical{background:none;}" // 上箭头图标
                                                 "QScrollBar::down-arrow:vertical{background:none;}" // 下箭头图标
                                                 );

    } else if(WidgetStyle::themeColor == 0) {
        this->setStyleSheet(
                    "QWidget {"
                    "    border-radius: 12px;"
                    "    opacity: 0;"
                    "    background: #FFFFFF;"
                    "}"
                    "QTextEdit {"
                    "    background-color: #FFFFFF;" // 明确指定背景颜色为白色
                    "    border: none;"
                    "    color: #4d000000;" // 确保文字颜色是黑色
                    "    text-align: center;" // 文本居中
                    "}"
                    "QTextEdit:focus {"
                    "    background-color: #FFFFFF;" // 确保获取焦点时背景颜色不变
                    "    border: none;" // 确保获取焦点时没有边框
                    "    color: #4d000000;" // 确保获取焦点时文字颜色是黑色
                    "}"
                    "QScrollArea {"
                    "    border: none;"
                    "    background: #FFFFFF;"
                    "}"
                    );

        containerWidget->setStyleSheet(
                    "QWidget {"
                    "    border-radius: 12px;"
                    "    opacity: 0;"
                    "    background: #FFFFFF;"
                    "}"
                    "QTextEdit {"
                    "    background-color: #FFFFFF;" // 明确指定背景颜色为白色
                    "    border: none;"
                    "    color: #4d000000;" // 确保文字颜色是黑色
                    "    text-align: center;" // 文本居中
                    "}"
                    "QTextEdit:focus {"
                    "    background-color: #FFFFFF;" // 确保获取焦点时背景颜色不变
                    "    border: none;" // 确保获取焦点时没有边框
                    "    color: #4d000000;" // 确保获取焦点时文字颜色是黑色
                    "}"
                    "QWidget {"
                    "    background: #FFFFFF;" // 确保 containerWidget 背景透明
                    "    border: none;" // 确保 containerWidget 没有边框
                    "}"
                    );
        containerWidget->setStyleSheet("QScrollBar:vertical{width:16px; border-radius:4px;}" // 可设置可区分背景颜色进行调节，整个滑动条控件的宽度width = 滑块宽度 + margin-left + margin-right
                                                 "QScrollBar::handle:vertical{background:rgba(0,0,0, 10%); width: 4px;border-radius: 2px; margin-left:6px;margin-right:6px;}" // 滑动条滑块handle两端变椭圆,和滑轨之间的间隙
                                                 "QScrollBar::handle:vertical:hover{background:rgba(0,0,0, 15%); width: 8px; border-radius:4px;margin-left:4px;margin-right:4px;}" // 鼠标放到滑块上颜色加深
                                                 "QScrollBar::sub-line:vertical{border: none; background: none; color:none;}" // 设置上箭头不可见
                                                 "QScrollBar::add-line:vertical{border: none; background: none; color:none;}" // 设置下箭头不可见
                                                 "QScrollBar::sub-page:vertical{background:none;}" // 滑块还没经过的滑轨区域的颜色
                                                 "QScrollBar::add-page:vertical{background:none;}" // 滑块已经经过的滑轨区域的颜色
                                                 "QScrollBar::up-arrow:vertical{background:none;}" // 上箭头图标
                                                 "QScrollBar::down-arrow:vertical{background:none;}" // 下箭头图标
                                                 );
    }
    this->setAutoFillBackground(true);
    containerWidget->setAutoFillBackground(true);
}

void LyricsWidget::initSetModel()
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_DeleteOnClose);
    //禁用界面拖拽
    this->setProperty("useStyleWindowManager", false);
    this->setBackgroundRole(QPalette::Base);
    this->setMinimumSize(750, 390); // 设置固定大小

    QHBoxLayout *layout = new QHBoxLayout(this);
    QHBoxLayout *imagelayout = new QHBoxLayout(this);
    imageLabel = new QLabel(this);
    imageLabel->setMinimumSize(240, 532); // 左侧图片占位图
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setPixmap(QIcon(":/img/fengmian1x.png").pixmap(QSize(240,240)));
    imagelayout->setContentsMargins(72, 0, 0, 0);
    imagelayout->addWidget(imageLabel);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true); // 确保内容可以调整大小以适应滚动区域
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 只允许垂直滚动

    textlabel = new QLabel(this);
    textlabel->setText(tr("No lyrics"));
    textlabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);

    textEdit = new QTextEdit(this); // 创建 QTextEdit 作为歌词显示
    textEdit->setAccessibleName("kylin-music_lyricswidget_QTextEdit_textEdit");
    textEdit->setReadOnly(true); // 设置为只读
    textEdit->setWordWrapMode(QTextOption::NoWrap); // 禁用自动换行
    textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 隐藏水平滚动条
    textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded); // 显示垂直滚动条
    textEdit->setAlignment(Qt::AlignCenter); // 文本居中对齐
    textEdit->document()->setDefaultTextOption(QTextOption(Qt::AlignCenter)); // 设置默认文本选项为居中
    textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); // 让 QTextEdit 扩展以填充可用空间
    textEdit->setMinimumHeight(this->height() - 200);
    textEdit->setVisible(false);

    QVBoxLayout* verticalLayout = new QVBoxLayout();
    verticalLayout->addStretch(50); // 添加伸缩空间在上部
    verticalLayout->addWidget(textlabel);
    verticalLayout->addWidget(textEdit); // 添加 QTextEdit
    verticalLayout->addStretch(50); // 添加伸缩空间在下部
    // 创建一个 QWidget 作为 QVBoxLayout 的容器
    containerWidget = new QWidget(this);
    containerWidget->setLayout(verticalLayout);

    containerWidget->setContentsMargins(0,0,0,0);
    scrollArea->setWidget(containerWidget); // 将容器设置为 QScrollArea 的 widget
    scrollArea->setWidgetResizable(true); // 使内容可以调整大小
    scrollArea->setAlignment(Qt::AlignCenter); // 内容居中对齐
    layout->addLayout(imagelayout);
    layout->addWidget(scrollArea);
    layout->setContentsMargins(0,0,0,0);
    this->setContentsMargins(0,0,0,0);

    connect(this, &LyricsWidget::currentTimeChanged, this, [this](int currentTimeMs) {
        scrollLyrics(m_widtimestampedLyrics, currentTimeMs);
    });
    connect(&playController::getInstance(),&playController::singalChangePath,this,&LyricsWidget::slotSongInfo);

    //初始化定时器
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, [this]() {
        m_currentTimeMs += 2000; // 每次增加100毫秒，模拟播放
        emit currentTimeChanged(m_currentTimeMs);
    });

    if (QGSettings::isSchemaInstalled(FITTHEMEWINDOWS)){
        themeData = new QGSettings(FITTHEMEWINDOWS);
        hightLightColor = rgbaToHex(themeData->get("theme-color").toString());
       //监听变化
        connect(themeData,&QGSettings::changed,this,[=](){
           hightLightColor = rgbaToHex(themeData->get("theme-color").toString());
           emit currentTimeChanged(m_currentTimeMs);
       });
    }else{
        hightLightColor = "#3790FA";
    }

}

QString LyricsWidget::rgbaToHex(const QString& rgbaStr) {
    QStringList parts = rgbaStr.split(",");
    if (parts.size() >= 3) {
        int r = parts[0].toInt();
        int g = parts[1].toInt();
        int b = parts[2].toInt();

            // 转换为十六进制
        return QString("#%1%2%3")
            .arg(r, 2, 16, QChar('0'))
            .arg(g, 2, 16, QChar('0'))
            .arg(b, 2, 16, QChar('0'))
            .toUpper();
    }
    //默认
    return "#3790FA";
}

void LyricsWidget::resizeEvent(QResizeEvent *event)
{
    // 更新 QTextEdit 的最小高度
    textEdit->setMinimumHeight(event->size().height() - 120);
    QWidget::resizeEvent(event); // 调用基类的 resizeEvent
}

void LyricsWidget::changeLyricsPos(int posX, int posY, int width, int height)
{
    lyricsWidPosX = posX;
    lyricsWidPosY = posY;
    lyricsWidPosWidth = width;
    lyricsWidPosHeight = height;
}

void LyricsWidget::toggleVisibility()
{
    if (Widget::mutual->m_lyricsWid->isVisible()) {
        this->hide();
        Widget::mutual->musicListTable->setVisible(true);
        Widget::mutual->m_lyricsWid->setVisible(false);
        Widget::mutual->m_titleBar->toggleButton->setVisible(false);
        g_user_signal->lyricsButtonIsShow = false;
    }
}
void LyricsWidget::normalVisibility()
{
    if (g_user_signal->lyricsButtonIsShow) {
        this->show();
        g_user_signal->lyricsButtonIsShow = true;
        Widget::mutual->musicListTable->setVisible(false);
    }
}

void LyricsWidget::showLyricsWid()
{
    if(g_user_signal->lyricsButtonIsShow){
        this->hide();
        g_user_signal->lyricsButtonIsShow = false;
        Widget::mutual->musicListTable->show();
        Widget::mutual->m_titleBar->toggleButton->setVisible(false);
    }else{
        g_user_signal->lyricsButtonIsShow = true;
        Widget::mutual->m_titleBar->toggleButton->setVisible(true);
        QString playPath = playController::getInstance().getPath();
        loadLyricsForSongWid(playPath);

        Widget::mutual->musicListTable->setVisible(false);
        slotSongInfo(playPath);
        this->show();
        this->widinitstyle();

        if(playController::getInstance().getState() == playController::PlayState::PLAY_STATE){
            qint64 movePos = playController::getInstance().getMpvCorePointer()->position();
            PlayButtonClicked(movePos);
        }
    }
}

void LyricsWidget::loadLyricsForSongWid(const QString &songFilePath) {
    qInfo() << "[LyricsWidget::loadLyricsForSongWid] 开始加载歌词，歌曲路径:" << songFilePath;

    // 检查控制栏是否有有效的歌曲信息,核心问题是，信息一部分存在数据库，一部分存在gsettings，会导致状态不一致
    musicDataStruct musicStruct;
    QString checkPath = songFilePath;
    if (!checkPath.isEmpty()) {
        checkPath = checkPath.remove("file://");
        g_db->getSongInfoFromDB(checkPath, musicStruct);
        qInfo() << "[LyricsWidget::loadLyricsForSongWid] 查询数据库结果，title:" << musicStruct.title << ",filepath:" << checkPath;
    } else {
        qInfo() << "[LyricsWidget::loadLyricsForSongWid] 歌曲路径为空";
    }

    // 如果没有有效的歌曲信息（title为空），不显示歌词
    if (musicStruct.title.isEmpty()) {
        qInfo() << "[LyricsWidget::loadLyricsForSongWid] 数据库中没有歌曲信息，不显示歌词";
        m_widtimestampedLyrics.clear();
        m_hasTimestamps = false;
        m_islrcfile = false;
        textEdit->setVisible(false);
        textlabel->setVisible(true);
        textlabel->setText(tr("No lyrics"));
        m_timer->stop();
        return;
    }

    qInfo() << "[LyricsWidget::loadLyricsForSongWid] 数据库中有歌曲信息，继续加载歌词，歌曲标题:" << musicStruct.title;

    // 有有效歌曲信息，继续加载歌词
    QString lrcExt = ".lrc";
    QString lyrics;
    bool fileFound = false;

    QFileInfo fileInfo(songFilePath);
    QString lrcFilePath = fileInfo.absolutePath() + "/" + fileInfo.baseName() + lrcExt;
    QFile file(lrcFilePath);

    if (!fileFound) {
        m_islrcfile = false;
        textEdit->setVisible(false);
        textlabel->setVisible(true);
        textlabel->setText(tr("No lyrics"));
        m_timer->stop();
    }
    if (file.exists()) {
        qInfo() << "[LyricsWidget::loadLyricsForSongWid] 找到歌词文件:" << lrcFilePath;
        m_islrcfile = true;
        if (!file.open(QIODevice::ReadOnly)) {
            qDebug() << "Failed to open file:" << file.errorString();
        }else
        {
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

            if (!hasGetLyrics) {
                qInfo() << "异常编码格式，先试用utf8解码，保证流程正常";
                lyrics = QString::fromUtf8(content);
            }

            bool hasTimestamps = lyrics.contains(QRegularExpression("\\[\\d{2}:\\d{2}(\\.\\d{2,3})?\\]"));
            if (hasTimestamps) {
                qInfo() << "[LyricsWidget::loadLyricsForSongWid] 解析到带时间戳的歌词，行数:" << m_widtimestampedLyrics.size();
                textEdit->setVisible(true);
                textlabel->setVisible(false);
                m_widtimestampedLyrics = parseLrc(lyrics);
                m_hasTimestamps = true; // 标记为有时戳
                int time = playController::getInstance().getMpvCorePointer()->position();
                scrollLyrics(m_widtimestampedLyrics, time); // 显示第一行歌词
            } else {
                qInfo() << "[LyricsWidget::loadLyricsForSongWid] 解析到无时间戳的歌词";
                textEdit->setVisible(false);
                textlabel->setVisible(true);
                textlabel->setText(lyrics); // 直接显示全部歌词
                m_hasTimestamps = false; // 标记为无时戳
            }

            file.close();
            fileFound = true;
        }
    }
}

// 解析 .lrc 文件
QList<m_TimestampedLyric> LyricsWidget::parseLrc(const QString& lyrics) {
    QList<m_TimestampedLyric> m_widtimestampedLyrics;
    QRegularExpression re("\\[(\\d{2}):(\\d{2})(?:\\.(\\d{2,3}))?\\]");
    QStringList lines = lyrics.split('\n');

    for (const auto& line : lines) {
        QRegularExpressionMatch match = re.match(line);
        if (match.hasMatch()) {
            m_TimestampedLyric tl;
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

            tl.milliseconds = match.captured(3).toInt(&ok);
            if (!ok) {
                qWarning() << "Failed to parse milliseconds in line: " << line;
                continue;
            }

            int timeEndPos = match.capturedEnd();
            tl.lyric = line.mid(timeEndPos).trimmed();

            m_widtimestampedLyrics.append(tl);
        }
    }

    return m_widtimestampedLyrics;
}

void LyricsWidget::scrollLyrics(const QList<m_TimestampedLyric>& m_widtimestampedLyrics, int currentTime)
{
    if (!m_hasTimestamps) return; // 如果没有时间戳则直接返回

    int currentLineIndex = 0;

    for (int i = 0; i < m_widtimestampedLyrics.size(); ++i) {
        const auto& tl = m_widtimestampedLyrics[i];
        int totalMilliseconds = tl.minutes * 60 * 1000 + tl.seconds * 1000 + tl.milliseconds;

        if (totalMilliseconds <= currentTime) {
            currentLineIndex = i; // 更新当前时间对应的行索引
        }
    }

    // 获取 QTextEdit 的可见区域高度
    int visibleHeight = textEdit->height();

    // 使用 QTextDocument 测量一行歌词的高度
    QTextDocument doc;
    if(!m_widtimestampedLyrics.isEmpty()){
        doc.setPlainText(m_widtimestampedLyrics[currentLineIndex].lyric);
    }
    doc.setTextWidth(textEdit->width());
    int lineHeight = doc.documentLayout()->blockBoundingRect(doc.firstBlock()).height();

    // 计算需要多少空白行才能使当前行居中
    int linesAbove = (visibleHeight / lineHeight) / 2 - currentLineIndex;
    int linesBelow = (visibleHeight / lineHeight) / 2 - (m_widtimestampedLyrics.size() - currentLineIndex - 1);

    // 重新设置歌词颜色，靠近边缘的行显示为灰色
    setEdgeLinesGray(linesAbove, linesBelow, m_widtimestampedLyrics, currentLineIndex);

    QTextCursor cursor = textEdit->textCursor();
    cursor.movePosition(QTextCursor::Start);
    for (int i = 0; i < currentLineIndex; ++i) {
        cursor.movePosition(QTextCursor::Down);
    }
    textEdit->setTextCursor(cursor);

    // 确保光标可见并将当前行居中
    ensureCurrentLineCentered(currentLineIndex);
}

void LyricsWidget::ensureCurrentLineCentered(int currentLineIndex)
{
    // 获取 QTextEdit 的可见区域高度
    int visibleHeight = textEdit->height();

    // 使用 QTextDocument 测量一行歌词的高度
    QTextDocument doc;
    if(!m_widtimestampedLyrics.isEmpty())
    doc.setPlainText(m_widtimestampedLyrics[currentLineIndex].lyric);
    doc.setTextWidth(textEdit->width());
    int m_lineHeight = visibleHeight / 9;

    // 获取垂直滚动条
    QScrollBar *verticalScrollBar = textEdit->verticalScrollBar();

    int lineHeight = doc.documentLayout()->blockBoundingRect(doc.firstBlock()).height();
    int middleline = (visibleHeight/lineHeight)/2;
    int realcurrline = currentLineIndex + 1;
    int scrollValue = 0;
    if(currentLineIndex < middleline){
        scrollValue = 0;
    }else{
        // 确保滚动值在有效范围内
        scrollValue = (realcurrline - middleline + 1) * m_lineHeight;
    }
    // 滚动到指定位置
    verticalScrollBar->setValue(scrollValue);
}

void LyricsWidget::setEdgeLinesGray(int linesAbove, int linesBelow, const QList<m_TimestampedLyric>& m_widtimestampedLyrics, int currentLineIndex)
{
    // 重新构建歌词HTML字符串，将靠近边缘的行设置为灰色
    QString lyricsHtml;

    for (int i = 0; i < m_widtimestampedLyrics.size(); ++i) {
        const auto& tl = m_widtimestampedLyrics[i];

        // 根据位置决定颜色
        QColor color;
        if (i == currentLineIndex) {
            color = hightLightColor;
        } else {
            if(WidgetStyle::themeColor == 0){
                color = QColor::fromRgba(0x99000000); // 其他行不应用高亮
            }else if(WidgetStyle::themeColor == 1){
                color = Qt::white; // 其他行不应用高亮
            }
        }

        // 构建 HTML 字符串
        lyricsHtml += "<p style='color: " + color.name(QColor::HexArgb) + "; text-align: center;'>" + tl.lyric + "</p>";
    }

        // 添加空白行以使当前行居中
        for (int i = 0; i < linesBelow + 1; ++i) {
            lyricsHtml += "<p style='text-align: center;'>&nbsp;</p>"; // 在底部添加空白行
        }

    // 设置富文本
    textEdit->setHtml(lyricsHtml);
}

void LyricsWidget::setCurrentTime(int currentTimeMs) {
    m_currentTimeMs = currentTimeMs;
    emit currentTimeChanged(currentTimeMs);
}

void LyricsWidget::PlayButtonClicked(qint64 m_nowtime) {
    if(m_islrcfile == true){
        if (m_hasTimestamps) { // 只有有时戳时才更新时间和启动计时器
            setCurrentTime(m_nowtime); // 更新歌词显示
            m_timer->start(2000);
        }}
}
void LyricsWidget::StopButtonClicked() {
    m_timer->stop();
}

void LyricsWidget::slotSongInfo(QString path)
{
    filePath = path.remove("file://");
    QPixmap pix = MusicFileInformation::getInstance().getCoverPhotoPixmap(filePath);

    if(pix.isNull()) {
        pix = QIcon(":/img/fengmian1x.png").pixmap(QSize(240,240));
    } else {
        pix = setCoverPhotoPixmapRadius(pix);
        pix = pix.scaled(QSize(240,240),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    }

    imageLabel->setPixmap(pix);
}

QPixmap LyricsWidget::setCoverPhotoPixmapRadius(QPixmap pixmap)
{
    QSize size(240, 240);
    QBitmap mask(size);

    QPainter painter(&mask);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.fillRect(0, 0, size.width(), size.height(), Qt::white);
    painter.setBrush(QColor(0, 0, 0));
    painter.drawRoundedRect(0, 0, size.width(), size.height(), 50, 50); //修改这个值，可以改弧度，和直径相等就是圆形

    QPixmap image = pixmap.scaled(size);
    image.setMask(mask);

    return image;

}

