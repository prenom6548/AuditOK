/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "installermainwidget.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QMouseEvent>
#include <QProcess>
#include <QDebug>
#include "../PluginService/ui_unit/messagebox.h"
#include <QToolTip>
#include <QDesktopWidget>
#include "ksetting.h"
#include "../PluginService/newdemand_unit.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/ui_unit/middleframemanager.h"
#include "../PluginService/base_unit/file_unit.h"
#include "../PluginService/ui_unit/xrandrobject.h"
#include "../PluginService/ksystemsetting_unit.h"
#include "qscreen.h"
#include "../plugins/KInstallAPP/appcheckframe.h"
#include "../plugins/KChoiceEdition/choiseeditionui.h"
#include "../plugins/KUserRegister/userframe.h"

using namespace KServer;
//桌面背景色
#define BGRGB qRgb(33, 83, 156)

namespace KInstaller {

InstallerMainWidget::InstallerMainWidget(QWidget *parent)
    : QWidget(parent)
      ,m_curIndex(0)
    ,m_InstallMode("")
    ,AllStep(0)
    ,w(nullptr)
    ,keyboard(nullptr)
    ,m_flagofGhost(false)
    ,m_ignoreUIsofGhost(0)
{
    m_stackPageMap = Ksetting::getInstance()->iPlugins;
    AllStep = m_stackPageMap.count();
    it = m_stackPageMap.begin();
    this->initUI();
    this->initBackground();
    this->initAllConnect();
    this->addStyleSheet();
    addStackWidget();

    //清理导航栏
    bool isIgnoreAppcheck = false;//是否隐藏“选择你的应用”界面
    for(int k=0;k<m_stackWidget->count();k++)
    {
        if(ReadSettingIni("config","first_mode").contains("oem2")==1 && ReadSettingIni("config", "data-unformat")=="true")//bug175442 如果处于OOBE阶段且上一步安装时选中了保留数据用户，就清除掉用户注册界面对应的导航图标
        {
//            isIgnoreAppcheck = true;
            if(m_stackWidget->widget(k)->objectName().compare("UserFrame")==0)
            {
//                int nRemove = m_stackWidget->count() - k;
//                for (int var = 0; var < nRemove; ++var) {
                    --AllStep;
                    QLabel* pL= m_steps.last();
                    m_steps.removeLast();
                    delete pL;
//                }
//                break;
            }
        }

        if(m_stackWidget->widget(k)->objectName().compare("appcheckframe")==0)//bug174699 当安装第三方应用界面内无内容时清除掉对应的导航图标
        {
            appcheckframe * pAppW = (appcheckframe*)m_stackWidget->widget(k);
            appcheckpage* pApp = (appcheckpage*)pAppW->stack->widget(0);
            if(pApp)
            {
                if(pApp->lst.count() < 1)//如果没有第三方应用可以安装，清除这个界面对应的导航图标
                {
                    --AllStep;
                    QLabel* pL= m_steps.last();
                    m_steps.removeLast();
                    delete pL;
                    isIgnoreAppcheck = true;
                }
            }
            else//如果无apps-third目录，即无选择你的应用功能，也清除这个界面对应的导航图标
            {
                     --AllStep;
                     QLabel* pL= m_steps.last();
                     m_steps.removeLast();
                     delete pL;
                     isIgnoreAppcheck = true;
            }
//            break;
        }
	if(m_stackWidget->widget(k)->objectName().compare("choiseEditionUI")==0)//当“连接安装的版本”界面内无内容时清除掉对应的导航图标
        {
            choiseEditionUI * pChoiEdit = (choiseEditionUI*)m_stackWidget->widget(k);
            if(pChoiEdit)
            {
                if(pChoiEdit->m_EditionsList.count() <= 0){
                    --AllStep;
                    QLabel* pL= m_steps.last();
                    m_steps.removeLast();
                    delete pL;
                    m_isIgnoreChoiceEdition = true;
                }
            }
//            break;
        }

    }

    if(isIgnoreAppcheck)
    {
        QWidget * qWidget = NULL;
        for(int k=0;k<m_stackWidget->count();k++)
        {
            if(m_stackWidget->widget(k)->objectName().compare("appcheckframe")==0)
            {
                qWidget = m_stackWidget->widget(k);
            }
        }
        if(qWidget != NULL)
        {
            m_stackWidget->removeWidget(qWidget);
        }
    }

    if(m_isIgnoreChoiceEdition)
    {
        QWidget * qWidget = NULL;
        for(int k=0;k<m_stackWidget->count();k++)
        {
            if(m_stackWidget->widget(k)->objectName().compare("choiseEditionUI")==0)
            {
                qWidget = m_stackWidget->widget(k);
            }
        }
        if(qWidget != NULL)
        {
            m_stackWidget->removeWidget(qWidget);
        }
    }

    changeLabelDotPix(m_curIndex);
    m_backbtn->setDisabled(true);
    m_backbtn->setVisible(false);

   //showInstallingFrame();
  //  m_installingFrame->showErrorFrame();
    //showInstallingErrorFrame("123");
    QScreen* NowScreen=QGuiApplication::primaryScreen();
    SCreenHeight=NowScreen->size().height();
    SCreenWidth=NowScreen->size().width();
    this->setFixedSize(SCreenWidth, SCreenHeight);

    timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,&InstallerMainWidget::ScreenAdapt);
    timer->start(1000);

