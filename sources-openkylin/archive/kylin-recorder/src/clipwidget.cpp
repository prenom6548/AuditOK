#include "clipwidget.h"

#include "tools.h"
#include "mainwindow.h"
#include <QFileInfo>
#include "libkydate.h"
#define MIN(x, y) (x) < (y) ? (x) : (y)
#define MAX(x, y) (x) > (y) ? (x) : (y)
ClipWidget::ClipWidget(QString clipAudioPath, QWidget *parent) : QWidget(parent)
{

    m_clipPath = clipAudioPath; //传入的某音频的路径
    qDebug() << "获取剪裁的音频路径" << m_clipPath;
    initClipWid();
    setClipWid();
    initThemeGsetting();
}

int ClipWidget::getInterval_Time(QString filePath)
{
    QString newFilePath = filePath;
    if(QFileInfo(newFilePath).completeSuffix() != "wav"){
        newFilePath = QFileInfo(newFilePath).absolutePath() + "/." + QFileInfo(newFilePath).baseName() + ".wav";
    }
    QFile file(newFilePath);

    int time = file.size() / 96000;
    int x = 1;
    if (time >= 1) {
        x = time * 1000 / railWid->width();
    } else {
        x = (time + 1) * 1000 / railWid->width();
    }
    qDebug() << "间隔" << x << "ms运动一次"
             << "railWid宽度:" << railWid->width();
    return x;
}

void ClipWidget::initClipWid()
{
    m_splitLine = new QFrame(this);
    m_splitLine->setAccessibleName("kylin-recorder_clipwidget_QFrame_m_splitLine");
    m_Wavefile = new AudioFileProcess(this);
    process = new QProcess(this);
    dur_timelb = new QLabel(this); //总时长标签
    dur_timelb->setAccessibleName("kylin-recorder_QLabel_dur_timelb");
    clip_playBtn = new CustomButton(":/png/png/start_mini.png", ":/png/png/start_mini_hover.png",
                                    ":/png/png/start_mini_click.png", this);
    clip_playBtn->setAccessibleName("kylin-recorder_clipwidget_CustomButton_clip_playBtn");
    clip_pauseBtn = new CustomButton(":/png/png/pause_mini.png", ":/png/png/pause_mini_hover.png",
                                     ":/png/png/pause_mini_click.png", this);
    clip_pauseBtn->setAccessibleName("kylin-recorder_clipwidget_CustomButton_clip_pauseBtn");
    cancelButton = new kdk::KBorderlessButton(this); //取消
    cancelButton->setAccessibleName("kylin-recorder_clipwidget_KBorderlessButton_cancelButton");
    finishButton = new kdk::KBorderlessButton(this); //完成
    finishButton->setAccessibleName("kylin-recorder_clipwidget_KBorderlessButton_finishButton");
    railWid = new QWidget(this);          //轨道的Wid
    railWid->setAccessibleName("kylin-recorder_clipwidget_QWidget_railWid");
    scopeFrame = new ScopeFrame(this);
    scopeFrame->setAccessibleName("kylin-recorder_clipwidget_ScopeFrame_scopeFrame");
    bottomWid = new QWidget(this); //底部Wid
    bottomWid->setAccessibleName("kylin-recorder_clipwidget_QWidget_bottomWid");
//    leftBtn = new ClipButton(QIcon::fromTheme("ukui-start-symbolic"), this);
//    rightBtn = new ClipButton(QIcon::fromTheme("ukui-end-symbolic"), this);
    leftBtn = new ClipButton(":/svg/svg/ukui-start-symbolic.svg", this);
    leftBtn->setAccessibleName("kylin-recorder_clipwidget_ClipButton_leftBtn");
    rightBtn = new ClipButton(":/svg/svg/ukui-end-symbolic.svg", this);
    rightBtn->setAccessibleName("kylin-recorder_clipwidget_ClipButton_rightBtn");
    cursor = new QFrame(this); //设置游标在父类railWid上
    cursor->setAccessibleName("kylin-recorder_clipwidget_QFrame_cursor");

    waveLayout = new QHBoxLayout();
    bottomLayout = new QHBoxLayout();
    m_lineClipLayout = new QVBoxLayout();
    clipLayout = new QVBoxLayout();

    FFUtil fu;
    fu.open(m_clipPath);
    m_currentEndPos = fu.getDuration(m_clipPath); //获取的是ms
    m_audioDuration = m_currentEndPos;
//    fu.close();
    connect(cancelButton, &QPushButton::clicked, this, &ClipWidget::cancel_ClipSlot);
    connect(finishButton, &QPushButton::clicked, this, &ClipWidget::finish_ClipSlot);

    connect(leftBtn, &ClipButton::leftBtn_ReleaseStartPlayer_Signal, this,
            &ClipWidget::leftBtn_ReleaseStartPlayer_Slot);
    connect(this, &ClipWidget::computeLeftBtnL_To_RightBtnRWidth, scopeFrame, &ScopeFrame::computeClipAreaWidth);
    connect(rightBtn, &ClipButton::rightBtn_ReleaseGetEndPositon_Signal, this,
            &ClipWidget::rightBtn_ReleaseGetEndPositon_Slot);
    connect(leftBtn, &ClipButton::btn_MoveSignal, this, &ClipWidget::leftBtn_Move_Slot);
    connect(rightBtn, &ClipButton::btn_MoveSignal, this, &ClipWidget::rightBtn_Move_Slot);
    connect(clip_playBtn, &CustomButton::clicked, this, &ClipWidget::play_ClipFile);
    connect(clip_pauseBtn, &CustomButton::clicked, this, &ClipWidget::pause_ClipFile);
    connect(this, &ClipWidget::wave_AnalysisStart, MainWindow::mutual, &MainWindow::handlingSlot);
    connect(this, &ClipWidget::wave_AnalysisFinish, this, &ClipWidget::wave_Analysis_Tips);
    connect(PlayControl::getInstance().clipMoveTimer, &QTimer::timeout, this, &ClipWidget::cursorMove);
    connect(PlayControl::getInstance().getPlayer(), SIGNAL(positionChanged(qint64)), this, SLOT(positionChange(qint64)));

    connect(process, SIGNAL(finished(int)), this, SLOT(processFinish(int)), Qt::UniqueConnection);
    connect(m_Wavefile, &AudioFileProcess::toPaintWave, this, &ClipWidget::toPaintWave);
    connect(MainWindow::mutual, &MainWindow::itemFontStyleSignal,this,&ClipWidget::itemFontStyle);
    connect(MainWindow::mutual, &MainWindow::themeColorchange, this, &ClipWidget::changeCursorColorSlots);
}

