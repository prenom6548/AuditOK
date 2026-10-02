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

#include "choiseeditionui.h"

#define EDITIONS_DIR_NOMAL  "/cdrom/Editions"

using namespace KServer;

choiseEditionUI::choiseEditionUI(QWidget *parent) : MiddleFrameManager(parent)
{
    initUI();
    this->initNextBnt();
//    nextBtnAvailable();
    addStyleSheet();
    initAllConnect();
    translateStr();
   // this->setFocus();
    initFocusPolicy();
    this->setFocusPolicy(Qt::NoFocus);
}

void choiseEditionUI::initUI()
{
    setObjectName("choiseEditionUI");
    m_mainLayout = new QGridLayout();
    m_Widget->setLayout(m_mainLayout);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
//    m_mainLayout->setSpacing(0);

    m_mainLayout->setColumnStretch(0, 3);
    m_mainLayout->setColumnStretch(1, 2);
    m_mainLayout->setColumnStretch(2, 3);

    //标题：连接安装的版本
    m_mainTitle = new QLabel();
    m_mainTitle->setObjectName("mainTitle");
    m_mainLayout->addItem(new QSpacerItem(10, 30, QSizePolicy::Expanding, QSizePolicy::Preferred), 0, 1, 1, 1);
    m_mainLayout->addWidget(m_mainTitle, 1, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop );
    m_mainLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Fixed), 2, 1, 1, 1);

    //加载全部版本
    m_pScroll = new QScrollArea();
    m_pScroll->setFocusPolicy(Qt::NoFocus);
    m_pScroll->setObjectName("pScroll");
    m_pScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_pScroll->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    m_pScroll->setFrameShape(QFrame::NoFrame);
    m_pScroll->setFixedWidth(340);
    m_pScroll->setFixedHeight(QApplication::desktop()->screenGeometry().height()*0.3);
    m_pScroll->setStyleSheet("QScrollArea{background-color:transparent;");
    m_pScroll->viewport()->setStyleSheet("background-color:transparent;");
    m_psWidget = new QWidget();
    m_psWidget->setObjectName("psWidget");
    m_vtlayout = new QVBoxLayout;
    m_vtlayout->setSpacing(10);
    m_psWidget->setLayout(m_vtlayout);
    m_psWidget->setFixedWidth(m_pScroll->width());
  //  m_psWidget->setStyleSheet("QWidget{background-color:transparent;}");
    m_pScroll->setWidget(m_psWidget);
    m_pScroll->setWidgetResizable(true);
    readEditionsFile();    //读取文件内容
    int widgetHight = 0;
    for (int var = 0; var < m_EditionsList.size(); ++var) {
        m_vtlayout->addWidget(m_EditionsList[var]->m_EditionButton);
        widgetHight = widgetHight + 48 + 10;
    }
    m_vtlayout->addItem(new QSpacerItem(8, 20, QSizePolicy::Expanding, QSizePolicy::Expanding));
    widgetHight = widgetHight + 20;
    m_psWidget->setMinimumHeight( (widgetHight>m_pScroll->height())?(widgetHight):(m_pScroll->height()) );
    m_pScroll->adjustSize();
    m_psWidget->adjustSize();

    m_mainLayout->addWidget(m_pScroll, 3, 1, Qt::AlignCenter);
    m_mainLayout->addItem(new QSpacerItem(8, 20, QSizePolicy::Expanding, QSizePolicy::Expanding), 4, 1, 1, 1);
}
void choiseEditionUI::initAllConnect()
{
//...
}

void choiseEditionUI::clickNextButton()
{
    WriteSettingToIni("config","edition",m_strSelectValue);
    emit signalStackPageChanged(1);//跳转显示下个插件界面
}

//void choiseEditionUI::nextBtnAvailable()
//{
////...
//    if(true){
//        m_nextBtn->setEnabled(true);
//    } else {
//        m_nextBtn->setEnabled(false);edits
//    }
////...
//}

void choiseEditionUI::translateStr()
{
    m_mainTitle->setText(tr("Select Install Edition"));//选择安装的版本
    m_nextBtn->setText(tr("Next"));
    for (int var = 0; var < m_EditionsList.count(); ++var) {
        m_EditionsList[var]->translater(ReadSettingIni("config", "language"));;
    }
}

void choiseEditionUI::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {//响应国际化语言切换
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
//...
}

