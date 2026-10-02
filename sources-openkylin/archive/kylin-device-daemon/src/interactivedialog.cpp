/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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

#include "interactivedialog.h"
#include "config/xatom-helper.h"
#include <KWindowEffects>
#include <ukuistylehelper/ukuistylehelper.h>

interactiveDialog::interactiveDialog(QString strDevId, QWidget *parent):QWidget(parent)
{
    this->setFixedSize(300,86);
    this->setWindowFlags(Qt::Dialog|Qt::WindowStaysOnTopHint);
    this->setFocusPolicy(Qt::NoFocus);
    installEventFilter(this);

    //窗管协议
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

    this->setAttribute(Qt::WA_TranslucentBackground);
//    setProperty("useSystemStyleBlur", true);  //毛玻璃效果
    const QByteArray idd(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(idd))
    {
        m_themeSettings = new QGSettings(idd);
    }
    initThemeMode();
    initWidgets(strDevId);
    connect(chooseBtnCancle,SIGNAL(clicked()),this,SLOT(close()));
    connect(chooseBtnContinue,SIGNAL(clicked()),this,SLOT(convert()));
    initTransparentState();
    getTransparentData();
}

void interactiveDialog::initTransparentState()
{
    const QByteArray idtrans(THEME_QT_TRANS);

    if(QGSettings::isSchemaInstalled(idtrans))
    {
        m_transparency_gsettings = new QGSettings(idtrans);
    }
}

void interactiveDialog::getTransparentData()
{
    if (!m_transparency_gsettings)
    {
       m_transparency = 0.95;
       return;
    }

    connect(m_transparency_gsettings, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "transparency") {
            QStringList keys = m_transparency_gsettings->keys();
            if (keys.contains("transparency")) {
                m_transparency = m_transparency_gsettings->get("transparency").toString().toDouble();
                //setWindowOpacity(m_transparency);
                repaint();
            }
        }
    });

    QStringList keys = m_transparency_gsettings->keys();
    if (keys.contains("transparency"))
    {
        m_transparency = m_transparency_gsettings->get("transparency").toDouble();
    }
}

void interactiveDialog::initWidgets(QString strDevId)
{
    contentLable = new QLabel(this);
    contentLable->setWordWrap(true);
    updateContentLable(strDevId);
    content_H_BoxLayout = new QHBoxLayout();
    content_H_BoxLayout->addWidget(contentLable, 1, Qt::AlignCenter);

    #ifdef UDISK_SUPPORT_FORCEEJECT
    chooseBtnCancle = new QPushButton();
    chooseBtnCancle->setText(tr("cancle"));
    chooseBtnCancle->setFlat(true);

    chooseBtnContinue = new QPushButton();
    chooseBtnContinue->setText(tr("yes"));
    chooseBtnContinue->setFlat(true);

    chooseBtn_H_BoxLayout = new QHBoxLayout();

    chooseBtn_H_BoxLayout->addSpacing(70);
    chooseBtn_H_BoxLayout->addWidget(chooseBtnCancle);
    chooseBtn_H_BoxLayout->addWidget(chooseBtnContinue);
    chooseBtn_H_BoxLayout->setSpacing(0);
    #else

    chooseBtnCancle = new QPushButton;
    chooseBtnCancle->setText(tr("yes"));
    chooseBtnCancle->setProperty("needTranslucent",true);

    chooseBtnContinue = new QPushButton;
    chooseBtnContinue->hide();

    chooseBtn_H_BoxLayout = new QHBoxLayout();

    chooseBtn_H_BoxLayout->addWidget(chooseBtnCancle, 1, Qt::AlignCenter);
    chooseBtn_H_BoxLayout->addWidget(chooseBtnContinue);
    chooseBtn_H_BoxLayout->setSpacing(0);
    #endif

    main_V_BoxLayout = new QVBoxLayout();
    main_V_BoxLayout->addLayout(content_H_BoxLayout);
    main_V_BoxLayout->addLayout(chooseBtn_H_BoxLayout);

    this->setLayout(main_V_BoxLayout);
}

void interactiveDialog::updateContentLable(QString strDevId)
{
    m_strDevId = strDevId;
    #ifdef UDISK_SUPPORT_FORCEEJECT
    if (m_strDevId.startsWith("/dev/sr")) {
        contentLable->setText(tr("cdrom is occupying,do you want to eject it"));
    } else if (m_strDevId.startsWith("/dev/mmcblk")) {
        contentLable->setText(tr("sd is occupying,do you want to eject it"));
    } else {
        contentLable->setText(tr("usb is occupying,do you want to eject it"));
    }
    #else
    if (m_strDevId.startsWith("/dev/sr")) {
        contentLable->setText(tr("cdrom is occupying"));
    } else if (m_strDevId.startsWith("/dev/mmcblk")) {
        contentLable->setText(tr("sd is occupying"));
    } else {
        contentLable->setText(tr("usb is occupying"));
    }
    #endif
}

void interactiveDialog::convert()
{
    Q_EMIT FORCESIG();
}

