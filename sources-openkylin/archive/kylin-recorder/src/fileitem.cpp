#include "fileitem.h"

#include "mythread.h"
#include "mainwindow.h"
#include "tools.h"
#include "clipwidget.h"
#include "audiofileprocess.h"
#include <QApplication>
#include <QTimer>
#include "libkydate.h"
#include <libkydatacollect.h>

namespace {
constexpr int kDefaultListItemHeight = 66;
constexpr int kMinimumTextLabelHeight = 32;
constexpr int kTextLabelPadding = 4;
constexpr int kListItemVerticalPadding = 12;
}

QString FileItem::m_lastSavePath = "/";
FileItem* FileItem::s_currentplaying = nullptr;

FileItem::FileItem(QString fileAbsolutePath, QWidget *parent) : QStackedWidget(parent)
{
    m_filePath = fileAbsolutePath; //每个Item的文件绝对路径
    this->setFixedWidth(500);
    initItemWid();                 //初始化ItemWid
    setItemWid();                  //设置ItemWid的界面
    initThemeGsetting();
}

void FileItem::initItemWid()
{
    m_pal = palette();                         //声明调色板
    m_splitLine = new QFrame(this);            //分割线
    m_splitLine->setAccessibleName("kylin-recorder_fileitem_QFrame_m_splitLine");
    m_splitLineWidget = new QWidget(this);
    m_splitLineWidget->setAccessibleName("kylin-recorder_fileitem_QWidget_m_splitLineWidget");
    m_splitLineLayout = new QVBoxLayout(this);
    m_playSlider = new CustomSlider(this);     //播放划块条
    m_playSlider->setAccessibleName("kylin-recorder_fileitem_CustomSlider_m_playSlider");
    m_recordFileNameLabel = new MyLabel(this); //录音文件名
    m_recordFileNameLabel->setAccessibleName("kylin-recorder_fileitem_MyLabel_m_recordFileNameLabel");
    m_startRecordTimeLabel = new QLabel(this); //录音时间标签
    m_startRecordTimeLabel->setAccessibleName("kylin-recorder_QLabel_m_startRecordTimeLabel");
    m_timelengthLabel = new QLabel(this);      //时间长度标签
    m_timelengthLabel->setAccessibleName("kylin-recorder_QLabel_m_timelengthLabel");
    m_seatLabel = new QLabel(this);            //占位标签
    m_seatLabel->setAccessibleName("kylin-recorder_QLabel_m_seatLabel");

    m_pal.setColor(QPalette::Text, QColor(140, 140, 140));
    m_startRecordTimeLabel->setPalette(m_pal);
    m_timelengthLabel->setPalette(m_pal);
    //Buttons
    m_playButton = new QPushButton(this);
    m_playButton->setAccessibleName("kylin-recorder_QPushButton_m_playButton");
    m_playButton->setFixedSize(30, 30);
    m_playButton->setToolTip(tr("Play"));
    m_playButton->setIcon(QIcon::fromTheme("media-playback-start-symbolic")); //主题库的开始图标

    m_pauseButton = new QPushButton(this);
    m_pauseButton->setAccessibleName("kylin-recorder_QPushButton_m_pauseButton");
    m_pauseButton->setFixedSize(30, 30);
    m_pauseButton->setToolTip(tr("Pause"));
    m_pauseButton->setIcon(QIcon::fromTheme("media-playback-pause-symbolic")); //主题库的暂停图标
    m_pauseButton->hide();

    m_deleteButton = new QPushButton(this);
    m_deleteButton->setAccessibleName("kylin-recorder_QPushButton_m_deleteButton");
    m_deleteButton->setFixedSize(30, 30);
    m_deleteButton->setToolTip(tr("Delete"));
    m_deleteButton->setIcon(QIcon::fromTheme("edit-delete-symbolic")); //主题库的删除图标

    m_clipButton = new QPushButton(this);
    m_clipButton->setAccessibleName("kylin-recorder_QPushButton_m_clipButton");
    m_clipButton->setFixedSize(30, 30);
    m_clipButton->setToolTip(tr("Clip"));
    m_clipButton->setIcon(QIcon::fromTheme("ukui-object-crop-symbolic")); //主题库的剪裁图标

    m_flagButton = new QPushButton(this);
    m_flagButton->setAccessibleName("kylin-recorder_QPushButton_m_flagButton");
    m_flagButton->setFixedSize(30, 30);
    m_flagButton->setToolTip(tr("Flag"));
    m_flagButton->setIcon(QIcon::fromTheme("ukui-label-symbolic")); //主题库的标记图标
    m_flagButton->setEnabled(false);                                //音频未播放时不可添加标记

    m_splitLinestackWidget = new QStackedWidget(this); //分割线堆叠wid
    m_splitLinestackWidget->setAccessibleName("kylin-recorder_fileitem_QStackedWidget_m_splitLinestackWidget");
    m_splitLinestackWidget->setFixedWidth(490);
    m_buttonStackWidget = new QStackedWidget(this);    //按钮的堆叠布局
    m_buttonStackWidget->setAccessibleName("kylin-recorder_fileitem_QStackedWidget_m_buttonStackWidget");
   // this = new QStackedWidget(this);  //整个item界面的堆叠布局(包括播放、剪裁两个界面)
    m_buttonStackWidget->installEventFilter(this); //给stackWid安装事件过滤器
    m_firstWidget = new QWidget(this);             //时间长短第一层Wid
    m_firstWidget->setAccessibleName("kylin-recorder_fileitem_QWidget_m_firstWidget");
    m_threeButtonWidget = new QWidget(this);       //三个按钮的Wid
    m_threeButtonWidget->setAccessibleName("kylin-recorder_fileitem_QWidget_m_threeButtonWidget");

    m_itemBottomWidget = new QWidget(this); //列表项的底部Wid，有label和button两组控件
    m_itemBottomWidget->setAccessibleName("kylin-recorder_fileitem_QWidget_m_itemBottomWidget");
    m_itemWidget = new QWidget(this);
    m_itemWidget->setAccessibleName("kylin-recorder_fileitem_QWidget_m_itemWidget");
    m_fileNameDateTimeWidget = new QWidget(this); //文件名和日期时间的Wid
    m_fileNameDateTimeWidget->setAccessibleName("kylin-recorder_fileitem_QWidget_m_fileNameDateTimeWidget");

    m_itemBottomLayout = new QHBoxLayout(m_itemBottomWidget);             //列表项的底部布局
    m_fileNameDateTimeLayout = new QVBoxLayout(m_fileNameDateTimeWidget); //文件名和录制时间布局
    m_timeLengthLayout = new QVBoxLayout(m_firstWidget);                  //时间的布局
    m_threeButtonLayout = new QHBoxLayout(m_threeButtonWidget);           //三个按钮布局
    m_itemLayout = new QVBoxLayout(m_itemWidget);

    connect(m_playButton, &CustomButton::clicked, this, &FileItem::handlePlay);
    connect(m_pauseButton, &CustomButton::clicked, this, &FileItem::handlePause);
    connect(m_clipButton, &QToolButton::clicked, this, &FileItem::toClipPage);
    connect(m_deleteButton, &CustomButton::clicked, this, &FileItem::delAudioFile);
    connect(m_playSlider, &CustomSlider::sliderReleasedAt, this, &FileItem::setSliderPos);
    connect(m_flagButton, &CustomButton::clicked, this, &FileItem::toFlagFile);
    // 确保一次触发多次使用,所有的item的文件都随字号变化,所以放在FileItem里
    connect(MainWindow::mutual, &MainWindow::itemFontStyleSignal, this, &FileItem::itemFontStyle);
    connect(this, &FileItem::addListItemSignal, MainWindow::mutual, &MainWindow::slotListItemAdd);
    connect(this, &FileItem::currentPos_PlaySignal, this, &FileItem::setSliderPos);
    connect(this, &FileItem::startdelete, MainWindow::mutual->m_subThread, &MyThread::delAudioFile);

    m_fileWather = new QFileSystemWatcher(this);
    m_fileWather->addPath(getfilePath());
    connect(m_fileWather, &QFileSystemWatcher::fileChanged, this, &FileItem::monitorFileChange);


    QDBusConnection::sessionBus().connect("com.kylin.kysdk.DateServer",
                                          "/com/kylin/kysdk/Date",
                                          "com.kylin.kysdk.DateInterface",
                                          "LongDateSignal",
                                          this,
                                          SLOT(updateDateFormat(QString)));

}

