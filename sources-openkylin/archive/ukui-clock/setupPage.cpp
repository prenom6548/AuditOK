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
#include "setupPage.h"
#include "ui_setupPage.h"
#include "QSlider"
#include <QPixmap>
#include "setAlarmRepeatDialog.h"
#include <QDebug>
#include <QBitmap>
#include <QPainterPath>
#include "theme.h"

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

static const int DIALOG_WIDTH = 200;

static const int DIALOG_LEFT_SPACE = 130;

setuppage::setuppage( double position_x, double position_y, QWidget *parent  ) :
    QWidget(parent),
    pos_x(position_x),
    pos_y(position_y),
    ui(new Ui::setuppage)
{
//    setAttribute(Qt::WA_TranslucentBackground);
//    QPainterPath blurPath;
//    setProperty("useSystemStyleBlur", true);
//    setProperty("blurRegion", QRegion(blurPath.toFillPolygon().toPolygon()));//使用QPainterPath的api生成多边形Region

//    this->setProperty("blurRegion", QRegion(QRect(1, 1, 1, 1)));
    Qt::WindowFlags m_flags = windowFlags();
    this->setWindowFlags(m_flags | Qt::WindowStaysOnTopHint);

    ui->setupUi(this);
    //时间格式
    Time_format = new  set_alarm_repeat_Dialog(DIALOG_WIDTH,138,3,ui->widget);
    //稍后提醒时间间隔
    Reminder_off = new  set_alarm_repeat_Dialog(DIALOG_WIDTH,190,5,ui->widget);
    //默认铃声
    Default_ringtone = new  set_alarm_repeat_Dialog(DIALOG_WIDTH,169,4,ui->widget);

    connect(Time_format->listWidget,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(Time_format_listClickslot()));
    connect(Reminder_off->listWidget,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(Reminder_off_listClickslot()));
    connect(Default_ringtone->listWidget,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(Default_ringtone_listClickslot()));
    //时间格式
    timeformatBtn = new SelectBtn( tr("  time"), this);
    //时间格式
    timeFormatLabel = new QLabel(this);
    updateLabelStyle(timeFormatLabel,tr("  time"),108);
    //提醒时间
    remindTimeLabel = new QLabel(this);
    updateLabelStyle(remindTimeLabel,tr("  duration"),163);
    //默认铃声
    defaultBellLabel = new QLabel(this);
    updateLabelStyle(defaultBellLabel,tr("  ringtone"),218);
    //稍后提醒时间间隔

    remindDurationBtn = new SelectBtn( tr("  duration"), this);
    //默认铃声
    defaultBellBtn = new SelectBtn( tr("  ringtone"), this);
    int selectBtnLeftSpace = 102;
    timeformatBtn->move(selectBtnLeftSpace, 108);
    remindDurationBtn->move(selectBtnLeftSpace, 163);
    defaultBellBtn->move(selectBtnLeftSpace, 218);

    timeformatBtn->resize(DIALOG_WIDTH,36);
    remindDurationBtn->resize(DIALOG_WIDTH,36);
    defaultBellBtn->resize(DIALOG_WIDTH,36);

    //绑定下拉框
    connect(timeformatBtn, SIGNAL(clicked()), this, SLOT(Time_format_set()) );
    connect(remindDurationBtn, SIGNAL(clicked()), this, SLOT(Reminder_off_set()) );
    connect(defaultBellBtn, SIGNAL(clicked()), this, SLOT(Default_ringtone_set()) );

    connect(Time_format->listWidget,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(Time_format_listClickslot()));
    connect(Reminder_off->listWidget,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(Reminder_off_listClickslot()));
    connect(Default_ringtone->listWidget,SIGNAL(itemClicked(QListWidgetItem*)),this,SLOT(Default_ringtone_listClickslot()));
    connect(Time_format,&set_alarm_repeat_Dialog::dialogClose,this,[=](){
        timeformatBtn->updateIconLabel(0);
    });
    connect(Reminder_off,&set_alarm_repeat_Dialog::dialogClose,this,[=](){
        remindDurationBtn->updateIconLabel(0);
    });
    connect(Default_ringtone,&set_alarm_repeat_Dialog::dialogClose,this,[=](){
         defaultBellBtn->updateIconLabel(0);
    });

    model_setup = new QSqlTableModel(this);
    model_setup->setTable("setup");
    model_setup->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model_setup->select();
    model_setup->submitAll();

    repeat_on_Pixmap = QPixmap(":/image/object-select-symbolic.png");
    repeat_off_Pixmap = QPixmap("");

    QString werk_day;
    QString Time = model_setup->index(0, 15).data().toString();
    QString Pop_up = model_setup->index(0, 16).data().toString();
    QString Reminder = model_setup->index(0, 17).data().toString();
    QString Default = model_setup->index(0, 5).data().toString();

    int werk = 0;
    for (int i=0; i<7; i++) {
        if (model_setup->index(0, i+7).data().toInt()) {
            if(i == 0){
                werk_day = werk_day + tr("Mon");
            }else if(i == 1){
                werk_day = werk_day + tr("Tue");
            }else if(i == 2){
                werk_day = werk_day + tr("Wed");
            }else if(i == 3){
                werk_day = werk_day + tr("Thu");
            }else if(i == 4){
                werk_day = werk_day + tr("Fri");
            }else if(i == 5){
                werk_day = werk_day + tr("Sat");
            }else if(i == 6){
                werk_day = werk_day + tr("Sun");
            }
        }else{
            werk = 1;
        }
    }

    //可以比较 Time.compare(tr("Following system")

    if(Time.compare("Following system") == 0 || Time.compare("跟随系统") == 0){
        Time = tr("Following system");
    }else if(Time.compare("24 hour system") == 0 || Time.compare("24小时制") == 0){
        Time = tr("24 hour system");
    }else if(Time.compare("12 hour system") == 0 || Time.compare("12小时制") == 0){
        Time = tr("12 hour system");
    }
    if(Pop_up.compare("Notification") == 0 || Pop_up.compare("通知栏弹窗") == 0){
        Pop_up = tr("Notification");
    }else if(Pop_up.compare("Full screen") == 0 || Pop_up.compare("全屏弹窗") == 0){
        Pop_up = tr("Full screen");
    }
    if(Reminder.compare("Alert in 2 minutes") == 0 || Reminder.compare("2分钟后提醒") == 0){
        Reminder = tr("Alert in 2 minutes");
    }else if(Reminder.compare("Alert in 5 minutes") == 0 || Reminder.compare("5分钟后提醒") == 0){
        Reminder = tr("Alert in 5 minutes");
    }else if(Reminder.compare("Alert in 10 minutes") == 0 || Reminder.compare("10分钟后提醒") == 0){
        Reminder = tr("Alert in 10 minutes");
    }else if(Reminder.compare("Alert in 30 minutes") == 0 || Reminder.compare("30分钟后提醒") == 0){
        Reminder = tr("Alert in 30 minutes");
    }else if(Reminder.compare("Alert in 60 minutes") == 0 || Reminder.compare("60分钟后提醒") == 0){
        Reminder = tr("Alert in 60 minutes");
    }

    if(Default.compare("glass") == 0 || Default.compare("玻璃") == 0){
        Default = tr("glass");
    }else if(Default.compare("bark") == 0 || Default.compare("犬吠") == 0){
        Default = tr("bark");
    }else if(Default.compare("sonar") == 0 || Default.compare("声呐") == 0){
        Default = tr("sonar");
    }else if(Default.compare("drip") == 0 || Default.compare("雨滴") == 0){
        Default = tr("drip");
    }
    timeformatBtn->textLabel->setText(Time);
    remindDurationBtn->textLabel->setText(Reminder);
    defaultBellBtn->textLabel->setText(Default);

    Time_format->hide() ;
    Reminder_off->hide() ;
    Default_ringtone->hide() ;

    QColor ColorPlaceholderText3(255,255,255,0);
    QBrush brush;
    brush.setColor(ColorPlaceholderText3);

    ui->closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    ui->closeBtn->setProperty("isWindowButton", 0x2);
    ui->closeBtn->setProperty("useIconHighlightEffect", 0x8);
    ui->closeBtn->setFlat(true);
    connect(ui->closeBtn, &QPushButton::clicked, this, [=](){
        this->hide();
    });

    muteBtn = new CustomButton(this,54,24,1);
    muteBtn->move(236,66);
    //静音labe
    QString selfFont = Utils::loadFontHuaKangJinGangHeiRegularTTF();
    QFont f(selfFont);
    ui->label_3->setFont(f);

    //    主题框架1.0.6-5kylin2

    //关闭按钮去掉聚焦状态
    ui->closeBtn->setFocusPolicy(Qt::NoFocus);
    //初始化
    musicSelectPlay = new QSoundEffect(this);
    this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明

}

