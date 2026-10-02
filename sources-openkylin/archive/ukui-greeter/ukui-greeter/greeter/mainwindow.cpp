/* mainwindow.cpp
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#include "mainwindow.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QDir>
#include <QScreen>
#include <QWindow>
#include <QtConcurrent/QtConcurrentRun>
#include <QX11Info>
#include <QByteArray>
#include <QProcess>
#include <QtMath>
#include <QTimer>
#include "globalv.h"
#include "greeterwindow.h"
#include "common/configuration.h"
#include "common/monitorwatcher.h"
#include "display-switch/displayservice.h"
#include <xcb/xcb.h>
#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <X11/keysymdef.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/extensions/XInput2.h>
#include <X11/Xutil.h>
#include <libintl.h>
#include <unistd.h>
#include "xeventmonitor.h"
bool MainWindow::m_first = true;

QT_BEGIN_NAMESPACE
extern void qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);
QT_END_NAMESPACE

#define BLUR_RADIUS 300

QPixmap * blurPixmap(QPixmap *pixmap)
{
    QPainter painter(pixmap);
    QImage srcImg = pixmap->toImage();
    qt_blurImage(&painter, srcImg, BLUR_RADIUS, false, false);
    
    //在设置Qt::WA_TranslucentBackground属性后，模糊图片会导致锁屏界面透明
    //因此这里修改image图形的alpha值为255.
    for (int y = 0;y < srcImg.height();++y) {
      QRgb *row = (QRgb*)srcImg.scanLine(y);
      for (int x = 0; x < srcImg.width(); ++x) {
        ((unsigned char*)&row[x])[3] = 255;
      }
    }

    QPixmap *pix = new QPixmap(QPixmap::fromImage(srcImg));
    return pix;

}

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent),
      m_screenModel(new ScreenModel(this)),
      m_configuration(Configuration::instance()),
      m_activeScreen(nullptr),
      m_background(nullptr),
      m_monitorCount(0)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    if(QX11Info::isPlatformX11()){
        XRRQueryExtension(QX11Info::display(), &rr_event_base, &rr_error_base);
        XRRSelectInput(QX11Info::display(), QX11Info::appRootWindow(), RRScreenChangeNotifyMask);
    }
    QDesktopWidget *_desktop = QApplication::desktop();
    connect(_desktop, &QDesktopWidget::resized, this, &MainWindow::onScreenResized);
    connect(_desktop, &QDesktopWidget::screenCountChanged, this, &MainWindow::onScreenResized);
    //设置窗口大小
    qDebug()<<_desktop->geometry();
    setGeometry(_desktop->geometry());
    //设置监控鼠标移动
    setMouseTracking(true);
    //背景图片 优先级：用户桌面背景、背景图片、背景颜色
    m_defaultBackgroundPath = m_configuration->getDefaultBackgroundName();
    bool drawUserBackground = m_configuration->getValue("draw-user-background").toBool();
    if(drawUserBackground) {
        m_backgroundMode = DRAW_USER_BACKGROUND;
    } else {
        m_background = QSharedPointer<Background>(new Background);
        QString backgroundPath = m_configuration->getValue("background").toString();
        if(!backgroundPath.isEmpty()) {
            m_backgroundMode = DRAW_BACKGROUND;

            m_background->type = BACKGROUND_IMAGE;
            m_background->image = backgroundPath;
        } else {
            QString color = m_configuration->getValue("background-color").toString();
            if(!color.isEmpty()) {
                m_backgroundMode = DRAW_COLOR;

                m_background->type = BACKGROUND_COLOR;
                m_background->color = color;
            } else {
                m_backgroundMode = DRAW_DEFAULT;

                m_background->type = BACKGROUND_IMAGE;
                m_background->image = m_defaultBackgroundPath;
            }
        }
    }

    m_transition.started = false;

     qDebug()<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~``new greeterwindow";
    m_greeterWnd = new GreeterWindow(this);
#if !USEKSCREEN
    connect(m_greeterWnd, &GreeterWindow::currentUserChange, this, [&](QString strUserName) {
        qDebug()<<"GreeterWindow::currentUserChange:"<<strUserName;
        DisplayService::instance()->setCurUserName(strUserName);
        m_greeterWnd->setCurUserName(strUserName);
    });
#else
    connect(m_greeterWnd, &GreeterWindow::authenticationSucess, this, [&](QString userName) {
        qDebug()<<"GreeterWindow::authenticationSucess:"<<userName;
//        if (m_screenManager)
//            m_screenManager->resumeDisplayParam(userName);
    });
#endif
    future = QtConcurrent::run([=](){
        QString backgroundname = m_greeterWnd->guessBackground();
        QString resolution = QString("%1x%2").arg(QApplication::primaryScreen()->geometry().width()).arg(QApplication::primaryScreen()->geometry().height());
        QPair<QString, QString> key(backgroundname, resolution);
        if(!m_backgrounds.contains(key)){
            QPixmap *pixmap =  new QPixmap(scaledPixmap(QApplication::primaryScreen()->geometry().width(), QApplication::primaryScreen()->geometry().height(), backgroundname));
            m_backgrounds[key] = blurPixmap(pixmap);
        }
    });

    //登录界面移动到鼠标所在的屏幕上
    if(QApplication::screens().count() > 1){
        moveToPrimaryScreen();
        //多屏时，触屏和显示接口映射，由于无法智能，所有触摸设备选择主屏幕作为触屏映射
        mapTouchScreen();
    } else {
        moveToPrimaryScreen();

    }
    qDebug()<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~``initui";
    m_greeterWnd->initUI();
    qDebug()<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~``";
    QtConcurrent::run([=](){
        RRScreenChangeEvent(true);
    });
    qDebug()<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~``";
    qApp->installNativeEventFilter(this);
    installEventFilter(this);
    showLater();
}

//识别触摸屏设备，并获取第一个设备id
QStringList MainWindow::findTouchScreen(){
    QStringList list;
    int  ndevices = 0;
    Display *dpy = XOpenDisplay(NULL);
    XIDeviceInfo *info = XIQueryDevice(dpy, XIAllDevices, &ndevices);
    QString devicesid="";

    for (int i = 0; i < ndevices; i++)
    {
        XIDeviceInfo* dev = &info[i];
        // 判断当前设备是不是触摸屏
        if(dev->use != XISlavePointer) continue;
        if(!dev->enabled) continue;
        for (int j = 0; j < dev->num_classes; j++)
        {
            if (dev->classes[j]->type == XITouchClass)
            {
                list.append(QString::number(dev->deviceid));
            }
        }
    }

    XIFreeDeviceInfo(info);
    XCloseDisplay(dpy);

    return list;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::WindowDeactivate){
    
    }else if(event->type() == QEvent::WindowActivate){
    
    }else if(event->type() == QEvent::Show){
	/*分辨率改变时,窗口会调用一次hide，show，导致窗口失去焦点，因此需要在重新show出来时再设置一次*/
        QTimer::singleShot(200, [&]{
	    /*直接调用activateWindow概率性不起作用，必须设置一次输入焦点*/
            XSetInputFocus(QX11Info::display(),this->winId(),RevertToParent,CurrentTime);
    	    raise();
            activateWindow();
	});
    }
    return QWidget::eventFilter(obj, event);;
}

