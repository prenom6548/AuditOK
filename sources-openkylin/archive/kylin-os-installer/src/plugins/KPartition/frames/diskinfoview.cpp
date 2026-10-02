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


#include "diskinfoview.h"
#include <QMouseEvent>
#include <QTextStream>
#include <QEvent>
#include <QPainter>
#include <QTimer>
#include "../PluginService/ui_unit/xrandrobject.h"
#include "QDebug"
#include "setPartitionsSize.h"

using namespace KServer;

DiskInfoView::DiskInfoView(QWidget *parent) : QWidget(parent)
  ,m_checkable(true)
{
    QPalette pal = palette();
    pal.setColor(QPalette::Background, QColor(0,0,0,0));
    setPalette(pal);
    this->setObjectName(QStringLiteral("DiskInfoView"));
    this->installEventFilter(this);
    this->setAttribute(Qt::WA_Hover,true);
    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowOpacity(1);
    this->setAttribute(Qt::WA_TranslucentBackground, true);

    widget = new QWidget(this);
    widget->setObjectName(QStringLiteral("widget"));
    widget->installEventFilter(this);
    widget->setAttribute(Qt::WA_Hover,true);
    widget->setFixedSize(200, 200);

    layoutWidget = new QWidget(widget);
    layoutWidget->setObjectName(QStringLiteral("layoutWidget"));
    layoutWidget->setFixedSize(200, 200);
//    layoutWidget->setFixedWidth(200);
    verticalLayout = new QVBoxLayout(layoutWidget);

    verticalLayout->setAlignment(Qt::AlignCenter);
    verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout->setContentsMargins(0, 80, 0, 0);
//    verticalLayout->setContentsMargins(15, 75, 15, 0);
    verticalLayout->setSpacing(5);

//    verticalLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Fixed, QSizePolicy::Fixed));

//    diskType = new QLabel(layoutWidget);
//    diskType->setObjectName(QStringLiteral("diskType"));
//    diskType->setAlignment(Qt::AlignCenter);

//    verticalLayout->addWidget(diskType, Qt::AlignCenter);

    m_diskPathandNameWidget = new QWidget(layoutWidget);
    m_diskPathandNameWidget->setObjectName(QStringLiteral("diskPathandNameWidget"));
    m_diskPathandNameWidget->installEventFilter(this);
    m_diskPathandNameWidget->setContentsMargins(0, 0, 0, 0);
    m_diskPathandNameWidget->setFixedSize(200, 24+24+6);
    QVBoxLayout *diskPathandNameLayout = new QVBoxLayout(m_diskPathandNameWidget);
    diskPathandNameLayout->setAlignment(Qt::AlignCenter);
    diskPathandNameLayout->setContentsMargins(0, 0, 0, 0);
    diskPathandNameLayout->setSpacing(5);

    diskPath = new QLabel(layoutWidget);
    diskPath->setObjectName(QStringLiteral("devPath"));
    diskPath->setAlignment(Qt::AlignCenter);
    diskPathandNameLayout->addWidget(diskPath,0,Qt::AlignCenter);

    diskName = new QLabel(layoutWidget);
    diskName->setObjectName(QStringLiteral("diskName"));
    diskName->setAlignment(Qt::AlignCenter);
    diskName->setMinimumWidth(198);
    diskName->setMinimumHeight(30);
    diskName->setWordWrap(true);
//    diskName->installEventFilter(this);
    diskPathandNameLayout->addWidget(diskName,0,Qt::AlignHCenter | Qt::AlignTop);

    verticalLayout->addWidget(m_diskPathandNameWidget, Qt::AlignHCenter | Qt::AlignTop);

//    QWidget *diskPathWidget = new QWidget();
//    diskPathWidget->setContentsMargins(0, 0, 0, 0);
//    diskPathWidget->setFixedSize(200-15-15, 24);
//    QHBoxLayout *diskPathLayout = new QHBoxLayout(diskPathWidget);
//    diskPathLayout->setAlignment(Qt::AlignCenter);
//    diskPathLayout->setContentsMargins(0, 0, 0, 0);
//    diskPathLayout->setSpacing(0);
//    diskPath = new QLabel(layoutWidget);
//    diskPath->setObjectName(QStringLiteral("devPath"));
//    diskPath->setAlignment(Qt::AlignCenter);
//    diskPathLayout->addWidget(diskPath,0,Qt::AlignHCenter);
//    verticalLayout->addWidget(diskPathWidget, Qt::AlignHCenter | Qt::AlignTop);

//    QWidget *diskNameWidget = new QWidget();
//    diskNameWidget->setContentsMargins(0, 0, 0, 0);
//    diskNameWidget->setFixedSize(200-15-15, 24);
//    QHBoxLayout *diskNameLayout = new QHBoxLayout(diskNameWidget);
//    diskNameLayout->setAlignment(Qt::AlignCenter);
//    diskNameLayout->setContentsMargins(0, 0, 0, 0);
//    diskNameLayout->setSpacing(0);
//    diskName = new QLabel(layoutWidget);
//    diskName->setObjectName(QStringLiteral("diskName"));
//    diskName->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
////    diskName->setWordWrap(true);
//    diskName->installEventFilter(this);
//    diskNameLayout->addWidget(diskName);
//    diskNameWidget->setLayout(diskNameLayout);
//    verticalLayout->addWidget(diskNameWidget, Qt::AlignHCenter);
////    verticalLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Expanding));

//    m_diskNameTip = new QLabel(widget);
//    m_diskNameTip->setFixedHeight(68);
//    m_diskNameTip->setFixedWidth(200);
//    m_diskNameTip->setAlignment(Qt::AlignHCenter|Qt::AlignBottom);
//    m_diskNameTip->setWordWrap(true);
//    m_diskNameTip->setVisible(false);

    QWidget *progressBarWidget = new QWidget();
    progressBarWidget->setContentsMargins(0, 0, 0, 0);
    progressBarWidget->setFixedSize(200, 12);
    QHBoxLayout *progressBarLayout = new QHBoxLayout(progressBarWidget);
    progressBarLayout->setAlignment(Qt::AlignCenter);
    progressBarLayout->setContentsMargins(30, 0, 30, 0);
    progressBarLayout->setSpacing(0);
//    progressBar = new QProgressBar(layoutWidget);
    progressBar = new QProgressBar();
    progressBar->setObjectName(QStringLiteral("progressBar"));
//    progressBar->setMinimumWidth(170);
//    progressBar->setFixedHeight(6);
    progressBar->setValue(24);
    progressBar->setTextVisible(false);
    progressBar->setAlignment(Qt::AlignHCenter);
//    progressBar->setMaximumHeight(5);
    progressBarLayout->addWidget(progressBar,0,Qt::AlignCenter);
    progressBarWidget->setLayout(progressBarLayout);
    verticalLayout->addWidget(progressBarWidget, Qt::AlignHCenter);
    m_SizeLineWidget = new QWidget();
    m_SizeLineWidget->setContentsMargins(0, 0, 0, 0);
    m_SizeLineWidget->setFixedSize(200, 36);
    QHBoxLayout *SizeLineHLayout = new QHBoxLayout;
    SizeLineHLayout->setAlignment(Qt::AlignCenter);
    SizeLineHLayout->setContentsMargins(15, 0, 15, 0);
    SizeLineHLayout->setSpacing(0);
    diskSize = new QLabel(m_SizeLineWidget);
    diskSize->setObjectName(QStringLiteral("diskSize"));
//    diskSize->setText("label_4");
    diskSize->setAlignment(Qt::AlignCenter);//bug221824
    diskSize->setFixedSize(140, 28);
//    verticalLayout->addWidget(diskSize, Qt::AlignCenter);
//    label_4->setStyleSheet(QStringLiteral("font-size:14px;color:white;background-color:transparent;"));
    SizeLineHLayout->addWidget(diskSize);
    m_SizeLineWidget->setLayout(SizeLineHLayout);
//    verticalLayout->addWidget(m_SizeLineWidget, Qt::AlignCenter);
    layoutWidget->setLayout(verticalLayout);

    hardDiskPng = new QLabel(widget);
    hardDiskPng->setObjectName(QStringLiteral("hardDiskPng"));
    hardDiskPng->installEventFilter(this);
    //hardDiskPng->setPixmap(QPixmap(":/res/pic/drive-harddisk-system.svg").scaled(QSize(84, 84)));
    hardDiskPng->setPixmap(QIcon(":/res/pic/drive-harddisk-system.svg").pixmap(84,84));

    hardDiskPng->setWindowOpacity(0);
    hardDiskPng->setStyleSheet(" background:transparent;");

    checkpic = new QLabel(widget);
    checkpic->setAttribute(Qt::WA_TranslucentBackground, true);
    checkpic->setObjectName("checkpic");
    checkpic->setAlignment(Qt::AlignRight);
    //checkpic->setPixmap(QPixmap(":/res/pic/checkpic.svg"));
    checkpic->setPixmap(QIcon(":/res/pic/checkpic.svg").pixmap(28,28));
    checkpic->setVisible(false);

   // this->adjustSize();
    this->resize(205, 320);

//    QLabel* contex=new QLabel(this);
    dataSel=new QCheckBox(this);
    dataSel->setObjectName("dataSel");
    dataSel->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Minimum);

   // contex->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);
    QSize m=dataSel->size();
    dataSel->move((widget->size().width()-m.width())/2-15, widget->size().height()+16);
    dataSel->show();
    dataSel->setChecked(false);
    addStyleSheet();
    initAllConnect();
    dataSel->setFocus();
    m_isEncrypted = 0;

    m_setSizeWidget = new QWidget(layoutWidget);
    m_setSizeWidget->setObjectName(QStringLiteral("setSizeWidget"));
    m_setSizeWidget->installEventFilter(this);
    m_setSizeWidget->setFixedSize(200,36);
    m_setSizeWidget->setAttribute(Qt::WA_Hover,true);
    m_setSizeWidget->setStyleSheet("background-color:transparent;");
    m_horizLayout = new QHBoxLayout(m_setSizeWidget);
