/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 * 
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
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

#include "fdclickwidget.h"
#include "MacroFile.h"
#include <KWindowEffects>
#include <QFontMetrics>
#include "flashdiskdata.h"
#include "UnionVariable.h"
#include <QDebug>

FDClickWidget::FDClickWidget(FDFrame *parent,
                          unsigned diskNo,
                          QString strDriveId,
                          QString strVolumeId,
                          QString strMountId,
                          QString driveName,
                          QString volumeName,
                          quint64 capacityDis,
                          QString strMountUri,
                          int   uVolumeSum)
    : m_frame(parent),
      m_uDiskNo(diskNo),
      m_driveName(driveName),
      m_volumeName(volumeName),
      m_capacityDis(capacityDis),
      m_mountUri(strMountUri),
      m_driveId(strDriveId),
      m_volumeId(strVolumeId),
      m_mountId(strMountId)
{
//union layout
/*
 * it's the to set the title interface,we get the drive name and add picture of a u disk
*/
    const QByteArray id(THEME_QT_SCHEMA);

    if(QGSettings::isSchemaInstalled(id))
    {
        fontSettings = new QGSettings(id);
    }

    const QByteArray idd(THEME_QT_SCHEMA);

    if(QGSettings::isSchemaInstalled(idd))
    {
        qtSettings = new QGSettings(idd);
    }

    initFontSize();
    initThemeMode();

    if (m_uDiskNo <= 1) {
        this->setFixedSize(388,100);
    } else {
        this->setFixedSize(288,22);
    }

    QString systemLang = QLocale::system().name();
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        m_letfToRight = false;
    }

    QHBoxLayout *drivename_H_BoxLayout = new QHBoxLayout();
    if (m_uDiskNo <= 1) {
        if (layoutDirection() == Qt::LeftToRight || m_letfToRight) {
            image_show_label = new QLabel(this);
            image_show_label->setFocusPolicy(Qt::NoFocus);
            image_show_label->installEventFilter(this);
            //to get theme picture for label
#if IFDISTINCT_DEVICON
            imgIcon = QIcon::fromTheme("drive-removable-media-usb-symbolic");
            if (imgIcon.isNull()) {
                imgIcon = QIcon::fromTheme("media-removable-symbolic");
            }
#else
            imgIcon = QIcon::fromTheme("media-removable-symbolic");
#endif
            QPixmap pixmap = imgIcon.pixmap(QSize(16, 16));
            image_show_label->setPixmap(pixmap);
            image_show_label->setProperty("useIconHighlightEffect", 0x2);
            image_show_label->setFixedSize(30,30);
            if(uVolumeSum == 1)
                image_show_label->move(26,39);
            else
                image_show_label->move(26,27);

            m_driveName_label = new QLabel(this);
            QFont font = m_driveName_label->font();
            font.setPointSize(fontSize);
            m_driveName_label->setFont(font);
            QString DriveName = getElidedText(m_driveName_label->font(), m_driveName, 180);
            m_driveName_label->setText(DriveName);
            if (DriveName != m_driveName) {
                m_driveName_label->setToolTip(m_driveName);
            }
            m_driveName_label->setStyleSheet("QLabel{color: palette(Text);}");
            m_driveName_label->setFixedSize(288,26);
            m_driveName_label->setObjectName("driveNameLabel");

            m_eject_button = new ClickLabel(this);
            m_eject_button->setProperty("useIconHighlightEffect", 0x2);
            m_eject_button->setProperty("useButtonPalette", true);
            //m_eject_button->setFlat(true);   //this property set that when the mouse is hovering in the icon the icon will move up a litte
            m_eject_button->setFixedSize(60,100);
            m_eject_button->move(m_eject_button->x()+320,m_eject_button->y()+10);
            m_eject_button->installEventFilter(this);
            m_eject_button->setParent(this);
            m_eject_button->setToolTip(tr("eject"));
            m_eject_button->show();
            connect(m_eject_button,SIGNAL(clicked()),SLOT(switchWidgetClicked()));  // this signal-slot function is to emit a signal which
            //is to trigger a slot in mainwindow
            drivename_H_BoxLayout->setContentsMargins(0,0,0,0);
            drivename_H_BoxLayout->addSpacing(45);
            drivename_H_BoxLayout->addWidget(m_driveName_label);     //add drive name label 2209以前设备名与设备名称使用同一标签
            drivename_H_BoxLayout->addStretch();
        } else {
            m_eject_button = new ClickLabel(this);
            m_eject_button->setProperty("useIconHighlightEffect", 0x2);
            m_eject_button->setProperty("useButtonPalette", true);
            m_eject_button->setFixedSize(60,100);
            m_eject_button->move(this->width() - m_eject_button->x() - 320 - m_eject_button->width(), m_eject_button->y()+10);
            m_eject_button->installEventFilter(this);
            m_eject_button->setParent(this);
            m_eject_button->setToolTip(tr("eject"));
            m_eject_button->show();
            connect(m_eject_button,SIGNAL(clicked()),SLOT(switchWidgetClicked()));  // this signal-slot function is to emit a signal which
                                                                                //is to trigger a slot in mainwindow
            driveWidget = new QWidget(this);
            driveWidget->setFixedSize(288,26);
            m_driveName_label = new QLabel;
            QFont font = m_driveName_label->font();
            font.setPointSize(fontSize);
            m_driveName_label->setFont(font);
            QString DriveName = getElidedText(m_driveName_label->font(), m_driveName, 180);
            m_driveName_label->setText(DriveName);
            if (DriveName != m_driveName) {
                m_driveName_label->setToolTip(m_driveName);
            }
            m_driveName_label->setStyleSheet("QLabel{color: palette(Text);}");
            m_driveName_label->setFixedSize(288,26);
            m_driveName_label->adjustSize();  //根据内容自动调整控件大小
            m_driveName_label->setObjectName("driveNameLabel");

            image_show_label = new QLabel(this);
            image_show_label->setFocusPolicy(Qt::NoFocus);
            image_show_label->installEventFilter(this);
            //to get theme picture for label
            #if IFDISTINCT_DEVICON
            imgIcon = QIcon::fromTheme("drive-removable-media-usb-symbolic");
            if (imgIcon.isNull()) {
                imgIcon = QIcon::fromTheme("media-removable-symbolic");
            }
            #else
            imgIcon = QIcon::fromTheme("media-removable-symbolic");
            #endif
            QPixmap pixmap = imgIcon.pixmap(QSize(16, 16));
            image_show_label->setFixedSize(30,30);
            image_show_label->setProperty("useIconHighlightEffect", 0x2);
            image_show_label->setPixmap(pixmap);

            if(uVolumeSum == 1)
                image_show_label->move(this->width() - 29 - image_show_label->width(),39);
            else
                image_show_label->move(this->width() - 29 - image_show_label->width(),27);

            drivename_H_BoxLayout->addSpacing(60);
            drivename_H_BoxLayout->setMargin(0);
            drivename_H_BoxLayout->addWidget(m_driveName_label);
            drivename_H_BoxLayout->addSpacing(20);
            drivename_H_BoxLayout->addStretch();
            driveWidget->setLayout(drivename_H_BoxLayout);
        }
    }

    disWidgetNumOne = new QWidget(this);
    disWidgetNumOne->setFixedSize(288,23);
    //disWidgetNumOne->setStyleSheet("border:1px solid red;");//for test

    m_nameDis1_label = new ClickLabel(disWidgetNumOne);
    QFont font1 = m_nameDis1_label->font();
    font1.setPointSize(fontSize-2);
    m_nameDis1_label->setFont(font1);
   if (layoutDirection() == Qt::LeftToRight || m_letfToRight)
       m_nameDis1_label->setFixedSize(288,23);
   else
       m_nameDis1_label->setFixedSize(200,23);
    m_nameDis1_label->setFullOpacity();
    handleVolumeLabelForFat32Me(m_volumeName, m_volumeId);
    QString VolumeName = getElidedText(m_nameDis1_label->font(), m_volumeName, 120); //超出一定宽度后显示省略号
    m_nameDis1_label->adjustSize();  //根据内容自动调整控件大小
    m_nameDis1_label->setText("- "+VolumeName+":");
    if (m_volumeName != VolumeName) {
        m_nameDis1_label->setToolTip(m_volumeName);
    }

    m_capacityDis1_label = new QLabel();
    QString str_capacityDis1 = size_human(m_capacityDis);  //将容量转换为XXGB形式
    font1 = m_capacityDis1_label->font();
    font1.setPointSize(fontSize-2);
    m_capacityDis1_label->setFont(font1);
    m_capacityDis1_label->setText("("+str_capacityDis1+")");

    //卷名标签的横向布局
    QHBoxLayout *onevolume_h_BoxLayout = new QHBoxLayout();
    onevolume_h_BoxLayout->setSpacing(0);
    onevolume_h_BoxLayout->addSpacing(60);
    onevolume_h_BoxLayout->setMargin(0);   //使得widget上的label得以居中显示
    onevolume_h_BoxLayout->addWidget(m_nameDis1_label);
    onevolume_h_BoxLayout->addSpacing(20);
    onevolume_h_BoxLayout->addStretch();

    
    disWidgetNumOne->setObjectName("OriginObjectOnly");
    disWidgetNumOne->setStyleSheet("QLabel{color: palette(PlaceholderText);}"
                                   "QLabel:hover{color: palette(Highlight);}");
    disWidgetNumOne->setLayout(onevolume_h_BoxLayout);
    disWidgetNumOne->installEventFilter(this);

    //设置主页面布局
    main_V_BoxLayout = new QVBoxLayout(this);
    if(m_uDiskNo <= 1){
        if (uVolumeSum == 1) {
            main_V_BoxLayout->setContentsMargins(15, 10, 6, 20);
        } else {
            main_V_BoxLayout->setContentsMargins(15, 0, 6, 20);
        }
        main_V_BoxLayout->setSpacing(0);
        main_V_BoxLayout->addSpacing(20);
        if (layoutDirection() == Qt::LeftToRight || m_letfToRight)
            main_V_BoxLayout->addLayout(drivename_H_BoxLayout);
        else
            main_V_BoxLayout->addWidget(driveWidget);
    }
    if (uVolumeSum == 1){
        main_V_BoxLayout->setContentsMargins(15, 10, 6, 20);
    }
    else{
        main_V_BoxLayout->setContentsMargins(15, 0, 6, 20);
    }
    main_V_BoxLayout->addSpacing(0);
    main_V_BoxLayout->addWidget(disWidgetNumOne);
    main_V_BoxLayout->addSpacing(0);
    this->setLayout(main_V_BoxLayout);

    if (m_mountUri.isEmpty()) {
        m_capacityDis1_label->setText(tr("Unmounted"));
    }

    //this->setAttribute(Qt::WA_TranslucentBackground, true); //设置顶层面板背景透明

    // check capacity lable width
    m_strCapacityDis = m_capacityDis1_label->text();  //获取qlabel中的字符串
    qDebug()<<"----添加主窗口控件-卷名为"<<VolumeName;
    m_nameDis1_label->setText("- "+VolumeName+":"+m_strCapacityDis);

    connect(this, &FDClickWidget::themeFontChange, this, &FDClickWidget::onThemeFontChange);
    //弹出按钮滑动效果
    connect(this, &FDClickWidget::hoverEjectBotton, m_frame, &FDFrame::on_updateSize);
    connect(this, &FDClickWidget::leaveEjectBotton, m_frame, &FDFrame::on_cursorLeave);
}