void choiseEditionUI::readEditionsFile()
{

    QDir dir(EDITIONS_DIR_NOMAL);
    qDebug()<<"the Editions path="<<EDITIONS_DIR_NOMAL;
    if(!dir.exists()){
        qDebug()<<"Editions dir path not exist!";
        return;
    }
    QList<editionDirInfo*> EditionsListTmp , EditionsListTmpDefault, EditionsListTmpProfessional, EditionsListTmpDefaultProfessional;
    QFileInfoList flst=dir.entryInfoList();
    foreach (QFileInfo one, flst) {
        if(one.isDir() && !one.isHidden())
        {
            QDir subdir(one.filePath());
            QFileInfoList dirflst=subdir.entryInfoList();
            if(subdir.exists()){
                foreach (QFileInfo dirFile, dirflst){
                    if(dirFile.isFile() && dirFile.suffix()=="conf"){
                        QSettings* seting=new QSettings(dirFile.filePath(),QSettings::IniFormat);
                        seting->setIniCodec("UTF-8");
                        QStringList lst=seting->allKeys();
                        qDebug()<<"["<< dirFile.filePath()<< "]:" <<lst;
                        if( !lst.contains("edition") )
                        {
                            continue;
                        }

                        editionDirInfo* edits = new editionDirInfo;
                        edits->m_filepath = dirFile.filePath();

//                        app_packge_name=fil.dir().dirName();
                        edits->m_edition=seting->value("edition").toString();
                        if( lst.contains("name_zh"))
                        {
                            edits->m_LangtoNameMap["zh_CN"] = seting->value("name_zh").toString();
                        }
                        if( lst.contains("name_hk"))
                        {
                            edits->m_LangtoNameMap["zh_HK"] = seting->value("name_hk").toString();
                        }
                        if( lst.contains("name_bo"))
                        {
                            edits->m_LangtoNameMap["bo_CN"] = seting->value("name_bo").toString();
                        }
                        if( lst.contains("name_mn"))
                        {
                            edits->m_LangtoNameMap["mn_MN"] = seting->value("name_mn").toString();
                        }
                        if( lst.contains("name_en"))
                        {
                            edits->m_LangtoNameMap["en_USedits"] = seting->value("name_en").toString();
                        }
                        edits->m_script=seting->value("script").toString();
                        edits->m_default=seting->value("default").toString();
//                        edits->m_EditionButton = new QPushButton;
                        edits->translater(ReadSettingIni("config", "language"));
//                        connect(edits->m_EditionButton,&QPushButton::clicked,this,&choiseEditionUI::getSelect);
                        connect(edits,&editionDirInfo::checkButton_sig,this,&choiseEditionUI::getSelect);
                        connect(this,&choiseEditionUI::changeButton_sig,edits,&editionDirInfo::other_clicked_slot);
//                        connect(this,&choiseEditionUI::changeButton_sig,edits,&PushButtonCheck::other_clicked_slot);

                        if(!edits->m_default.isEmpty() && edits->m_default=="yes"){
                            if( !edits->m_edition.isEmpty() && edits->m_edition=="professional" ){
                                EditionsListTmpDefaultProfessional.append(edits);
                            }else{
                                EditionsListTmpDefault.append(edits);
                            }
                        }else{
                            if( !edits->m_edition.isEmpty() && edits->m_edition=="professional" ){
                                EditionsListTmpProfessional.append(edits);
                            }else{
                                EditionsListTmp.append(edits);
                            }
                        }
                    }
                }
            }
        }
    }

    if(EditionsListTmp.size() < 1 && EditionsListTmpProfessional.size() < 1 && EditionsListTmpDefault.size() < 1 && EditionsListTmpDefaultProfessional.size() < 1){
        qDebug()<<"not find subdirectory in Editions!";
        WriteSettingToIni("config","edition","default");
        return;
    }

    m_EditionsList.append(EditionsListTmpDefaultProfessional);
    m_EditionsList.append(EditionsListTmpDefault);
    m_EditionsList.append(EditionsListTmpProfessional);
    m_EditionsList.append(EditionsListTmp);

    emit m_EditionsList[0]->checkButton_sig(m_EditionsList[0]);
}

void choiseEditionUI::addStyleSheet()
{
    QFile file(":/res/qss/KChoiceEdition.css");
    qDebug() << Q_FUNC_INFO << file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void choiseEditionUI::initFocusPolicy()
{
    int count = m_EditionsList.count();
    if (count <= 1) return;

    // 设置相邻按钮的焦点顺序
    for (int i = 0; i < count - 1; i++)
    {
        this->setTabOrder(m_EditionsList.at(i)->m_EditionButton, m_EditionsList.at(i+1)->m_EditionButton);
    }

    this->setTabOrder(m_EditionsList.last()->m_EditionButton, this->m_nextBtn);
}

void choiseEditionUI::getSelect(editionDirInfo* getChoiceButt)
{
    if(getChoiceButt){
        m_strSelectValue = getChoiceButt->m_edition;
        m_strSelectButt = getChoiceButt;
    }
    emit changeButton_sig(getChoiceButt);
}

void choiseEditionUI::keyPressEvent(QKeyEvent *event)
{
    this->setFocus();
    qDebug() << Q_FUNC_INFO << "keyPressEvent";
    qDebug() << event->key() << "key pressed!";
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return){
        clickNextButton();
    } else if (event->key() == Qt::Key_Up){//向上键
        int k = 0;
        for (k = 0; k < m_EditionsList.count(); k++) {
            if (m_EditionsList[k]==m_strSelectButt)
                break;
        }
        if (k == 0) {
            k = m_EditionsList.count()-1;
        } else {
            k = k - 1;
        }
        getSelect(m_EditionsList[k]);
    } else if (event->key() == Qt::Key_Down) {//向下键
        int k = 0;
        for (k = 0; k < m_EditionsList.count(); k++) {
            if (m_EditionsList[k]==m_strSelectButt)
                break;
        }
        if ((++k) >= m_EditionsList.count())
            k=0;
        getSelect(m_EditionsList[k]);
    }else
        QWidget::keyPressEvent(event);
}
