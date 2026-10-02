/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *               2014  Hong Jen Yee (PCMan) <pcman.tw@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#include "abnormalwindow.h"
#include <QColor>
#include <QDesktopWidget>
#include <QApplication>
#include <QPainter>
#include <QScreen>
#include <QDebug>
#include <QTimer>
#include <QPaintEvent>
#include <QDBusInterface>
#include <QFileInfo>
#include <QIcon>
#include <QGSettings/QGSettings>
#include <QMessageBox>
#include <QPushButton>
#include <syslog.h>
#include "kwindowsystem.h"

extern void
qt_blurImage(QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0);

QT_END_NAMESPACE

QPixmap blurPixmap(QPixmap pixmap)
{
    QPainter painter(&pixmap);
    QImage img = pixmap.toImage();
    double radius = 300;
    int height = img.height();
    int width = img.width();
    int smallerSide = height > width ? width : height;
    if (smallerSide / 2 > 300) {
        radius = 300;
    } else {
        radius = smallerSide / 3;
    }

    qt_blurImage(&painter, img, radius, false, false);

    for (int y = 0; y < img.height(); ++y) {
        QRgb *row = (QRgb *)img.scanLine(y);
        for (int x = 0; x < img.width(); ++x) {
            ((unsigned char *)&row[x])[3] = 255;
        }
    }
    return QPixmap::fromImage(img);
}
AbnormalWindow::AbnormalWindow(QWidget *parent) : QWidget(parent)
{
    if (!m_windowHelper) {
        m_windowHelper = new UkuiWindowHelper(this);
        m_windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::Logout);
    }

    //  翻译如果放在paintevent中，就会读取不到翻译文件，原因未知
    m_tipText = (QObject::tr(
        "The lock screen program has been corrupted, please press the power button to reboot the system"));
    m_feedbackText = (QObject::tr("For technical support, please call 400-089-1870 or scan the QR code below"));

    /* QGSettings for screensaver */
    saverGsettings = new QGSettings("org.ukui.screensaver", "", this);
    connect(saverGsettings, &QGSettings::changed, this, [this](QString key) {
        if (key == "background") {
            m_background = saverGsettings->get("background").toString();
            initBackgroundPix();
        }
    });
    QStringList keysScreenSaver = saverGsettings->keys();
    if (keysScreenSaver.contains("background")) {
        m_background = saverGsettings->get("background").toString();
    }
    initBackgroundPix();

    installEventFilter(this);

    QDesktopWidget *desktop = QApplication::desktop();
    if (desktop) {
        connect(desktop, &QDesktopWidget::resized, this, &AbnormalWindow::onDesktopResized);
    }
}

void AbnormalWindow::initBackgroundPix()
{
    QFileInfo fileimage(m_background);
    if (fileimage.isFile() && fileimage.exists() && fileimage.isReadable() && m_backgroundPix.load(m_background)) {
        m_backgroundPix = blurPixmap(m_backgroundPix);
    } else {
        m_background = "/usr/share/backgrounds/1-warty-final-ubuntukylin.jpg";
        QFileInfo deffileimage(m_background);
        if (deffileimage.isFile() && deffileimage.exists() && deffileimage.isReadable()
            && m_backgroundPix.load(m_background)) {
            m_backgroundPix = blurPixmap(m_backgroundPix);
        }
    }
}

void AbnormalWindow::hideAbnormalWindow()
{
    this->hide();
}

void AbnormalWindow::showAbnormalWindow()
{
    if (!this->isVisible()) {
        m_isEmergency = false;
        update();
        show();
        syslog(LOG_INFO, "[ukui-session-abnormal] set current user!!");
    }
}

void AbnormalWindow::showEmergencyWindow()
{
    m_isEmergency = true;
    blockSomeShortCut(true);
    update();
    show();
}