void FDClickWidget::initFontSize()
{
    if (!fontSettings)
    {
       fontSize = 11;
       return;
    }
    connect(fontSettings,&QGSettings::changed,[=](QString key)
    {
        if("systemFont" == key || "systemFontSize" == key)
        {
            fontSize = fontSettings->get(FONT_SIZE).toString().toFloat();
            Q_EMIT themeFontChange(fontSize);
        }
    });

    QStringList keys = fontSettings->keys();
    if (keys.contains("systemFont") || keys.contains("systemFontSize"))
    {
        fontSize = fontSettings->get(FONT_SIZE).toInt();
    }
}

void FDClickWidget::onThemeFontChange(qreal lfFontSize)
{
}

void FDClickWidget::on_hideinterface()
{
    m_frame->hide();
}

void FDClickWidget::initThemeMode()
{
    if(!qtSettings)
    {
        currentThemeMode = "ukui-white";
    }
    QStringList keys = qtSettings->keys();
    if(keys.contains("styleName"))
    {
        currentThemeMode = qtSettings->get("style-name").toString();
    }
}

FDClickWidget::~FDClickWidget()
{
    if(chooseDialog)
        delete chooseDialog;
    if(gpartedface)
        delete gpartedface;
    if (fontSettings) {
        delete fontSettings;
        fontSettings = nullptr;
    }
    if (qtSettings) {
        delete qtSettings;
        qtSettings = nullptr;
    }
}