void MainWindow::mapTouchScreen()
{
    QString strScreenName = QApplication::primaryScreen()->name();
    QProcess enableMonitors;
    QStringList strDevIDList = findTouchScreen();
    for (const QString &strDevID : strDevIDList) {
        //设置映射关系  示例 xinput map-to-output 7 HDMI-1
        QString xinputCmd = "xinput map-to-output " + strDevID + "  " + strScreenName;
        qDebug() << "find touchscreen " << strDevID << " " << strScreenName;
        enableMonitors.start(xinputCmd);
        enableMonitors.waitForFinished(-1);
    }
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    for(QScreen *screen : QApplication::screens()){
        //在每个屏幕上绘制背景
        QRect rect = screen->geometry();


        if(m_transition.started)
            drawTransitionAlpha(rect);
        else {
            drawBackground(m_background, m_background, rect);
        }

        QPainter painter(this);
    }

    return QWidget::paintEvent(e);
}


/**
 * 根据鼠标指针移动位置移动Greeter窗口所在屏幕
 */
void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    return QWidget::mouseMoveEvent(e);
}

void MainWindow::showLater()
{
    if(!QX11Info::isPlatformX11()){
    	return ;
    }
    if(m_configuration->hasValue("numlock") && m_configuration->getValue("numlock").toBool()){
        unsigned int num_mask = XkbKeysymToModifiers (QX11Info::display(), XK_Num_Lock);
        XkbLockModifiers (QX11Info::display(), XkbUseCoreKbd, num_mask, 0);
        XkbLockModifiers (QX11Info::display(), XkbUseCoreKbd, num_mask, num_mask);
    }else if(m_configuration->hasValue("numlock") && !m_configuration->getValue("numlock").toBool()){
        unsigned int num_mask = XkbKeysymToModifiers (QX11Info::display(), XK_Num_Lock);
        XkbLockModifiers (QX11Info::display(), XkbUseCoreKbd, num_mask, num_mask);
        XkbLockModifiers (QX11Info::display(), XkbUseCoreKbd, num_mask, 0);
    }
    else if(checkNumLockState()){
        unsigned int num_mask = XkbKeysymToModifiers (QX11Info::display(), XK_Num_Lock);
        XkbLockModifiers (QX11Info::display(), XkbUseCoreKbd, num_mask, 0);
        XkbLockModifiers (QX11Info::display(), XkbUseCoreKbd, num_mask, num_mask);
    }else{
        unsigned int num_mask = XkbKeysymToModifiers (QX11Info::display(), XK_Num_Lock);
        XkbLockModifiers (QX11Info::display(), XkbUseCoreKbd, num_mask, num_mask);
        XkbLockModifiers (QX11Info::display(), XkbUseCoreKbd, num_mask, 0);
    }

    if(checkCapsState()){
        unsigned int caps_mask = XkbKeysymToModifiers (QX11Info::display(), XK_Caps_Lock);
        XkbLockModifiers (QX11Info::display(), XkbUseCoreKbd, caps_mask, 0);
        XkbLockModifiers (QX11Info::display(), XkbUseCoreKbd, caps_mask, caps_mask);
    }else{
        unsigned int caps_mask = XkbKeysymToModifiers (QX11Info::display(), XK_Caps_Lock);
        XkbLockModifiers (QX11Info::display(), XkbUseCoreKbd, caps_mask, caps_mask);
        XkbLockModifiers (QX11Info::display(), XkbUseCoreKbd, caps_mask, 0);
    }

    XDefineCursor(QX11Info::display(), QX11Info::appRootWindow(), XCreateFontCursor(QX11Info::display(), XC_arrow));

    bindtextdomain("Linux-PAM","/usr/share/locale");
    textdomain("Linux-PAM");
    
    XEventMonitor::instance()->start();
}

