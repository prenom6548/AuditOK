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
#include "tinycountdown.h"
#include <kysdk/applications/kpushbutton.h>
using namespace kdk;
#include "ui_tinycountdown.h"
#include "theme.h"
#include <QBitmap>
#include "utils.h"
#include <kshadowhelper.h>
#include "globalinstance.h"
#ifdef NOTKAIMING
#include "ukuiwindowhelper.h"
#endif
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);
tinyCountdown::tinyCountdown(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tinyCountdown)
{
    ui->setupUi(this);

    //右上角关闭
    closeStyle();
    switchStyle();
    suspendRunBtnStyle();
    finishBtnStyle();
    connect(ui->closeBtn, SIGNAL(clicked()), this, SLOT(set_dialog_close()) );
    //左上闹钟图标
   ui->clockIcon->setPixmap(QIcon::fromTheme("kylin-alarm-clock").pixmap(24,24));
   //闹钟时间字体改为灰色
   QPalette pa;
   pa.setColor(QPalette::WindowText,Qt::gray);
   //主窗口connect
   connect(ui->mainBtn, SIGNAL(clicked()), this, SLOT(showMainWindow()) );
   //显示，但不可移动
   settingsStyle();
   updateWidgetRadius();

}


void tinyCountdown::updateWidgetRadius()
{

    this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明

    Qt::WindowFlags m_flags = windowFlags();
    this->setWindowFlags(m_flags | Qt::WindowStaysOnTopHint);


    // 添加窗管协议
//    XAtomHelper::setStandardWindowHint(this->winId());
//    kdk::UkuiStyleHelper::self()->removeHeader(this);
    //圆角

}

void tinyCountdown::showThisWindow()
{
#ifdef NOTKAIMING
    if(GlobalInstance::getInstance()->session_desktop() == GlobalInstance::KYLIN_WLCOM){
        UkuiWindowHelper *windowHelper = new UkuiWindowHelper(this);
        windowHelper->removeTitleBar();
    } else {
        kdk::UkuiStyleHelper::self()->removeHeader(this);
    }
#else
    kdk::UkuiStyleHelper::self()->removeHeader(this);
#endif
    setWindowFlag(Qt::FramelessWindowHint);
    kdk::effects::KShadowHelper::self()->setWidget(this);
    this->show();

    // 更新省略显示
    updateTimeLabelElided();
}

tinyCountdown::~tinyCountdown()
{
    delete ui;
}
/**
 * @brief 显示倒计时时间
 */
void tinyCountdown::updateTimeInfo(QString str)
{
    ui->timeLabel->setText(str);
    ui->timeLabel->setAlignment(Qt::AlignVCenter);

    // 保存原始文本到property
    ui->timeLabel->setProperty("fullText", str);

    // 处理省略显示和tooltip
    updateTimeLabelElided();
}

void tinyCountdown::updateTimeLabelElided()
{
    QString str = ui->timeLabel->property("fullText").toString();
    if (str.isEmpty())
        str = ui->timeLabel->text();

    if (str.isEmpty())
        return;

    QFontMetrics fm(ui->timeLabel->font());
    int availableWidth = ui->timeLabel->width() - ui->timeLabel->margin() * 2 - 4;
    QString elidedText = fm.elidedText(str, Qt::ElideRight, availableWidth);

    ui->timeLabel->setText(elidedText);
    if (elidedText != str) {
        ui->timeLabel->setToolTip(str);
    } else {
        ui->timeLabel->setToolTip(QString());
    }
}


/*
void tinyCountdown::focusOutEvent(QFocusEvent *event)
{
    qDebug()<<"dbq-"<<"focusOutEvent";
}

void tinyCountdown::focusInEvent(QFocusEvent *event)
{
    qDebug()<<"dbq-"<<"focusInEvent";
}
*/



void tinyCountdown::closeEvent(QCloseEvent *event)
{
    event->ignore();//忽视
    this->set_dialog_close();
}

void tinyCountdown::paintEvent(QPaintEvent * event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.setRenderHint(QPainter::SmoothPixmapTransform);  // 平滑变换
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect(), 10, 10); // 左上右下


    QColor mainColor;
    mainColor = theme::selectBtnBackColor;
    p.fillPath(rectPath,QBrush(mainColor));

}




/**
 * @brief 清空数据
 */
void tinyCountdown::clearData()
{
    ui->timeLabel->setText("00:00:00");
    this->set_dialog_close();
    updateOnRunState(true);

    // 更新省略显示
    updateTimeLabelElided();
}



void tinyCountdown::set_dialog_close()
{
    this->hide();
}

void tinyCountdown::showMainWindow()
{
    this->set_dialog_close();
    emit mainWindowClick();
}



void tinyCountdown::showTop()
{
    this->set_dialog_close();
    this->update();
    this->show();
}

void tinyCountdown::updateOnRunState(int onRun)
{
    m_onRun = onRun;
    suspendRunBtnIcon();
}

void tinyCountdown::settingsStyle()
{
    GsettingSubject * subject = GsettingSubject::getInstance();;
    connect(subject,&GsettingSubject::iconChnaged, this,[=](){
       ui->clockIcon->setPixmap(QIcon::fromTheme("kylin-alarm-clock").pixmap(24,24));
    });
    subject->iniWidgetStyle();
}


void tinyCountdown::closeStyle()
{
    ui->closeBtn->setToolTip(tr("close"));
    ui->closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    ui->closeBtn->setProperty("isWindowButton", 0x2);
    ui->closeBtn->setProperty("useIconHighlightEffect", 0x8);
    ui->closeBtn->setFlat(true);
}
void tinyCountdown::switchStyle()
{
    ui->mainBtn->setToolTip(tr("main window"));
    ui->mainBtn->setProperty("useButtonPalette", true);
    ui->mainBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));
    ui->mainBtn->setIconSize(QSize(16,16));
    //手动改成透明色
    Utils::setBtnBackgroundColorTransparent(ui->mainBtn);
}
/**
 * @brief 暂停运行按钮样式
 */
void tinyCountdown::suspendRunBtnStyle()
{
    ui->suspendRunBtn->setToolTip(tr("suspend"));
    ui->suspendRunBtn->setButtonType(kdk::KPushButton::CircleType);
    ui->suspendRunBtn->setBorderRadius(16);
    //初始化是运行中
    m_onRun = true;
    suspendRunBtnIcon();
    connect(ui->suspendRunBtn,&QPushButton::clicked,this,[=](){
        m_onRun = !m_onRun;
        suspendRunBtnIcon();
        emit suspendClick(m_onRun);
    });
}

void tinyCountdown::finishBtnStyle()
{
    ui->finishBtn->setToolTip(tr("finish"));
    ui->finishBtn->setButtonType(kdk::KPushButton::CircleType);
    ui->finishBtn->setIcon(QIcon::fromTheme("media-playback-stop-symbolic"));
    ui->finishBtn->setIconHighlight(true);
//    ui->finishBtn->setTranslucent(true);
    connect(ui->finishBtn,&QPushButton::clicked,this,[=](){
        emit finishClick();
    });
}

void tinyCountdown::suspendRunBtnIcon()
{
    if(m_onRun){
        ui->suspendRunBtn->setIcon(QIcon::fromTheme("media-playback-pause-symbolic"));
    }else{
        ui->suspendRunBtn->setIcon(QIcon::fromTheme("media-playback-start-solid-24-symbolic"));
    }
}




