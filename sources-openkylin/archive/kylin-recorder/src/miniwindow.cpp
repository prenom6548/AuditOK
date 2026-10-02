/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 *
 *  Authors: baijincheng <baijincheng@kylinos.cn>
 */
#include "miniwindow.h"
#include "mainwindow.h"
// #include "xatom-helper.h"
#include "playcontrol.h"
#include <kshadowhelper.h>
#include <ukuistylehelper/ukuistylehelper.h>
MiniWindow::MiniWindow(QWidget *parent) : QMainWindow(parent)
{
    MainWindow::mutual->recorderBuriedPoint(kdk::kabase::BuriedPoint::KylinRecorderTopicSwitch);
    initMiniWindow();
    setMiniWindow();
    kdk::effects::KShadowHelper::self()->setWidget(this);
    this->setAttribute(Qt::WA_AlwaysShowToolTips);//设置始终保持tooltip
    setWindowFlags(Qt::WindowStaysOnTopHint);
}

MiniWindow::~MiniWindow() {}

void MiniWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);

    QRect rect = geometry();
    qDebug() << "Mini模式" << rect << "miniWid的宽度:" << rect.width() << "miniWid的高度:" << rect.height();
    m_miniWidget->setFixedSize(rect.width(), rect.height());
}
void MiniWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->m_isPress = true;
        this->m_windowPos = this->pos();
        this->m_dragPos = event->globalPos();
        event->accept();
    }
}

void MiniWindow::mouseReleaseEvent(QMouseEvent *event)
{
    this->m_isPress = false;
    this->setCursor(Qt::ArrowCursor);
}

void MiniWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (this->m_isPress) {
        this->move(this->m_windowPos - (this->m_dragPos - event->globalPos()));
        this->setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
}
void MiniWindow::initMiniWindow()
{
    // 用户手册功能
    m_DaemonIpcDbus = new DaemonDbus();

    this->setFixedSize(m_WIDTH, m_HEIGHT);
    QScreen *screen = QGuiApplication::primaryScreen();
    this->move((screen->geometry().width() - m_WIDTH) / 2, (screen->geometry().height() - m_HEIGHT) / 2);

    installEventFilter(this);
    m_miniWidget = new QWidget(this);
    m_miniWidget->setAccessibleName("kylin-recorder_miniwindow_QWidget_m_miniWidget");

    m_recordButtonWidget = new QWidget(this);
    m_recordButtonWidget->setAccessibleName("kylin-recorder_miniwindow_QWidget_m_recordButtonWidget");
    m_pageTwoWidget = new QWidget(this);              //包括停止、开始/暂停两个按钮的Wid
    m_pageTwoWidget->setAccessibleName("kylin-recorder_miniwindow_QWidget_m_pageTwoWidget");
    m_recordStackedWidget = new QStackedWidget(this); //录音标签和开始暂停按钮的堆叠布局
    m_recordStackedWidget->setAccessibleName("kylin-recorder_miniwindow_QStackedWidget_m_recordStackedWidget");
    m_restoreOrMiniAndCloseWid = new QWidget(this);   //最大最小和关闭的Wid
    m_restoreOrMiniAndCloseWid->setAccessibleName("kylin-recorder_miniwindow_QWidget_m_restoreOrMiniAndCloseWid");


    m_miniLayout = new QHBoxLayout();
    m_pageTwoLayout = new QHBoxLayout();
    m_restoreOrMiniAndCloseLayout = new QHBoxLayout();
    m_recordLayout = new QHBoxLayout();
    m_line = new QFrame(m_restoreOrMiniAndCloseWid); //分割线
    m_line->setAccessibleName("kylin-recorder_miniwindow_QFrame_m_line");

    this->setWindowTitle(tr("Recorder"));

    m_miniRecordButton = new QPushButton(this);
    m_miniRecordButton->setAccessibleName("kylin-recorder_QPushButton_m_miniRecordButton");
    m_miniRecordButton->setToolTip(tr("Recording Ctrl+R"));
    m_miniRecordButton->setFixedSize(QSize(22, 22));
    m_miniRecordButton->setIconSize(QSize(22, 22));
    m_miniRecordButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-default.svg);}"
                                       "QPushButton::hover{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-hover.svg);}"
                                       "QPushButton::pressed{border:0px; background:transparent; border-image:url(:/svg/svg/mini-rec-click.svg);}");