void ClipWidget::setClipWid()
{
    for (int i = 0; i < CLIP_RECTANGLE_COUNT; i++) //频率直方图
    {
        myWave *wave = new myWave(true, this); //初始化小矩形
        wave->setAccessibleName("kylin-recorder_clipwidget_myWave_wave");
        wave->setRange(0, 100);
        mywave.append(wave);
        waveLayout->addWidget(wave, 0, Qt::AlignCenter);
    }
    m_splitLine->setFixedHeight(1);
    m_splitLine->setStyleSheet("background-color:#F0F0F0;");

    cancelButton->setFixedWidth(50);
    cancelButton->setText(tr("Cancel"));
    cancelButton->setFlat(true);

    finishButton->setFixedWidth(50);
    finishButton->setText(tr("Finish"));
    this->displayBtn(finishButton,tr("Finish"));
    finishButton->setFlat(true);

    qDebug() << "初始化矩形框" << mywave.count();
    waveLayout->setSpacing(0);
    waveLayout->setContentsMargins(0, 0, 0, 0);

    railWid->setFixedHeight(24);
    railWid->setLayout(waveLayout);

    leftBtn->setObjectName("leftBtn");
    rightBtn->setObjectName("rightBtn");
    cursor->setFrameShape(QFrame::VLine);
    cursor->setFixedSize(1, 24);
    QColor color = QApplication::palette().color(QPalette::Active, QPalette::Highlight);
    QString colorRGB = QString("rgba(%1, %2, %3)").arg(color.red()).arg(color.green()).arg(color.blue());
    QString curStyle = QString("background-color:%1;").arg(colorRGB);
    cursor->setStyleSheet(curStyle);
    cursor->setParent(scopeFrame);

    scopeFrame->setParent(railWid);

    leftBtn->setFixedSize(16, 23);
    leftBtn->setParent(scopeFrame); //设置左按钮在父类railWid上
    rightBtn->setFixedSize(16, 23);
    rightBtn->setParent(scopeFrame); //设置右按钮在父类railWid上

    bottomLayout->addWidget(dur_timelb, 0, Qt::AlignLeft);
    bottomLayout->addStretch();
    bottomLayout->addWidget(clip_playBtn, 0, Qt::AlignCenter);
    bottomLayout->addStretch();
    bottomLayout->removeWidget(clip_pauseBtn);
    //clip_pauseBtn->hide();
    bottomLayout->addWidget(cancelButton, 0, Qt::AlignRight);
    bottomLayout->addSpacing(1);
    bottomLayout->addWidget(finishButton, 0, Qt::AlignRight);
    bottomLayout->addSpacing(-1);
    bottomLayout->setContentsMargins(0, 0, 0, 0);
    bottomWid->setLayout(bottomLayout);

    clipLayout->addWidget(railWid);
    clipLayout->addWidget(bottomWid);
    clipLayout->setContentsMargins(30, 0, 25, 0);

    m_lineClipLayout->addStretch();
    m_lineClipLayout->addWidget(m_splitLine, 0, Qt::AlignVCenter);
    m_lineClipLayout->addSpacing(4);
    m_lineClipLayout->addLayout(clipLayout);
    m_lineClipLayout->addStretch();
    m_lineClipLayout->setContentsMargins(0, 0, 0, 0);

    this->setLayout(m_lineClipLayout);

    qDebug() << "轨道的宽度:" << railWid->width() << railWid->height() << leftBtn->width();

    dur_timelb->setText(MainWindow::mutual->playerTotalTime(m_clipPath));
    clip_playBtn->setFixedSize(16, 16);
    clip_playBtn->setToolTip(tr("Audition"));
    clip_pauseBtn->setFixedSize(16, 16);
    clip_pauseBtn->setToolTip(tr("Pause"));
}

