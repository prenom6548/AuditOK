// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QApplication>
#include <QDesktopWidget>
#include <QHBoxLayout>
#include <QScreen>

#include "com_kylin_Wlcom_Effect.h"
#include "magnifier.h"
#include "org_ukui_Magnifier.h"

#include <ukuiwindowhelper.h>

#define GSETTINGS_THEME_NAME "org.ukui.style"
#define GSETTINGS_THEME_KEY "styleName"

static void registerMetaTypes()
{
    qRegisterMetaType<WlcomData::DBusEffectData>("WlcomData::DBusEffectData");
    qRegisterMetaType<WlcomData::DBusEffectDataList>("WlcomData::DBusEffectDataList");
    qDBusRegisterMetaType<WlcomData::DBusEffectData>();
    qDBusRegisterMetaType<WlcomData::DBusEffectDataList>();
}

Q_COREAPP_STARTUP_FUNCTION(registerMetaTypes)

Magnifier::Magnifier(int type, QWidget *parent) : QWidget(parent), m_type(type)
{
    setupUI();
    setupConnection();
    enableZoom();
    on_themeChanged();

    ukui_magnifier = new OrgUkuiMagnifierInterface("org.ukui.KWin", "/Magnifier",
                                                   QDBusConnection::sessionBus(), this);
    if (ukui_magnifier->isValid()) {
        connect(ukui_magnifier, &OrgUkuiMagnifierInterface::scaleChanged, this,
                &Magnifier::on_scaleChanged);
        connect(ukui_magnifier, &OrgUkuiMagnifierInterface::zoomStop, this,
                &Magnifier::on_zoomStop);

        on_scaleChanged(ukui_magnifier->GetScale());
    } else {
        qDebug() << "start org.ukui.Magnifier service failed";
    }

    UkuiWindowHelper *windowHelper = new UkuiWindowHelper(this);
    windowHelper->setSkipSwitcher(true);
}

Magnifier::~Magnifier()
{
    if (ukui_magnifier && ukui_magnifier->isValid()) {
        ukui_magnifier->StopZoom();
    }
}

void Magnifier::on_zoomIn_clicked()
{
    if (!ukui_magnifier || !ukui_magnifier->isValid()) {
        qDebug() << "ZoomIn failed: DBus interface not available";
        return;
    }

    if (!ukui_magnifier->ZoomIn()) {
        qDebug() << "ZoomIn failed !";
    }
}

void Magnifier::on_zoomOut_clicked()
{
    if (!ukui_magnifier || !ukui_magnifier->isValid()) {
        qDebug() << "ZoomIn failed: DBus interface not available";
        return;
    }

    if (!ukui_magnifier->ZoomOut()) {
        qDebug() << "ZoomOut failed !";
    }
}

void Magnifier::on_scaleChanged(uint scale)
{
    if (scale >= 16.0) {
        button_zoomIn->setEnabled(false);
    } else if (scale <= 1.0) {
        button_zoomOut->setEnabled(false);
        scale = 1;
    } else {
        button_zoomOut->setEnabled(true);
        button_zoomIn->setEnabled(true);
    }
    scale_label->setText(QString("%1%").arg(scale * 100));
}

void Magnifier::on_zoomStop()
{
    close();
}

void Magnifier::on_themeChanged()
{
    bool isDark = false;
    if (m_settingsTheme) {
        QString themeName = m_settingsTheme->get(GSETTINGS_THEME_KEY).toString();
        isDark = themeName.contains("dark", Qt::CaseInsensitive) ||
                 themeName.contains("black", Qt::CaseInsensitive);
    }

    m_bgColor = isDark ? QColor(18, 18, 18) : QColor(255, 255, 255);

    if (!flag_zoomIn) {
        QString in_svg = isDark ? "in-white.svg" : "in-black.svg";
        button_zoomIn->setStyleSheet("QPushButton {"
                                     "   background: transparent;"
                                     "   background-image:url(:/" +
                                     in_svg +
                                     ");"
                                     "   background-repeat: no-repeat;"
                                     "   background-position: center;"
                                     "   icon-size: 24px;"
                                     "}"
                                     "QPushButton:hover {"
                                     "   background-image:url(:/in-hover.svg);"
                                     "   icon-size: 24px;"
                                     "}"
                                     "QPushButton:pressed {"
                                     "   background-image:url(:/in-press.svg);"
                                     "   icon-size: 24px;"
                                     "}"
                                     "QPushButton:disabled {"
                                     "   background-image:url(:/in-hide.svg);"
                                     "   icon-size: 24px;"
                                     "}");
    }
    if (!flag_zoomOut) {
        QString out_svg = isDark ? "out-white.svg" : "out-black.svg";

        button_zoomOut->setStyleSheet("QPushButton {"
                                      "   background: transparent;"
                                      "   background-image:url(:/" +
                                      out_svg +
                                      ");"
                                      "   background-repeat: no-repeat;"
                                      "   background-position: center;"
                                      "   icon-size: 24px;"
                                      "}"
                                      "QPushButton:hover {"
                                      "   background-image:url(:/out-hover.svg);"
                                      "   icon-size: 24px;"
                                      "}"
                                      "QPushButton:pressed {"
                                      "   background-image:url(:/out-press.svg);"
                                      "   icon-size: 24px;"
                                      "}"
                                      "QPushButton:disabled {"
                                      "   background-image:url(:/out-hide.svg);"
                                      "   icon-size: 24px;"
                                      "}");
    }

    scale_label->setStyleSheet(isDark ? "color: rgb(250, 242, 242);" : "color: rgb(17, 16, 16);");

    repaint();
}

