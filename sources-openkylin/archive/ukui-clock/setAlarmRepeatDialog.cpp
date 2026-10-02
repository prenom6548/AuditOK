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
#include "setAlarmRepeatDialog.h"
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <QBitmap>
#include <QPainter>
#include <QDebug>
#include <QScroller>
#include <QPainterPath>
#include "theme.h"

extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

set_alarm_repeat_Dialog::set_alarm_repeat_Dialog(int width, int Length, int rowNum , QWidget *parent ) :
    QWidget(parent),
    rowNum_all(rowNum),
    width_num(width),
    Length_num(Length)
{
    setupUi(this);
    this->resize(width_num, Length_num);

    this->setWindowTitle(tr("Alarm"));

    setWindowFlags(Qt::Dialog);
    this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明
//    this->setWindowFlags(Qt::FramelessWindowHint|Qt::ToolTip); //设置无边框窗口

//    this->setAttribute(Qt::WA_TranslucentBackground);
//    QPainterPath blurPath;
//    blurPath.addRoundedRect(rect().adjusted(10, 10, -10, -10), 10, 10);      //增加圆角
//    setProperty("useSystemStyleBlur", true);
//    setProperty("blurRegion", QRegion(blurPath.toFillPolygon().toPolygon()));//使用QPainterPath的api生成多边形Region

    //增加圆角
    QBitmap bmp(this->size());
    bmp.fill();
    QPainter p(&bmp);
    p.setPen(Qt::black);
    p.setBrush(Qt::black);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawRoundedRect(bmp.rect(),10,10);
    setMask(bmp);

    for (int i = 0; i < rowNum_all; i++) {
        set_aItem(i,0);
    }

    settingsStyle();
}

set_alarm_repeat_Dialog::~set_alarm_repeat_Dialog()
{
    for (int i =0 ; i <rowNum_all; i++) {
        delete widget[i];
        delete aItem[i];
    }
    delete listWidget;
}

/*
*监听主题
*/
void set_alarm_repeat_Dialog::settingsStyle()
{
    GsettingSubject * subject = GsettingSubject::getInstance();;
    connect(subject,&GsettingSubject::blackStyle, this,[=](){
        this->blackStyle();
    });
    connect(subject,&GsettingSubject::whiteStyle, this,[=](){
        this->whiteStyle();
    });
    connect(subject,&GsettingSubject::fontChanged, this,[=](int size){
        this->CURRENT_FONT_SIZE=size;
        updateSelectItemLabelFont(size);

    });
    subject->iniWidgetStyle();
    subject->iniFontSize();
}
void set_alarm_repeat_Dialog::updateSelectItemLabelFont(int size)
{
    QListWidget * itemList = listWidget;
    for (int i=0;i<itemList->count() ;i++ ) {
        QListWidgetItem * varItem =  itemList->item(i);
        set_alarm_repeat_widget * varWidget = static_cast<set_alarm_repeat_widget*>(itemList->itemWidget(varItem)) ;
        updateLabelFront(varWidget->alarmLabel0,qRound(1.3*size));
    }
}
/**
 * @brief 更新麒麟闹钟字体
 */
void set_alarm_repeat_Dialog::updateLabelFront(QLabel *label, int size)
{
    QString styleSheet = "QLabel{ font-size: ";
    styleSheet.append(QString::number(size)).append("px;");
    styleSheet.append("background-color: rgb();}");
    label->setStyleSheet(styleSheet);
}
void set_alarm_repeat_Dialog::set_aItem(int rowNum,int flags)
{
    aItem[rowNum] =new QListWidgetItem;
    //将列表项的大小提示设置为大小。如果未设置大小提示，则item委托将基于item数据计算大小提示。
    aItem[rowNum]->setSizeHint(QSize(340, 38));
    aItem[rowNum]->setTextColor(QColor(255, 0, 0, 255));
    listWidget->addItem(aItem[rowNum]);
    listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    widget[rowNum] = new set_alarm_repeat_widget(listWidget);
    listWidget->setItemWidget(aItem[rowNum],widget[rowNum]);
    QScroller::grabGesture(listWidget,QScroller::LeftMouseButtonGesture); //设置鼠标左键拖动
    listWidget -> setVerticalScrollMode(QAbstractItemView::ScrollPerPixel); // 设置像素级滑动
    if(flags){
        Length_num = rowNum_all*36+40;
        this->resize(width_num, Length_num);
        //增加圆角
        QBitmap bmp(this->size());
        bmp.fill();
        QPainter p(&bmp);
        p.setPen(Qt::black);
        p.setBrush(Qt::black);
        p.setRenderHint(QPainter::Antialiasing);
        p.drawRoundedRect(bmp.rect(),10,10);
        setMask(bmp);
    }
    listWidget->setGeometry(QRect(0, 0, width_num-20, Length_num-22));
    //消除不使用样式表后直角和圆角重叠影响
    listWidget->setFrameShape(QListWidget::NoFrame);
    listWidget->move(10,10);
}

