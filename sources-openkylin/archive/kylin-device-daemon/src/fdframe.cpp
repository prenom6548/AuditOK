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

#include <QTimer>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QPainterPath>
#include <KWindowEffects>
#include <QGSettings>
#include <QPixmap>
#include <QGraphicsColorizeEffect>
#include <kwindowsystem.h>
#include <kwindowsystem_export.h>
#include <ukuistylehelper/ukuistylehelper.h>

#include "config/xatom-helper.h"
#include "fdframe.h"
#include "UnionVariable.h"

#define THEME_UKFD_TRANS "org.ukui.control-center.personalise"
#define IN_THEME_BLACKLIST 0

FDFrame::FDFrame(QWidget* parent) : QFrame(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    initOpacityGSettings();
    installEventFilter(this);

    //窗管协议
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    //XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

    QString systemLang = QLocale::system().name();
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        m_letfToRight = false;
        qInfo() << "ug_CN || ky_KG || kk_KZ";
    }

    this->setFixedSize(388, 112);

    m_animLabel = new ClickLabel(this);
    m_animLabel->setFixedSize(320,112);

    if (layoutDirection() == Qt::LeftToRight || m_letfToRight)
        m_animLabel->move(0,0);
    else
        m_animLabel->move(this->width() - m_animLabel->width(),0);
    m_animLabel->setAutoFillBackground(true);
    setanimColor();
    m_animLabel->installEventFilter(this);
    m_animLabel->setParent(this);
    m_animLabel->show();

    //弹出 文本
    m_ejectLabel = new QLabel(this);
    QString DriveName = getElidedText(m_ejectLabel->font(), tr("eject"), 47);
    m_ejectLabel->setText(DriveName);
    m_ejectLabel->setStyleSheet("QLabel{color: palette(Text);}");
    m_ejectLabel->setParent(this);
    m_ejectLabel->setFixedSize(45,40);
    // QFontMetrics fm(m_ejectLabel->font());
    // int ascent = fm.ascent();      // 基线以上的高度
    // int descent = fm.descent();    // 基线以下的高度
    int moveY = m_height / 2 - m_ejectLabel->height() / 2;
    if (layoutDirection() == Qt::LeftToRight || m_letfToRight)
        m_ejectLabel->move(328,moveY);//之前35
    else
        m_ejectLabel->move(this->width() - 328 - m_ejectLabel->width(), moveY);
    m_ejectLabel->hide();

    //弹出 图标
    m_ejectIcon = new QLabel(this);
    m_ejectIcon->setProperty("useIconHighlightEffect", 0x2);
    m_ejectIcon->setProperty("useButtonPalette", true);
    QPixmap ejectpixmap =QIcon::fromTheme("media-eject-symbolic").pixmap(16,16);
    m_ejectIcon->setPixmap(ejectpixmap);
    m_ejectIcon->setStyleSheet("QLabel{color: palette(PlaceholderText);}");
    m_ejectIcon->setFixedSize(16,16);
    moveY = m_height / 2 - m_ejectIcon->height() / 2;
    if (layoutDirection() == Qt::LeftToRight || m_letfToRight)
        m_ejectIcon->move(343,moveY);//此前47，这里必须和下面动画起始状态的y保持一致
    else
        m_ejectIcon->move(this->width() - 343 - m_ejectIcon->width(), moveY);
    m_ejectIcon->show();

    kdk::UkuiStyleHelper::self()->removeHeader(this);

#ifdef KY_SDK_WAYLANDHELPER
    // 初始化 UkuiWindowHelper
    m_windowHelper = new UkuiWindowHelper(this);
    m_windowHelper->setSkipTaskBar(true);
    m_windowHelper->setSkipSwitcher(true);
    m_windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::Notification);
#endif
}

FDFrame::~FDFrame()
{
    if (m_gsTransOpacity) {
        delete m_gsTransOpacity;
        m_gsTransOpacity = nullptr;
    }
#ifdef KY_SDK_WAYLANDHELPER
    if (m_windowHelper) {
        delete m_windowHelper;
        m_windowHelper = nullptr;
    }
#endif
}

void FDFrame::initOpacityGSettings()
{
    const QByteArray idtrans(THEME_UKFD_TRANS);
    if(QGSettings::isSchemaInstalled(idtrans)) {
        m_gsTransOpacity = new QGSettings(idtrans);
    }

    if (!m_gsTransOpacity) {
        m_curTransOpacity = 1;
        return;
    }

    connect(m_gsTransOpacity, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "transparency") {
            QStringList keys = m_gsTransOpacity->keys();
            if (keys.contains("transparency")) {
                m_curTransOpacity = m_gsTransOpacity->get("transparency").toString().toDouble();
                repaint();
            }
        }
    });

    QStringList keys = m_gsTransOpacity->keys();
    if(keys.contains("transparency")) {
        m_curTransOpacity = m_gsTransOpacity->get("transparency").toString().toDouble();
    }
}



