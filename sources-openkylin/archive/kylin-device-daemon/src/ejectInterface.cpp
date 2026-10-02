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
#include "ejectInterface.h"
#include <qgsettings.h>
#include <KWindowEffects>
#include "flashdiskdata.h"

#include <ukuistylehelper/ukuistylehelper.h>

//MARGIN 为到任务栏或屏幕边缘的间隔
#define MARGIN 8

ejectInterface::ejectInterface(QWidget *parent,bool forceEject) : QWidget(parent),eject_image_label(nullptr),show_text_label(nullptr),
    mount_name_label(nullptr)
{
    initFontSetting();
    getFontSize();

    this->setFocusPolicy(Qt::NoFocus);

#ifdef KY_SDK_WAYLANDHELPER
    m_windowHelper = new UkuiWindowHelper(this);
    m_windowHelper->setSkipTaskBar(true);
    m_windowHelper->setSkipSwitcher(true);
    m_windowHelper->removeTitleBar();
    m_windowHelper->setBlurEffect(QRegion());
#endif

    EjectScreen = qApp->primaryScreen();
    eject_image_label = new QLabel(this);
    eject_image_label->setFixedSize(32,32);
    eject_image_icon = QIcon::fromTheme("media-removable-symbolic");
    QPixmap pixmap = eject_image_icon.pixmap(QSize(32, 32));
    eject_image_label->setPixmap(pixmap);
    eject_image_label->setProperty("useIconHighlightEffect", 0x2);

    QHBoxLayout *ejectTipLayout = new QHBoxLayout();
    ejectTipLayout->setContentsMargins(0,0,0,0);
    ejectTipLayout->setSpacing(16);
    QLabel *ejectTip = new QLabel();
    ejectTip->setText(tr("Storage device removed"));
    ejectTip->setAlignment(Qt::AlignLeft|Qt::AlignTop);
    //ejectTip->setWordWrap(true);  自动换行会影响特殊字体显示
    ejectTipLayout->addSpacing(40);
    ejectTipLayout->addStretch();
    ejectTipLayout->addWidget(eject_image_label,0,Qt::AlignHCenter|Qt::AlignVCenter);
    ejectTipLayout->addWidget(ejectTip,0,Qt::AlignHCenter|Qt::AlignVCenter);
    ejectTipLayout->addStretch();
    ejectTipLayout->addSpacing(40);

    main_V_BoxLayput = new QVBoxLayout;
    main_V_BoxLayput->addLayout(ejectTipLayout);
    this->setFixedHeight(88);
    this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明
    this->setAttribute(Qt::WA_DeleteOnClose); //设置窗口关闭时自动销毁
    this->setLayout(main_V_BoxLayput);

    //set the main signal-slot function to complete the eject interface to let it disappear automatically
    interfaceHideTime = new QTimer(this);
    interfaceHideTime->setTimerType(Qt::PreciseTimer);
    connect(interfaceHideTime, SIGNAL(timeout()), this, SLOT(on_interface_hide()));
    interfaceHideTime->start(1000);
    moveEjectInterfaceRight();
    initTransparentState();
    this->getTransparentData();
}

ejectInterface::~ejectInterface()
{
    if (m_transparency_gsettings) {
        delete m_transparency_gsettings;
        m_transparency_gsettings = nullptr;
    }
    if (fontSettings) {
        delete fontSettings;
        fontSettings = nullptr;
    }

#ifdef KY_SDK_WAYLANDHELPER
    if (m_windowHelper) {
        delete m_windowHelper;
        m_windowHelper = nullptr;
    }
#endif
}

//If the fillet does not take effect
void ejectInterface::paintEvent(QPaintEvent *event)
 {
    QPainterPath path;

    auto rect = this->rect();
    rect.adjust(1, 1, -1, -1);
    path.addRect(rect);

    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    QRect rectReal = this->rect();
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    p.setBrush(opt.palette.color(QPalette::Base));
    p.setOpacity(m_transparency);
    p.setPen(Qt::NoPen);
    p.drawRect(rectReal);
    QWidget::paintEvent(event);

    //边框阴影
//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing, true); //渲染，抗锯齿
//    QColor color = this->palette().shadow().color();
//    QPainterPath shadowPath;
//    shadowPath.setFillRule(Qt::WindingFill);
//    shadowPath.addRoundedRect(0, 0, this->width(),this->height(), 12, 12);
//    painter.setPen(color);
//    painter.drawPath(shadowPath);
 }