void FileItem::monitorFileChange()
{
#ifndef WHITE_TEST
    qDebug() << "文件目录发生改变";
    QHash<QString, bool> fileExistsMap;
    fileExistsMap.clear();
    for (int i = 0; i < MainWindow::mutual->m_list->count(); i++) {
        QListWidgetItem* item = MainWindow::mutual->m_list->item(i);
        QString filePath = item->data(Qt::UserRole).toString();
        qInfo()<<"monitorFileChange:"<<filePath;
        fileExistsMap.insert(filePath, false);
    }
    QDir dir(QFileInfo(m_filePath).path());
    QStringList fileList = dir.entryList(QDir::Files);
    int i = 0;
    foreach (QString file, fileList) {
        QString filePath = dir.absoluteFilePath(file);
        qInfo()<<"i:"<<i++<<", file:"<<file<<", filePath:"<<filePath;
        if (fileExistsMap.contains(filePath)) {
            qInfo()<<" ----------------------filePath:"<<filePath;
            fileExistsMap.insert(filePath, true);
        }
    }
    for (auto iter = fileExistsMap.begin(); iter != fileExistsMap.end(); ++iter) {
        if (iter.key() == m_filePath && !iter.value()) {
            int x = this->parent()->findChildren<FileItem *>().indexOf(this);
            qDebug() << "第" << x << "个不存在,直接删除:";
            QStringList files;
            QStringList times;
            DATABASE_INSTANCE.getAllFiles(files,times);
            if(files.count() != times.count()){
                qDebug() << "Error: database count do not match...";
            }
            int fileCount = 0;
            for(auto i = 0; i < files.count();i++) {
                QFileInfo info(files.at(i));
                if(!info.exists()){
                    continue;
                }
                fileCount ++ ;
            }
            qDebug() << "fileCount" << fileCount;
            delete MainWindow::mutual->m_list->takeItem(MainWindow::mutual->m_list->count() - 1 - x); //删除list列表的item操作
            if(fileCount == 0){
                MainWindow::mutual->m_list->clear();
            }
            MainWindow::mutual->isFileNull(MainWindow::mutual->m_list->count());
            return;
        }
    }
#endif
}

void FileItem::updateDateFormat(QString dateFormat)
{
    Q_UNUSED(dateFormat)
    tm timeFormat;
    timeFormat.tm_year = m_recordTime.date().year();
    timeFormat.tm_mon = m_recordTime.date().month();
    timeFormat.tm_mday = m_recordTime.date().day();
    QString d = kdk_system_shortformat_transform(&timeFormat);
    QString date;
    date = d.append(" ").append(m_recordTime.toString("hh:mm:ss"));

    m_startRecordTimeLabel->setText(date);
}

int FileItem::getInternalMoveTime()
{
//    QFile file(m_filePath);
    QTime times = QTime::fromString(m_timelengthLabel->text());
    int time = ((times.hour() * 60 * 60) + (times.minute() * 60) + times.second());
    int x = 1;
    if (time >= 1) {
        x = time * 1000 / m_playSlider->maximum();
    } else {
        x = (time + 1) * 1000 / m_playSlider->maximum();
    }
    qDebug() << "间隔" << x << "ms运动一次";
    return x;
}