void FDClickWidget::mousePressEvent(QMouseEvent *ev)
{
    mousePos = QPoint(ev->x(), ev->y());
    this->m_fristClickWidget->m_mousePressFlag = true;
    this->m_fristClickWidget->disWidgetNumOne->setStyleSheet("QLabel{color: palette(Highlight);}"
                                                         "QLabel:hover{color: palette(Highlight);}");
    this->m_fristClickWidget->repaint();
}

void FDClickWidget::mouseReleaseEvent(QMouseEvent *ev)
{
    if(mousePos == QPoint(ev->x(), ev->y())) Q_EMIT clicked();

    qDebug()<<"mousePos is x:"<<ev->x()<<"y:"<<ev->y();
    this->m_fristClickWidget->m_mousePressFlag = false;
    this->m_fristClickWidget->disWidgetNumOne->setStyleSheet("QLabel{color: palette(PlaceholderText);}"
                                                              "QLabel:hover{color: palette(Highlight);}");
    this->m_fristClickWidget->repaint();
    if(ev->x()<=65)
        openFirstVolume();
    else if(ev->x()<=330)
        on_volume_clicked();
}

void FDClickWidget::openFirstVolume()
{
    if (!this->m_fristClickWidget->m_mountUri.isEmpty()) {
        qDebug()<<"----uri is "<<this->m_fristClickWidget->m_mountUri;
        QString aaa = "peony "+this->m_fristClickWidget->m_mountUri;
        QProcess::startDetached(aaa.toUtf8().data());
        this->topLevelWidget()->hide();
    }
}

