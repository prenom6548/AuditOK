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

#include "languageframe.h"
#include <QWidget>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QProcess>
#include <QApplication>
#include <QLocale>
#include <QKeyEvent>

#include <QFile>
#include <QTextStream>
#include "../PluginService/klinguist.h"
#include "../PluginService/ksystemenv.h"
#include "../PluginService/kdir.h"
#include "../PluginService/ui_unit/keyeventcontrol.h"
#include "../PluginService/ui_unit/xrandrobject.h"
#include <QDir>
#include <QFileInfo>
#include <QScrollBar>
using namespace KServer;

namespace KInstaller {

LanguageFrame::LanguageFrame(QWidget *parent) : MiddleFrameManager(parent)
{
 #ifdef   LANGUAGE_ZH_CN
    QPair<QString,QPair<PushButtonCheck*,QString>> pir1;
    pir1.first="zh_CN";
    pir1.second.first=NULL;
    pir1.second.second="中文(简体)";
    emulang.append(pir1);
  #endif

 #ifdef LANGUAGE_ZH_HK
   QPair<QString,QPair<PushButtonCheck*,QString>> pir5;
   pir5.first="zh_HK";
   pir5.second.first=NULL;
   pir5.second.second="中文(繁體)";
   emulang.append(pir5);
 #endif

 #ifdef LANGUAGE_EN_US
    QPair<QString,QPair<PushButtonCheck*,QString>> pir2;
    pir2.first="en_US";
    pir2.second.first=NULL;
    pir2.second.second="English";
    emulang.append(pir2);
   #endif

 #ifdef LANGUAGE_BO_CN
    QPair<QString,QPair<PushButtonCheck*,QString>> pir3;
    pir3.first="bo_CN";
    pir3.second.first=NULL;
    pir3.second.second="བོད་ཡིག";//藏语
    emulang.append(pir3);
   #endif

 #ifdef LANGUAGE_RU_RU
    QPair<QString,QPair<PushButtonCheck*,QString>> pir4;
    pir4.first="ru_RU";
    pir4.second.first=NULL;
    pir4.second.second="русский язык";//俄语
    emulang.append(pir4);
  #endif

 #ifdef LANGUAGE_UG_CN
   QPair<QString,QPair<PushButtonCheck*,QString>> pir6;
   pir6.first="ug_CN";
   pir6.second.first=NULL;
   pir6.second.second="Uyghur";//维吾尔语
   emulang.append(pir6);
 #endif

#ifdef LANGUAGE_KK_CN
  QPair<QString,QPair<PushButtonCheck*,QString>> pir7;
  pir7.first="kk_CN";
  pir7.second.first=NULL;
  pir7.second.second="Қазақ";//哈萨克语
  emulang.append(pir7);
#endif

#ifdef LANGUAGE_KY_CN
  QPair<QString,QPair<PushButtonCheck*,QString>> pir8;
  pir8.first="ky_CN";
  pir8.second.first=NULL;
  pir8.second.second="قىرعىز تىلى";//柯尔克孜语
  emulang.append(pir8);
#endif

    //翻译列表，按照从上倒下的按键顺序排列
  //  emulang.append(qMakePair("zh_CN","中文(简体)"));
//    emulang.append(qMakePair("en_US","English"));
//    emulang.append(qMakePair("bo_CN","བོད་ཡིག"));
//    emulang.append(qMakePair("ru_RU","русский язык"));

    QList<LanguageSetInINI> LanguageList = ReadLanguageIni();//获取安装器已支持语言的列表
    for (int i = 0; i < LanguageList.size(); ++i)
    {
        if(LanguageList.at(i).lsInCurrentList)//加载显示语言列表
        {
            QPair<QString,QPair<PushButtonCheck*,QString>> pir;
            pir.first=LanguageList.at(i).Language;
            pir.second.first=NULL;
            pir.second.second=LanguageList.at(i).display;
            emulang.append(pir);
        }
    }

    m_pQtTranslator = new QTranslator;
    m_pCurTranslator = new QTranslator;
    this->initNextBnt();
    initUI();
    addStyleSheet();
    this->setObjectName("plugin_KChoiceLanguage");
    initFocusPolicy();
//    readSettingIni();
}

LanguageFrame::~LanguageFrame()
{
    for(int k=0;k< emulang.count();k++)
    {
        PushButtonCheck * m_Btn = emulang[k].second.first;
        if(m_Btn)
            delete m_Btn;
    }
}

void LanguageFrame::initUI()
{
    // 1. 主布局：垂直居中，控制整体间距
    QVBoxLayout* mainLayout = new QVBoxLayout();
    //mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setContentsMargins(0, 0, 0, 10);
    m_Widget->setLayout(mainLayout);

    // 2. 标题标签：水平居中，使用自定义QLabel类，若字体高度大于控件高度，等比例缩放
    m_mainTitle = new ScalableLabel();
    m_mainTitle->setObjectName("mainTitle");
    m_mainTitle->setAlignment(Qt::AlignCenter);
    m_mainTitle->setFixedHeight(50);

    
    m_mainTitle->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    mainLayout->addWidget(m_mainTitle);
    mainLayout->addSpacing(45);

    m_pBtnScroll = new QScrollArea();
    m_pBtnScroll->setObjectName("m_pBtnScroll");
    m_pBtnScroll->setFrameShape(QFrame::NoFrame);//无边框
    m_pBtnScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_pBtnScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pBtnScroll->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
  
    m_pBtnScroll->verticalScrollBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);