void FileItem::handlePlay()
{
#ifndef WHITE_TEST
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"Handle_Play", (char *)"fileitem");
    kdk_dia_data_free(node);
    MainWindow::mutual->recorderBuriedPoint(kdk::kabase::BuriedPoint::KylinRecorderPlayPause);
    MainWindow::mutual->setFocus();


    // 表示切换了播放文件
    if (PlayControl::getInstance().getPlayPath() != m_filePath) {
        m_nowPos = 0;
        m_lastPos = 0;
        // 切换文件播放前，先关闭
    }

    //取此类的父类的所有FileItem类；findChildren：找所有的子类，findChild为找一个子类
    QList<FileItem *> items = this->parent()->findChildren<FileItem *>();
    qDebug() << "item的个数：" << items.count();
    for (FileItem *item : items) {
        item->isOtherClipWidOpen();
    }
    if (Tools::fileExists(m_filePath)) {
        m_flagButton->setEnabled(true); //音频播放时可添加标记
        m_threeButtonLayout->insertWidget(0, m_pauseButton);
        m_pauseButton->show();
        m_threeButtonLayout->removeWidget(m_playButton);
        m_playButton->setParent(NULL);

        displayFlag(m_filePath); //显示标记
        m_internalTime = getInternalMoveTime();
        PlayControl::getInstance().moveTimer->start(m_internalTime); //每隔n秒进度加1
        PlayControl::getInstance().play_Audio(m_filePath);
        m_splitLinestackWidget->setCurrentIndex(1);

        s_currentplaying = this;
        m_playRecorder = true;
        m_pauseState = false;

        connect(PlayControl::getInstance().moveTimer, &QTimer::timeout, this, &FileItem::handleMove); //用的时候连接
        connect(PlayControl::getInstance().getPlayer(), &MMediaPlayer::stateChanged, this, &FileItem::stateChanged); //用的时候连接
    } else {
        QMessageBox::warning(MainWindow::mutual, tr("Warning"),
                             tr("The file path does not exist or has been deleted!"));
    }
#endif
}

void FileItem::handlePause()
{
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"Handle_Pause", (char *)"fileitem");
    kdk_dia_data_free(node);
    MainWindow::mutual->recorderBuriedPoint(kdk::kabase::BuriedPoint::KylinRecorderPlayPause);
    qDebug() << "暂停的路径:" << m_filePath;
    m_flagButton->setEnabled(false); //音频未播放时不可添加标记

    m_threeButtonLayout->insertWidget(0, m_playButton);
    m_pauseButton->hide();
    m_threeButtonLayout->removeWidget(m_pauseButton);
    m_pauseButton->setParent(NULL);

    PlayControl::getInstance().pause_Audio();
    m_splitLinestackWidget->setCurrentIndex(0);

    if(s_currentplaying == this){
        s_currentplaying = nullptr;
    }
    m_pauseState = true;

    PlayControl::getInstance().moveTimer->stop();
    connect(PlayControl::getInstance().getPlayer(), &MMediaPlayer::stateChanged, this,
            &FileItem::stateChanged); //用的时候连接
}

void FileItem::sendFlagView(){
	KCustomProperty property[1];	
	property[0].key = (char *)"flagName";
    	property[0].value = (char *)"leftButton";	
	KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
	kdk_dia_append_custom_property(node, property, 1);
	kdk_dia_upload_default(node, (char *)"audio_flag_event", (char *)"fileitem");
	kdk_dia_data_free(node);
}

//文件列表中给播放的音频添加标记
void FileItem::toFlagFile()
{
    MainWindow::mutual->recorderBuriedPoint(kdk::kabase::BuriedPoint::KylinRecorderSign);
    QFileInfo fileInfo(m_filePath);

    QTime times = QTime::fromString(m_timelengthLabel->text());

//    int totalTime = fileInfo.size() / 96000;
    int totalTime = (times.hour() * 60 * 60) + (times.minute() * 60) + times.second();
    if (totalTime < 1) {
        totalTime = 1;
    }
    int curTime = totalTime * m_movePos / m_playSlider->maximum();
    QTime showTime(0, 0, 0);
    showTime = showTime.addSecs(curTime);
    QString timeStr = showTime.toString("hh:mm:ss");
    qDebug() << "获取当前无后缀的文件名：" << fileInfo.baseName() << "获取当前时间:" << timeStr;

    emit DATABASE_INSTANCE.insertDataSignal(fileInfo.absoluteFilePath(), timeStr);

    if (DATABASE_INSTANCE.getStatus() == DB_OP_SUCC) {
        qDebug() << "添加成功";
	this->sendFlagView();
        displayFlag(m_filePath);
    }
}

void FileItem::updateLabelNameSlots(QString newfilepath)
{
#ifndef WHITE_TEST
    m_recordFileNameLabel->setText(tr("Recorder")+ m_renameWidget->getNewName());
    emit updateName(tr("Recorder")+ m_renameWidget->getNewName());
    m_filePath = newfilepath;
#endif
}