void set_alarm_repeat_Dialog::setLabelHoverStyle(int num)
{
    if(theme::themetype==0){
        for (int i = 0;i<rowNum_all;i++) {
            auto item = this->widget[i];
            if(i==num){
                QPalette ptext;
                ptext.setColor(QPalette::Text,Qt::white);
                item->alarmLabel0->setPalette(ptext);
            }else{
                QPalette ptext;
                ptext.setColor(QPalette::Text,Qt::black);
                item->alarmLabel0->setPalette(ptext);
            }
        }
    }
}

void set_alarm_repeat_Dialog::setupUi(QWidget( *set_alarm_repeat_Dialog))
{
    if (set_alarm_repeat_Dialog->objectName().isEmpty())
        set_alarm_repeat_Dialog->setObjectName(QString::fromUtf8("set_alarm_repeat_Dialog"));
    listWidget = new QListWidget(set_alarm_repeat_Dialog);
    listWidget->setObjectName("ukui-clock_QListWidget_listWidget");
    listWidget->setObjectName(QString::fromUtf8("listWidget"));
    listWidget->setGeometry(QRect(0, 0, width_num-20, Length_num-22));
    //消除不使用样式表后直角和圆角重叠影响
    listWidget->setFrameShape(QListWidget::NoFrame);
    listWidget->move(10,10);

    retranslateUi(set_alarm_repeat_Dialog);

    QMetaObject::connectSlotsByName(set_alarm_repeat_Dialog);
}   // setupUi

void set_alarm_repeat_Dialog::retranslateUi(QWidget( *set_alarm_repeat_Dialog))
{
    set_alarm_repeat_Dialog->setWindowTitle(QApplication::translate("set_alarm_repeat_Dialog", "Dialog", nullptr));
}   // retranslateUi

void set_alarm_repeat_Dialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(SHADOW_RANGE, SHADOW_RANGE, -SHADOW_RANGE, -SHADOW_RANGE), WINDOWN_RADIUS, WINDOWN_RADIUS);
    // 画一个黑底
    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    auto shadowColor = palette().text().color();
    shadowColor.setAlphaF(SHADOWCOLOR_ALPHAF);
    pixmapPainter.setBrush(shadowColor);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();
    // 模糊这个黑底
    QImage img = pixmap.toImage();
    qt_blurImage(img, VAGUE_RADIUS, false, false);
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
    // 绘制一个背景
    p.save();
    //描边
    QColor borderColor = palette().text().color();
    borderColor.setAlphaF(BORDERCOLOR_ALPHAF);
    p.setPen(borderColor);
    p.translate(BORDER_RANGE, BORDER_RANGE);
    p.setBrush(palette().color(QPalette::Base));
    p.drawPath(rectPath);
    p.restore();

}

void set_alarm_repeat_Dialog::closeEvent(QCloseEvent *event)
{
    emit dialogClose();
    QWidget::closeEvent(event);
}


//黑色主题
void  set_alarm_repeat_Dialog::blackStyle()
{
//    QString itemRadius = QString::number(ITEM_RADIUS);
//    listWidget->setStyleSheet("QListWidget{background-color: rgba(0, 0, 0, 0);}\
//                                  QListWidget::item::selected{background-color:rgba("+hoverColor+");border-radius:"+itemRadius+"px;border:1px solid rgba(131, 131, 131,0);}\
//                                  QListWidget::item:hover{background-color:rgba("+hoverColor+");border-radius:"+itemRadius+"px;}\
//                                  ");

    for(int i = 0; i < rowNum_all; i++){
        widget[i]->alarmLabel1->setProperty("useIconHighlightEffect", true);
    }
}
//白色主题
void  set_alarm_repeat_Dialog::whiteStyle()
{
//    QString itemRadius = QString::number(ITEM_RADIUS);
//    listWidget->setStyleSheet("QListWidget{background-color: rgba(0, 0, 0, 0);}\
//                                  QListWidget::item::selected{background-color:rgba("+hoverColor+");border-radius:"+itemRadius+"px;border:1px solid rgba(131, 131, 131,0);}\
//                                  QListWidget::item:hover{background-color:rgba("+hoverColor+");border-radius:"+itemRadius+"px;}\
//                                  ");
    for(int i = 0; i < rowNum_all; i++){
        widget[i]->alarmLabel1->setProperty("useIconHighlightEffect", false);
    }
}

set_alarm_repeat_widget::set_alarm_repeat_widget(QWidget *parent):
    QWidget(parent)
{
    this->setFixedSize(340, 38);
    //选项
    alarmLabel0 = new QLabel(this);
    alarmLabel0->setObjectName("ukui-clock_QLabel_alarmLabel0");
    alarmLabel0->move(16, 0);
    alarmLabel0->setFixedSize(210, 38);
    alarmLabel0->setStyleSheet("background-color: rgb();");
    alarmLabel0->setText("选项");

    //对号
    alarmLabel1 = new QLabel(this);
    alarmLabel1->setObjectName("ukui-clock_QLabel_alarmLabel1");
    alarmLabel1->move(235, 0);
    alarmLabel1->setFixedSize(34, 38);
    alarmLabel1->setText("");
//    alarmLabel1->setProperty("useIconHighlightEffect", true);
    alarmLabel1->setProperty("iconHighlightEffectMode", 1);

}

set_alarm_repeat_widget::~set_alarm_repeat_widget()
{

}