    int nScrolWid = KServer::GetScreenRect().width()*0.3;
    if(nScrolWid < 420){
        nScrolWid=420;
    }
    int nSrolHeigh = KServer::GetScreenRect().height() - 323;
    m_pBtnScroll->setMinimumWidth(nScrolWid);

    if(nSrolHeigh%(45+40) < 40){
        m_pBtnScroll->setFixedHeight(nSrolHeigh/(45+40)*(45+40) - (45+40));
    }
    else{
        m_pBtnScroll->setFixedHeight(nSrolHeigh/(45+40)*(45+40));
    }

    m_pBtnScroll->setStyleSheet("QScrollArea{background-color:transparent;}");
    m_pBtnScroll->setWidgetResizable(true);


    // 3. 语言按钮区域：垂直排列，水平居中
    QWidget* buttonsWidget = new QWidget();
    m_pBtnScroll->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
    buttonsWidget->setFixedWidth(nScrolWid);
    QVBoxLayout* buttonsLayout = new QVBoxLayout(buttonsWidget);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
   // buttonsLayout->setSpacing(40);
    buttonsLayout->setSpacing(10);
    //buttonsLayout->setAlignment(Qt::AlignCenter);
    buttonsLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
   
    buttonsWidget->setStyleSheet("QWidget{background-color:transparent;}");


    for (int k = 0; k < emulang.count(); k++) {
        PushButtonCheck* m_Btn = new PushButtonCheck(emulang.at(k).second.second);
        m_Btn->setFixedSize(320, 48);
        m_Btn->setObjectName(emulang.at(k).first);


        if(k==0)
        {
            m_Btn->installEventFilter(this);
        }

       	connect(m_Btn, &PushButtonCheck::clicked_obj, this, &LanguageFrame::changeLanguage);
        connect(this, &LanguageFrame::changeLanguage_sig, m_Btn, &PushButtonCheck::other_clicked_slot);
        emulang[k].second.first = m_Btn;

        m_curSystenlocal = QLocale::system().name();
        if (m_curSystenlocal == emulang.at(k).first || (m_curSystenlocal == "C" && emulang.at(k).first == "en_US") ) {
            changeLanguage(m_Btn);          // 默认语言为当前系统语言
        }
        else if (0 == k) {
            changeLanguage(m_Btn);
        }
        m_Btn->setStyleSheet("QPushButton{font-size: 14px;font-family: Noto Sans CJK SC;font-weight: 500;line-height: 35px;color: #FFFFFF;background-color: rgba(255, 255, 255, 0.1);}"
                             "QPushButton:focus{border: 2px solid white;outline: none;}"
                             "QPushButton:hover{background-color: rgba(255, 255, 255, 0.2);}"
                             "QPushButton:checked{background-color: rgba(255, 255, 255, 0.35);}");
        buttonsLayout->addWidget(m_Btn);
        m_Btn->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
        m_Btn->setFocusPolicy(Qt::TabFocus);
        m_LanguageBtnList.append(m_Btn);
    }
    
    // 4. 底部间隔：自动填充剩余空间（让按钮区域保持垂直居中)
    m_pBtnScroll->setWidget(buttonsWidget);
    m_pBtnScroll->setAlignment(Qt::AlignCenter);
    
    QWidget* gWidget = new QWidget();
    QGridLayout* gLayoutl = new QGridLayout(gWidget);
    gLayoutl->addWidget(m_pBtnScroll, 3, 1, Qt::AlignCenter);
    mainLayout->addWidget(gWidget);   // 按钮组加入主布局
    //mainLayout->addWidget(m_pBtnScroll);   // 按钮组加入主布局
    QSpacerItem* bottomSpacer = new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding);
    mainLayout->addItem(bottomSpacer);
    translateStr();
}

