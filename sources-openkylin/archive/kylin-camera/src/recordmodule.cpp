#include "recordmodule.h"
#include <QPainter>
#include <QStyleOption>

bool isPause = false; //非暂停状态

RecordModule::RecordModule(QWidget *parent)
    : QWidget(parent)
{
    videoStopBtn = new QPushButton(this);  //停止录制button
    videoStopBtn->setAccessibleName("kylin-camera_QPushButton_videoStopBtn");
    videoStopBtn->setFixedSize(30,30);
    videoStopBtn->setIcon(QIcon(":/image/finish.jpg"));
    videoStopBtn->setToolTip(tr("stop"));
    videoStopBtn->setFocusPolicy(Qt::NoFocus);

    videoContinueBtn = new QPushButton(this);   //继续录制button
    videoContinueBtn->setAccessibleName("kylin-camera_QPushButton_videoContinueBtn");
    videoContinueBtn->setFixedSize(30,30);
    videoContinueBtn->setIcon(QIcon(":/image/countinue.svg"));
    videoContinueBtn->setToolTip(tr("countinue"));
    videoContinueBtn->setFocusPolicy(Qt::NoFocus);

    videoPauseBtn = new QPushButton(this); //暂停录制button
    videoPauseBtn->setAccessibleName("kylin-camera_QPushButton_videoPauseBtn");
    videoPauseBtn->setFixedSize(30,30);
    videoPauseBtn->setIcon(QIcon(":/image/pause.svg"));
    videoPauseBtn->setToolTip(tr("pause"));
    videoPauseBtn->setFocusPolicy(Qt::NoFocus);


    recorderTimeLabel = new QLabel(this);//录像计时容器
    recorderTimeLabel->setAccessibleName("kylin-camera_QLabel_recorderTimeLabel");
    QPalette a(recorderTimeLabel->palette());//录像计时显示设置
    a.setColor(QPalette::WindowText,QColor(Qt::GlobalColor::red)); //红色字
    recorderTimeLabel->setPalette(a);

    recordModuleLayout = new QHBoxLayout;
    recordModuleLayout->addSpacing(15);
    recordModuleLayout->addWidget(videoStopBtn);
    recordModuleLayout->addWidget(videoContinueBtn);
    recordModuleLayout->addWidget(videoPauseBtn);
    recordModuleLayout->addSpacing(20);
    recordModuleLayout->addWidget(recorderTimeLabel);
    recordModuleLayout->addSpacing(15);

    this->setFixedSize(222,50);
    this->setLayout(recordModuleLayout);

    //信号和槽
    connect(videoContinueBtn,&QPushButton::clicked,this,&RecordModule::startVideoPress); //点击继续录像button
    connect(videoPauseBtn,&QPushButton::clicked,this,&RecordModule::suspendVideoPress); //点击暂停录像button
    connect(videoStopBtn,&QPushButton::clicked,this,&RecordModule::stopVideoPress);   //点击停止录像button
}
//点击继续录像button
void RecordModule::startVideoPress(){
    videoContinueBtn->hide();
    videoPauseBtn->show();
    isPause = false;
    Q_EMIT recordSuspendSig(false);
}
//点击暂停录像button
void RecordModule::suspendVideoPress(){
    videoContinueBtn->show();
    videoPauseBtn->hide();
    isPause = true;
    Q_EMIT recordSuspendSig(true);
}
//点击停止录像button
void RecordModule::stopVideoPress(){
    if(isPause){
        isPause = false;
        Q_EMIT recordSuspendSig(false);
    } 
    videoPauseBtn->show();
    Q_EMIT recordStopSig();
    hour = 0;
    min = 0;
    sec = 0;
    this->hide();
}
void RecordModule::clickRecordSlot(){
    videoContinueBtn->hide();
//    //显示录像数字
    this->show();
    QString str = "00:00:00";
    recorderTimeLabel->setText(str);
    QFont font;
    font.setPointSize(14);
    recorderTimeLabel->setFont(font);
}
void RecordModule::updateRecordingTime(int hourSig, int minSig, int secSig){
    //int hourSig, int minSig, int secSig
    int hour = hourSig;
    int min = minSig;
    int sec = secSig;

    QString str_hour = (hour >= 10) ? QString::number(hour) : "0" + QString::number(hour);
    QString str_min = (min >= 10) ? QString::number(min) : "0" + QString::number(min);
    QString str_sec = (sec >= 10) ? QString::number(sec) : "0" + QString::number(sec);
    QString str_time = str_hour + ":" + str_min + ":" + str_sec;
    recorderTimeLabel->setText(str_time);
}
RecordModule::~RecordModule(){

}
//圆角及背景色特效
void RecordModule::paintEvent(QPaintEvent *event)   //毛玻璃及圆角特效
{
   QPainter painter(this);
   painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
   QPainterPath rectPath;
   rectPath.addRoundedRect(this->rect(), 10, 10); // 左上右下
   QStyleOption opt;
   opt.init(this);
   QString nowThemeStyle = gsettingItem.getStyleName();
   QColor mainColor = Qt::black;
   if(nowThemeStyle == "ukui-dark" || nowThemeStyle == "ukui-black"){
       mainColor = Qt::black;
   }
   else{
       mainColor = Qt::white;
   }
   mainColor.setAlpha(77);
   painter.fillPath(rectPath,QBrush(mainColor));
   QWidget::paintEvent(event);
}