//    m_miniStopButton = new CustomButton(":/svg/svg/finish_mini.svg", ":/svg/svg/finish_mini_hover.svg",
//                                        ":/svg/svg/finish_mini_click.svg", this); //停止按钮
//    m_miniStopButton->setFixedSize(25, 25);
//    m_miniStopButton->setToolTip(tr("Finish"));
    m_miniStopButton = new QPushButton(this);
    m_miniStopButton->setAccessibleName("kylin-recorder_QPushButton_m_miniStopButton");
    m_miniStopButton->setIconSize(QSize(16, 16));
    m_miniStopButton->setFixedSize(16, 16);
    m_miniStopButton->setToolTip(tr("Stop/Finish Esc"));
    m_miniStopButton->setStyleSheet("QPushButton{border:0px; background:transparent; border-image:url(:/svg/svg/finish_mini.svg);}");
//  m_miniStartButton = new CustomButton(":/svg/svg/mini-play-light.svg", ":/svg/svg/mini-play-_hover.svg",
//                                         ":/svg/svg/mini-play-click.svg");
//    m_miniStartButton->setFixedSize(25, 25);
//    m_miniStartButton->setToolTip(tr("Start"));

    m_miniStartButton = new kdk::KBorderlessButton();
    m_miniStartButton->setAccessibleName("kylin-recorder_miniwindow_KBorderlessButton_m_miniStartButton");
    m_miniStartButton->setIcon(QIcon::fromTheme(":/svg/svg/mini-play-light.svg"));
    m_miniStartButton->setIconSize(QSize(16, 16));
    m_miniStartButton->setFixedSize(24, 24);
    m_miniStartButton->setToolTip(tr("Start"));

//    m_miniPauseButton = new CustomButton(":/svg/svg/mini-suspend-light.svg", ":/svg/svg/mini-suspend-hover.svg",
//                                         ":/svg/svg/mini-suspend-click.svg", this); //开始和暂停按钮
//    m_miniPauseButton->setFixedSize(25, 25);
//    m_miniPauseButton->setToolTip(tr("Pause"));

    m_miniPauseButton = new kdk::KBorderlessButton();
    m_miniPauseButton->setAccessibleName("kylin-recorder_miniwindow_KBorderlessButton_m_miniPauseButton");
    m_miniPauseButton->setIcon(QIcon::fromTheme(":/svg/svg/mini-suspend-light.svg"));
    m_miniPauseButton->setIconSize(QSize(16, 16));
    m_miniPauseButton->setFixedSize(24, 24);
    m_miniPauseButton->setToolTip(tr("Start"));

    m_miniTimeLabel = new QLabel(this); //显示录制时间的标签
    m_miniTimeLabel->setAccessibleName("kylin-recorder_QLabel_m_miniTimeLabel");
    m_miniTimeLabel->setText("00:00:00");
    m_miniTimeLabel->setStyleSheet("font-size:18px;"); //修改字体显示
    m_miniTimeLabel->setFixedSize(80, 18);
    m_miniButton = new QToolButton(m_restoreOrMiniAndCloseWid);         //复原按钮
    m_miniButton->setAccessibleName("kylin-recorder_miniwindow_QToolButton_m_miniButton");
    m_miniButton->setIcon(QIcon::fromTheme("window-restore-symbolic")); //主题库的mini图标
    m_miniButton->setFixedSize(24, 24);
    m_miniButton->setProperty("isWindowButton", 0x1);
    m_miniButton->setProperty("useIconHighlightEffect", 0x2);
    m_miniButton->setAutoRaise(true);
    m_miniButton->setToolTip(tr("Restore"));

    m_miniCloseButton = new QToolButton(m_restoreOrMiniAndCloseWid);       //关闭按钮
    m_miniCloseButton->setAccessibleName("kylin-recorder_miniwindow_QToolButton_m_miniCloseButton");
    m_miniCloseButton->setIcon(QIcon::fromTheme("window-close-symbolic")); //主题库的叉子图标
    m_miniCloseButton->setFixedSize(24, 24);
    m_miniCloseButton->setProperty("isWindowButton", 0x2);
    m_miniCloseButton->setProperty("useIconHighlightEffect", 0x8);
    m_miniCloseButton->setAutoRaise(true);
    m_miniCloseButton->setToolTip(tr("Close"));

    connect(m_miniRecordButton, &QToolButton::clicked, this, &MiniWindow::switchPage);
    connect(m_miniButton, &QToolButton::clicked, this, &MiniWindow::normalShow);
    connect(m_miniCloseButton, &QToolButton::clicked, this, &MiniWindow::closeWindow);
    connect(m_miniPauseButton, &CustomButton::clicked, this, &MiniWindow::handleStartPauseSlot);
    connect(m_miniStartButton, &CustomButton::clicked, this, &MiniWindow::handleStartPauseSlot);
    connect(this, &MiniWindow::startOrPauseSignal, this, &MiniWindow::startOrPauseSlot);
    connect(m_miniStopButton, &CustomButton::clicked, this, &MiniWindow::finishSlot);
}