//    m_horizLayout->setAlignment(Qt::AlignCenter);
    m_horizLayout->setObjectName(QStringLiteral("horizLayout"));
    m_horizLayout->setContentsMargins(0, 0, 0, 0);
    m_setSizeWidget->setLayout(m_horizLayout);

    m_space = new QLabel();
    m_space->setFixedSize(25,20);
    m_horizLayout->addWidget(m_space);

//    m_horizLayout->addItem(new QSpacerItem(20, 20, QSizePolicy::Fixed, QSizePolicy::Fixed));
    QVBoxLayout *vLayout = new QVBoxLayout();
//    vLayout->setAlignment(Qt::AlignCenter);
    vLayout->setObjectName(QStringLiteral("vLayout"));
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    m_sysParSize = new QLabel(m_setSizeWidget);
    m_sysParSize->setObjectName(QStringLiteral("sysParSize"));
//    m_sysParSize->setMaximumHeight(16);
//    m_sysParSize->setMinimumWidth(180);
    m_sysParSize->setStyleSheet("font: 29 8pt Noto Sans CJK SC;background-color:transparent; Noto Sans CJK SC");
    m_sysParSize->setAlignment(Qt::AlignLeft);
    vLayout->addWidget(m_sysParSize, Qt::AlignLeft);
    m_dataParSize = new QLabel(m_setSizeWidget);
    m_dataParSize->setObjectName(QStringLiteral("dataParSize"));