void AbnormalWindow::blockSomeShortCut(bool isBlock /* = false*/)
{
    static bool sIsBlocked = false;
    if (sIsBlocked == isBlock) {
        return;
    }

    qDebug() << " blockSomeShortCut  isBlock: " << isBlock;

    sIsBlocked = isBlock;
    QDBusInterface kwinInterface(
        "org.kde.kglobalaccel", "/kglobalaccel", "org.kde.KGlobalAccel", QDBusConnection::sessionBus());
    // kwin shortcut
    if (kwinInterface.isValid()) {
        QDBusMessage blockMessage = kwinInterface.call("blockGlobalShortcuts", isBlock);
        qDebug() << " kwin blockAllShortcut !!! ";
        if (blockMessage.type() == QDBusMessage::ErrorMessage) {
            qWarning() << "Kwin dbus error:" << blockMessage.errorMessage();
        }
    }
    if (sIsBlocked) {
        // usd shortcut
        if (!m_usdBlockShortCut) {
            m_usdBlockShortCut = new USDBlockShortCut(this);
            qDebug() << " usd blockAllShortcut !!! ";
        }
    } else {
        // usd shortcut
        if (m_usdBlockShortCut) {
            delete m_usdBlockShortCut;
            m_usdBlockShortCut = nullptr;
        }
    }
}

void AbnormalWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
    if (!m_backgroundPix.isNull()) {
        painter.setPen(Qt::transparent);
        painter.setBrush(QColor(0, 0, 0, 78)); // 178
        for (QScreen *screen : QApplication::screens()) {
            QRect rect = screen->geometry();
            painter.drawPixmap(rect, m_backgroundPix.scaled(screen->size()));
            painter.drawRect(rect);
        }
    } else {
        qDebug() << "The background does not exist!!!!!!";
        painter.fillRect(0, 0, width(), height(), Qt::black);
    }
    if (m_isEmergency) {
        QPixmap errorPixmap("/usr/share/ukui/ukui-session-manager/images/dialog-error.png");
        QPixmap qrcodePixmap("/usr/share/ukui/ukui-session-manager/images/qrcode.png");
        QPixmap scaledPixmap = qrcodePixmap.scaled(120, 120);

        painter.setPen(Qt::white);
        QFont f = painter.font();
        f.setBold(true);
        f.setPointSize(24);
        painter.setFont(f);

        QRect tipTextRect = painter.boundingRect(QRect(), Qt::AlignHCenter, m_tipText);

        f.setPointSize(14);
        painter.setFont(f);

        QRect feedbackTextRect = painter.boundingRect(QRect(), Qt::AlignHCenter, m_feedbackText);

        for (auto s : QGuiApplication::screens()) {
            int errorImgX = (s->geometry().width() - errorPixmap.width()) / 2;
            int errorImgY = (s->geometry().height()
                             - (errorPixmap.height() + 40 + tipTextRect.height() + 40 + feedbackTextRect.height() + 16
                                + scaledPixmap.height()))
                            / 2;
            painter.drawPixmap(errorImgX, errorImgY, errorPixmap);

            f.setPointSize(24);
            painter.setFont(f);
            int tipTextX = s->geometry().width() / 2 - tipTextRect.width() / 2;
            int tipTextY = errorImgY + errorPixmap.height() + 40;
            painter.drawText(tipTextX, tipTextY, m_tipText);

            f.setPointSize(14);
            painter.setFont(f);
            int feedbackTextX = s->geometry().width() / 2 - feedbackTextRect.width() / 2;
            int feedbackTextY = tipTextY + tipTextRect.height() + 40;
            painter.drawText(feedbackTextX, feedbackTextY, m_feedbackText);

            int qrcodeImgX = (s->geometry().width() - scaledPixmap.width()) / 2;
            int qrcodeImgY = feedbackTextY + feedbackTextRect.height() + 16;
            painter.drawPixmap(qrcodeImgX, qrcodeImgY, scaledPixmap);
        }
    }
    return QWidget::paintEvent(event);
}

void AbnormalWindow::onDesktopResized()
{
    QDesktopWidget *desktop = QApplication::desktop();
    if (!desktop) {
        qWarning() << " get desktop size failed";
        return;
    }
    setGeometry(desktop->geometry());
}