//字体的截断显示显示函数
void ClipWidget::displayBtn(kdk::KBorderlessButton *btn, QString btnTooltip)
{
    QLocale locale;
    if(locale.language()==QLocale::English)
    {
#ifndef WHITE_TEST
        btn->setText(btnTooltip);
        QFontMetrics  fontWidth(btn->font());//每个字符的宽度
        int fontSize = fontWidth.width(btnTooltip);
        if (fontSize > (btn->width() -7 - 14)) {
            QString elideNote = fontWidth.elidedText(btnTooltip, Qt::ElideRight, 63-14-7);  //固定宽度63 布局-14 省略7
            btn->setText(elideNote);
            btn->setToolTip(btnTooltip);
        } else {
            btn->setText(btnTooltip);
            btn->setToolTip("");
        }
#endif
    }
}

void ClipWidget::resizeEvent(QResizeEvent *event)
{
#ifndef WHITE_TEST
    Q_UNUSED(event);

    QRect rect = railWid->geometry();
    qDebug() << "触发了!" << rect << "railWid的宽度:" << rect.width();
    scopeFrame->setFixedSize(rect.width(), rect.height());
    scopeWidth = rect.width();
    rightBtn->move(scopeFrame->width() - rightBtn->width(), 0);
    leftBtn->move(0, 0);
    cursor->move(leftBtn->pos().rx(), 0);
    emit computeLeftBtnL_To_RightBtnRWidth(leftBtn->pos().rx(), rightBtn->pos().rx() + rightBtn->width(),
                                           railWid->width(), mywave); //用于框出初始剪裁范围
#endif
}

int ClipWidget::display_Wave()
{
#ifndef WHITE_TEST
    if (m_clipPath == ""){
        return -2;
    }

    emit wave_AnalysisStart();
    QTimer::singleShot(100, this, [](){qDebug() << "解析弹窗" ;});
    return m_Wavefile->wavFile_Analy(m_clipPath);
#endif

}

void ClipWidget::wave_Analysis_Tips(bool isOk)
{
    qDebug() << "完成" << isOk;
    MainWindow::mutual->m_tipWindow->hide();
}

void ClipWidget::toPaintWave(int index, int value)
{
    if (value > 32000)
        value = 32000;
    if (index < CLIP_RECTANGLE_COUNT - 1) {
        mywave.at(index)->setValue(value / 300);
    } else {
        emit wave_AnalysisFinish(true);
    }
}