//    m_dataParSize->setMaximumHeight(16);
//    m_dataParSize->setMinimumWidth(180);
    m_dataParSize->setStyleSheet("font: 29 8pt Noto Sans CJK SC;background-color:transparent; Noto Sans CJK SC");
    m_dataParSize->setAlignment(Qt::AlignLeft);
    vLayout->addWidget(m_dataParSize, Qt::AlignLeft);
    m_horizLayout->addLayout(vLayout, Qt::AlignLeft);
    m_horizLayout->addStretch();

//    m_sysTip = new QLabel(widget);
//    m_sysTip->setFixedHeight(14);
//    m_sysTip->setFixedWidth(200);
//    m_sysTip->setVisible(false);
//    m_sysTip->setStyleSheet("font: 29 8pt Noto Sans CJK SC;background-color:transparent; Noto Sans CJK SC");
//    m_dataTip = new QLabel(widget);
//    m_dataTip->setFixedHeight(14);
//    m_dataTip->setFixedWidth(200);
//    m_dataTip->setVisible(false);
//    m_dataTip->setStyleSheet("font: 29 8pt Noto Sans CJK SC;background-color:transparent; Noto Sans CJK SC");

    m_setPartitSize = new QLabel(this);
    m_setPartitSize->setObjectName(QStringLiteral("m_setPartitSize"));