void MiniWindow::setMiniWindow()
{
    //    line->setFixedWidth(1);
    //    line->setFrameShape(QFrame::VLine);

    m_pageTwoLayout->addWidget(m_miniStopButton);
    m_pageTwoLayout->addWidget(m_miniPauseButton);
    m_pageTwoLayout->setSpacing(0);
    m_pageTwoLayout->setContentsMargins(10, 0, 5, 0);
    m_pageTwoWidget->setLayout(m_pageTwoLayout);
    m_pageTwoWidget->setFixedHeight(32);


    m_recordLayout->addWidget(m_miniRecordButton, 0, Qt::AlignCenter);
    m_recordLayout->setContentsMargins(20, 5, 20, 5); //左上右下
    m_recordButtonWidget->setLayout(m_recordLayout);

    m_recordStackedWidget->addWidget(m_recordButtonWidget);
    m_recordStackedWidget->addWidget(m_pageTwoWidget);

    //    max_minAndCloseLayout->addWidget(line);
    m_restoreOrMiniAndCloseLayout->addWidget(m_miniButton);
    m_restoreOrMiniAndCloseLayout->addWidget(m_miniCloseButton);
    m_restoreOrMiniAndCloseLayout->setContentsMargins(10, 5, 10, 5); //左上右下
    m_restoreOrMiniAndCloseWid->setLayout(m_restoreOrMiniAndCloseLayout);
    m_restoreOrMiniAndCloseWid->setFixedWidth(78);

    m_miniLayout->addWidget(m_recordStackedWidget, 0, Qt::AlignVCenter);
    m_miniLayout->addWidget(m_miniTimeLabel);
    m_miniLayout->addWidget(m_restoreOrMiniAndCloseWid);
    m_miniLayout->setSpacing(0);
    m_miniLayout->setContentsMargins(0, 0, 0, 0);
    m_miniWidget->setLayout(m_miniLayout);
}

void MiniWindow::initShortcutStyle()
{
    QList<QPushButton*> list = this->findChildren<QPushButton*>();
    for(QPushButton *btn :list) {
        btn->setFocusPolicy(Qt::NoFocus);
    }
}