void ClipWidget::leftBtn_ReleaseStartPlayer_Slot(int leftButton_absolutePos)
{
    int padding = 0;
    if (leftButton_absolutePos > 0 && leftButton_absolutePos < railWid->width()) {
        padding = rightBtn->pos().rx() - leftBtn->pos().rx();
        if (padding > 0) {
            //除去两个滑块所占位置剩余的部分即为轨道,轨道宽度为wave除去两个滑块的宽度
            m_currentStartPos = static_cast<int64_t>(m_audioDuration) * leftButton_absolutePos / railWid->width();
            m_currentEndPos =
                static_cast<int64_t>(m_audioDuration) * (rightBtn->pos().rx() + rightBtn->width()) / railWid->width();
            qDebug() << "左滑块左边界相对轨道的位置" << leftBtn->pos().rx() << "轨道长度:" << railWid->width()
                     << "歌曲路径:" << m_clipPath << "当前播放歌曲的总长度" << m_audioDuration
                     << "当前播放位置:" << m_currentStartPos;
            QTime currentTime(static_cast<int64_t>(m_currentStartPos) / 1000 / 3600,
                              static_cast<int64_t>(m_currentStartPos) / 60000,
                              static_cast<int64_t>((m_currentStartPos % 60000) / 1000.0));
            QString current_timeStr = currentTime.toString("hh:mm:ss");
            m_timeEditStartTime =
                QTime(0, 0, 0, 0).addMSecs(int(m_currentStartPos)).toString(QString::fromLatin1("hh:mm:ss.zzz"));
            m_timeEditEndTime =
                QTime(0, 0, 0, 0).addMSecs(int(m_currentEndPos)).toString(QString::fromLatin1("hh:mm:ss.zzz"));
            qDebug() << "timeEditStartTime:剪辑开始毫秒级" << m_timeEditStartTime << "剪裁结束时间:" << m_timeEditEndTime;
            dur_timelb->setText(current_timeStr); //获取裁剪开始时间
            bottomLayout->removeWidget(clip_playBtn);
            bottomLayout->insertWidget(2,clip_pauseBtn,0, Qt::AlignCenter);
            clip_pauseBtn->show();
            clip_playBtn->setParent(NULL);
//            PlayControl::getInstance().play_Audio_Reload(m_clipPath); //后面放进点试听按钮时再开始播放，防止没开始播放音频会导致获取当前播放歌曲的总长度为0
            if (PlayControl::getInstance().getAudioState() == QMediaPlayer::PlayingState) {
                qDebug() << "相对位置+按钮宽度=游标的位置";
                clipMovePos = leftBtn->pos().rx(); //相对位置+按钮宽度=游标的位置
                cursor->move(clipMovePos, 0);
                emit computeLeftBtnL_To_RightBtnRWidth(leftBtn->pos().rx(), rightBtn->pos().rx() + rightBtn->width(),
                                                       railWid->width(), mywave);
            } else {
                PlayControl::getInstance().play_Audio_Reload(m_clipPath); //后面放进点试听按钮时再开始播放，防止没开始播放音频会导致获取当前播放歌曲的总长度为0

                qDebug() << "没播放过从0开始";
                clipMovePos = 0;
            }
            PlayControl::getInstance().setPosition(m_currentStartPos);
            qInfo() << "-------------------leftBtn_ReleaseStartPlayer_Slot";
            PlayControl::getInstance().clipMoveTimer->start(
                getInterval_Time(m_clipPath)); //左按钮移动后,游标也从左按钮移动的位置处开始
            qDebug() << "左按钮左边界相对位置:" << leftButton_absolutePos;
        }
    }
    if (leftButton_absolutePos > rightBtn->pos().rx()) {
        QTime currentTime(static_cast<int64_t>(m_currentStartPos) / 1000 / 3600,
                          static_cast<int64_t>(m_currentStartPos) / 60000,
                          static_cast<int64_t>((m_currentStartPos % 60000) / 1000.0));
        QString start_timeStr = currentTime.toString("hh:mm:ss");
        dur_timelb->setText(start_timeStr); //获取裁剪开始时间=右按钮左边界的相对位置-5
    }
    if (leftButton_absolutePos <= 0) {
        dur_timelb->setText("00:00:00"); //获取裁剪开始时间
        m_timeEditStartTime =
            QTime(0, 0, 0, 0).addMSecs(int(m_currentStartPos)).toString(QString::fromLatin1("hh:mm:ss.zzz"));
    }
}

void ClipWidget::positionChange(qint64 pos)
{
    if (PlayControl::getInstance().getAudioState() == QMediaPlayer::PlayingState) {

        QTime currentTime(static_cast<int64_t>(pos) / (60 * 60 * 1000),
                          static_cast<int64_t>(pos) % (60 * 60 * 1000) / 60000,
                          static_cast<int64_t>((pos % (60 * 1000)) / 1000.0));
        QString current_timeStr = currentTime.toString("hh:mm:ss");
        dur_timelb->setText(current_timeStr);
    }
}