void FileItem::displayFlag(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
//    int time_T = fileInfo.size() / 96000;
    QTime times = QTime::fromString(m_timelengthLabel->text());
    int time_T = (times.hour() * 60 * 60) + (times.minute() * 60) + times.second();
    if (time_T < 1) {
        time_T = 1;
    }
    emit DATABASE_INSTANCE.searchDataSignal(fileInfo.absoluteFilePath());
    if (!DATABASE_INSTANCE.getDataList().isEmpty()) {
#ifndef WHITE_TEST
        QStringList flagList = DATABASE_INSTANCE.getDataList();
        for (int i = 0; i < flagList.count(); i++) {
            QString timeStr = flagList.at(i);
            QPushButton *flag = new QPushButton(m_playSlider);
            flag->setAccessibleName("kylin-recorder_QPushButton_flag");
            flag->setObjectName("flag");
            connect(flag, &QPushButton::clicked, this, &FileItem::currentPos_Play);
            const QSize BUTTON_SIZE = QSize(3, 6);
            flag->resize(BUTTON_SIZE);
            flag->setToolTip(timeStr);

            m_pal.setColor(QPalette::Button,QApplication::palette().highlight().color());
            flag->setPalette(m_pal);
            QTime time = QTime::fromString(timeStr);
            // 灰色标签：CustomSlider 用的是 midlight
            QColor gray = QApplication::palette().midlight().color();
            // 用 QSS 强制直角 + 同色填充（比 setPalette 更不受主题按钮样式影响）
            flag->setStyleSheet(QString(
                                    "QPushButton#flag{"
                                    "background-color:%1;"
                                    "border:none;"
                                    "border-radius:0px;"
                                    "padding:0px;"
                                    "margin:0px;"
                                    "}"
                                    ).arg(gray.name()));
            int cur_Time = time.hour() * 3600 + time.minute() * 60 + time.second();
            qDebug() << "秒数:" << cur_Time;
            qDebug() << "playSlider位置:" << m_playSlider->pos() << "flag位置:" << flag->pos() << "playSlider的宽度"
                     << m_playSlider->width();
            flag->move(cur_Time * m_playSlider->width() / time_T, m_playSlider->pos().ry() + 10);
            flag->show();
        }
#endif

    }
    qDebug() << "此文件名:" << m_filePath << "查询的结果集:" << DATABASE_INSTANCE.getDataList();
}

void FileItem::currentPos_Play()
{
#ifndef WHITE_TEST

    QPushButton *btn = qobject_cast<QPushButton *>(sender()); //获取发射信号的对象
    if (btn->objectName() == "flag") {
        qDebug() << "当前按钮位置坐标" << btn->pos() << "playSlider的宽度:" << m_playSlider->width();
        emit currentPos_PlaySignal(btn->pos().rx() * m_playSlider->maximum() / m_playSlider->width());
    }
#endif
}

void FileItem::setSliderPos(qint64 value)
{
    m_nowPosKeep = false;
    //计算公式：当前player进度 = player总进度 * playSlider的当前值 / playSlider总长度
    qint64 currentPlayerPos = PlayControl::getInstance().getPlayer()->duration() * value / m_playSlider->maximum();
    qDebug() << "滑动条value:" << value << "当前player进度:" << currentPlayerPos;

//    PlayControl::getInstance().play_Audio(m_filePath);
    if (PlayControl::getInstance().getAudioState() == QMediaPlayer::PlayingState) {
        qDebug() << "播放的位置";
        m_movePos = value; //相对位置+按钮宽度=游标的位置
        m_playSlider->setValue(m_movePos);
    } else {
        qDebug() << "没播放过从0开始";
        m_movePos = 0;
    }
    PlayControl::getInstance().setPosition(currentPlayerPos);
    m_internalTime = getInternalMoveTime();
    PlayControl::getInstance().moveTimer->start(m_internalTime); //每隔n秒进度加1
}


void FileItem::handleMove()
{
#ifndef WHITE_TEST
//     qInfo() << "PlayControl::getInstance().getPlayer()->position():" << PlayControl::getInstance().getPlayer()->position();
//     qInfo() << "PlayControl::getInstance().getPlayer()->duration():" << PlayControl::getInstance().getPlayer()->duration();

    // 1,处理暂停/播放场景，mpv会处理缓冲帧，导致时间戳异常，所有一开始直接增加
    // 2,mpv的时间戳变化比较慢，如果按照原本的时间戳进行变化，进度条看上去不连续，所以自动加了个时间间隔
    if (m_nowPosKeep) {
        m_nowPos += m_internalTime;
    }
    else if (m_lastPos == (double)PlayControl::getInstance().getPlayer()->position()) {
        m_nowPos += m_internalTime;
    } else {
        m_nowPos = (double)PlayControl::getInstance().getPlayer()->position();
        m_lastPos = (double)PlayControl::getInstance().getPlayer()->position();
    }

    // mpv的duration有可能慢一点，导致这个值变成0，所以这里等于一下。如果mpv返回的duration真的是0，会直接停止播放
    if (PlayControl::getInstance().getPlayer()->duration() == 0) {
        m_movePos = 0;
    } else {
        m_movePos = m_nowPos /
                    (double)PlayControl::getInstance().getPlayer()->duration() *
                    m_playSlider->maximum();
    }

//    qInfo() << "m_nowPos:" << m_nowPos << ", m_movePos:" << m_movePos;


    if ((m_movePos < m_playSlider->maximum() - 1)) {
        m_movePos++;
        m_playSlider->setValue(m_movePos);
    } else {
        qDebug() << "停止了";
        PlayControl::getInstance().moveTimer->stop();
        PlayControl::getInstance().stop();
        m_movePos = 0;
        m_playRecorder = false;
        m_pauseState = false;
    }
#endif
}

void FileItem::stateChanged(QMediaPlayer::State stateNow)
{
#ifndef WHITE_TEST

    if (PlayControl::getInstance().getAudioState() == QMediaPlayer::StoppedState) {
        qDebug() << "列表播放结束 停止" << m_filePath;
        m_movePos = 0;
        m_nowPos = 0;
        m_lastPos = 0;
        m_playSlider->setValue(m_movePos);
        m_splitLinestackWidget->setCurrentIndex(0);
        m_threeButtonLayout->insertWidget(0, m_playButton);
        m_threeButtonLayout->removeWidget(m_pauseButton);
        m_pauseButton->setParent(NULL);
        m_flagButton->setEnabled(false); //音频未播放时不可添加标记
        m_playRecorder = false;
        m_pauseState = false;
    }

    if (lastState == QMediaPlayer::PausedState && stateNow == QMediaPlayer::PlayingState) {
        m_nowPosKeep = true;
        QTimer::singleShot(100, [this]() {
            m_nowPosKeep = false;
        });
    }

    lastState = stateNow;

    disconnect(PlayControl::getInstance().moveTimer, &QTimer::timeout, this,
               &FileItem::handleMove); //防止多次连接，每次状态改变后需要断开连接
    disconnect(PlayControl::getInstance().getPlayer(), &MMediaPlayer::stateChanged, this,
               &FileItem::stateChanged); //防止多次连接，每次状态改变后需要断开连接
#endif
}