//    m_setPartitSize->setStyleSheet("border:0px;padding:5px;border-radius:5px;");
    m_setPartitSize->setFixedSize(26,26);
//    m_setPartitSize->setScaledContents(true);
//    QImage img;
//    img.load(":/res/pic/change.svg");
//    img.scaled(m_setPartitSize->width()-5, m_setPartitSize->height()-5, Qt::KeepAspectRatioByExpanding);//缩放图片
//    m_setPartitSize->setPixmap(QPixmap::fromImage(img));
    m_setPartitSize->setPixmap(QIcon(":/res/pic/change.svg").pixmap(14,14));
    m_setPartitSize->setAlignment(Qt::AlignCenter);
//    m_setPartitSize->setPixmap(QPixmap(":/res/pic/change.svg").scaled(QSize(14, 14)));
    m_setPartitSize->installEventFilter(this);
    m_setPartitSize->setAttribute(Qt::WA_Hover,true);
    m_setPartitSize->setVisible(false);

    m_dataSelandSet = new QStackedWidget;
    m_dataSelandSet->addWidget(m_SizeLineWidget);
    m_dataSelandSet->addWidget(m_setSizeWidget);
    m_dataSelandSet->setCurrentWidget(m_SizeLineWidget);
    verticalLayout->addWidget(m_dataSelandSet);

    m_DateFocseWidget = new QWidget(widget);
    m_DateFocseWidget->setObjectName(QStringLiteral("m_DateFocseWidget"));
    m_DateFocseWidget->installEventFilter(this);
    m_DateFocseWidget->setFixedSize(128,36);
//    m_DateFocseWidget->setAttribute(Qt::WA_Hover,true);
    m_DateFocseWidget->setStyleSheet("background-color:transparent;");

    m_DateBackgroundWidget = new QWidget(widget);
    m_DateBackgroundWidget->setObjectName(QStringLiteral("m_DateBackgroundWidget"));
    m_DateBackgroundWidget->installEventFilter(this);
    m_DateBackgroundWidget->setFixedSize(60,60);
//    m_DateBackgroundWidget->setAttribute(Qt::WA_Hover,true);
    m_DateBackgroundWidget->setStyleSheet("background-color:transparent;");

    moveToLocal();
    setPartiSizeBtVisible(false);
    translateStr();
    dataSel->setFocusPolicy(Qt::NoFocus);
 //   this->setFocusPolicy(Qt::FocusPolicy::StrongFocus);
}

void DiskInfoView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    /* 0x20为透明层颜色，可自定义设置为0x0到0xff */
    painter.fillRect(this->rect(), QColor(0, 0, 0, 0));
}

void DiskInfoView::initAllConnect()
{
    connect(this, &DiskInfoView::signalClicked, this, &DiskInfoView::slotFrameClicked);
    connect(dataSel,&QCheckBox::clicked,this,&DiskInfoView::clickDataSel);
}