void MainWindow::RRScreenChangeEvent(bool isFirst)
{
    if(!QX11Info::isPlatformX11())
        return;
    XRRScreenResources *screen;
    screen = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
    XRROutputInfo *info;
    QList<QString> listMonitors;

    for (int i = 0; i < screen->noutput; i++) {
        info = XRRGetOutputInfo(QX11Info::display(), screen, screen->outputs[i]);
        if (info->connection == RR_Connected) {
            listMonitors.push_back(info->name);
        }
        XRRFreeOutputInfo(info);
    }

    qDebug()<<"monitors = "<<listMonitors;
    if (isFirst && DisplayService::instance()->isSaveParamInUsed()) {
        m_listMonitors = listMonitors;
    } else {
        onScreensChanged(listMonitors);
    }
    XRRFreeScreenResources(screen);
}

bool MainWindow::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);
    if (qstrcmp(eventType, "xcb_generic_event_t") != 0) {
        return false;
    }
    xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t*>(message);
    const uint8_t responseType = event->response_type & ~0x80;
#if !USEKSCREEN
    if(responseType == rr_event_base + RRScreenChangeNotify){
        RRScreenChangeEvent(false);
    }
#endif
    return false;
}

/**
 * 有屏幕分辨率发生改变,移动Greeter窗口位置
 */
void MainWindow::onScreenResized()
{
    hide();
    QDesktopWidget *desktop = QApplication::desktop();
    setGeometry(desktop->geometry());

    qDebug() << "screen resize to " << geometry();

    moveToPrimaryScreen();
    show();
    m_greeterWnd->setRootWindow();
    activateWindow();
    setFocus();
    m_greeterWnd->setFocus();

}