void FileItem::itemFontStyle(QFont fontStyle)
{
    m_startRecordTimeLabel->setFont(fontStyle);
    m_recordFileNameLabel->setFont(fontStyle);
    m_timelengthLabel->setFont(fontStyle);
    updateListItemHeight();
}

void FileItem::setItemWid()
{
    QString fontType = MainWindow::mutual->m_themeData->get("systemFont").toString();
    int fontSize = MainWindow::mutual->m_themeData->get("systemFontSize").toInt();
    QFont fontStyle(fontType, fontSize);
    m_recordFileNameLabel->setFont(fontStyle);
    m_startRecordTimeLabel->setFont(fontStyle); //录音时间标签
    m_timelengthLabel->setFont(fontStyle);      //时间长度标签

    m_splitLine->setFixedHeight(1);
    m_splitLine->setStyleSheet("background-color:#F0F0F0;"); //分割线
    m_splitLineLayout->addWidget(m_splitLine);
    m_splitLineLayout->setContentsMargins(30, 0, 0, 0);
    m_splitLineWidget->setLayout(m_splitLineLayout);

    m_fileNameDateTimeLayout->addWidget(m_recordFileNameLabel);
    m_fileNameDateTimeLayout->addWidget(m_startRecordTimeLabel);
    m_fileNameDateTimeLayout->setSpacing(0);                  //调整这俩Label的内距
    m_fileNameDateTimeLayout->setContentsMargins(0, 0, 0, 0); //调整这俩Label的外距
    m_fileNameDateTimeWidget->setLayout(m_fileNameDateTimeLayout);
    m_fileNameDateTimeWidget->setFixedWidth(270);

    m_timeLengthLayout->addWidget(m_seatLabel, 0, Qt::AlignRight);
    m_timeLengthLayout->addWidget(m_timelengthLabel, 0, Qt::AlignRight);
    m_timeLengthLayout->setSpacing(0);
    m_timeLengthLayout->setContentsMargins(0, 0, 0, 0);
    m_firstWidget->setLayout(m_timeLengthLayout);
    m_firstWidget->setFixedWidth(160);

    m_splitLinestackWidget->addWidget(m_splitLineWidget);
    m_splitLinestackWidget->addWidget(m_playSlider);

    m_threeButtonLayout->addWidget(m_playButton); // 2020.11.20 由于依赖问题此播放问题解决。
    m_threeButtonLayout->removeWidget(m_pauseButton);
    m_threeButtonLayout->addWidget(m_clipButton);
    m_threeButtonLayout->addWidget(m_flagButton);
    m_threeButtonLayout->addWidget(m_deleteButton, 0, Qt::AlignRight);
    m_threeButtonLayout->setContentsMargins(20, 0, 0, 0);
    m_threeButtonWidget->setLayout(m_threeButtonLayout);

    m_buttonStackWidget->addWidget(m_firstWidget);
    m_buttonStackWidget->addWidget(m_threeButtonWidget);

    this->setContentsMargins(0,0,0,0);
    //this->setSpacing(0);
    this->addWidget(m_itemWidget);

    m_itemBottomLayout->setMargin(8);
    m_itemBottomLayout->addWidget(m_fileNameDateTimeWidget);               //录音名与开始录制时间的布局
    m_itemBottomLayout->addWidget(m_buttonStackWidget, 0, Qt::AlignRight); //按钮和时间长度的堆叠布局
    m_itemBottomLayout->setSpacing(0);
    m_itemBottomLayout->setContentsMargins(30, 0, 20, 0);
    m_itemBottomWidget->setLayout(m_itemBottomLayout);

    m_itemLayout->addWidget(m_splitLinestackWidget, 0, Qt::AlignVCenter);
    m_itemLayout->addSpacing(-15);
    m_itemLayout->addWidget(m_itemBottomWidget);
    m_itemLayout->setSpacing(0);
    m_itemLayout->setContentsMargins(0, 0, 0, 0);
    m_itemWidget->setLayout(m_itemLayout);
    updateListItemHeight();
}

void FileItem::initThemeGsetting()
{
    connect(MainWindow::mutual->m_themeData, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "styleName") {
            m_itemThemeColor = MainWindow::mutual->m_themeData->get("styleName").toString();
            themeStyle(m_itemThemeColor);
        }
    });
    themeStyle(MainWindow::mutual->m_themeData->get("styleName").toString());
}

void FileItem::themeStyle(QString themeColor)
{
    if (themeColor == "ukui-dark" || themeColor == "ukui-black") {
        m_splitLine->setStyleSheet("background-color:#262626;");
    } else {
        m_splitLine->setStyleSheet("background-color:#F0F0F0;");
    }
}

//void FileItem::setName_Time_Length(QString nameStr, QString timeStr, QString lengthStr)
void FileItem::setName_Time_Length(QString nameStr, QDateTime recordTime, QString lengthStr)
{
    m_recordTime = recordTime;
    // lc change
    tm timeFormat;
    timeFormat.tm_year = recordTime.date().year();
    timeFormat.tm_mon = recordTime.date().month();
    timeFormat.tm_mday = recordTime.date().day();
    QString d = kdk_system_shortformat_transform(&timeFormat);
    QString date;
    date = d.append(" ").append(recordTime.toString("hh:mm:ss"));

    m_startRecordTimeLabel->setText(date);
    m_recordFileNameLabel->setText(nameStr);
    //m_startRecordTimeLabel->setText(timeStr);
    m_timelengthLabel->setText(lengthStr);
    updateListItemHeight();
}

int FileItem::textLabelHeight() const
{
    const int labelFontHeight = qMax(QFontMetrics(m_recordFileNameLabel->font()).height(),
                                     QFontMetrics(m_startRecordTimeLabel->font()).height());
    return qMax(kMinimumTextLabelHeight, labelFontHeight + kTextLabelPadding);
}