void DiskInfoView::slotFrameClicked()
{
    this->setWindowOpacity(1);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
}

void DiskInfoView::clickDataSel()
{
    emit signalClickView(mIndex);
}

bool DiskInfoView::eventFilter(QObject *object, QEvent *event)
{
    if(object->objectName() == "widget") {//只在点击浮框内有效
        if(event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseevent = (QMouseEvent*)event;
            if(mouseevent->button() == Qt::LeftButton) {
               //如果选择为数据盘则点击无效
                 if(dataSel->isChecked()==false)
                    emit signalClicked(devPath);
                return true;
            }
        }
        if(event->type() == QEvent::HoverMove) {
            if(widget->isEnabled()){
                widget->setStyleSheet(" #widget:hover{background-color: rgba(255, 255, 255, 0.3);}");
            }
            return true;
        }
        if(event->type() == QEvent::Leave) {
//            if(checkpic->isVisible()){
//                widget->setStyleSheet(" #widget{background-color: rgba(255, 255, 255, 0.25);}");
//            }else{
//                if(widget->isEnabled()){
//                    widget->setStyleSheet(" #widget{background-color: rgba(255, 255, 255, 0.15);}");
//                }else{
//                    widget->setStyleSheet(" #widget{background-color: rgba(255, 255, 255, 0.05);}");
//                }
//            }
            if(widget->isEnabled()){
                widget->setStyleSheet(" #widget{background-color: rgba(0, 0, 0, 0.15);}");
            }
            return true;
        }
    }

    if(object->objectName() == "m_setPartitSize") {
        if(event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);//将事件强制转换
            if(mouseEvent->button() == Qt::LeftButton)
            {
                setSizeButtClicked();
                return true;
            }
        }
        else if(event->type() == QEvent::Enter)
        {
            widget->setStyleSheet(" #widget{background-color: rgba(255, 255, 255, 0.3);}");
            m_setPartitSize->setStyleSheet("background-color:rgba(255, 255, 255, 0.6);border-radius:4px;");
        }else if(event->type() == QEvent::Leave)
        {
            m_setPartitSize->setStyleSheet("background-color:transparent;");//
        }
    }

    if(object->objectName() == "m_DateFocseWidget" && m_setSizeWidget->isVisible()) {
        if(event->type() == QEvent::Enter){
            if(!m_strSysFull.isEmpty()&& !m_strSysShow.isEmpty() && !m_strdateFull.isEmpty()&& !m_strDataShow.isEmpty() ){
                m_sysParSize->setText(m_strSysFull);
                m_dataParSize->setText(m_strdateFull);

                if((m_strSysFull != m_strSysShow) || (m_strdateFull != m_strDataShow)){
                    m_setPartitSize->setVisible(false);
                }else
                    m_setPartitSize->setVisible(true);
            }
//            if(!m_sysTip->text().isEmpty() || !m_dataTip->text().isEmpty()){//bug244867
//                if(m_sysTip->text().isEmpty()){
//                    m_sysTip->setText(m_sysParSize->text());
//                }
//                if(m_dataTip->text().isEmpty()){
//                    m_dataTip->setText(m_dataParSize->text());
//                }
//                m_dataTip->setVisible(true);
//                m_dataParSize->setVisible(false);
//                m_sysTip->setVisible(true);
//                m_sysParSize->setVisible(false);
//                m_setPartitSize->setVisible(false);
//            }
////            if(!m_sysTip->text().isEmpty()){
////                m_sysTip->setVisible(true);
////                m_sysParSize->setVisible(false);
////                m_setPartitSize->setVisible(false);
////            }
////            if(!m_dataTip->text().isEmpty()){
////                m_dataTip->setVisible(true);
////                m_dataParSize->setVisible(false);
////                m_setPartitSize->setVisible(false);
////            }
            return true;
        }else if(event->type() == QEvent::Leave){
//            m_sysTip->setVisible(false);
//            m_sysParSize->setVisible(true);
//            m_dataTip->setVisible(false);
//            m_dataParSize->setVisible(true);
            m_setPartitSize->setVisible(true);
            m_sysParSize->setText(m_strSysShow);
            m_dataParSize->setText(m_strDataShow);
            return true;
        }
    }