int FDFrame::readlabeltWidth()
{
    return this->m_animLabel->width();
}

void FDFrame::setlabeltWidth(int width)
{
    if (layoutDirection() == Qt::LeftToRight || m_letfToRight) {
        this->m_animLabel->setFixedWidth(width);
        m_ejectIcon->move(width+23,m_mainWindowHigh/2 - m_ejectIcon->height() / 2);
        qInfo() << "left->right  setlabeltWidth width" << width << "m_ejectIcon.move(" << width+23 << ")";

        if(width == 288) {
            m_ejectIcon->setStyleSheet("QLabel{color: palette(Text);}");
            m_ejectIcon->setProperty("useIconHighlightEffect", 0x2);
            QFont systemFont = QApplication::font();
            // 设置m_ejectLabel的字体为系统字体,这两行代码先放在这里，保证每次U盘插入时都能对m_ejectLabel进行处理
            m_ejectLabel->setFont(systemFont);
            m_ejectLabel->show();
        } else {
            m_ejectIcon->setStyleSheet("QLabel{color: palette(PlaceholderText);}");
            m_ejectLabel->hide();
        }
    } else {
        this->m_animLabel->setFixedWidth(320-width);
        this->m_animLabel->move(this->width() - m_animLabel->width(), 0);
        m_ejectIcon->move(this->width() - readlabeltWidth() - 23 - m_ejectIcon->width(), m_mainWindowHigh/2 - m_ejectIcon->height() / 2);
        qInfo() << "right->left  setlabeltWidth width" << width << "m_ejectIcon.move(" << this->width() - readlabeltWidth() - 23 - m_ejectIcon->width() << ")";

        if(width == 32) {
            m_ejectIcon->setStyleSheet("QLabel{color: palette(Text);}");
            m_ejectIcon->setProperty("useIconHighlightEffect", 0x2);
            QFont systemFont = QApplication::font();
            // 设置m_ejectLabel的字体为系统字体,这两行代码先放在这里，保证每次U盘插入时都能对m_ejectLabel进行处理
            m_ejectLabel->setFont(systemFont);
            m_ejectLabel->show();
        } else {
            m_ejectIcon->setStyleSheet("QLabel{color: palette(PlaceholderText);}");
            m_ejectLabel->hide();
        }

    }
}

void FDFrame::setanimColor()
{
#if !IN_THEME_BLACKLIST
    QGSettings* gSettings = nullptr;
    if (QGSettings::isSchemaInstalled("org.ukui.style")){
         gSettings = new QGSettings("org.ukui.style");
    }

    QString borderRadius = QString::number(
                gSettings->get("window-radius").toString().split(",").value(0).toInt()) + "px";

    if (gSettings != nullptr) {
       m_currentThemeMode = gSettings->get("styleName").toString();
       connect(gSettings,&QGSettings::changed,this,[=](const QString &key) {
          if ((m_currentThemeMode != nullptr)&&(m_animLabel != nullptr)) {
            m_currentThemeMode = gSettings->get("styleName").toString();
            if (("ukui-white" == m_currentThemeMode)||("ukui-light" == m_currentThemeMode)) {
                m_animLabel->setStyleSheet(
                            QString("background-color:rgb(255,255,255); border-radius:%1;").arg(borderRadius));
            } else {
                m_animLabel->setStyleSheet(
                            QString("background-color:rgb(13,13,13); border-radius:%1;").arg(borderRadius));
            }
            repaint();
          }
       });
    }


    style()->unpolish(this->m_animLabel);
    if (("ukui-white" == m_currentThemeMode)||("ukui-light" == m_currentThemeMode)) {
        m_animLabel->setStyleSheet(
                    QString("background-color:rgb(255,255,255); border-radius:%1;").arg(borderRadius));
    } else {
        m_animLabel->setStyleSheet(
                    QString("background-color:rgb(13,13,13); border-radius:%1;").arg(borderRadius));
    }
    style()->polish(this->m_animLabel);
#else
    m_animLabel->setStyleSheet(
                QString("QLabel { background-color:rgb(13,13,13); border-radius:%1;}").arg(borderRadius));
#endif
}