// 实现键盘响应
void MiniWindow::keyPressEvent(QKeyEvent *event)
{
    // F1快捷键打开用户手册
    if (event->key() == Qt::Key_F1) {
        if (!m_DaemonIpcDbus->daemonIsNotRunning()) {
            // F1快捷键打开用户手册，如kylin-recorder
            //由于是小工具类，下面的showGuide参数要填写"kylin-recorder"
            m_DaemonIpcDbus->showGuide("kylin-recorder");
        }
        return;
    }
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_R) {
        qDebug() << "mini key: ctrl + R" << " "<<  MainWindow::mutual->m_isRecording
                 << "main  start:" << MainWindow::mutual->m_stratOrPause << " mini start:" << m_miniStartOrPause;
        if (!MainWindow::mutual->m_isRecording && !MainWindow::mutual->m_stratOrPause) {
            qDebug() << "可以录音";
            switchPage();
        } else if (MainWindow::mutual->m_isRecording || MainWindow::mutual->m_stratOrPause
                   || m_miniStartOrPause) {
            qDebug() << "可以结束";
            finishSlot();
        }
    }
    // Esc 键停止/完成录制
    if (event->key() == Qt::Key_Escape) {
        if (MainWindow::mutual->m_isRecording || MainWindow::mutual->m_stratOrPause || m_miniStartOrPause) {
            qDebug() << "mini key Esc: 结束录音";
            finishSlot();
        }
    }

    // Space 键暂停/继续录制
    if (event->key() == Qt::Key_Space) {
        if (MainWindow::mutual->m_isRecording || MainWindow::mutual->m_stratOrPause || m_miniStartOrPause) {
            qDebug() << "mini key Space: 暂停/继续录音";
            startOrPauseSlot();
        }
    }

    // M 键添加标记
    if (event->key() == Qt::Key_M) {
        if (MainWindow::mutual->m_isRecording || MainWindow::mutual->m_stratOrPause || m_miniStartOrPause) {
            qDebug() << "mini key M: 添加标记";
            MainWindow::mutual->handleInsertSlot(); // 调用主窗口的打标记槽函数
        }
    }
}

void MiniWindow::normalShow()
{
    // 一定要用showNormal()，不要用showMaximized(),防止点击最小化时，依次点击mini再点击复原导致原窗口尖角
    qDebug() << "切换到正常模式";
    MainWindow::mutual->showNormal();
    kdk::UkuiStyleHelper::self()->removeHeader(MainWindow::mutual);
    MainWindow::mutual->activateWindow();
    MainWindow::mutual->setFocus();
    this->hide();
}
void MiniWindow::closeWindow() //关闭mini和主窗体
{
    MainWindow::mutual->closeWindow();
}
void MiniWindow::handleStartPauseSlot()
{
    emit startOrPauseSignal();
}

void MiniWindow::startOrPauseSlot()
{
    static QTime pauseTime;
    if (m_miniStartOrPause) {
        if (PlayControl::getInstance().getAudioState() == QMediaPlayer::PlayingState) {
            QMessageBox::warning(this, tr("Warning"), tr("Audio is playing, please stop and record again!"));
            return;
        }
        MainWindow::mutual->m_stratOrPause = true; //区分迷你模式中的start_pause
        m_miniStartOrPause = false;
        MainWindow::mutual->play_pause_clicked(); //开始录制
        QTime base_time(0, 0, 0);
        int run_time = MainWindow::mutual->get_sys_runtime(1);
        QTime cut = base_time.addSecs(run_time);  //记录开始时的时间
        int t = pauseTime.secsTo(cut);            //点击暂停时时间与点击恢复计时的时间差值
        qDebug() << t;
        m_miniBaseTime = m_miniBaseTime.addSecs(t);

    } else {
        MainWindow::mutual->m_stratOrPause = false; //区分迷你模式中的start_pause
        m_miniStartOrPause = true;
        MainWindow::mutual->play_pause_clicked();   //暂停录制
        QTime base_time(0, 0, 0);
        int run_time = MainWindow::mutual->get_sys_runtime(1);
        pauseTime = base_time.addSecs(run_time);    //记录一下当前暂停时的时间
        qDebug() << pauseTime;
    }
}