 //    QScreen* NowScreen=QGuiApplication::primaryScreen();
  //   connect(NowScreen, SIGNAL(physicalSizeChanged(QSizeF)), this, SLOT(ScreenAdapt(QSizeF)));

    initFocusPolicy();
    installEventFilter(this);
}

InstallerMainWidget::~InstallerMainWidget()
{
    if(w != nullptr) {
        w->close();
        w->deleteLater();
    }
}

void InstallerMainWidget::setInstallMode(QString flagstr)
{
    m_InstallMode = flagstr;
}

void InstallerMainWidget::ScreenAdapt()
{
    QScreen* NowScreen=QGuiApplication::primaryScreen();
    if((NowScreen->size().height()!=SCreenHeight) || (NowScreen->size().width()!=SCreenWidth))
    {
        initBackground();
        SCreenHeight=NowScreen->size().height();
        SCreenWidth=NowScreen->size().width();
        //setFixedHeight();
      //  this->adjustSize();
      //  m_stackWidget->currentWidget()->adjustSize();
        this->setFixedSize(SCreenWidth, SCreenHeight);
        this->updateGeometry();
         m_stackWidget->currentWidget()->updateGeometry();
    }
}

// bool InstallerMainWidget::focusNextPrevChild(bool next)
// {
//     if(next) {
//         if(keyboard!=NULL)
//         {
//             if(keyboard->isVisible())//设置当虚拟键盘换起时不响应键盘焦点
//             {
//                 return false;
//             }
//             return QWidget::focusNextPrevChild(next);
//         }
//         return QWidget::focusNextPrevChild(next);
//     }
// }