void FDFrame::paintEvent(QPaintEvent * event)
{
    QPainterPath path;

    QPainter painter(this);
    painter.setOpacity(m_curTransOpacity);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
//    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    path.addRect(this->rect());
    path.setFillRule(Qt::WindingFill);
    painter.setBrush(this->palette().color(QPalette::Window));
    painter.setPen(Qt::transparent);
    painter.drawPath(path);
#if !IN_THEME_BLACKLIST
    //setanimColor();
#endif
    QFrame::paintEvent(event);
}

bool FDFrame::eventFilter(QObject *obj, QEvent *event)
{
    static bool bIsEnterFlag = false;

    if(event->type() == QEvent::Leave)
    {

        if(bIsEnterFlag){
            m_curTransOpacity-=0.1;
            bIsEnterFlag = false;
        }else{
            bIsEnterFlag = true;
        }

        update();
        return false;

    }
    if(event->type() == QEvent::Enter)
    {
        //由于从底部或左侧进入会重复出现两次ENTER事件导致透明度设置异常，故暂先使用以下手段规避问题
        if(!bIsEnterFlag){
            m_curTransOpacity+=0.1;
            bIsEnterFlag = true;
        }else{
            bIsEnterFlag = false;
        }

        update();

//        if (m_hideFrameTime->isActive()) {
//          m_hideFrameTime->stop();
//        }
        return false;
    }
    return QWidget::eventFilter(obj, event);

}
void FDFrame::showEvent(QShowEvent *event) {
    QFrame::showEvent(event);
    
    // this->update();
    
    //鼠标已经在内部了，延迟一段时间再启动动画，确保背景先显示完
    if (this->underMouse()) {
        this->repaint();
        QTimer::singleShot(50, this, [this]() {
            if (this->underMouse()) {
                on_updateSize();
            }
        });
    }
}
void FDFrame::on_updateSize()
{
    // 强制刷新 m_animLabel的样式
    m_animLabel->style()->unpolish(m_animLabel);
    m_animLabel->style()->polish(m_animLabel);
    m_animLabel->repaint();  // 组件本身立即重绘
    
    //在子控件动画开始前，先刷新FDFrame背景
    this->update();

    QPropertyAnimation * m_enterAnimation = new QPropertyAnimation(this, "labeltWidth");
    m_enterAnimation->setDuration(300);
    if (layoutDirection() == Qt::LeftToRight || m_letfToRight) {
        m_enterAnimation->setStartValue(320);
        m_enterAnimation->setEndValue(288);
        qInfo() << "left->right  on_cursorLeave 320->288";
    } else {
        m_enterAnimation->setStartValue(0);
        m_enterAnimation->setEndValue(32);
        qInfo() << "right->left  on_cursorLeave 0->32";
    }
    m_enterAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void FDFrame::on_cursorLeave()
{
    if(this->m_animLabel->width() != 320){  //在某些架构上初始会触发光标离开事件，触发原因未知
        QPropertyAnimation * m_leaveAnimation = new QPropertyAnimation(this, "labeltWidth");
        m_leaveAnimation->setDuration(300);
        if (layoutDirection() == Qt::LeftToRight || m_letfToRight) {
            m_leaveAnimation->setStartValue(288);
            m_leaveAnimation->setEndValue(320);
            qInfo() << "left->right  on_cursorLeave 288->320";
        } else {
            m_leaveAnimation->setStartValue(32);
            m_leaveAnimation->setEndValue(0);
            qInfo() << "right->left  on_cursorLeave 32->0";
        }
        m_leaveAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    }

}

void FDFrame::showNoIcon(){

    //不在任务栏处显示窗口图标
    const KWindowInfo info(this->winId(), NET::WMState);
    if (!info.hasState(NET::SkipTaskbar) || !info.hasState(NET::SkipPager)) {
        KWindowSystem::setState(this->winId(), NET::SkipTaskbar | NET::SkipPager);
    }

    this->show();
}

void FDFrame::setEjectOnCentra(int height)
{
    if (layoutDirection() == Qt::LeftToRight || m_letfToRight) {
        m_ejectIcon->move(343,height/2-11);
        m_ejectLabel->move(328,height/2-22);  //减去字体高度
    } else {
        m_ejectIcon->move(this->width() - 343 - m_ejectIcon->width(), height/2-11);
        m_ejectLabel->move(this->width() - 328 - m_ejectLabel->width(), height/2-22);  //减去字体高度
    }
}

//void FDFrame::on_hideframe()
//{
//    //m_hideFrameTime->stop();
//    //this->hide();

//}