//右按钮释放获取剪裁结束位置
void ClipWidget::rightBtn_ReleaseGetEndPositon_Slot(int rightButton_absolutePos)
{
    int padding = 0;
    if (rightButton_absolutePos > 0 && rightButton_absolutePos < railWid->width()) {
        padding = rightBtn->pos().rx() - leftBtn->pos().rx();
        if (padding >= 0) {
            m_currentStartPos = static_cast<int64_t>(m_audioDuration) * leftBtn->pos().rx() / railWid->width();
            m_currentEndPos = static_cast<int64_t>(m_audioDuration) * rightButton_absolutePos / railWid->width();
            QTime currentTime = QTime(0, 0, 0).addSecs(static_cast<int64_t>(m_currentEndPos) / 1000);
            QString end_timeStr = currentTime.toString("hh:mm:ss");
            m_timeEditStartTime =
                QTime(0, 0, 0, 0).addMSecs(int(m_currentStartPos)).toString(QString::fromLatin1("hh:mm:ss.zzz"));
            m_timeEditEndTime =
                QTime(0, 0, 0, 0).addMSecs(int(m_currentEndPos)).toString(QString::fromLatin1("hh:mm:ss.zzz"));
            qDebug() << "timeEditEndTime:剪辑结束毫秒级" << m_timeEditEndTime;
            dur_timelb->setText(end_timeStr); //获取裁剪结束时间
            emit computeLeftBtnL_To_RightBtnRWidth(leftBtn->pos().rx(), rightBtn->pos().rx() + rightBtn->width(),
                                                   railWid->width(), mywave);

        } else {
            qDebug() << "内边距为负值!";
        }
    }
    //如果绝对位置>轨道长度,则剪裁结束时间为文件总时长
    if (rightButton_absolutePos >= railWid->width()) {
        QTime totalTime(static_cast<int64_t>(m_audioDuration) / 1000 / 3600,
                        (static_cast<int64_t>(m_audioDuration) / 60000) % 60,
                        (static_cast<int64_t>(m_audioDuration) / 1000) % 60);
        dur_timelb->setText(totalTime.toString("hh:mm:ss")); //获取裁剪结束时间
        m_timeEditEndTime = QTime(0, 0, 0, 0).addMSecs(int(m_currentEndPos)).toString(QString::fromLatin1("hh:mm:ss.zzz"));
    }
    //如果绝对位置<0,则剪裁结束时间为
    if (rightButton_absolutePos < 0 || padding < 0) {
        int currentPos = static_cast<int>(PlayControl::getInstance().getPlayer()->duration()) / railWid->width()
                         * (leftBtn->pos().rx() + 5);
        QTime currentTime(0, static_cast<int>(currentPos) / 60000, static_cast<int>((currentPos % 60000) / 1000.0));
        QString end_timeStr = currentTime.toString("hh:mm:ss");
        dur_timelb->setText(end_timeStr); //获取裁剪结束时间=左按钮右边界的相对位置+5
        qDebug() << "左按钮右边界的相对位置+5:" << leftBtn->pos().rx() + 5;
    }
}

void ClipWidget::cancel_ClipSlot()
{
    //点击取消剪裁按钮
    emit cancel_ClipSignal();
}

void ClipWidget::finish_ClipSlot()
{
#ifndef WHITE_TEST
    //点击剪裁完成的按钮
    qDebug() << "按压了左键!弹出Menu菜单";
    menu = new QMenu();
    menu->setAccessibleName("kylin-recorder_QMenu_menu");
    actionSave = new QAction();
    actionCover = new QAction();

    connect(actionSave, &QAction::triggered, this, &ClipWidget::save_NewRecord);
    connect(actionCover, &QAction::triggered, this, &ClipWidget::cover_CurRecord);

    actionSave->setText(tr("Save New"));
    actionCover->setText(tr("Cover Current"));

    menu->addAction(actionSave);
    menu->addAction(actionCover); // 2020/12/12禁用
    menu->exec(QCursor::pos());

    menu->deleteLater();
    actionSave->deleteLater();
    actionCover->deleteLater();
#endif
}

/*
1. 获取路径
2. 生成文件到临时文件夹
3. （可选）删除旧的
4. 生成的文件移动成目标文件
*/