void interactiveDialog::moveChooseDialogRight()
{
    int position=0;
    int panelSize=0;
    if(QGSettings::isSchemaInstalled(QString("org.ukui.panel.settings").toLocal8Bit()))
    {
        QGSettings* gsetting=new QGSettings(QString("org.ukui.panel.settings").toLocal8Bit());
        if(gsetting->keys().contains(QString("panelposition")))
            position=gsetting->get("panelposition").toInt();
        else
            position=0;
        if(gsetting->keys().contains(QString("panelsize")))
            panelSize=gsetting->get("panelsize").toInt();
        else
            panelSize=SmallPanelSize;
    }
    else
    {
        position=0;
        panelSize=SmallPanelSize;
    }

    int x=QApplication::primaryScreen()->geometry().x();
    int y=QApplication::primaryScreen()->geometry().y();
    qDebug()<<"QApplication::primaryScreen()->geometry().x();"<<QApplication::primaryScreen()->geometry().x();
    qDebug()<<"QApplication::primaryScreen()->geometry().y();"<<QApplication::primaryScreen()->geometry().y();
    if(position==0)
        kdk::WindowManager::setGeometry(this->windowHandle(), QRect(x + QApplication::primaryScreen()->geometry().width()-this->width() - DISTANCEPADDING - DISTANCEMEND,y+QApplication::primaryScreen()->geometry().height()-panelSize-this->height() - DISTANCEPADDING,this->width(),this->height()));
    else if(position==1)
        kdk::WindowManager::setGeometry(this->windowHandle(), QRect(x + QApplication::primaryScreen()->geometry().width()-this->width() - DISTANCEPADDING - DISTANCEMEND,y+panelSize + DISTANCEPADDING,this->width(),this->height())); // Style::minw,Style::minh the width and the height of the interface  which you want to show
    else if(position==2)
        kdk::WindowManager::setGeometry(this->windowHandle(), QRect(x+panelSize + DISTANCEPADDING,y + QApplication::primaryScreen()->geometry().height() - this->height() - DISTANCEPADDING,this->width(),this->height()));
    else
        kdk::WindowManager::setGeometry(this->windowHandle(), QRect(x+QApplication::primaryScreen()->geometry().width()-panelSize-this->width() - DISTANCEPADDING,y + QApplication::primaryScreen()->geometry().height() - this->height() - DISTANCEPADDING,this->width(),this->height()));
}

void interactiveDialog::initThemeMode()
{
    if(!m_themeSettings)
    {
        currentThemeMode = "ukui-white";
    }else{
        QStringList keys = m_themeSettings->keys();
        if(keys.contains("styleName"))
        {
            currentThemeMode = m_themeSettings->get("style-name").toString();
        }
        connect(m_themeSettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "styleName") {
                QStringList keys = m_themeSettings->keys();
                if (keys.contains("styleName")) {
                    currentThemeMode = m_themeSettings->get("styleName").toString();
                    setBackgroundColor();
                    repaint();
                }
            }
        });
    }
    setBackgroundColor();

}

void interactiveDialog::setBackgroundColor()
{
    if(currentThemeMode == "ukui-light" || currentThemeMode == "ukui-white")
        m_backgroundColor.setRgb(255,255,255);
    else
        m_backgroundColor.setRgb(13,13,13);
}

void interactiveDialog::showInteractivDialog()
{
    kdk::UkuiStyleHelper::self()->removeHeader(this);
    this->show();
    this->moveChooseDialogRight();  //使用SDK接口替换setgeometry后需要在show后重新设置位置
    this->setFocus();

}



interactiveDialog::~interactiveDialog()
{
    if (m_transparency_gsettings) {
        delete m_transparency_gsettings;
        m_transparency_gsettings = nullptr;
    }
}

bool interactiveDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this)
    {
        if (event->type() == QEvent::WindowDeactivate)  {
            this->hide();
            return false;
        }
    }
    return false;
}

void interactiveDialog::paintEvent(QPaintEvent *event)
{
//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    QRect rect = this->rect();
//    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    p.setBrush(opt.palette.color(QPalette::Base));
////    p.setBrush(QColor(Qt::red));
////    p.setOpacity(m_transparency);
//    p.setPen(Qt::NoPen);
//    p.drawRoundedRect(rect, 6, 6);
//    qDebug()<<__FUNCTION__;
//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    QRect rect = this->rect();
//    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    p.setBrush(opt.palette.color(QPalette::Base));
//    p.setOpacity(m_transparency);
//    p.setPen(Qt::NoPen);
//    p.drawRoundedRect(rect, 6, 6);
//    QWidget::paintEvent(event);


//    qDebug()<<__FUNCTION__;
//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    QRect rect = this->rect();
//    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    p.setBrush(opt.palette.color(QPalette::Base));
//    p.setOpacity(m_transparency);
//    p.setPen(Qt::NoPen);
//    p.drawRect(rect);
//    QWidget::paintEvent(event);


//    QStyleOption opt;
//    opt.init(this);
//    QPainter p(this);
//    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    p.setBrush(opt.palette.color(QPalette::Base));
//    p.setOpacity(m_transparency);
//    QPainterPath path;
//    auto rect = this->rect();
//    path.addRoundedRect(rect, 12, 12);
//    path.drawRoundedRect
//    QRegion region(path.toFillPolygon().toPolygon());
//    this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明
//    QWidget::paintEvent(event);

    //    QStyleOption opt;
    //    opt.init(this);
    //    QPainter p(this);
    //    QPainterPath path;
    //    QRect rect = this->rect();
    //    p.setOpacity(0);
    //    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    //    p.setClipping(true);
    //    p.setPen(Qt::transparent);
    //    qInfo()<<"----透明度-"<<m_transparency;

    //    path.addRoundedRect(rect, 12, 12);
    //    path.setFillRule(Qt::WindingFill);
    //    p.setBrush(this->palette().color(QPalette::Base));
    //    p.setPen(Qt::transparent);
    //    p.drawPath(path); //绘制底色
    //    QWidget::paintEvent(event);

    QPainter painter(this);
    painter.setPen(Qt::transparent);
    painter.setBrush(m_backgroundColor);
    painter.drawRect(this->rect());

}