int FileItem::listItemHeight() const
{
    return qMax(kDefaultListItemHeight, textLabelHeight() * 2 + kListItemVerticalPadding);
}

void FileItem::updateListItemHeight()
{
    const int labelHeight = textLabelHeight();
    const int itemHeight = listItemHeight();

    m_recordFileNameLabel->setFixedHeight(labelHeight);
    m_startRecordTimeLabel->setFixedHeight(labelHeight);
    setFixedHeight(itemHeight);

    if (MainWindow::mutual == nullptr || MainWindow::mutual->m_list == nullptr) {
        return;
    }

    for (int i = 0; i < MainWindow::mutual->m_list->count(); ++i) {
        QListWidgetItem *item = MainWindow::mutual->m_list->item(i);
        if (MainWindow::mutual->m_list->itemWidget(item) == this) {
            item->setSizeHint(QSize(item->sizeHint().width(), itemHeight));
            break;
        }
    }
}

QString FileItem::getfilePath()
{
    return m_filePath;
}

void FileItem::toClipPage()
{
#ifndef WHITE_TEST

    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"Clip_Page", (char *)"fileitem");
    kdk_dia_data_free(node);
    MainWindow::mutual->recorderBuriedPoint(kdk::kabase::BuriedPoint::KylinRecorderClip);
    PlayControl::getInstance().stop();
    QList<FileItem *> items = this->parent()->findChildren<FileItem *>(); //取此类的父类的所有FileItem类；findChildren：找所有的子类，findChild为找一个子类
    qDebug() << "item的个数：" << items.count();
    for (FileItem *item : items) {
        item->isOtherClipWidOpen();
    }

    ClipWidget *clipWid = new ClipWidget(m_filePath, this);
    clipWid->setAccessibleName("kylin-recorder_fileitem_ClipWidget_clipWid");
    clipWid->setObjectName("clipWid");
    int value_Type = clipWid->display_Wave();

    if (value_Type == 1) {
        this->addWidget(clipWid);
        connect(clipWid, &ClipWidget::cancel_ClipSignal, this, &FileItem::cancel);
        connect(clipWid, &ClipWidget::finish_ClipSignal, this, &FileItem::finish);
        connect(clipWid, &ClipWidget::coverUpdateListSignal, this, &FileItem::updateListDisplay);
        connect(clipWid, &ClipWidget::addListSignal,this,&FileItem::addListDisplay);
    } else if (value_Type == 0) {
        clipWid->deleteLater();
        QMessageBox::warning(MainWindow::mutual, tr("Warning"), tr("Time is too short"));
        return;
    } else if (value_Type == -2) {
        clipWid->deleteLater();
        QMessageBox::warning(MainWindow::mutual, tr("Warning"),
                             tr("The file path does not exist or has been deleted!"));
        return;
    } else if (value_Type == -1) {
        clipWid->deleteLater();
        //不能解析非录音生成的录音文件
        QMessageBox::warning(MainWindow::mutual, tr("Warning"),
                             tr("Unable to parse the waveform of audio file generated by non recorder！"));
        return;
    }
    this->setCurrentIndex(1);
#endif
}

void FileItem::updateListDisplay(QString filePath)
{
#ifndef WHITE_TEST

    //与6.1版本之前的数据不兼容，因数据库存储格式问题，历史音频无法正确显示，需要清空历史数据进行测试，数据库问题暂时不作处理；新出镜像测试不存在历史数据问题；
    int x = this->parent()->findChildren<FileItem *>().indexOf(this);
    delete MainWindow::mutual->m_list->takeItem(MainWindow::mutual->m_list->count() - 1 - x); //删除操作
    qDebug() << "文件名:" << filePath << "所在列表的索引:" << x;
    QFileInfo info(filePath);

    tm *timeFormat = new tm();
    QDateTime dateTime = QDateTime::currentDateTime();
    timeFormat->tm_year = dateTime.date().year();
    timeFormat->tm_mon = dateTime.date().month();
    timeFormat->tm_mday = dateTime.date().day();
    QString d = kdk_system_shortformat_transform(timeFormat);
    QString date;
    date = d.append(" hh:mm:ss");

    //fileName = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    //recordTime = dateTime.toString(date);
    emit addListItemSignal(filePath, dateTime);
#endif
}

void FileItem::addListDisplay(QString filePath)
{
#ifndef WHITE_TEST

    qDebug() << "文件名:" << filePath;
    QFileInfo info(filePath);

    tm *timeFormat = new tm();
    QDateTime dateTime = QDateTime::currentDateTime();
    timeFormat->tm_year = dateTime.date().year();
    timeFormat->tm_mon = dateTime.date().month();
    timeFormat->tm_mday = dateTime.date().day();
    QString d = kdk_system_shortformat_transform(timeFormat);
    QString date;
    date = d.append(" hh:mm:ss");
    emit addListItemSignal(filePath, info.created());
#endif
}

void FileItem::cancel()
{
#ifndef WHITE_TEST

    qDebug() << "剪裁取消"; //
    PlayControl::getInstance().setPosition(0);
    PlayControl::getInstance().clipMoveTimer->stop(); //游标随时间移动也要停止
    PlayControl::getInstance().getPlayer()->stop();
    if (this->currentWidget() != nullptr && this->currentIndex() == 1) {

        qDebug() << "有其他剪裁界面的释放掉" << this->currentWidget()->objectName();
        this->currentWidget()->deleteLater();
    }
    this->setCurrentIndex(0);
#endif
}

void FileItem::finish()
{
#ifndef WHITE_TEST

    qDebug() << "剪裁完成";
    PlayControl::getInstance().setPosition(0);
    PlayControl::getInstance().clipMoveTimer->stop(); //游标随时间移动也要停止
    PlayControl::getInstance().getPlayer()->stop();
    if (this->currentWidget() != nullptr && this->currentIndex() == 1) {
        qDebug() << "有其他剪裁界面的释放掉" << this->currentWidget()->objectName();
        this->currentWidget()->deleteLater();
    }
    this->setCurrentIndex(0);
#endif
}