const char* TEMP_RECORD_FILE = "/tmp/record.wav";
const char* TEMP_RECORD_FILE_MP3 = "/tmp/record.mp3";
const char* TEMP_RECORD_FILE_M4A = "/tmp/record.m4a";
const char* CLIP_FORMAT = "ffmpeg -i '%1' -y -ss %2 -to %3 '%4'";
bool ClipWidget::generateClipRecord(QString &outputPath)
{
#ifndef WHITE_TEST
    if(outputPath.isEmpty()){
        QString recordPath = MainWindow::mutual->m_defaultPathData->get("path").toString();
        QFileDialog dialog(MainWindow::mutual, tr("Select a file storage directory"), recordPath);
        dialog.setOption(QFileDialog::ShowDirsOnly);
        dialog.setFileMode(QFileDialog::Directory);
        dialog.setLabelText(QFileDialog::Accept, tr("Save"));
        dialog.setLabelText(QFileDialog::Reject, tr("Cancel"));
        if (dialog.exec() == QDialog::Accepted) {
            QString selectDir = dialog.selectedFiles().first();
            qDebug() << "target path" << selectDir;
            outputPath = selectDir;
        } else {
            return false;
        }
    }

    if(Tools::inputEditJudge(outputPath+"/") != 1) {
        QMessageBox::warning(this, tr("Alert"), tr("Clip Path is illegal."));
        qDebug () << "Clip Path is illegal";
        return false;
    }

    if(m_currentEndPos - m_currentStartPos < 1000){
        QMessageBox::warning(this, tr("Alert"), tr("Clip Time is too short."));
        qDebug () << "Clip Time is too short";
        return false;
    }
    qDebug() << m_currentEndPos << m_currentStartPos;
    m_timeEditStartTime =
        QTime(0, 0, 0, 0).addMSecs(int(m_currentStartPos)).toString(QString::fromLatin1("hh:mm:ss.zzz"));
    m_timeEditEndTime =
        QTime(0, 0, 0, 0).addMSecs(int(m_currentEndPos)).toString(QString::fromLatin1("hh:mm:ss.zzz"));
    qDebug() << "timeEditStartTime:剪辑开始毫秒级" << m_timeEditStartTime << "剪裁结束时间:" << m_timeEditEndTime;
    //if(QFileInfo(m_clipPath).completeSuffix() == "mp3"){
    if(MainWindow::mutual->m_setWindow.m_formatDownList->currentIndex() == 0){
        QString cmd = QString(CLIP_FORMAT).arg(m_clipPath).arg(m_timeEditStartTime).arg(m_timeEditEndTime).arg(TEMP_RECORD_FILE_MP3);
        qDebug() << cmd;
        QStringList args;
        args<<"-c"<<cmd;
        process->start("/bin/sh",args);
        process->waitForFinished();
        process->waitForReadyRead();
        qDebug() << process->readAllStandardError();
        qDebug() << process->readAllStandardOutput();
        process->close();
    //}else if(QFileInfo(m_clipPath).completeSuffix() == "m4a"){
        return true;
      }else if(MainWindow::mutual->m_setWindow.m_formatDownList->currentIndex() == 1){
        QString cmd = QString(CLIP_FORMAT).arg(m_clipPath).arg(m_timeEditStartTime).arg(m_timeEditEndTime).arg(TEMP_RECORD_FILE_M4A);
        qDebug() << cmd;
        QStringList args;
        args<<"-c"<<cmd;
        process->start("/bin/sh",args);
        process->waitForFinished();
        process->waitForReadyRead();
        qDebug() << process->readAllStandardError();
        qDebug() << process->readAllStandardOutput();
        process->close();
        return true;
    }

    QString cmd = QString(CLIP_FORMAT).arg(m_clipPath).arg(m_timeEditStartTime).arg(m_timeEditEndTime).arg(TEMP_RECORD_FILE);
    qDebug() << cmd;
    QStringList args;
    args<<"-c"<<cmd;
    process->start("/bin/sh",args);
    process->waitForFinished();
    process->waitForReadyRead();
    qDebug() << process->readAllStandardError();
    qDebug() << process->readAllStandardOutput();
    process->close();
#endif
    return true;
}

void ClipWidget::initThemeGsetting()
{
    connect(MainWindow::mutual->m_themeData, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "styleName") {
            m_itemThemeColor = MainWindow::mutual->m_themeData->get("styleName").toString();
            themeStyle(m_itemThemeColor);
        }
    });
    themeStyle(MainWindow::mutual->m_themeData->get("styleName").toString());
}

void ClipWidget::themeStyle(QString themeColor)
{
    if (themeColor == "ukui-dark" || themeColor == "ukui-black") {
        m_splitLine->setStyleSheet("background-color:#262626;");
        clip_playBtn->setImagePath(":/svg/svg/play_mini_default_dark.svg", ":/svg/svg/play_mini_hover_dark.svg",
                                        ":/svg/svg/play_mini_click_dark.svg");
        clip_pauseBtn->setImagePath(":/svg/svg/pause_mini_default_dark.svg", ":/svg/svg/pause_mini_hover_dark.svg",
                                         ":/svg/svg/pause_mini_click_dark.svg");
        if(!(PlayControl::getInstance().getPlayer()->state())){
            clip_pauseBtn->hide();
        }
    } else {
        m_splitLine->setStyleSheet("background-color:#F0F0F0;");
        clip_playBtn->setImagePath(":/png/png/start_mini.png", ":/png/png/start_mini_hover.png",
                                        ":/png/png/start_mini_click.png");
        clip_pauseBtn->setImagePath(":/png/png/pause_mini.png", ":/png/png/pause_mini_hover.png",
                                         ":/png/png/pause_mini_click.png");
        if(!(PlayControl::getInstance().getPlayer()->state())){
            clip_pauseBtn->hide();
        }
    }
}