//    if(object->objectName() == "diskName" ) {
//        if(event->type() == QEvent::Enter){
//            if(!m_diskNameTip->text().isEmpty()){
//                m_diskNameTip->setVisible(true);
//                diskName->setVisible(false);
//                diskPath->setVisible(false);
//            }
//            return true;
//        }else if(event->type() == QEvent::Leave){
//            m_diskNameTip->setVisible(false);
//            diskName->setVisible(true);
//            diskPath->setVisible(true);
//            return true;
//        }
//    }

    if(object->objectName() == "diskPathandNameWidget" ) {
        if(event->type() == QEvent::Enter){
            if(!m_strdiskNameShow.isEmpty() && !m_strDiskNameReal.isEmpty()){
                if(m_strdiskNameShow != m_strDiskNameReal){
                    diskPath->setVisible(false);
//                    diskName->setWordWrap(true);
                    diskName->setText(m_strDiskNameReal);
                    diskName->adjustSize();
                    diskName->setAlignment(Qt::AlignHCenter|Qt::AlignTop);
//                    diskName->setStyleSheet("QLabel { font-size:12px; color:#FFFFFF; }");
                }else{
                    diskPath->setVisible(true);
//                    diskName->setStyleSheet("QLabel { font-size:14px; color:#FFFFFF; }");
//                    diskName->setWordWrap(true);
                    diskName->setText(m_strdiskNameShow);
                }
            }
            return true;
        }else if(event->type() == QEvent::Leave){
            diskPath->setVisible(true);
//            diskName->setStyleSheet("QLabel { font-size:14px; color:#FFFFFF; }");
//            diskName->setWordWrap(true);
            if(m_strdiskNameShow.isEmpty()){
                diskName->setText(m_strDiskNameReal);
            }else{
                diskName->setText(m_strdiskNameShow);
            }
            return true;
        }
    }

    return QWidget::eventFilter(object,event);
}