void FileItem::isOtherClipWidOpen()
{
    if (this->currentIndex() != 0) //若有其他的剪裁界面则先将其他的收回去
    {
        qDebug() << "有其他剪裁界面可切换,state:" << PlayControl::getInstance().getAudioState();
        if (PlayControl::getInstance().getAudioState() == QMediaPlayer::PlayingState) {
            PlayControl::getInstance().clipMoveTimer->stop(); //游标随时间移动也要停止
            PlayControl::getInstance().getPlayer()->stop();
        }
        if (this->currentWidget() != nullptr && this->currentIndex() == 1) {
            qDebug() << "有其他剪裁界面的释放掉" << this->currentWidget()->objectName();
            this->currentWidget()->deleteLater();
        }
        this->setCurrentIndex(0);
    }
}

void FileItem::delAudioFile()
{
#ifndef WHITE_TEST
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CLICK);
    kdk_dia_upload_default(node, (char *)"Delete_Audio_File", (char *)"fileitem");
    kdk_dia_data_free(node);
    int fileCount = Tools::getRecordingFileinfos().count(); //获取不同目录下的所有文件个数
    QString delFilePath = m_filePath;                       // filePath作为FileItem的一个属性
    qDebug() << "删除的是：" << delFilePath; //前提是鼠标悬浮有焦点之后才可以锁定此FileItem的属性
    if (Tools::fileExists(delFilePath)) {
        if (PlayControl::getInstance().getPlayer()->state() == QMediaPlayer::PlayingState) {
            QMessageBox::warning(MainWindow::mutual, tr("Warning"), tr("Playing, please stop and delete!"));
            return;
        }
        //  qDebug()<<"输出删除之前的Item地址"<<this->parent()->findChildren<FileItem*>();
        int x = this->parent()->findChildren<FileItem *>().indexOf(this);

        delete MainWindow::mutual->m_list->takeItem(MainWindow::mutual->m_list->count() - 1 - x); //删除操作
        qDebug() << "路径存在，删除" << delFilePath;
        MainWindow::mutual->isFileNull(MainWindow::mutual->m_list->count()); //传item个数
        deleteFile(delFilePath);                                             //移入回收站
        //为保证还原后还在，数据库不删除，需要在读取时判断文件是否存在
        return;

    } else {

        int y = this->parent()->findChildren<FileItem *>().indexOf(this);
        qDebug() << "第" << y << "个不存在,直接删除:";
        delete MainWindow::mutual->m_list->takeItem(MainWindow::mutual->m_list->count() - 1 - y); //删除list列表的item操作
        MainWindow::mutual->isFileNull(MainWindow::mutual->m_list->count());               //传item个数
        return;
    }
#endif
}

void FileItem::deleteFile(QString savepath)
{
    MainWindow::mutual->recorderBuriedPoint(kdk::kabase::BuriedPoint::KylinRecorderDelete);
    _processStart("gio", QStringList() << "trash" << savepath);
}

void FileItem::_processStart(QString cmd, QStringList arguments)
{
    if (!m_processDEL) {
        m_processDEL = new QProcess(this);
    }
    m_processDEL->start(cmd, arguments);
    m_processDEL->waitForStarted();
    m_processDEL->waitForFinished();
}
void FileItem::rightClickedMenuRequest() //右击弹出Menu菜单
{
#ifndef WHITE_TEST

    qDebug() << "按压了右键!弹出Menu菜单";
    m_menu = new QMenu(MainWindow::mutual);
    m_menu->setAccessibleName("kylin-recorder_QMenu_m_menu");
    m_actionSave = new QAction();
    m_actionOpenFolder = new QAction();
    m_actionRename = new QAction();
    m_actionEmptyList = new QAction();

    connect(m_actionSave, &QAction::triggered, this, &FileItem::actionSaveasSlot);
    connect(m_actionOpenFolder, &QAction::triggered, this, &FileItem::actionOpenFolderSlot);
    connect(m_actionRename, &QAction::triggered, this, &FileItem::actionRenameSlot);
    connect(m_actionEmptyList, &QAction::triggered, this, &FileItem::actionEmptyListSlot);

    m_actionSave->setText(tr("Save as"));
    m_actionOpenFolder->setText(tr("Open folder position"));
    m_actionRename->setText(tr("Rename"));
    m_actionEmptyList->setText(tr("Empty the list"));
// 新需求进来后，把下面两句解开就行
//    m_menu->addAction(m_actionRename);
    m_menu->addAction(m_actionOpenFolder); // 2020/12/12禁用
//    m_menu->addAction(m_actionEmptyList);
    m_menu->addAction(m_actionSave);

    m_menu->exec(QCursor::pos());

    m_menu->deleteLater();
    m_actionSave->deleteLater();
    m_actionOpenFolder->deleteLater();
    m_actionRename->deleteLater();
    m_actionEmptyList->deleteLater();
#endif
}

//右键另存为其他路径
void FileItem::actionSaveasSlot()
{
#ifndef WHITE_TEST

    QString oldFilePath = this->m_filePath;
    QString newFilePath = "";
    if (Tools::fileExists(oldFilePath)) {
        //选择文件目录
        QString selectDirPath  =
            QFileDialog::getExistingDirectory(MainWindow::mutual, tr("Select a file storage directory"), m_lastSavePath.isEmpty() ? "/" : m_lastSavePath);

        if(!selectDirPath.isEmpty()){
            int value = Tools::inputEditJudge(selectDirPath + "/");
            qDebug() << "另存为的目录:" << selectDirPath << "value:" << value;
            if (value == 1) {
                newFilePath = selectDirPath + "/" + QFileInfo(oldFilePath).fileName();
                qDebug() << "将老文件路径" << oldFilePath << "拷贝到新文件路径:" << newFilePath;
                QFile::copy(oldFilePath, newFilePath);
                m_lastSavePath = selectDirPath;
            } else {
                return;
            }
        }
    } else {
        QMessageBox::warning(MainWindow::mutual, tr("Warning"),
                             tr("The file path does not exist or has been deleted!"));
    }
#endif
}