void MainWindow::screenCountEvent()
{
	
    QDesktopWidget *desktop = QApplication::desktop();
    setGeometry(desktop->geometry());
    moveToPrimaryScreen();
    //需要重新绘制，否则背景图片大小会不正确
    repaint();
    activateWindow();
    setFocus();
    m_greeterWnd->setFocus();
}

/**
 * 有屏幕插拔,移动GreeterWindow到主屏幕
 */
void MainWindow::onScreenCountChanged(int newCount)
{
    qDebug()<<"newCount = "<<newCount<<" m_monitorCount = "<<m_monitorCount;
    if(newCount == m_monitorCount){
	if(newCount >= 2) {
            mapTouchScreen();
        }

        return;
    }
    
    m_monitorCount = newCount;
    if(newCount < 2) {
#if !USEKSCREEN
        //默认设置显示最大分辨率
        DisplayService::instance()->setOneDisplayMode();
#endif
    } else {
#if !USEKSCREEN
        int mode = m_configuration->getValue("display-mode").toInt();
        DisplayService::instance()->switchDisplayMode((DisplayMode)mode);
#endif
        //多屏时，触屏和显示接口映射，由于无法智能，所有触摸设备选择主屏幕作为触屏映射
        mapTouchScreen();
    }
    //在调用xrandr打开显示器以后，不能马上设置窗口大小，会设置不正确的
    //分辨率，延时1000ms正常。
    QTimer::singleShot(1000,this,SLOT(screenCountEvent()));
}

void MainWindow::onScreensChanged(QList<QString> listMonitors)
{
    qDebug()<<"newList = "<<listMonitors<<" listMonitors = "<<m_listMonitors << DisplayService::instance()->isJJW7200();
    if(DisplayService::instance()->isJJW7200() == 0 && m_listMonitors.size() == listMonitors.size()) {
        bool isAllFound = true;
        for (auto monitor : listMonitors) {
            if (!m_listMonitors.contains(monitor)) {
                isAllFound = false;
                break;
            }
        }
        if (isAllFound) {
            if(m_listMonitors.size() >= 2) {
                mapTouchScreen();
	    }
   	    return;
        }
    }

    m_listMonitors = listMonitors;
    if(m_listMonitors.size() < 2) {
#if !USEKSCREEN
        //默认设置显示最大分辨率
        DisplayService::instance()->setOneDisplayMode();
#endif
    } else {
#if !USEKSCREEN
        int mode = m_configuration->getValue("display-mode").toInt();
        DisplayService::instance()->switchDisplayMode((DisplayMode)mode);
#endif
        //多屏时，触屏和显示接口映射，由于无法智能，所有触摸设备选择主屏幕作为触屏映射
        mapTouchScreen();
    }
    //在调用xrandr打开显示器以后，不能马上设置窗口大小，会设置不正确的
    //分辨率，延时1000ms正常。
    QTimer::singleShot(1000,this,SLOT(screenCountEvent()));
}

/**
 * 移动Greeter窗口到screen屏幕上
 */
void MainWindow::moveToScreen(QScreen *screen)
{
    m_activeScreen = screen;
    QRect activeScreenRect = m_activeScreen->geometry();

    qDebug() << "moveToScreen activeScreenRect " << activeScreenRect;
    m_greeterWnd->hide();
    if(QApplication::desktop()->screenCount() == 1)
        m_greeterWnd->setGeometry(QApplication::primaryScreen()->geometry());
    else
        m_greeterWnd->setGeometry(activeScreenRect);
    m_greeterWnd->show();

   // Q_EMIT activeScreenChanged(activeScreenRect);
 
    update();
    activateWindow();
    setFocus();
    m_greeterWnd->setFocus();
}

void MainWindow::moveToPrimaryScreen()
{
    bool chkInOneScreen = false;
    for (auto screen : QGuiApplication::screens()) {
        if(screen == qApp->primaryScreen()){
            qInfo()<<"greeterWnd:"<<screen->geometry()<<m_greeterWnd->geometry();
            moveToScreen(screen);
            chkInOneScreen = true;
            break;
        }
    }
    if (!chkInOneScreen) {
        for(auto screen : QGuiApplication::screens()) {
            if(!screen){
                qInfo()<<"greeterWnd:"<<screen->geometry()<<m_greeterWnd->geometry()<<","<<screen;
                moveToScreen(screen);
                chkInOneScreen = true;
                break;
            }
        }
    }
}

