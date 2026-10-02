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
#include "customcombox.h"
#include "gsettingsubject.h"
#include "theme.h"
#include <QLineEdit>
#include <QDebug>
#include <QEvent>
#include <QBitmap>
#include <QPainter>
#include <QIcon>
#include <QHoverEvent>
#include "globalinstance.h"

CustomComBox::CustomComBox(int rowNum, QWidget *parent)
    : QComboBox(parent)
    , rowNum_all(rowNum)
    , show_flag_(false)
{
    this->resize(256,36);
    list_widget_ = new CustomListWidget();
    list_widget_->setObjectName("ukui-clock_CustomListWidget_list_widget_");
    list_widget_->installEventFilter(this);
    line_edit_ = new QLineEdit();
    line_edit_->setAccessibleName("ukui-clock_QLineEdit_line_edit_");
    /*设置文本框*/
    //line_edit_绑定combox，显示选择内容
    this->setLineEdit(line_edit_);
    //设为只读，因为该输入框只用来显示选中的选项，称为文本框更合适些
    line_edit_->setReadOnly(true);
    //把当前对象安装(或注册)为事件过滤器，当前也称为过滤器对象。事件过滤器通常在构造函数中进行注册。
    line_edit_->installEventFilter(this);
    //在选择完成后，取消lineedit的焦点
    line_edit_->setFocusPolicy(Qt::NoFocus);
    this->setModel(list_widget_->model());//下拉表格
    this->setView(list_widget_);//表格内容

    for (int i = 0; i < rowNum_all; i++) {
        set_aItem(i);
    }
    this->setProperty("useButtonPalette",true);
    settingsStyle();
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &CustomComBox::onTimerTimeout);
}

CustomComBox::~CustomComBox()
{
    if(list_widget_ != nullptr){
        delete list_widget_;
        list_widget_ = nullptr;
    }
    if(line_edit_ != nullptr){
        delete line_edit_;
        line_edit_ = nullptr;
    }
    // for (int i =0 ; i <rowNum_all; i++) {
    //     delete btn[i];
    //     delete aItem[i];
    // }
    // 仅删除没有父对象管理的btn和aItem（需确保它们确实没有父对象）
    for (int i = 0; i < rowNum_all; i++) {
        // 检查指针是否为空，避免野指针访问
        if (btn[i] != nullptr) {
            delete btn[i];
            btn[i] = nullptr;  // 避免悬空指针
        }
        if (aItem[i] != nullptr) {
            delete aItem[i];
            aItem[i] = nullptr;
        }
    }
}

void CustomComBox::set_aItem(int rowNum)
{
    aItem[rowNum] =new QListWidgetItem(list_widget_);
    aItem[rowNum]->setData(Qt::AccessibleTextRole,QString("ukui-clock_QListWidgetItem_aItem"));
    btn[rowNum] = new item_button(list_widget_);
    list_widget_->addItem(aItem[rowNum]);
    list_widget_->setItemWidget(aItem[rowNum],btn[rowNum]);
    aItem[rowNum]->setSizeHint(QSize(248,36));
   list_widget_->setGeometry(QRect(0, 0,248,36));
    btn[rowNum]->setFlat(true);
    //设置鼠标穿透
    btn[rowNum]->setWindowFlag(Qt::FramelessWindowHint);
    btn[rowNum]->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    btn[rowNum]->setAttribute(Qt::WA_TranslucentBackground, true);
}

int CustomComBox::getCurrentHoverNum() const
{
    return hoverNum;
}

void CustomComBox::setCurrentHoverNum(int value)
{
    hoverNum=value;
}


void CustomComBox::showPopup()
{

    m_ignoreMouseRelease = true;
    m_timer.start(500);

    QComboBox::showPopup();
    QWidget *popup = this->findChild<QFrame*>();
    popup->move(popup->x(), popup->y());
    //    qWarning()<<"zds-位置"<<popup->x()<<popup->y();
    if(comboxClicked){
        setTextHoverStyle(hoverNum);
    }else{
        if(hoverNum == -1){
            setTextHoverStyle(0);
        }else{
            setTextHoverStyle(hoverNum);
        }
    }
    emit comboxPopup();
}

void CustomComBox::hidePopup()
{
    // wlcom 环境下，在一定时间内忽略鼠标弹起的事件处理
    if(GlobalInstance::getInstance()->session_desktop() == GlobalInstance::KYLIN_WLCOM && m_ignoreMouseRelease){
        return;
    }
    show_flag_ = false;
    QComboBox::hidePopup();
    emit comboxClose();
}