void DiskInfoView::addStyleSheet()
{
    QFile file(":/res/qss/DiskInfoView.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = "";
    stylesheet = filetext.readAll();
    file.close();
    this->setStyleSheet(stylesheet);
}

void DiskInfoView::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

QSize DiskInfoView::getFramesize()
{
    return this->size();
}

void DiskInfoView::translateStr()
{
    dataSel->setText(tr("as data disk"));

    m_strSysFull = tr("System Partitions Size:");
    qint64 dataSizeNow = (m_sizeDataSetted<=0)?m_sizeDataCurrent:m_sizeDataSetted;
    m_sizeSysShow = (double)(m_sizeTotal-dataSizeNow)/1024/1024/1024;
    m_strSysFull = m_strSysFull + QString::number(m_sizeSysShow,'f',1) +"GB";
    m_strSysShow = m_sysParSize->fontMetrics().elidedText(m_strSysFull,Qt::ElideRight,115);
    m_sysParSize->setText(m_strSysShow);
//    if(m_strSysShow == m_strSysFull)
//        m_sysTip->clear();
//    else
//        m_sysTip->setText(m_strSysFull);

    m_strdateFull = tr("Data Partition Size:");
    m_sizeDataShow = (double)dataSizeNow/1024/1024/1024;
    m_strdateFull = m_strdateFull + QString::number(m_sizeDataShow,'f',1) +"GB";
    m_strDataShow = m_dataParSize->fontMetrics().elidedText(m_strdateFull,Qt::ElideRight,115);
    m_dataParSize->setText(m_strDataShow);
//      if(m_strDataShow == m_strdateFull)
//          m_dataTip->clear();
//      else
//          m_dataTip->setText(m_strdateFull);

      m_strdiskNameShow = diskName->fontMetrics().elidedText(m_strDiskNameReal,Qt::ElideRight,200-15-15);
      diskName->setText(m_strdiskNameShow);
//        if(m_strdiskNameShow == m_strDiskNameReal)
//            m_diskNameTip->clear();
//        else
//            m_diskNameTip->setText(m_strDiskNameReal);

    moveToLocal();

    if(1 == m_isEncrypted)
    {
        diskSize->setText(tr("Encrypted Partition"));//bug159507 含有被加密分区的磁盘直接显示“加密磁盘”
        diskSize->setStyleSheet("color: #ffa10a");
        progressBar->setValue(100);
        update();
    }
}

void DiskInfoView::setWidgetDisabled()
{
    this->setEnabled(false);
    widget->setDisabled(true);
    progressBar->setDisabled(true);
    this->setFocusPolicy(Qt::NoFocus);
}

void DiskInfoView::setProgressStyle()
{
    progressBar->setStyleSheet("QProgressBar#progressBar::chunk {"
                               "max-width:140px;min-width:140px;max-height:6px;min-height:6px;border:none;border-radius:3px;background-color:rgba(251, 145, 64, 0.4);}");
}

void DiskInfoView::setPartiSizeBtVisible(bool bStatus)
{
    if(!checkpic->isVisible()){//bug221729
//        m_setSizeWidget->setVisible(false);
        m_dataSelandSet->setCurrentWidget(m_SizeLineWidget);
    }else{
        if(m_history&&m_saveHistory){//bug228943
//            m_setSizeWidget->setVisible(false);
            m_dataSelandSet->setCurrentWidget(m_SizeLineWidget);
        }else{
//            m_setSizeWidget->setVisible(bStatus);
            if(bStatus)
                m_dataSelandSet->setCurrentWidget(m_setSizeWidget);
            else
                m_dataSelandSet->setCurrentWidget(m_SizeLineWidget);
        }
    }

//    diskSize->setVisible(!m_setSizeWidget->isVisible());
    if(m_dataSelandSet->currentWidget() == m_setSizeWidget){
        if((m_strSysFull != m_strSysShow) || (m_strdateFull != m_strDataShow)){
            if((m_sysParSize->text() == m_strSysFull) && (m_dataParSize->text() == m_strdateFull))
                m_setPartitSize->setVisible(false);
            else
                m_setPartitSize->setVisible(true);
        }else
            m_setPartitSize->setVisible(true);
    }
    else
        m_setPartitSize->setVisible(false);

//    if(!m_setSizeWidget->isVisible()){//bug245183
//        m_sysParSize->setVisible(true);
//        m_dataParSize->setVisible(true);
//        m_setSizeWidget->setVisible(false);
//    }

    if(m_dataSelandSet->currentWidget() != m_setSizeWidget){
        m_sysParSize->setText(m_strSysShow);
        m_dataParSize->setText(m_strDataShow);
        m_setPartitSize->setVisible(false);
    }

//    if(!checkpic->isVisible()){//bug221729
//        m_setSizeWidget->setVisible(false);
//    }else{
//        if(m_history&&m_saveHistory){//bug228943
//            m_setSizeWidget->setVisible(false);
//        }else{
//            m_setSizeWidget->setVisible(bStatus);
//        }
//    }

////    if(m_saveHistory){
////        m_setSizeWidget->setVisible(false);
////    }else{
////        if(checkpic->isVisible()){//bug221729
////            m_setSizeWidget->setVisible(bStatus);
////        }else {
////            m_setSizeWidget->setVisible(false);
////        }
////    }
//    diskSize->setVisible(!m_setSizeWidget->isVisible());
//    m_setPartitSize->setVisible(m_setSizeWidget->isVisible());

//    if(!m_setSizeWidget->isVisible()){//bug245183
////        m_sysTip->setVisible(false);
////        m_dataTip->setVisible(false);
//        m_sysParSize->setVisible(true);
//        m_dataParSize->setVisible(true);
//        m_setSizeWidget->setVisible(false);
//    }

////    if(m_setSizeWidget->isVisible())

////    if(checkpic->isVisible()){
////        diskSize->setVisible(false);
////    }else {
////        if(m_checkable)
////            diskSize->setVisible(true);
////    }

////    if(bStatus)
////    {
////        this->resize(200, 350);
////    }
////    else
////    {
////        this->resize(200, 320);
//////        WriteSettingToIni("config", "data-customize-size", "false");
//////        qDebug()<< Q_FUNC_INFO <<" data size setted:false";
////    }

////    m_sysParSize->setVisible(bStatus);
////    m_dataParSize->setVisible(bStatus);
////    m_setPartitSizeBTN->setVisible(bStatus);
}

void DiskInfoView::setSizeButtClicked()
{
    SetPartitionsSize *frame = new SetPartitionsSize(this);
    frame->getDiskInfo(m_strDiskNameReal,m_strDiskpath,m_sizeTotal,m_sizeDataCurrent,m_sizeDataSetted,m_nDataMiniSize,m_nDataMaxSize);
    int ret = frame->exec();
    QApplication::processEvents(QEventLoop::ExcludeSocketNotifiers|QEventLoop::ExcludeUserInputEvents);
    this->activateWindow();
    if(ret == QDialog::Accepted) {
        m_setSize = true;
        m_sizeDataSetted = frame->m_sizeSetting;
//        WriteSettingToIni("config", "data-customize-size", QString::number(m_sizeDataSetted/1024/1024));
//        qDebug()<< Q_FUNC_INFO <<" data size setted(MiB):"<<m_sizeDataSetted/1024/1024;
    } else if (ret == QDialog::Rejected){
//        m_setSize = false;
//        WriteSettingToIni("config", "data-customize-size", "false");
//        qDebug()<< Q_FUNC_INFO <<" data size setted:false";
    } else {
//        m_setSize = false;
//        WriteSettingToIni("config", "data-customize-size", "false");
//        qDebug()<< Q_FUNC_INFO <<" data size setted:false";
    }
    translateStr();
}

void DiskInfoView::setSaveHistoryStatus(bool bStatus)
{
    m_saveHistory = bStatus;
    setPartiSizeBtVisible(!bStatus);
}

void DiskInfoView::moveToLocal(void)
{
    hardDiskPng->move(58, 0);
    QSize mm=m_setSizeWidget->size();
//    m_setSizeWidget->move(0, this->size().height()-mm.height()-108-24);
//    m_setSizeWidget->move((this->size().width()-mm.width()-m_setPartitSize->width()-10)/2-2, this->size().height()-mm.height()-108-24);
//    m_setPartitSize->move(this->size().width()/2+mm.width()/2-2,this->size().height()-mm.height()-107-24);//bug221711
    m_setPartitSize->move(152,160);

//    QString strLangu = KServer::ReadSettingIni("config", "language");
//    if(strLangu=="mn_MN"){
//        m_sysTip->move(2,this->size().height()-mm.height()-107-24);
//        m_dataTip->move(2,this->size().height()-mm.height()-107+14-24);
//    }else{
//        m_sysTip->move((this->size().width()-mm.width()-m_setPartitSize->width()-10)/2-2,this->size().height()-mm.height()-107-24);
//        m_dataTip->move((this->size().width()-mm.width()-m_setPartitSize->width()-10)/2-2,this->size().height()-mm.height()-107+14-24);
//    }

    m_DateFocseWidget->move(24,158);
    m_DateBackgroundWidget->move(152-20,160-20);

//    if(m_diskNameTip->text().size()>50 && m_diskNameTip->text().trimmed().count(" ") > 1){
//        m_diskNameTip->move(2,75);//bug235750
//    }else{
//        m_diskNameTip->move(2,68);
//    }
}