//slot function to hide eject interface
void ejectInterface::on_interface_hide()
{
    this->hide();
    this->deleteLater();
}

//set the location of the eject interface
void ejectInterface::moveEjectInterfaceRight()
{
    //show the ejectinterface by primaryscreen()
    int position=0;
    int panelSize=0;
    if(QGSettings::isSchemaInstalled(QString("org.ukui.panel.settings").toLocal8Bit())){
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
    else{
        position=0;
        panelSize=SmallPanelSize;
    }

    int x=QApplication::primaryScreen()->geometry().x();
    int y=QApplication::primaryScreen()->geometry().y();
    qDebug()<<"position"<<position<<",x"<<x + QApplication::primaryScreen()->geometry().width()-this->width()<<",y"<<y+QApplication::primaryScreen()->geometry().height()-panelSize-this->height();
    if(position==0)
        kdk::WindowManager::setGeometry(this->windowHandle(), QRect(x + QApplication::primaryScreen()->geometry().width()-this->width()-MARGIN,y+QApplication::primaryScreen()->geometry().height()-panelSize-this->height()-MARGIN,this->width(),this->height()));
    else if(position==1)
        kdk::WindowManager::setGeometry(this->windowHandle(), QRect(x + QApplication::primaryScreen()->geometry().width()-this->width()-MARGIN,y+panelSize+MARGIN,this->width(),this->height()));    // Style::minw,Style::minh the width and the height of the interface  which you want to show
    else if(position==2)
        kdk::WindowManager::setGeometry(this->windowHandle(), QRect(x+panelSize+MARGIN,y + QApplication::primaryScreen()->geometry().height() - this->height()-MARGIN,this->width(),this->height()));
    else
        kdk::WindowManager::setGeometry(this->windowHandle(), QRect(x+QApplication::primaryScreen()->geometry().width()-panelSize-this->width()-MARGIN,y + QApplication::primaryScreen()->geometry().height() - this->height()-MARGIN,this->width(),this->height()));
}

int ejectInterface::getPanelPosition(QString str)
{
    QDBusInterface interface( "com.ukui.panel.desktop",
                              "/",
                              "com.ukui.panel.desktop",
                              QDBusConnection::sessionBus() );
    QDBusReply<int> reply = interface.call("GetPanelPosition", str);

    return reply;
}

/*
    use the dbus to get the height of the panel
*/
int ejectInterface::getPanelHeight(QString str)
{
    QDBusInterface interface( "com.ukui.panel.desktop",
                              "/",
                              "com.ukui.panel.desktop",
                              QDBusConnection::sessionBus() );
    QDBusReply<int> reply = interface.call("GetPanelSize", str);
    return reply;
}

void ejectInterface::initTransparentState()
{
    const QByteArray idtrans(THEME_QT_TRANS);

    if(QGSettings::isSchemaInstalled(idtrans))
    {
        m_transparency_gsettings = new QGSettings(idtrans);
    }
}

void ejectInterface::getTransparentData()
{
    if (!m_transparency_gsettings)
    {
       m_transparency = 0.95;
       return;
    }

    QStringList keys = m_transparency_gsettings->keys();
    if (keys.contains("transparency"))
    {
        m_transparency = m_transparency_gsettings->get("transparency").toDouble();
    }

}

void ejectInterface::initFontSetting()
{
    const QByteArray id(THEME_QT_SCHEMA);

    if(QGSettings::isSchemaInstalled(id))
    {
        fontSettings = new QGSettings(id);
    }
}

void ejectInterface::showEjectInterface()
{
    //m_windowHelper->setSlideEffect(UkuiWindowHelper::Position(3), 0, true);
    this->show();
    this->moveEjectInterfaceRight();
    kdk::KSoundEffects::playSound(SoundType::DEVICE_REMOVED);
}

void ejectInterface::getFontSize()
{

    if (!fontSettings)
    {
       fontSize = 11;
       return;
    }

    QStringList keys = fontSettings->keys();
    if (keys.contains("systemFont") || keys.contains("systemFontSize"))
    {
        fontSize = fontSettings->get("system-font").toInt();
    }
}