setuppage::~setuppage()
{
    delete  model_setup;
    delete  Time_format ;
    delete  Reminder_off ;
    delete  Default_ringtone ;
    delete  ui;
}

void setuppage::Mute_starting()
{

}


//时间格式设置回调
// Time format callback
void setuppage::Time_format_set()
{
    timeformatBtn->updateIconLabel(1);
    timeformatBtn->setAttribute(Qt::WA_UnderMouse, false);
    QHoverEvent hoverEvent(QEvent::HoverLeave, QPoint(40, 40), QPoint(0, 0));
    QCoreApplication::sendEvent(timeformatBtn, &hoverEvent);

    Time_format->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    //Time_format->setAttribute(Qt::WA_TranslucentBackground);
    QPointF position = parentWidget()->pos();
    Time_format->move(position.x()+DIALOG_LEFT_SPACE,position.y()+255);
    Time_format->resize(DIALOG_WIDTH,138);
    Time_format->listWidget->setFixedSize(DIALOG_WIDTH-20,130);
    Time_format->widget[0]->alarmLabel0->setText(tr("Following system"));
    Time_format->widget[1]->alarmLabel0->setText(tr("24 hour system"));
    Time_format->widget[2]->alarmLabel0->setText(tr("12 hour system"));

    Time_format->show();

}