//click the first area to show the interface
void FDClickWidget::on_volume_clicked()
{
    if (!m_mountUri.isEmpty()) {
        qDebug()<<"----uri is "<<m_mountUri;
        QString aaa = "peony "+m_mountUri;
        QProcess::startDetached(aaa.toUtf8().data());
        this->topLevelWidget()->hide();
    }
}

void FDClickWidget::switchWidgetClicked()
{
    EjectDeviceInfo eDeviceInfo;
    eDeviceInfo.strDriveId = m_driveId;
    eDeviceInfo.strDriveName = m_driveName;
    eDeviceInfo.strVolumeId = m_volumeId;
    eDeviceInfo.strVolumeName = m_volumeName;
    eDeviceInfo.strMountId = m_mountId;
    eDeviceInfo.strMountUri = m_mountUri;
    m_frame->hide();
    Q_EMIT clickedEject(eDeviceInfo);
}

QPixmap FDClickWidget::drawSymbolicColoredPixmap(const QPixmap &source)
{
    if(currentThemeMode == "ukui-light" || currentThemeMode == "ukui-white")
    {
        QImage img = source.toImage();
        for (int x = 0; x < img.width(); x++)
        {
            for (int y = 0; y < img.height(); y++)
            {
                auto color = img.pixelColor(x, y);
                if (color.alpha() > 0)
                {
                        color.setRed(0);
                        color.setGreen(0);
                        color.setBlue(0);
                        img.setPixelColor(x, y, color);
                }
            }
        }
        return QPixmap::fromImage(img);
    }

    else if(currentThemeMode == "ukui-dark" || currentThemeMode == "ukui-black" || currentThemeMode == "ukui-default" )
    {
        QImage img = source.toImage();
        for (int x = 0; x < img.width(); x++)
        {
            for (int y = 0; y < img.height(); y++)
            {
                auto color = img.pixelColor(x, y);
                if (color.alpha() > 0)
                {
                        color.setRed(255);
                        color.setGreen(255);
                        color.setBlue(255);
                        img.setPixelColor(x, y, color);
                }
            }
        }
        return QPixmap::fromImage(img);
    }

    else
    {
        QImage img = source.toImage();
        for (int x = 0; x < img.width(); x++)
        {
            for (int y = 0; y < img.height(); y++)
            {
                auto color = img.pixelColor(x, y);
                if (color.alpha() > 0)
                {
                        color.setRed(0);
                        color.setGreen(0);
                        color.setBlue(0);
                        img.setPixelColor(x, y, color);
                }
            }
        }
        return QPixmap::fromImage(img);
    }
}