void LanguageFrame::addStyleSheet()
{
    QFile file(":/res/qss/KChoiceLanguage.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = QString("");
    stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

//读写systemsetting.ini文件
void LanguageFrame::readSettingIni()
{
//    m_curSystenlocal = ReadSettingIni("config", "language");
//    should not read config
//    get locale from system
      m_curSystenlocal = QLocale::system().name();
}

void LanguageFrame::writeSettingIni()
{
    QString strLang = ReadSettingIni("config", "language");
    if(strLang != m_curSystenlocal)
        WriteSettingToIni("config", "language", m_curSystenlocal);
}

void LanguageFrame::changeLanguage(PushButtonCheck* obj)
{
    QString langPath = GetKylinInstallPath() + "language/";
    emit changeLanguage_sig(obj->objectName());

    if (m_curSystenlocal != obj->objectName()) {
        qApp->removeTranslator(m_pCurTranslator);
        qApp->removeTranslator(m_pQtTranslator);
    }

    m_curSystenlocal = obj->objectName();
    if (m_pCurTranslator->load(langPath + m_curSystenlocal + ".qm")) {
        qApp->installTranslator(m_pCurTranslator);
    }

    if (m_pQtTranslator->load(langPath + "qt_"+m_curSystenlocal+".qm")) {
        qApp->installTranslator(m_pQtTranslator);
    }

    qDebug()<<"language qm file path="<<langPath;
    qDebug()<<"current language="<<m_curSystenlocal;
    unsetenv("LANG");
    setenv("LANG", obj->objectName().toStdString().c_str(),1);
    writeSettingIni();
    // this->setFocus();
    obj->setChecked(true);
    qApp->setProperty("Language", m_curSystenlocal);
}

void LanguageFrame::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

void LanguageFrame::showEvent(QShowEvent *event)
{
    m_firstfocusget=true;
    m_LanguageBtnList.first()->setFocus();
}

bool LanguageFrame::eventFilter(QObject *watched, QEvent *event)
{
    if(m_LanguageBtnList.count()!=0)
    {
        if(watched==m_LanguageBtnList.first()&&event->type()==QEvent::FocusIn)
        {
            if(m_firstfocusget==true)
            {
                m_LanguageBtnList.first()->setStyleSheet(QString(
                    "QPushButton{font-size: 14px;font-family: Noto Sans CJK SC;font-weight: 500;line-height: 35px;color: #FFFFFF;background-color: rgba(255, 255, 255, 0.1);}"
                    " QPushButton#%1:focus {"
                    " border: 0px solid transparent;"
                    "outline: none;"
                    "}"
                    "QPushButton:hover{background-color: rgba(255, 255, 255, 0.2);}"
                    "QPushButton:checked{background-color: rgba(255, 255, 255, 0.35);}"
                    ).arg(m_LanguageBtnList.first()->objectName()));
                m_firstfocusget=false;
            }
            else
            {
                m_LanguageBtnList.first()->setStyleSheet(
                    "QPushButton{font-size: 14px;font-family: Noto Sans CJK SC;font-weight: 500;line-height: 35px;color: #FFFFFF;background-color: rgba(255, 255, 255, 0.1);}"
                    " QPushButton:focus {"
                    " border: 2px solid white;"
                    "outline: none;"
                    "}"
                    "QPushButton:hover{background-color: rgba(255, 255, 255, 0.2);}"
                    "QPushButton:checked{background-color: rgba(255, 255, 255, 0.35);}"
                    );
            }

        }
       // update();
    }
    return QObject::eventFilter(watched,event);
}
void LanguageFrame::keyPressEvent(QKeyEvent * event)
{
	if((event->key() == Qt::Key_Enter|| event->key() == Qt::Key_Return))
		clickNextButton();
	else
		QWidget::keyPressEvent(event);
}

void LanguageFrame::translateStr()
{
    m_mainTitle->setText(tr("Select Language"));
    m_nextBtn->setText(tr("Next"));
}

void LanguageFrame::clickNextButton()
{
    writeSettingIni();
    emit signalStackPageChanged(1);
}

void LanguageFrame::initFocusPolicy()
{
    int count = m_LanguageBtnList.count();
    if (count <= 1) return;

    // 设置相邻按钮的焦点顺序
    for (int i = 0; i < count - 1; i++)
    {
        this->setTabOrder(m_LanguageBtnList.at(i), m_LanguageBtnList.at(i+1));
    }

    this->setTabOrder(m_LanguageBtnList.last(), this->m_nextBtn);


}

}