//右键打开文件位置并选中
void FileItem::actionOpenFolderSlot()
{
#ifndef WHITE_TEST

    QString openedPath = m_filePath;
    if (Tools::fileExists(openedPath)) {
        QDBusInterface sessionDbus("org.freedesktop.FileManager1", "/org/freedesktop/FileManager1",
                                   "org.freedesktop.FileManager1", QDBusConnection::sessionBus(), this);
        QStringList items;
        items.push_back(openedPath);
        QString startUpId = "";
        sessionDbus.call("ShowItems", items, startUpId);
    } else {
        QMessageBox::warning(MainWindow::mutual, tr("Warning"),
                             tr("The file path does not exist or has been deleted!"));
    }
#endif
}

void FileItem::actionRenameSlot()
{
#ifndef WHITE_TEST

    QString renamePath = m_filePath;
    if(Tools::fileExists(renamePath)){
        m_renameWidget = new RenameWidget();
        m_renameWidget->setAccessibleName("kylin-recorder_fileitem_RenameWidget_m_renameWidget");
        m_renameWidget->setLineEditContent(renamePath);
        connect(m_renameWidget, &RenameWidget::changeLabel, this, &FileItem::updateLabelNameSlots);
    }else{
        QMessageBox::warning(MainWindow::mutual, tr("Warning"), tr("The file path dose not exist or has been deleted!"));
        delAudioFile();
    }
#endif
}

void FileItem::actionEmptyListSlot()
{
#ifndef WHITE_TEST

    // 清空列表
    QStringList filePaths = MainWindow::mutual->getFilePathList();
    QMessageBox::StandardButton box;
    QString tipsStr = tr("Are you sure you want to clear the recording list?");
    box = QMessageBox::warning(this->parentWidget(), "", tipsStr, QMessageBox::Cancel|QMessageBox::Ok);
    if(box == QMessageBox::Ok){
        MainWindow::mutual->m_list->clear();
        MainWindow::mutual->isFileNull(0);
        emit startdelete(filePaths);
        MainWindow::mutual->clearFilePathList();
    }
#endif
}


void FileItem::mousePressEvent(QMouseEvent *mouseEvent)
{
#ifndef WHITE_TEST

    if (mouseEvent->button() == Qt::RightButton) {
        if (mouseEvent->type() == QEvent::MouseButtonPress) {
            qDebug() << "右键点击";
            rightClickedMenuRequest(); //右击弹窗:1另存为、2打开文件
        }
    }
#endif
}

bool FileItem::eventFilter(QObject *obj, QEvent *event) //鼠标滑块点击
{
    if (obj == m_buttonStackWidget) //悬浮特效、点击特效、右击特效
    {
        m_buttonStackWidget->setAttribute(Qt::WA_Hover, true); //****关键代码,若不加此行代码则没有悬浮特效****

        if (event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverMove) //显示浮窗
        {
            if (MainWindow::mutual->m_isRecording) {
                m_buttonStackWidget->setCurrentIndex(0); //切换至录音按钮stackLayout
            } else {
                m_buttonStackWidget->setCurrentIndex(1); //切换至录音按钮stackLayout
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void FileItem::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    QRect rect = m_splitLinestackWidget->geometry();
    //    qDebug()<<"触发了!"<<rect<<"railWid的宽度:"<<rect.width();
    m_playSlider->setFixedWidth(rect.width());
}

void FileItem::processLog()
{
    qDebug() << "*******process error*******\n"
             << QString::fromLocal8Bit(m_processDEL->readAllStandardError()) << "\n*******process error*******";
}

void FileItem::enterEvent(QEvent *event)
{
    m_pal.setColor(QPalette::Text, QApplication::palette().highlight().color());
    m_recordFileNameLabel->setPalette(m_pal);
    m_startRecordTimeLabel->setPalette(m_pal);
    m_timelengthLabel->setPalette(m_pal);

    //显示浮窗
    m_mouseState = true;
    if (MainWindow::mutual->m_isRecording) {
        m_buttonStackWidget->setCurrentIndex(0); //切换至录音按钮stackLayout
    } else {
        m_buttonStackWidget->setCurrentIndex(1); //切换至录音按钮stackLayout
    }
//  QColor color = qApp->palette().color(QPalette::Active, QPalette::Highlight);
//  m_playButton->setProperty("setIconHighlightEffectDefaultColor", color);
//  m_deleteButton->setProperty("setIconHighlightEffectDefaultColor", color);
//  m_clipButton->setProperty("setIconHighlightEffectDefaultColor", color);
//  m_pauseButton->setProperty("setIconHighlightEffectDefaultColor", color);
}

void FileItem::leaveEvent(QEvent *event)
{
    //    qDebug()<<"离开";
    int R = 255, G = 255, B = 255;
    m_itemThemeColor = MainWindow::mutual->m_themeData->get("styleName").toString();
    if (m_itemThemeColor == "ukui-dark" || m_itemThemeColor == "ukui-black") {
        R = 255, G = 255, B = 255;
    } else {
        R = 0, G = 0, B = 0;
    }
    m_pal.setColor(QPalette::Text, QColor(R, G, B));
    m_recordFileNameLabel->setPalette(m_pal);
    m_pal.setColor(QPalette::Text, QColor(140, 140, 140));
    m_timelengthLabel->setPalette(m_pal);
    m_startRecordTimeLabel->setPalette(m_pal);
    m_buttonStackWidget->setCurrentIndex(0); //切换至显示时长的布局
}

FileItem::~FileItem()
{
    //    qDebug()<<"释放FileItem";
    m_pauseButton->deleteLater();
    this->deleteLater();
    if(s_currentplaying == this){
        s_currentplaying = nullptr;
    }
}