void InstallerMainWidget::initUI()
{
    this->setObjectName("InstallerMainWidget");

    QGridLayout* gLayoutl = new QGridLayout();
    gLayoutl->setContentsMargins(24, 24, 24, 24);

    gLayoutl->setColumnStretch(0, 1);
    gLayoutl->setColumnStretch(1, 7);
    gLayoutl->setColumnStretch(2, 1);

    QHBoxLayout* horizontalLayout1 = new QHBoxLayout();
    horizontalLayout1->setMargin(0);
    //加载
    this->m_backbtn = new QPushButton();
    m_backbtn->setFocusPolicy(Qt::TabFocus);
    horizontalLayout1->addWidget(m_backbtn);
    m_backbtn->setObjectName("backbtn");
    m_backbtn->setFlat(true);
    m_backbtn->installEventFilter(this);
    horizontalLayout1->addSpacerItem(new QSpacerItem(10, 48, QSizePolicy::Fixed, QSizePolicy::Fixed));

//    m_backbtn->setPixmap(QPixmap(":/data/svg/back.svg"));

    //加载关闭按钮
    m_clostBtn = new QPushButton();
    m_clostBtn->setFocusPolicy(Qt::TabFocus);
    m_clostBtn->setObjectName("clostBtn");
    horizontalLayout1->addStretch();
//    horizontalLayout1->addWidget(m_clostBtn);
    horizontalLayout1->addSpacing(5);
    m_clostBtn->installEventFilter(this);

    m_clostBtn->setFlat(true);
//    m_clostBtn->setEnabled(false);
//    m_clostBtn->setVisible(false);

    //加载中间窗口显示
   // m_middleFrame = new MiddleFrameManager(this);
    m_stackWidget = new QStackedWidget(this);

    m_dotlayout = new QHBoxLayout;
    m_dotlayout->setSpacing(36);
    m_dotlayout->setMargin(0);
    for (int i = 0; i < AllStep; i++) {
        QLabel* label = new QLabel;
        label->setMaximumHeight(8);
        label->setMinimumHeight(8);
        //label->setPixmap(QPixmap(":/data/png/greydot.svg").scaled(QSize(8, 8)));
        label->setPixmap(QIcon(":/data/png/greydot.svg").pixmap(8,8));
        m_steps.append(label);
        m_dotlayout->addWidget(label);
    }
    m_dotlayout->setAlignment(Qt::AlignHCenter);

    QHBoxLayout* horizontalLayout2 = new QHBoxLayout();
    horizontalLayout2->setMargin(0);

    //加载键盘按钮
    m_keyboardBtn = new QPushButton(this);
    m_keyboardBtn->setFocusPolicy(Qt::TabFocus);
    m_keyboardBtn->setObjectName("keyboardBtn");
    m_keyboardBtn->setFlat(true);
    m_keyboardBtn->installEventFilter(this);

    //加载电源关闭按钮
//    m_powerOffBtn = new QPushButton(this);
//    m_powerOffBtn->setObjectName("powerOffBtn");
//    m_powerOffBtn->setFlat(true);
//    m_powerOffBtn->installEventFilter(this);


    horizontalLayout2->addStretch();
    horizontalLayout2->addWidget(m_keyboardBtn);
    m_keyboardBtn->setEnabled(false);
    m_keyboardBtn->setVisible(false);
    horizontalLayout2->addWidget(m_clostBtn);
    horizontalLayout2->addSpacing(5);

    gLayoutl->addLayout(horizontalLayout1, 0, 0, 1, 3);
    gLayoutl->addWidget(m_stackWidget, 1, 1, 1, 1, Qt::AlignHCenter);
    gLayoutl->addItem(new QSpacerItem(10, 16, QSizePolicy::Expanding, QSizePolicy::Minimum), 2, 1, 1, 1);
    gLayoutl->addLayout(m_dotlayout, 3, 1, 1, 1, Qt::AlignTop | Qt::AlignHCenter);
    gLayoutl->addLayout(horizontalLayout2, 3, 2, 1, 1);

    gLayoutl->setColumnMinimumWidth(0,106);
    gLayoutl->setColumnMinimumWidth(2,106);

    QSizePolicy m_backPolicy= m_backbtn->sizePolicy();
    m_backPolicy.setRetainSizeWhenHidden(true);
    m_backbtn->setSizePolicy(m_backPolicy);

    QSizePolicy m_keyboardPolicy= m_keyboardBtn->sizePolicy();
    m_keyboardPolicy.setRetainSizeWhenHidden(true);
    m_keyboardBtn->setSizePolicy(m_keyboardPolicy);

    this->setLayout(gLayoutl);

}
void InstallerMainWidget::initBackground()
{
    // 无边框
    setWindowFlags(Qt::FramelessWindowHint);

    //全屏
   showFullScreen();

//    QDesktopWidget* desktopWidget = QApplication::desktop();

//    //获取可用桌面大小
//    QRect deskRect = desktopWidget->availableGeometry();

//    //获取设备屏幕大小
//    QRect screenRect = desktopWidget->screenGeometry();

//    int g_nActScreenX = screenRect.width();

//    int g_nActScreenY = screenRect.height();

//    //g_nActScreenX = deskRect.width();

//    //g_nActScreenY = deskRect.height();

//    //获取系统设置的屏幕个数(屏幕拷贝方式该值为1)
//    int g_nScreenCount = desktopWidget->screenCount();

    //设置背景blue色
    QRect rect = KServer::GetScreenRect();
    QPixmap pixmap = QPixmap(":/data/png/openkylin-bk.jpg").scaled(rect.width(), rect.height(), Qt::KeepAspectRatioByExpanding);
    QPalette pal(this->palette());
    pal.setBrush(backgroundRole(), QBrush(pixmap));
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();
}