void ClipWidget::save_NewRecord()
{
#ifndef WHITE_TEST
    QString path;
    bool res = generateClipRecord(path);
    if(!res){
        return ;
    }
    QDateTime curTime = QDateTime::currentDateTime();
    //QString targetFile =path + "/" + curTime.toString("yyyyMMddhhmmss") + ".mp3";
    QString targetFile;
    switch(MainWindow::mutual->m_setWindow.m_formatDownList->currentIndex()) {
    case 0:
    targetFile =path + "/" + curTime.toString("yyyyMMddhhmmss") + ".mp3";
    break;
    case 1:
    targetFile =path + "/" + curTime.toString("yyyyMMddhhmmss") + ".m4a";
    break;
    default:
    targetFile =path + "/" + curTime.toString("yyyyMMddhhmmss") + ".wav";
    }
    QString targetTime = curTime.toString("yyyy-MM-dd hh:mm:ss");
    qDebug() << TEMP_RECORD_FILE << targetFile;

    if(QFileInfo(targetFile).completeSuffix() == "wav"){
        QFile::rename(TEMP_RECORD_FILE, targetFile);
        if(QFile(TEMP_RECORD_FILE_MP3).exists()){
            QFile(TEMP_RECORD_FILE_MP3).remove();
        }
        if(QFile(TEMP_RECORD_FILE_M4A).exists()){
            QFile(TEMP_RECORD_FILE_M4A).remove();
        }
    }else if(QFileInfo(targetFile).completeSuffix() == "mp3"){
        QFile::rename(TEMP_RECORD_FILE_MP3, targetFile);
        if(QFile(TEMP_RECORD_FILE).exists()){
            QFile(TEMP_RECORD_FILE).remove();;
        }
        if(QFile(TEMP_RECORD_FILE_M4A).exists()){
            QFile(TEMP_RECORD_FILE_M4A).remove();
        }
    }else if(QFileInfo(targetFile).completeSuffix() == "m4a"){
        QFile::rename(TEMP_RECORD_FILE_M4A, targetFile);
        if(QFile(TEMP_RECORD_FILE_MP3).exists()){
            QFile(TEMP_RECORD_FILE_MP3).remove();
        }
        if(QFile(TEMP_RECORD_FILE).exists()){
            QFile(TEMP_RECORD_FILE).remove();
        }
    }

    emit DATABASE_INSTANCE.addNewFile(targetFile,targetTime);
    emit finish_ClipSignal();
    emit addListSignal(targetFile);
#endif
}

void ClipWidget::cover_CurRecord()
{
#ifndef WHITE_TEST
    QString targetFile = m_clipPath;
    QString path  = MainWindow::mutual->m_defaultPathData->get("path").toString();;
    bool res = generateClipRecord(path);
    if(!res){
        return ;
    }
    //弹窗
    QMessageBox msg3(QMessageBox::Question, tr("Hint"),
                     tr("This will overwrite the original file path,are you sure?"), QMessageBox::NoButton);
    msg3.setParent(MainWindow::mutual);
    msg3.addButton(QMessageBox::Cancel);
    msg3.setEscapeButton(QMessageBox::Cancel);
    QPushButton *btnYes = msg3.addButton(tr("OK"), QMessageBox::YesRole);
    msg3.exec();
    if ((QPushButton *)msg3.clickedButton() == btnYes) {
        QFile::remove(targetFile);
        int fmtIndex = MainWindow::mutual->m_setWindow.m_formatDownList->currentIndex();

        if (fmtIndex == 0) { // 设置格式：MP3
            targetFile = QFileInfo(targetFile).absolutePath() + "/" + QFileInfo(targetFile).baseName() + ".mp3";
            QFile::rename(TEMP_RECORD_FILE_MP3, targetFile);
            QString wavFilePath = QFileInfo(targetFile).absolutePath() + "/." + QFileInfo(targetFile).baseName() + ".wav";
            QFile::rename(TEMP_RECORD_FILE, wavFilePath);
        } else if (fmtIndex == 1) { // 设置格式：M4A
            targetFile = QFileInfo(targetFile).absolutePath() + "/" + QFileInfo(targetFile).baseName() + ".m4a";
            QFile::rename(TEMP_RECORD_FILE_M4A, targetFile);
            QString wavFilePath = QFileInfo(targetFile).absolutePath() + "/." + QFileInfo(targetFile).baseName() + ".wav";
            QFile::rename(TEMP_RECORD_FILE, wavFilePath);
        } else { // 默认：WAV
            targetFile = QFileInfo(targetFile).absolutePath() + "/" + QFileInfo(targetFile).baseName() + ".wav";
            QFile::rename(TEMP_RECORD_FILE, targetFile);
        }

        QString name = m_clipPath.split("/").last();
        QFileInfo info(targetFile);
        QString timeStr = info.created().toString("yyyy-MM-dd hh:mm:ss");
        qDebug() << name << timeStr;
        DATABASE_INSTANCE.deleteDataSlot(m_clipPath);
        DATABASE_INSTANCE.addNewFile(targetFile, timeStr);
        emit finish_ClipSignal();
        emit coverUpdateListSignal(targetFile);
    } else {
        emit cancel_ClipSignal();
    }
#endif
}

void ClipWidget::processFinish(int x)
{
}