//to convert the capacity by another type
QString FDClickWidget::size_human(qlonglong capacity)
{
    //    float capacity = this->size();
    if(capacity > 1)
    {
        int conversionNum = 0;
        QStringList list;
        list << "KB" << "MB" << "GB" << "TB";

        QStringListIterator i(list);
        QString unit("B");

        qlonglong conversion = capacity;

        while(conversion >= 1024.0 && i.hasNext())
        {
            unit = i.next();
            conversion /= 1024.0;
            conversionNum++;
        }
        qlonglong remain  = capacity - conversion * qPow(1024,conversionNum);
        double showRemain = 0.0;
        if(conversionNum == 4) {
            showRemain = (float)remain /1024/1024/1024/1024;
        } else if(conversionNum == 3) {
            showRemain = (float)remain /1024/1024/1024;
        } else if(conversionNum == 2) {
            showRemain = (float)remain /1024/1024;
        } else if(conversionNum == 1) {
            showRemain = (float)remain /1024;
        }

        double showValue = conversion + showRemain;

        QString str2=QString::number(showValue,'f',1);

        QString str_capacity=QString(" %1%2").arg(str2).arg(unit);
        return str_capacity;
     //   return QString().setNum(capacity,'f',2)+" "+unit;
    }
#if (QT_VERSION < QT_VERSION_CHECK(5,7,0))
    if(capacity == NULL)
    {
       QString str_capaticity = tr("the capacity is empty");
       return str_capaticity;
    }
#endif
    if(capacity == 1)
    {
        QString str_capacity = tr("blank CD");
        return str_capacity;
    }
     QString str_capacity = tr("other user device");
     return str_capacity;
}



//set the style of the eject button and label when the mouse doing some different operations
bool FDClickWidget::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == m_eject_button)
    {

        if(event->type() == QEvent::Enter)
        {

            Q_EMIT hoverEjectBotton();
            return true;

        }

        if(event->type() == QEvent::Leave)
        {

            Q_EMIT leaveEjectBotton();
            return true;

        }
        return false;
    }

    return QWidget::eventFilter(obj, event);
}

void FDClickWidget::resizeEvent(QResizeEvent *event)
{
}

void FDClickWidget::paintEvent(QPaintEvent * event)
{
    //绘制图标底圆
    if(image_show_label != nullptr){
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//        painter.setClipping(true);
        painter.setPen(Qt::transparent);
        if(m_mousePressFlag == false ){
            painter.setBrush(QColor(this->palette().color(QPalette::BrightText).red(),
                                    this->palette().color(QPalette::BrightText).green(),
                                    this->palette().color(QPalette::BrightText).blue(),
                                    (0.1*255))
                                 );
        }
        else{
            painter.setBrush(QColor(this->palette().color(QPalette::BrightText).red(),
                                    this->palette().color(QPalette::BrightText).green(),
                                    this->palette().color(QPalette::BrightText).blue(),
                                    (0.3*255))
                             );
        }
        if (layoutDirection() == Qt::LeftToRight || m_letfToRight)
            painter.drawEllipse((image_show_label->pos().x()-10),(image_show_label->pos().y()-3),36,36);
        else
            painter.drawEllipse((image_show_label->pos().x()+5),(image_show_label->pos().y()-3),36,36);
    }
}