//时间格式选择单击回调
// Time format selection click callback
void setuppage::Time_format_listClickslot()
{
    model_setup->select();

    int num=Time_format->listWidget->currentRow();

    switch (num) {
    case 0:
        model_setup->setData(model_setup->index(0, 2), 0);
        model_setup->setData(model_setup->index(0, 15), tr("Following system"));
        break;

    case 1:
        model_setup->setData(model_setup->index(0, 2), 1);
        model_setup->setData(model_setup->index(0, 15), tr("24 hour system"));
        break;

    case 2:
        model_setup->setData(model_setup->index(0, 2), 2);
        model_setup->setData(model_setup->index(0, 15), tr("12 hour system"));
        break;
    default:
        break;
    }

    timeformatBtn->textLabel->setText(model_setup->index(0, 15).data().toString());
    Time_format->close();
    model_setup->submitAll();
}



//提醒关闭回调
// Reminder close callback
void setuppage::Reminder_off_set()
{
    remindDurationBtn->updateIconLabel(1);
    remindDurationBtn->setAttribute(Qt::WA_UnderMouse, false);
    QHoverEvent hoverEvent(QEvent::HoverLeave, QPoint(40, 40), QPoint(0, 0));
    QCoreApplication::sendEvent(remindDurationBtn, &hoverEvent);

    Reminder_off->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    //Reminder_off->setAttribute(Qt::WA_TranslucentBackground);
    QPointF position = parentWidget()->pos();
    Reminder_off->move(position.x()+DIALOG_LEFT_SPACE,position.y()+310);
    Reminder_off->resize(DIALOG_WIDTH,190);
    Reminder_off->listWidget->setFixedSize(DIALOG_WIDTH-20,170);
    Reminder_off->widget[0]->alarmLabel0->setText(tr("Alert in 2 minutes"));
    Reminder_off->widget[1]->alarmLabel0->setText(tr("Alert in 5 minutes"));
    Reminder_off->widget[2]->alarmLabel0->setText(tr("Alert in 10 minutes"));
    Reminder_off->widget[3]->alarmLabel0->setText(tr("Alert in 30 minutes"));
    Reminder_off->widget[4]->alarmLabel0->setText(tr("Alert in 60 minutes"));
    Reminder_off->show();
}

//提醒关闭选择单击回调
// Reminder off select click callback
void setuppage::Reminder_off_listClickslot()
{
    model_setup->select();
    int num=Reminder_off->listWidget->currentRow();
    //配置稍后提醒时间间隔
    model_setup->setData(model_setup->index(0, 4), num);

    switch (num) {
    case 0:
        model_setup->setData(model_setup->index(0, 17), tr("Alert in 2 minutes"));
        break;
    case 1:
        model_setup->setData(model_setup->index(0, 17), tr("Alert in 5 minutes"));
        break;
    case 2:
        model_setup->setData(model_setup->index(0, 17), tr("Alert in 10 minutes"));
        break;
    case 3:
        model_setup->setData(model_setup->index(0, 17), tr("Alert in 30 minutes"));
        break;
    case 4:
        model_setup->setData(model_setup->index(0, 17), tr("Alert in 60 minutes"));
        break;
    default:
        break;
    }
    remindDurationBtn->textLabel->setText(model_setup->index(0, 17).data().toString());
    model_setup->submitAll();
    Reminder_off->close();
}
//默认铃声设置回调
// Default ringtone setting callback
void setuppage::Default_ringtone_set()
{
    defaultBellBtn->updateIconLabel(1);
    defaultBellBtn->setAttribute(Qt::WA_UnderMouse, false);
    QHoverEvent hoverEvent(QEvent::HoverLeave, QPoint(40, 40), QPoint(0, 0));
    QCoreApplication::sendEvent(defaultBellBtn, &hoverEvent);

    Default_ringtone->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    //Default_ringtone->setAttribute(Qt::WA_TranslucentBackground);
    QPointF position = parentWidget()->pos();
    Default_ringtone->move(position.x()+DIALOG_LEFT_SPACE,position.y()+363);
    Default_ringtone->resize(DIALOG_WIDTH,169);
    Default_ringtone->listWidget->setFixedSize(DIALOG_WIDTH-20,149);
    Default_ringtone->widget[0]->alarmLabel0->setText(tr("glass"));
    Default_ringtone->widget[1]->alarmLabel0->setText(tr("bark"));
    Default_ringtone->widget[2]->alarmLabel0->setText(tr("sonar"));
    Default_ringtone->widget[3]->alarmLabel0->setText(tr("drip"));
    Default_ringtone->show();

}

