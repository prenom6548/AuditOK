/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "screensaverwidget.h"
#include "commonfunc.h"
#include <unistd.h>
#include <signal.h>
#include <QDebug>
#include <QTimer>
#include <QProcess>
#include <QPainter>
#include <QKeyEvent>
#include <QtX11Extras/QX11Info>
#include <QPluginLoader>
#include <X11/Xlib.h>
#include <sys/prctl.h>

QSharedPointer<ScreensaverPlugin> ScreenSaverWidget::m_ssPluginPtr = nullptr;
ScreenSaverWidget::ScreenSaverWidget(ScreenSaver *screensaver, QPixmap *background, QWidget *parent)
    : QWidget(parent), timer(nullptr), screensaver(screensaver), closing(false), m_background(background)
{
    qDebug() << "ScreenSaverWidget::ScreenSaverWidget";
    // setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);
    setFocus();
    this->installEventFilter(this);
    if (screensaver) {
        screensaver->setParent(this);
    }

    QPalette pal(palette());
    pal.setColor(QPalette::Background, Qt::black); // 设置背景黑色
    setAutoFillBackground(true);
    setPalette(pal);

    switch (screensaver->mode) {
        case SAVER_RANDOM:
        case SAVER_SINGLE:
            embedXScreensaver(screensaver->path);
            break;
        case SAVER_BLANK_ONLY:
            break;
        case SAVER_IMAGE: {
            setAutoFillBackground(true);
            screensaver->startSwitchImages();

            QPalette plt;
            plt.setBrush(QPalette::Background, Qt::black);
            setPalette(plt);

            connect(screensaver, &ScreenSaver::imagePathChanged, this, &ScreenSaverWidget::onBackgroundChanged);
            break;
        }
        case SAVER_DEFAULT:
            embedXScreensaver(screensaver->path);
            break;
        case SAVER_DEFAULT_CUSTOM:
            embedXScreensaver(screensaver->path);
            break;
        default:
            break;
    }

    show();
}

ScreenSaverWidget::~ScreenSaverWidget() {}

void ScreenSaverWidget::closeEvent(QCloseEvent *event)
{
    qDebug() << "ScreenSaverWidget::closeEvent---beginStop";
    if (process.state() != QProcess::NotRunning) {
        process.kill();
        process.waitForFinished(200);
    }

    if (!closing) {
        closing = true;
        screensaver->stopSwitchImages();
        delete screensaver;
        if (timer && timer->isActive())
            timer->stop();
    }
    qDebug() << "ScreenSaverWidget::closeEvent---endStop";
    return QWidget::closeEvent(event);
}

void ScreenSaverWidget::paintEvent(QPaintEvent *event)
{
    if (!screensaver->exists()) {
        QPainter painter(this);
        painter.fillRect(0, 0, this->width(), this->height(), Qt::black);
    }
    if (screensaver->mode == SAVER_IMAGE) {
        switch (screensaver->effect) {
            case TRANSITION_NONE: {
                const QPixmap pixmap(screensaver->path);
                QPainter painter(this);
                drawPixmapWallpaperFill(painter, pixmap, rect());
                break;
            }
            case TRANSITION_FADE_IN_OUT: {
                QPainter painter(this);
                const QPixmap pixmap1(screensaver->lastPath);
                painter.setOpacity(opacity);
                drawPixmapWallpaperFill(painter, pixmap1, rect());

                const QPixmap pixmap(screensaver->path);
                painter.setOpacity(1 - opacity);
                drawPixmapWallpaperFill(painter, pixmap, rect());
                break;
            }
        }
    }
    return QWidget::paintEvent(event);
}

bool ScreenSaverWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == 23) {
        if (QX11Info::isPlatformX11()) {
            XSetInputFocus(QX11Info::display(), this->winId(), RevertToParent, CurrentTime);
        }
    }
    return false;
}

/* Embed xscreensavers */
void ScreenSaverWidget::embedXScreensaver(const QString &path)
{
    qDebug() << "embedXScreensaver path = " << path;

    /*屏保模式为ukui或者自定义时，直接加载对象，为其他屏保时，使用调用命令方式实现*/
    if (screensaver->mode == SAVER_SINGLE) {
        QString cmd = path + " -window-id " + QString::number(winId());
        if (process.state() == QProcess::NotRunning)
            process.start(cmd);
    } else if (screensaver->mode == SAVER_DEFAULT || screensaver->mode == SAVER_DEFAULT_CUSTOM) {
        initScreenSaverPlugin();
    }
}

void ScreenSaverWidget::initScreenSaverPlugin()
{
    if (!m_ssPluginPtr) {
        QPluginLoader pluginLoader("/usr/lib/ukui-screensaver/libscreensaver-default.so");
        pluginLoader.load();
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            m_ssPluginPtr.reset(qobject_cast<ScreensaverPlugin *>(plugin));
        } else {
            qWarning() << "pluginLoader '/usr/lib/ukui-screensaver/libscreensaver-default.so' failed";
        }
    }
    if (m_ssPluginPtr && !m_wdgScreenSaver) {
        m_wdgScreenSaver = m_ssPluginPtr->createWidget(true, m_background, this);
        m_wdgScreenSaver->show();
    }
}

void ScreenSaverWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if (m_wdgScreenSaver) {
        m_wdgScreenSaver->setGeometry(0, 0, this->width(), this->height());
    }
}

void ScreenSaverWidget::onBackgroundChanged(const QString & /*path*/)
{
    switch (screensaver->effect) {
        case TRANSITION_NONE:
            repaint();
            break;
        case TRANSITION_FADE_IN_OUT:
            opacity = 1.0;
            timer = new QTimer(this);
            connect(timer, &QTimer::timeout, this, [&] {
                opacity -= 0.1;
                if (opacity <= 0)
                    timer->stop();
                else
                    repaint();
            });
            timer->start(50);
            break;
    }
}