void CustomComBox::setTextHoverStyle(int num)
{
    if(theme::themetype==0){
        for (int i = 0;i<rowNum_all;i++) {
            auto item = this->btn[i];
            if(i==num){
                QPalette ptext;
                ptext.setColor(QPalette::ButtonText,Qt::white);
                item->textLabel->setPalette(ptext);
            }else{
                QPalette ptext;
                ptext.setColor(QPalette::ButtonText,Qt::black);
                item->textLabel->setPalette(ptext);
            }
        }
    }
}

bool CustomComBox::eventFilter(QObject *watched, QEvent *event)
{
    //设置点击输入框也可以弹出下拉框
    if (watched == line_edit_ && event->type() == QEvent::MouseButtonPress && this->isEnabled())
    {
        if (show_flag_){
            hidePopup();
            show_flag_ = false;
        }    
        else{
            showPopup();
            show_flag_ = true;
        }  
        return true;
    }
    if(watched == list_widget_ && event->type() == QEvent::Hide){
        show_flag_ = false;
        return QObject::eventFilter(watched, event); 
    }

    return false;
}

void CustomComBox::settingsStyle()
{
    GsettingSubject * subject = GsettingSubject::getInstance();;
    connect(subject,&GsettingSubject::blackStyle, this,[=](){
        this->blackStyle();
    });
    connect(subject,&GsettingSubject::whiteStyle, this,[=](){
        this->whiteStyle();
    });
    connect(subject,&GsettingSubject::fontChanged, this,[=](int size){
        updateSelectItemLabelFont(size);
    });
    subject->iniWidgetStyle();
    subject->iniFontSize();
}

void CustomComBox::blackStyle()
{
    this->setStyleSheet("QComboBox{background-color:rgba(55, 55, 55, 1);color:rgba(222, 226, 231, 1);}\
                          QComboBox:hover{background-color:rgba(95, 95, 95, 1);}\
                          QListWidget{background-color: rgba(0, 0, 0, 0);margin-left:4px solid rgba(131, 131, 131,0);margin-right:4px solid rgba(131, 131, 131,0);}\
                        ");
}

void CustomComBox::whiteStyle()
{
    this->setStyleSheet("QComboBox{background-color:rgba(230, 230, 230, 1);color:rgba(38, 38, 38, 1);}\
                          QComboBox:hover{background-color:rgba(218, 218, 218, 1);}\
                          QListWidget{background-color: rgba(0, 0, 0, 0);margin-left:4px solid rgba(131, 131, 131,0);margin-right:4px solid rgba(131, 131, 131,0);}\
                       ");
}

void CustomComBox::updateLabelFront(QLabel *label, int size)
{
    QString styleSheet = "QLabel{ font-size: ";
    styleSheet.append(QString::number(size)).append("px;");
    styleSheet.append("background-color: rgb();}");
    label->setStyleSheet(styleSheet);
}

void CustomComBox::updateSelectItemLabelFont(int size)
{
    QListWidget * itemList = list_widget_;
    for (int i=0;i<itemList->count() ;i++ ) {
        QListWidgetItem * varItem =  itemList->item(i);
        item_button * varWidget = static_cast<item_button*>(itemList->itemWidget(varItem)) ;
        updateLabelFront(varWidget->textLabel,qRound(1.3*size));
    }
}

void CustomComBox::onTimerTimeout()
{
    m_ignoreMouseRelease = false;
}

item_button::item_button(QWidget *parent)
    : QPushButton(parent)
{
    this->resize(248,36);
    textLabel = new QLabel(this);
    textLabel->setObjectName("ukui-clock_QLabel_textLabel");
    iconLabel = new QLabel(this);
    iconLabel->setObjectName("ukui-clock_QLabel_iconLabel");
    textLabel->setFixedSize(100,23);
    iconLabel->setFixedSize(16,16);
    QString systemLang = QLocale::system().name();
    if(systemLang=="ug_CN"||systemLang=="kk_KZ"||systemLang=="ky_KG"){
        textLabel->move(104,6);
        iconLabel->move(216,10);
    }else{
        textLabel->move(32,6);
        iconLabel->move(8,10);
    }
    textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    iconLabel->setProperty("useIconHighlightEffect", true);
    iconLabel->setProperty("iconHighlightEffectMode", 1);
}

item_button::~item_button()
{
}