void Magnifier::setupUI()
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window);
    setAttribute(Qt::WA_AlwaysStackOnTop);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_AlwaysShowToolTips);
    setAttribute(Qt::WA_QuitOnClose);

    QPoint cursorPos = QCursor::pos();
    QScreen *currentScreen = QGuiApplication::screenAt(cursorPos);
    if (!currentScreen) {
        currentScreen = QGuiApplication::primaryScreen();
    }

    if (currentScreen) {
        QRect screenGeometry = currentScreen->availableGeometry();
        move(screenGeometry.center() - rect().center());
    } else {
        move(50, 20);
    }
    setFixedSize(250, 100);

    setWindowTitle(tr("Magnifier"));
    // 创建内容区域
    button_zoomIn = new QPushButton(this);
    button_zoomIn->setFixedSize(38, 38);
    button_zoomIn->setIconSize(QSize(24, 24));
    QIcon zoomIn = QIcon::fromTheme("zoom-in-symbolic");
    if (!zoomIn.isNull()) {
        button_zoomIn->setIcon(zoomIn);
        flag_zoomIn = true;
    }
    button_zoomIn->setFlat(true);
    button_zoomIn->setToolTip(tr("Zoom In"));

    button_zoomOut = new QPushButton(this);
    button_zoomOut->setFixedSize(38, 38);
    button_zoomOut->setIconSize(QSize(24, 24));
    QIcon zoomOut = QIcon::fromTheme("zoom-out-symbolic");
    if (!zoomOut.isNull()) {
        button_zoomOut->setIcon(zoomOut);
        flag_zoomOut = true;
    }
    button_zoomOut->setFlat(true);
    button_zoomOut->setToolTip(tr("Zoom Out"));

    scale_label = new QLabel("100%", this);
    scale_label->setAlignment(Qt::AlignCenter);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(30, 0, 30, 0);
    layout->addWidget(button_zoomOut);
    layout->addWidget(scale_label);
    layout->addWidget(button_zoomIn);
}

void Magnifier::setupConnection()
{
    connect(button_zoomIn, &QPushButton::clicked, this, &Magnifier::on_zoomIn_clicked);
    connect(button_zoomOut, &QPushButton::clicked, this, &Magnifier::on_zoomOut_clicked);

    if (QGSettings::isSchemaInstalled(GSETTINGS_THEME_NAME)) {
        m_settingsTheme = new QGSettings(GSETTINGS_THEME_NAME);
        connect(m_settingsTheme, &QGSettings::changed, this, &Magnifier::on_themeChanged);
    }
}

void Magnifier::enableZoom()
{
    static bool metaTypesRegistered = false;
    if (!metaTypesRegistered) {
        qDBusRegisterMetaType<WlcomData::DBusEffectData>();
        qDBusRegisterMetaType<WlcomData::DBusEffectDataList>();
        metaTypesRegistered = true;
    }

    wlcom_effect = new ComKylinWlcomEffectInterface("com.kylin.Wlcom", "/com/kylin/Wlcom/Effect",
                                                    QDBusConnection::sessionBus(), this);
    if (!wlcom_effect->isValid()) {
        qDebug() << "start com.kylin.Wlcom.Effect service failed";
        return;
    }

    QDBusPendingReply<WlcomData::DBusEffectDataList> reply = wlcom_effect->ListAllEffects();
    reply.waitForFinished();
    if (reply.isError()) {
        qDebug() << "Error:" << reply.error().message();
        return;
    } else {
        bool enabled_zoom = true;
        for (const WlcomData::DBusEffectData &effect : reply.value()) {
            if (effect.name == "zoom" && effect.enable == false) {
                enabled_zoom = false;
                break;
            }
        }
        if (!enabled_zoom) {
            wlcom_effect->EnableEffect("zoom", true);
        }
    }
    qDebug() << "Effect Zoom is enabled.";
}

int Magnifier::checkZoomIsStarted()
{
    if (ukui_magnifier && ukui_magnifier->isValid() && !ukui_magnifier->IsZooming()) {
        qDebug() << "Effect Zoom not started !";
        return 0;
    }
    return 1;
}

void Magnifier::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(rect(), m_bgColor);
    QWidget::paintEvent(event);
}

void Magnifier::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    if (!checkZoomIsStarted()) {
        QTimer::singleShot(100, this, &Magnifier::close);
    }
}