void MiniWindow::changeMiniContinuePauseState(bool isContinue)
{
    if (isContinue) { //若是正在录音的状态，切换为暂停的按钮
        m_pageTwoLayout->removeWidget(m_miniStartButton);
        m_miniStartButton->setParent(NULL);
        m_pageTwoLayout->insertWidget(1, m_miniPauseButton);
        m_miniPauseButton->setToolTip(tr("Pause Space"));
    } else { //若是暂停状态，切换为继续录音的按钮
        m_pageTwoLayout->removeWidget(m_miniPauseButton);
        m_miniPauseButton->setParent(NULL);
        m_pageTwoLayout->insertWidget(1, m_miniStartButton);
        m_miniStartButton->setToolTip(tr("Start Space"));
    }
}
//按照主题颜色切换改变继续和暂停按钮
void MiniWindow::changeMiniCoutinuePauseBtnThemeColor(QString themeColor)
{
//    if (themeColor == "ukui-dark" || themeColor == "ukui-black") {

//        m_miniPauseButton->setImagePath(":/svg/svg/mini-suspend-dark.svg", ":/svg/svg/mini-suspend-hover.svg",
//                                        ":/svg/svg/mini-suspend-click.svg");
//        m_miniStartButton->setImagePath(":/svg/svg/mini-play-dark.svg", ":/svg/svg/mini-play-_hover.svg",
//                                        ":/svg/svg/mini-play-click.svg");

//    } else {

//        m_miniPauseButton->setImagePath(":/svg/svg/mini-suspend-light.svg", ":/svg/svg/mini-suspend-hover.svg",
//                                        ":/svg/svg/mini-suspend-click.svg");
//        m_miniStartButton->setImagePath(":/svg/svg/mini-play-light.svg", ":/svg/svg/mini-play-_hover.svg",
//                                        ":/svg/svg/mini-play-click.svg");
//    }
}

void MiniWindow::finishSlot() //结束录音，并保存文件
{
    qDebug() << "录音结束！";
    MainWindow::mutual->stop_clicked(); //主线程停止录音
    if (m_miniStartOrPause) {
        //记得恢复初始状态
        m_miniStartOrPause = false;
        MainWindow::mutual->m_stratOrPause = false;
    }
    m_miniTimeLabel->setText("00:00:00");
    m_recordStackedWidget->setCurrentIndex(0);                // mini模式切换至录音按钮
    MainWindow::mutual->changePage(0); //主界面切换至录音按钮
}
void MiniWindow::timeDisplay()
{
    QTime base_time(0, 0, 0);
    int run_time = MainWindow::mutual->get_sys_runtime(1);
    QTime currTime = base_time.addSecs(run_time);
    // qDebug()<<currTime;
    int t = MainWindow::mutual->m_baseTime.secsTo(currTime);
    // qDebug()<<baseTime;
    QTime showTime(0, 0, 0);
    showTime = showTime.addSecs(t);
    // qDebug()<<t;
    this->m_timeStr = showTime.toString("hh:mm:ss");
    //    qDebug()<<"*********"<<timeStr;
    if (MainWindow::mutual->m_stratOrPause || MainWindow::mutual->m_isRecording || m_miniStartOrPause)  {
        this->m_miniTimeLabel->setText(m_timeStr);
    }
}
void MiniWindow::switchPage() //换页
{
    if (PlayControl::getInstance().getAudioState() != QMediaPlayer::PlayingState) {
        m_miniBaseTime = m_miniBaseTime.currentTime();
        MainWindow::mutual->switchPage();
        MainWindow::mutual->changePage(1); // mini模式切换至两个按钮页面 //???
        m_recordStackedWidget->setCurrentIndex(1);                //主界面切换至两个按钮页面
    } else {
        QMessageBox::warning(this, tr("Warning"), tr("Audio is playing, please stop and record again!"));
        return;
    }
}