//点试听按钮时再开始播放，防止没开始播放音频会导致获取当前播放歌曲的总长度为0
void ClipWidget::play_ClipFile()
{
    bottomLayout->insertWidget(3, clip_pauseBtn, 0, Qt::AlignCenter);
    clip_pauseBtn->show();
    bottomLayout->removeWidget(clip_playBtn);
    clip_playBtn->setParent(NULL);
    clipMovePos = leftBtn->pos().rx();
//    PlayControl::getInstance().setPosition(m_currentStartPos);
    qInfo() << "play_ClipFile m_currentStartPos:" << m_currentStartPos;
    PlayControl::getInstance().play_Audio(m_clipPath, m_currentStartPos); //点试听按钮时再开始播放，防止没开始播放音频会导致获取当前播放歌曲的总长度为0
//    PlayControl::getInstance().setPosition(m_currentStartPos);
qInfo() << "-------------------play_ClipFile";
    PlayControl::getInstance().clipMoveTimer->start(getInterval_Time(m_clipPath)); //播放时,游标也从左按钮移动的位置处开始
}

void ClipWidget::pause_ClipFile()
{
    qInfo() << "pause_ClipFile";

    bottomLayout->insertWidget(3, clip_playBtn, 0, Qt::AlignCenter);
    bottomLayout->removeWidget(clip_pauseBtn);
    clip_pauseBtn->setParent(NULL);
    PlayControl::getInstance().getPlayer()->stop();
    PlayControl::getInstance().clipMoveTimer->stop();
}

void ClipWidget::leftBtn_Move_Slot(int leftPos_x)
{
    //    qDebug()<<"左按钮移动:";
    Arightx = leftPos_x + leftBtn->width();
    Bleftx = rightBtn->pos().rx();
    //    qDebug()<<"左箭头的位置:"<<leftPos_x<<"左箭头的右侧:"<<Arightx<<";"<<"右箭头的左侧:"<<Bleftx;

    if (Arightx >= Bleftx - 5) {
        leftPos_x = Bleftx - 5 - leftBtn->width();
    }
    if (leftBtn->pos().rx() == leftPos_x) {
        return;
    }
    leftBtn->move(leftPos_x, leftBtn->pos().ry());
}

void ClipWidget::rightBtn_Move_Slot(int RightPos_x)
{
    //    qDebug()<<"右按钮移动:"<<RightPos_x;
    Arightx = leftBtn->pos().rx() + leftBtn->width();
    //    qDebug()<<"右箭头的左侧:"<<RightPos_x<<";"<<"左箭头的右侧:"<<Arightx;

    if (RightPos_x <= Arightx + 5) {
        RightPos_x = Arightx + 5;
    }
    if (rightBtn->pos().rx() == RightPos_x) {

        return;

    } else if (RightPos_x > railWid->width()) {

        rightBtn->move(railWid->width() - rightBtn->width() - 1, rightBtn->pos().ry());
        return;
    }
    rightBtn->move(RightPos_x, rightBtn->pos().ry());
}

void ClipWidget::cursorMove()
{
    cursor->raise();                                              //持续让游标置顶
    if (clipMovePos < (rightBtn->pos().rx() + rightBtn->width())) //游标位置到右滑块的右边界后停止
    {
        cursor->move(clipMovePos, 0);
        clipMovePos++;
    } else {
        qDebug() << "停止了" << clipMovePos << "右滑块右边界" << rightBtn->pos().rx() + rightBtn->width();
        PlayControl::getInstance().getPlayer()->stop();
        PlayControl::getInstance().clipMoveTimer->stop();
        bottomLayout->insertWidget(3, clip_playBtn, 0, Qt::AlignCenter);
        bottomLayout->removeWidget(clip_pauseBtn);
        clip_pauseBtn->setParent(NULL);
        clipMovePos = 0;
    }
}

void ClipWidget::itemFontStyle(QFont fontStyle)
{
    qDebug() << "font change.";
    dur_timelb->setFont(fontStyle);     //音频时间节点(开始/结束)
    cancelButton->setFont(fontStyle);   //取消
    finishButton->setFont(fontStyle);   //完成

    //改变字体大小时 截断字体省略显示
    this->displayBtn(finishButton,tr("Finish"));
}


void ClipWidget::changeCursorColorSlots()
{
    QColor color = QApplication::palette().color(QPalette::Active, QPalette::Highlight);
    QString colorRGB = QString("rgba(%1, %2, %3)").arg(color.red()).arg(color.green()).arg(color.blue());
    QString curStyle = QString("background-color:%1;").arg(colorRGB);
    cursor->setStyleSheet(curStyle);
}


ClipWidget::~ClipWidget()
{
    qDebug() << "释放:ClipWidget";
    this->deleteLater();
}