void MainWindow::setBackground(QSharedPointer<Background> &background)
{
    if(background)
        qDebug() << background->image;

    if(m_backgroundMode != DRAW_USER_BACKGROUND)
        return;

    stopTransition();

    if(background) {
        if(background->image.isEmpty())
            background->image = m_defaultBackgroundPath;
    }

    if(m_background && background &&
       m_background->image == background->image){
        repaint();
        return;
    }

    //如果是第一次绘制背景，则不需要渐变
    //现在由于绘制模糊背景，不需要渐变了
    if(!m_background.isNull()){
        startTransition(m_background, background);
    }

    m_background = background;

}

void MainWindow::startTransition(QSharedPointer<Background> &from,
                                 QSharedPointer<Background> &to)
{
    if(!m_timer){
         m_timer = new QTimer(this);
         connect(m_timer, &QTimer::timeout, this, &MainWindow::onTransition);
    }
    stopTransition();

    m_transition.from = from;
    m_transition.to = to;
    m_transition.stage = 0.0;
    m_transition.started = true;

    m_timer->start(10);
}

void MainWindow::stopTransition()
{
    if(m_timer && m_timer->isActive())
        m_timer->stop();
    m_transition.stage = 1.0;
    m_transition.started = false;
}

void MainWindow::onTransition()
{
    m_transition.stage += 0.05;//= (1 - cos(M_PI * m_transition.stage)) / 2;

    if(m_transition.stage >= 1.0)
        stopTransition();

    //repaint();
    update();
}

void MainWindow::drawTransitionAlpha(const QRect &rect)
{
    //drawBackground(m_transition.from, rect, 1.0 - m_transition.stage);
    drawBackground(m_transition.to, m_transition.from, rect, m_transition.stage);
}

void MainWindow::drawBackground(QSharedPointer<Background> &backgroundBack, QSharedPointer<Background> &backgroundFront,
                                const QRect &rect, float alpha)
{
    if(backgroundBack == nullptr)
        return;

    QPainter painter(this);

    switch(backgroundBack->type) {
    case BACKGROUND_IMAGE:
    {
        QPixmap *pixmapFront = getBackground(backgroundFront->image, rect);
        QPixmap *pixmapBack = getBackground(backgroundBack->image, rect);

        if(pixmapFront->isNull())
        {
            painter.setOpacity(alpha);
            QString color = m_configuration->getValue("background-color").toString();
            QColor cor;
            if(!color.isEmpty())
                cor = color;
            else
                cor = "#035290";
            painter.setBrush(cor);
            painter.drawRect(rect);
        }
        else
        {
            painter.drawPixmap(rect, *pixmapFront);
            painter.setOpacity(alpha);
            painter.drawPixmap(rect, *pixmapBack);
            QPainterPath path;
            QPainter painter1(this);
            painter1.setOpacity(0.25);
            painter1.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
            painter1.setClipping(true);
            painter1.setPen(Qt::transparent);
            path.addRect(this->rect());
            path.setFillRule(Qt::WindingFill);
            painter1.setBrush(QColor("#000000"));
            painter1.setPen(Qt::transparent);
            painter1.drawPath(path);
        }
        break;
    }
    case BACKGROUND_COLOR:
    {
        painter.setBrush(backgroundBack->color);
	painter.setOpacity(alpha);
	painter.setBrush(backgroundFront->color);
        painter.drawRect(rect);
        break;
    }
    }
}

QPixmap * MainWindow::getBackground(const QString &path, const QRect &rect)
{
    QString resolution = QString("%1x%2").arg(rect.width()).arg(rect.height());
    QPair<QString, QString> key(path, resolution);

    if(m_backgrounds.isEmpty()&&future.isRunning()){
            future.waitForFinished();
    }else{
        if(!future.isFinished() && future.isStarted()){
            future.waitForFinished();
        }
    }
    if(!m_backgrounds.contains(key)){
        QPixmap *pixmap =  new QPixmap(scaleBlurPixmap(width(), height(), path));
        m_backgrounds[key] = pixmap;
    }
    return m_backgrounds[key];
}