void InstallerMainWidget::addStyleSheet()
{
    QFile file(":/data/qss/installermainwidget.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void InstallerMainWidget::initAllConnect()
{
    QObject::connect(m_clostBtn, &QPushButton::clicked, this, &InstallerMainWidget::clickCloseButton);
    QObject::connect(m_keyboardBtn, &QPushButton::clicked, this, &InstallerMainWidget::clickkeyButton);
    QObject::connect(m_backbtn,&QPushButton::clicked,[=](){
        emit backwidgetcheck(m_stackWidget->currentWidget());

    });
   // QObject::connect(m_backbtn, &QPushButton::clicked, this, &InstallerMainWidget::clickBackButton);

}

//关闭按钮响应槽
void InstallerMainWidget::clickCloseButton()
{
    // 1. 保存弹窗弹出前的焦点控件
    QWidget* lastFocusWidget = QApplication::focusWidget();
    qDebug() << __func__ << "弹窗前焦点控件："
             << (lastFocusWidget ? lastFocusWidget->objectName() : "无");

    // 2. 判断是否为live系统
    QString infoMessage;
    if (!KServer::KReadFile("/proc/cmdline").contains("live")) {
        infoMessage = tr("About to exit the installer, restart the computer.");
    } else {
        infoMessage = tr("About to exit the installer and return to the trial interface.");
    }

    // 3. 创建并配置MessageBox
    MessageBox *message = new MessageBox(this);
    message->setMessageInfo(infoMessage);

    // 4. 弹窗关闭后，恢复焦点到之前保存的控件,无论点击确认/取消都会触发
    connect(message, &MessageBox::finished, this, [=](int result) {
        // 仅在弹窗关闭且未执行退出操作时恢复焦点（如果点击确认并退出，则无需恢复）
        bool isExit = (result == QDialog::Accepted && message->execid == 1);
        if (!isExit && lastFocusWidget)
            lastFocusWidget->setFocus();
    });

    // 显示弹窗（阻塞式）
    int msgResult = message->exec();

    // 处理弹窗结果（确认退出）
    if (msgResult == QDialog::Accepted && message->execid == 1) {
        qDebug() << Q_FUNC_INFO << "OK";
        if (!KServer::KReadFile("/proc/cmdline").contains("live")) {
            this->close();
            QProcess::execute("systemctl reboot");
        } else {
            this->close();
            this->destroy();
            exit(0);
        }
    }

    delete message;
}

//键盘操作响应槽
void InstallerMainWidget::clickkeyButton()
{
    //添加虚拟键盘
    if(keyboard != nullptr) {
//        keyboard->close();
        return; //筛选掉虚拟键盘在未聚焦时其中空格按钮对本函数的误触发 bug163220
    }
    keyboard = new VirtualKeyboard(this);
    connect(keyboard, &VirtualKeyboard::aboutToClose, [=]{
        keyboard->close();
        keyboard = nullptr;
    });
    keyboard->show();

    if(m_stackWidget->currentWidget()->objectName().compare("UserFrame")==0)//bug215368
    {
        UserFrame * pUserF = (UserFrame*)m_stackWidget->currentWidget();
        if(pUserF != NULL)
        {
            //pUserF->usregister->m_username->m_lineEdit->setFocus();
            //pUserF->usregister->m_username->setFocus();
        }
    }

}

//电源操作响应槽
void InstallerMainWidget::clickPowerButton()//
{
//    QProcess process;
//    process.startDetached("ukui-session-tools");
    QString info = tr("Installer is about to exit and the computer will be shut down.");

    MessageBox *message = new MessageBox();
    message->setMessageInfo(info);
    if(message->exec() == QDialog::Accepted && message->execid == 1) {
        QProcess::execute("systemctl poweroff");
    }
    m_stackWidget->currentWidget()->setFocus();
}

//加载所有视图显示的窗口
void InstallerMainWidget::addStackWidget()
{
    if(m_show){
        this->setVisible(false);
    }
    for (it; it != m_stackPageMap.end(); ++it) {
        IPlugin* iplugin = (*it).second;
        MiddleFrameManager* frame = (MiddleFrameManager*)iplugin->CreateObject(this);
        m_stackWidget->addWidget(frame);
	
	if(m_show){
                qDebug() << "set ui show";
                this->setVisible(true);
                m_show=false;
        }
        
	connect(frame, &MiddleFrameManager::signalStackPageChanged, this, &InstallerMainWidget::changeStackPage);
        connect(frame, &MiddleFrameManager::signalStackFrameManager, this, &InstallerMainWidget::addStackWidgetPage);
        connect(this, &InstallerMainWidget::signalInstalling, frame, &MiddleFrameManager::slotStartInstall);
        connect(frame, &MiddleFrameManager::signalKeyboradClose, [=]{
            if(keyboard != nullptr) {
                keyboard->close();
            }
            });
        connect(this,&InstallerMainWidget::sendmessage,frame,&MiddleFrameManager::getmessage);
        //connect(frame, &MiddleFrameManager::signalStartInstall, this, &InstallerMainWidget::showInstallingFrame);
        connect(frame, &MiddleFrameManager::signalInstallError, this, &InstallerMainWidget::showInstallingErrorFrame);
        connect(frame, &MiddleFrameManager::signalInstalScriptlFinished, [=]{emit signalScriptFinished();});
        connect(frame, &MiddleFrameManager::backspacepressed, this, &InstallerMainWidget::clickBackButton);
        connect(this,&InstallerMainWidget::backwidgetcheck,frame,&MiddleFrameManager::clickPrevButton);
        connect(this, &InstallerMainWidget::signalMainFrameSize, frame, &MiddleFrameManager::setSizebyFrame);

	if (frame->objectName() == "UserFrame"){
            m_UFplugin = (UserFrame *)frame;
            m_UFplugin->installEventFilter(this);
        }

//       qDebug() << "signalInstalScriptlFinished:" <<  connect(frame, &MiddleFrameManager::signalInstalScriptlFinished, this, &InstallerMainWidget::getFinishedSignal);
    }
    m_stackCount = m_stackWidget->count();
    m_stackWidget->setCurrentIndex(m_curIndex);
}

void InstallerMainWidget::setLabelDotVisible(bool flag)
{
    for (int i = 0;i < AllStep; i++) {
        m_steps.at(i)->setVisible(flag);
    }
}

void InstallerMainWidget::changeStackPage(int step)
{
    // qDebug()<< "changePage="<<step;
    if(step==100)
    {
        if(m_stackWidget->currentWidget()->objectName().compare("ImgFrame")==0)//跳转命令是从“选择安装途径”界面发出
        {
            if( KServer::ReadSettingIni("setting","ghost")=="true" )//跳转到“选择安装方式”界面
            {
                ImgIndex=m_curIndex;
                for(int k=0;k<m_stackWidget->count();k++)
                {
                    if(m_stackWidget->widget(k)->objectName().compare("KPartition")==0)//直接跳转到“选择安装方式”界面
                    {
                            if( !m_flagofGhost )//隐藏导航栏中相应的图标
                            {
                                m_flagofGhost = true;
                                m_ignoreUIsofGhost = m_stackWidget->count() - k - 1;
                                for (int var = 0; var < m_ignoreUIsofGhost; ++var) {
                                    --AllStep;
                                    QLabel* pL= m_steps.last();
                                    m_steps.removeLast();
                                    delete pL;
                                }
                            }

                        m_curIndex=k;
                        ImgNext=k;
                        m_stackWidget->setCurrentIndex(m_curIndex);
                        // m_stackWidget->currentWidget()->setFocus();
                        m_stackWidget->currentWidget()->update();
			if (m_stackWidget->currentWidget()->objectName() == "KPartition") {
                                MiddleFrameManager* MiddleFrame = (MiddleFrameManager*)m_stackWidget->currentWidget();
                                emit MiddleFrame->testSignal("KPartition");
                        }
                        break;
                    }
                }
            }
            else//跳转到下个插件
            {
                ImgIndex=0;
                ImgNext=0;

                //恢复被隐藏的导航栏中图标
                if(m_flagofGhost)
                {
                    for (int var = 0; var < m_ignoreUIsofGhost; ++var)
                    {
                        ++AllStep;
                        QLabel* label = new QLabel;
                        label->setMaximumHeight(8);
                        label->setMinimumHeight(8);
                        //label->setPixmap(QPixmap(":/data/png/greydot.svg").scaled(QSize(8, 8)));
                        label->setPixmap(QIcon(":/data/png/greydot.svg").pixmap(8,8));
                        m_steps.append(label);
                        m_dotlayout->addWidget(label);
                    }
                    m_ignoreUIsofGhost = 0;
                    m_flagofGhost = false;
                }

                //跳转
                ++m_curIndex;
                if(m_curIndex >= m_stackWidget->count()) {
                    emit signalInstalling();
                    if(keyboard)
                    {
                        keyboard->close();
                        keyboard = nullptr;
                    }
                    if(m_InstallMode.contains("oem2"))
                       showOEMInstallingFrame();
                    else
                       showInstallingFrame();
                    return;
                }

                m_stackWidget->setCurrentIndex(m_curIndex);
                // m_stackWidget->currentWidget()->setFocus();
                emit sendmessage(m_stackWidget->currentWidget(),"next");
                if (m_stackWidget->currentWidget()->objectName() == "KPartition") {
                    MiddleFrameManager* MiddleFrame = (MiddleFrameManager*)m_stackWidget->currentWidget();
                    emit MiddleFrame->testSignal("KPartition");
                }
            }
        }
    }
    else if (step == 88) {
                m_curIndex++;
                m_curIndex++;
                if(m_curIndex >= m_stackWidget->count())
                    m_curIndex = m_stackWidget->count() - 1;
                m_stackWidget->setCurrentIndex(m_curIndex);
                // m_stackWidget->currentWidget()->setFocus();
                //emit sendmessage(m_stackWidget->currentWidget(),"next");
    }
    else if (step > 0) {

        ++m_curIndex;
        if(step==2)
            ++m_curIndex;

        if(m_curIndex >= m_stackWidget->count()) {
            m_curIndex = m_stackWidget->count() - 1;
            emit signalInstalling();
            if(keyboard)
            {
                keyboard->close();
                keyboard = nullptr;
            }
            if(m_InstallMode.contains("oem2"))
               showOEMInstallingFrame();
            else
               showInstallingFrame();
            return;
        }

        m_stackWidget->setCurrentIndex(m_curIndex);
        // m_stackWidget->currentWidget()->setFocus();
        emit sendmessage(m_stackWidget->currentWidget(),"next");
        if (m_stackWidget->currentWidget()->objectName() == "KPartition") {
            MiddleFrameManager* MiddleFrame = (MiddleFrameManager*)m_stackWidget->currentWidget();
            emit MiddleFrame->testSignal("KPartition");
        }
    } 
    else if (step == -2) {
        --m_curIndex;
        --m_curIndex;
        if(m_curIndex <= 0)
            m_curIndex = 0;
        m_stackWidget->setCurrentIndex(m_curIndex);
        // m_stackWidget->currentWidget()->setFocus();
        emit sendmessage(m_stackWidget->currentWidget(),"prext");
    }
    else if (step < 0 ) {

        if(m_curIndex==ImgNext)
        {
            m_curIndex=ImgIndex;
            ImgIndex=0;
            ImgNext=0;
        }
        else
            --m_curIndex;

        if (m_curIndex <= 0) {
            m_curIndex = 0;
        }
        m_stackWidget->setCurrentIndex(m_curIndex);
        // m_stackWidget->currentWidget()->setFocus();
        emit sendmessage(m_stackWidget->currentWidget(),"prext");
    }

    qDebug()<<m_stackWidget->currentWidget()->objectName();

    if(m_stackWidget->currentWidget()->objectName() == "KPartition"){
        QScreen* NowScreen=QGuiApplication::primaryScreen();
        SCreenHeight=NowScreen->size().height();
        SCreenWidth=NowScreen->size().width();
       emit signalMainFrameSize(this, SCreenWidth, SCreenHeight);
    }

    changeLabelDotPix(m_curIndex);
    if (m_curIndex == 0) {
        m_backbtn->setEnabled(false);
        m_backbtn->setVisible(false);
    } else {
        m_backbtn->setEnabled(true);
        m_backbtn->setVisible(true);
    }
    
    if(m_stackWidget->currentWidget()->objectName() == "UserFrame") {
        m_keyboardBtn->setEnabled(true);
        m_keyboardBtn->setVisible(true);
	UserFrame * pUserF = (UserFrame*)m_stackWidget->currentWidget();
        if(pUserF != NULL){//bug218972
            //在OOBE阶段当全部文本框都为空时由第一个文本框自动获取焦点
            if( m_InstallMode.contains("oem2") &&
//                pUserF->usregister->m_username->m_lineEdit->text().isEmpty() &&
//                pUserF->usregister->m_hostname->m_lineEdit->text().isEmpty() &&
//                pUserF->usregister->m_passwd->m_lineEdit->text().isEmpty() &&
//                pUserF->usregister->m_confirm_passwd->m_lineEdit->text().isEmpty()
                    pUserF->usregister->m_username->text().isEmpty() &&
                    pUserF->usregister->m_hostname->text().isEmpty() &&
                    pUserF->usregister->m_passwd->text().isEmpty() &&
                    pUserF->usregister->m_confirm_passwd->text().isEmpty()
              ){
               // pUserF->usregister->m_username->m_lineEdit->setFocus();
                pUserF->usregister->m_username->setFocus();
            }
        }
    } else {
        m_keyboardBtn->setEnabled(false);
        m_keyboardBtn->setVisible(false);
    }
    //showInstallingFrame();
}

void InstallerMainWidget::clickBackButton()
{
    changeStackPage(-1);
    setLabelDotVisible(true);
    MiddleFrameManager* p=(MiddleFrameManager*)m_stackWidget->currentWidget();
}

void InstallerMainWidget::changeLabelDotPix(int step)
{
    for (int j = 0; j < AllStep; j++) {
        if (step == j) {
            //m_steps.at(j)->setPixmap(QPixmap(":/data/png/dot.svg").scaled(QSize(8, 8)));
            m_steps.at(j)->setPixmap(QIcon(":/data/png/dot.svg").pixmap(8,8));
        } else
         // m_steps.at(j)->setPixmap(QPixmap(":/data/png/greydot.svg").scaled(QSize(8, 8)));
            m_steps.at(j)->setPixmap(QIcon(":/data/png/greydot.svg").pixmap(8,8));
    }
}

bool InstallerMainWidget::eventFilter(QObject *watched, QEvent *event)
{

    if(watched->objectName() == "keyboardBtn") {
        if(event->type() == QEvent::Enter) {
            if(w != nullptr) {
                w->close();
                w=NULL;
            }
//            w = new ArrowWidget;
//            w->setObjectName("ArrowWidget");
//            w->setDerection(ArrowWidget::Derection::down);
//            w->setText(tr("keyboard"));//        w->setCenterWidget(textLabel);
////            QPoint pos = m_keyboardBtn->mapToGlobal(QPoint());
//            w->move(m_keyboardBtn->x() + (m_keyboardBtn->width() - w->width()) / 2, m_keyboardBtn->y() - w->height() - 4);
////            w->move(pos.x() - 20, pos.y() - w->height());
//            w->show();
            initToolTip(m_keyboardBtn);

            w->show();
            //connect(m_keyboardBtn, &QPushButton::clicked, [=]{w->close();});
            return true;

        } else if(event->type() == QEvent::Leave){
            if(w != nullptr) {
                w->close();
                w=NULL;
            }
            return false;
        }
        else if(event->type()==QEvent::KeyPress)
        {
            QKeyEvent *ev=(QKeyEvent*)event;
            if(ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return)
            {
                m_keyboardBtn->click();
                return true;
            }
        }
    }
    if(watched->objectName() == "clostBtn") {
        if(event->type() == QEvent::Enter) {
            if(w != nullptr) {
                w->close();
                w=NULL;
            }
//            w = new ArrowWidget;
//            w->setObjectName("ArrowWidget");
//            w->setDerection(ArrowWidget::Derection::down);
//            w->setText(tr("quit"));//        w->setCenterWidget(textLabel);
////            QPoint pos = m_clostBtn->mapToGlobal(QPoint());
//            w->move(m_clostBtn->x() + (m_clostBtn->width() - w->width()) / 2, m_clostBtn->y() - w->height() - 4);
////            w->move(pos.x() - 20, pos.y() - w->height());
//            w->show();
            initToolTip(m_clostBtn);

            w->show();
            //connect(m_clostBtn, &QPushButton::clicked, [=]{w->close();});
            return true;

        } else if(event->type() == QEvent::Leave){
            if(w != nullptr) {
                w->close();
                w=NULL;
            }
            return false;
        }
        else if(event->type()==QEvent::KeyPress)
        {
            QKeyEvent *ev=(QKeyEvent*)event;
            if(ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return)
            {
                m_clostBtn->click();
                return true;
            }
        }
    }
    if(watched->objectName() == "backbtn") {
        if(event->type() == QEvent::Enter) {
            if(w != nullptr) {
                w->close();
                w=NULL;
            }
//            w = new ArrowWidget;
//            w->setObjectName("ArrowWidget");
//            w->setDerection(ArrowWidget::Derection::up);
//            w->setText(tr("back"));//        w->setCenterWidget(textLabel);
////            QPoint pos = m_backbtn->mapToGlobal(QPoint());

//            w->move(m_backbtn->x() + (m_backbtn->width() - w->width()) / 2, m_backbtn->y() + m_backbtn->height() + 4);
//            w->show();
            initToolTip(m_backbtn);

            w->show();
            //connect(m_backbtn, &QPushButton::clicked, [=]{w->close();});
            return true;

        } else if(event->type() == QEvent::Leave){
            if(w != nullptr) {
                w->close();
                w=NULL;
            }
            return false;
        }
        else if(event->type()==QEvent::KeyPress)
        {
            QKeyEvent *ev=(QKeyEvent*)event;
            if(ev->key() == Qt::Key_Enter || ev->key() == Qt::Key_Return)
            {
                m_backbtn->click();
                return true;
            }
        }
    }
    return QWidget::eventFilter(watched, event);
}


void InstallerMainWidget::addStackWidgetPage(QWidget *widget)
{
    qDebug() << widget->objectName();
    setLabelDotVisible(true);
    for (int i = 0; i < m_stackWidget->count(); i++) {
        if (m_stackWidget->widget(i) == widget) {
            m_stackWidget->setCurrentWidget(widget);
            m_curIndex = i;
            return;
        }
    }
    m_stackWidget->insertWidget(m_curIndex+1,widget);
    m_curIndex++;
    m_stackWidget->setCurrentIndex(m_curIndex);
}

void InstallerMainWidget::showInstallingFrame()
{
    setLabelDotVisible(false);
    qDebug() << "showInstallingFrame:InstallingFrame";
    m_installingFrame = new InstallingFrame;
    m_installingFrame->setParent(this);
    m_installingFrame->slide->setFocus();
    m_stackWidget->addWidget(m_installingFrame);
    m_curIndex++;
    m_stackWidget->setCurrentWidget(m_installingFrame);
    connect(m_installingFrame, &InstallingFrame::signalFinished, this, &InstallerMainWidget::showFinishedFrame);
    connect(this, &InstallerMainWidget::signalScriptFinished, m_installingFrame, &InstallingFrame::findFile);
    connect(this, &InstallerMainWidget::signalInstallingClose, m_installingFrame, &InstallingFrame::closeW);

    updateButtonStatus();
}

void InstallerMainWidget::showInstallingErrorFrame(QString str)
{
    InstallAddFileToEFI();
    emit signalInstallingClose();
    setLabelDotVisible(false);
 //   m_installingFrame->showErrorFrame();
    qDebug() << "showInstallingErrorFrame:";
    m_installingFrame->showErrorFrame();
    //m_errorFrame = new InstallErrorFrame();
   // m_errorFrame->setParent(this);
  //  m_errorFrame->show();
  //  m_errorFrame->setFocus();
  //  m_stackWidget->addWidget(m_errorFrame);
 //   m_curIndex++;
//    m_stackWidget->setCurrentWidget(m_errorFrame);

    updateButtonStatus();
}

void InstallerMainWidget::showFinishedFrame()
{
    emit signalInstallingClose();
    setLabelDotVisible(false);
    qDebug() << Q_FUNC_INFO << "showFinishedFrame:";
    m_finishedFrame = new FinishedFrame;
    m_finishedFrame->setParent(this);
    m_finishedFrame->show();
    m_finishedFrame->setFocus();
    m_stackWidget->addWidget(m_finishedFrame);
    m_curIndex++;
    m_stackWidget->setCurrentWidget(m_finishedFrame);
    m_stackWidget->currentWidget()->setFocus();
    updateButtonStatus();
}

void InstallerMainWidget::updateButtonStatus()
{
    m_backbtn->setVisible(false);
    m_backbtn->setEnabled(false);

    m_clostBtn->setVisible(false);
    m_clostBtn->setEnabled(false);
    //    m_powerOffBtn->setVisible(false);
    //    m_powerOffBtn->setEnabled(false);

    m_keyboardBtn->setVisible(false);
    m_keyboardBtn->setEnabled(false);
}

bool InstallerMainWidget::showOEMInstallingFrame()
{
    qDebug() << "showOEMInstallingFrame";
//    int cur = m_stackWidget->currentIndex();
//    int count = m_stackWidget->count();
//    if( cur == count - 1) {
        qDebug() << "m_InstallMode =" <<m_InstallMode;
        if(m_InstallMode == "oem2") {
            setLabelDotVisible(false);
	    m_oemFrame = new InstallingOEMConfigFrame(m_UFplugin);
            m_oemFrame->setParent(this);
            m_oemFrame->show();
            connect(m_oemFrame, &InstallingOEMConfigFrame::signalExecError, this, &InstallerMainWidget::showInstallingErrorFrame);
            m_stackWidget->addWidget(m_oemFrame);
            m_curIndex++;
            m_stackWidget->setCurrentWidget(m_oemFrame);
            updateButtonStatus();
            return true;
        }
        else
            return false;
//    } else
        //        return false;
}

void InstallerMainWidget::initToolTip(QPushButton* button)
{

    w=new QDialog(this);
    w->setWindowFlags(Qt::FramelessWindowHint);
    w->setAttribute(Qt::WA_DeleteOnClose);
    //ww->setStyleSheet("background:red;");
   //QLayout*layout= ww->layout();
    QHBoxLayout* hMainLayout = new QHBoxLayout;
    hMainLayout->setContentsMargins(0,0,0,0);

    QLabel *tooltip_text = new QLabel(w);
    tooltip_text->setObjectName("tooltip_text");
    tooltip_text->setProperty("class", "font14px");

    tooltip_text->setWordWrap(true);
    tooltip_text->setAlignment(Qt::AlignLeft);

    tooltip_text->setStyleSheet("QWidget{border-radius: 4px;background:#FFFFFF;padding: 5px 5px 5px 5px;color:#000000;font-size:14px;}");


    hMainLayout->addWidget(tooltip_text);
    QString text;
    if(button==m_backbtn)
    {
        text=(tr("back"));
    }
    else if(button==m_keyboardBtn)
    {
        text=(tr("keyboard"));
    }
    else if(button==m_clostBtn)
    {
        text=(tr("quit"));
    }
    tooltip_text->setText(text);
    w->setLayout(hMainLayout);
    w->adjustSize();
    if(button==m_backbtn)
    {
       w->move(button->x() + (button->width() - w->width()) / 2, button->y() + button->height() + 4);
    }
    else
    {
       w->move(button->x() + (button->width() - w->width()) / 2, button->y() - w->height() - 4);
    }
    //w->show();
}

// 初始化走焦策略，默认焦点为 '选择语言' 插件的第一个按钮
void InstallerMainWidget::initFocusPolicy()
{
    //  窗口默认无焦点
    this->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_stackWidget->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_stackWidget->currentWidget()->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    QString pluginName = m_stackWidget->currentWidget()->objectName();

    if (pluginName == "plugin_KChoiceLanguage") {
        LanguageFrame* w = static_cast<LanguageFrame*>(m_stackWidget->currentWidget());
        w->m_LanguageBtnList.first()->setFocus();
       // this->setTabOrder(this, w->m_LanguageBtnList.first());
        // 自定义走焦顺序
        this->setTabOrder(w->m_nextBtn, m_clostBtn);
        this->setTabOrder(m_clostBtn, m_backbtn);
       // this->setTabOrder(m_backbtn, w->m_LanguageBtnList.first());
        //this->setFocus();
    }
}

}