//默认铃声选择单击回调
// Default ring tone selection click callback
void setuppage::Default_ringtone_listClickslot()
{
    model_setup->select();
    int num=Default_ringtone->listWidget->currentRow();
    delete musicSelectPlay;
    musicSelectPlay = new QSoundEffect(this);
#ifdef NOTKAIMING
    switch (num)
    {
    case 0:
        model_setup->setData(model_setup->index(0, 5), tr("glass"));
        musicSelectPlay->setSource(QUrl::fromLocalFile("/usr/share/ukui-clock/glass.wav"));
        break;
    case 1:
        model_setup->setData(model_setup->index(0, 5), tr("bark"));
        musicSelectPlay->setSource(QUrl::fromLocalFile("/usr/share/ukui-clock/bark.wav"));
        break;
    case 2:
        model_setup->setData(model_setup->index(0, 5), tr("sonar"));
        musicSelectPlay->setSource(QUrl::fromLocalFile("/usr/share/ukui-clock/sonar.wav"));
        break;
    case 3:
        model_setup->setData(model_setup->index(0, 5), tr("drip"));
        musicSelectPlay->setSource(QUrl::fromLocalFile("/usr/share/ukui-clock/drip.wav"));
        break;
    default:
        break;
    }
#else
      switch (num)
    {
    case 0:
        model_setup->setData(model_setup->index(0, 5), tr("glass"));
        musicSelectPlay->setSource(QUrl::fromLocalFile("/opt/apps/top.openkylin.ukui-clock/files/share/ukui-clock/glass.wav"));
        break;
    case 1:
        model_setup->setData(model_setup->index(0, 5), tr("bark"));
        musicSelectPlay->setSource(QUrl::fromLocalFile("/opt/apps/top.openkylin.ukui-clock/files/share/ukui-clock/bark.wav"));
        break;
    case 2:
        model_setup->setData(model_setup->index(0, 5), tr("sonar"));
        musicSelectPlay->setSource(QUrl::fromLocalFile("/opt/apps/top.openkylin.ukui-clock/files/share/ukui-clock/sonar.wav"));
        break;
    case 3:
        model_setup->setData(model_setup->index(0, 5), tr("drip"));
        musicSelectPlay->setSource(QUrl::fromLocalFile("/opt/apps/top.openkylin.ukui-clock/files/share/ukui-clock/drip.wav"));
        break;
    default:
        break;
    }
#endif
    defaultBellBtn->textLabel->setText(model_setup->index(0, 5).data().toString());
    Default_ringtone->close();
    model_setup->submitAll();
    musicSelectPlay->play();
}





void setuppage::updateLabelStyle(QLabel *temp, QString text, int heightMove)
{
    temp->setText(text);
    temp->resize(70,36);
    temp->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    temp->setStyleSheet("font-size:14px;");
    QString selfFont = Utils::loadFontHuaKangJinGangHeiRegularTTF();
    QFont f(selfFont);
    temp->setFont(f);
    int labelLeftSpace = 22;
    temp->move(labelLeftSpace, heightMove);
}


void setuppage::paintEvent(QPaintEvent *event)
{

    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 10, 10);
    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(QColor(0,0,0,100));
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();

    // 模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, 10, false, false);

    // 挖掉中心
    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);

    // 绘制阴影
    p.drawPixmap(this->rect(), pixmap, pixmap.rect());

    QStyleOption opt;
    opt.init(this);
    QColor mainColor;
    mainColor = theme::dialogBackcolcr;


    // 绘制一个背景
    p.save();
    p.fillPath(rectPath,mainColor);
    p.restore();


}

/**
 * @brief 事件拦截器
 */
bool setuppage::eventFilter(QObject *watched, QEvent *event)
{
    return QWidget::eventFilter(watched,event);
}

//实现响应函数
void setuppage::showPaint()
{
}

//实现响应函数
void setuppage::showPaint1()
{

}
/**
 * @brief 颜色切换
 */
void setuppage::showPaint2()
{
    QColor ColorPlaceholderText(248,163,76,255);
    QBrush brush3;
    brush3.setColor(ColorPlaceholderText);
    QStyleOption opt;
    opt.init(this);



}
