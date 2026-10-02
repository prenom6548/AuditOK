// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QApplication>
#include <QDesktopWidget>
#include <QFile>
#include <QHBoxLayout>
#include <QScreen>

#include "com_kylin_Wlcom_Effect.h"
#include "magnifier.h"
#include "org_ukui_Magnifier.h"
#if KYSDK_IS_SUPPORT
#include <ukuiwindowhelper.h>
#endif

#define GSETTINGS_THEME_NAME "org.ukui.style"
#define GSETTINGS_THEME_KEY "style-name"
#define GSETTINGS_HIGHLIGHT_KEY "theme-color"

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

#if KYSDK_IS_SUPPORT
    UkuiWindowHelper *windowHelper = new UkuiWindowHelper(this);
    windowHelper->setSkipSwitcher(true);
#endif
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
        qDebug() << "ZoomOut failed: DBus interface not available";
        return;
    }

    if (!ukui_magnifier->ZoomOut()) {
        qDebug() << "ZoomOut failed !";
    }
}

void Magnifier::on_scaleChanged(uint scale)
{
    if (scale >= 16.0) {
        button_zoomIn->setIcon(m_zoomInDisabled);
        button_zoomIn->setEnabled(false);
    } else if (scale <= 1.0) {
        button_zoomOut->setIcon(m_zoomOutDisabled);
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
    QString highlightColor = "#33aaff";
    if (m_settingsTheme) {
        QString themeName = m_settingsTheme->get(GSETTINGS_THEME_KEY).toString();
        isDark = themeName.contains("dark", Qt::CaseInsensitive) ||
                 themeName.contains("black", Qt::CaseInsensitive);
        QString highLight = m_settingsTheme->get(GSETTINGS_HIGHLIGHT_KEY).toString();
        QStringList list = highLight.split(",");
        if (list.size() >= 3) {
            int r = list[0].trimmed().toInt();
            int g = list[1].trimmed().toInt();
            int b = list[2].trimmed().toInt();
            highlightColor = QString::asprintf("#%02X%02X%02X", r, g, b);
        }
    }

    m_bgColor = isDark ? QColor(18, 18, 18) : QColor(255, 255, 255);
    QString iconColor = isDark ? "#f5f5f5" : "#191818";
    QString disabledColor = "#666363";

    m_zoomInNormal = loadSvgWithColor(":/zoom-in.svg", "#f5f5f5", iconColor);
    m_zoomInHover = loadSvgWithColor(":/zoom-in.svg", "#f5f5f5", highlightColor);
    m_zoomInDisabled = loadSvgWithColor(":/zoom-in.svg", "#f5f5f5", disabledColor);

    m_zoomOutNormal = loadSvgWithColor(":/zoom-out.svg", "#f5f5f5", iconColor);
    m_zoomOutHover = loadSvgWithColor(":/zoom-out.svg", "#f5f5f5", highlightColor);
    m_zoomOutDisabled = loadSvgWithColor(":/zoom-out.svg", "#f5f5f5", disabledColor);

    QIcon inIcon;
    inIcon.addPixmap(m_zoomInNormal, QIcon::Normal);
    inIcon.addPixmap(m_zoomInDisabled, QIcon::Disabled);
    button_zoomIn->setIcon(inIcon);
    button_zoomIn->setIconSize(QSize(24, 24));
    button_zoomIn->setStyleSheet("QPushButton,"
                                 "QPushButton:hover,"
                                 "QPushButton:pressed,"
                                 "QPushButton:focus,"
                                 "QPushButton:active {"
                                 "   background: transparent;"
                                 "   border: none;"
                                 "}");

    QIcon outIcon;
    outIcon.addPixmap(m_zoomOutNormal, QIcon::Normal);
    outIcon.addPixmap(m_zoomOutDisabled, QIcon::Disabled);
    button_zoomOut->setIcon(outIcon);
    button_zoomOut->setIconSize(QSize(24, 24));
    button_zoomOut->setStyleSheet("QPushButton,"
                                  "QPushButton:hover,"
                                  "QPushButton:pressed,"
                                  "QPushButton:focus,"
                                  "QPushButton:active {"
                                  "   background: transparent;"
                                  "   border: none;"
                                  "}");

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

    button_zoomIn = new QPushButton(this);
    button_zoomIn->setFixedSize(38, 38);
    button_zoomIn->setIconSize(QSize(24, 24));
    button_zoomIn->setFlat(true);
    button_zoomIn->setToolTip(tr("Zoom In"));
    button_zoomIn->installEventFilter(this);

    button_zoomOut = new QPushButton(this);
    button_zoomOut->setFixedSize(38, 38);
    button_zoomOut->setIconSize(QSize(24, 24));
    button_zoomOut->setFlat(true);
    button_zoomOut->setToolTip(tr("Zoom Out"));
    button_zoomOut->installEventFilter(this);

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

bool Magnifier::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == button_zoomIn) {
        if (event->type() == QEvent::Enter) {
            if (button_zoomIn->isEnabled()) {
                button_zoomIn->setIcon(m_zoomInHover);
            }
        } else if (event->type() == QEvent::Leave) {
            if (button_zoomIn->isEnabled()) {
                button_zoomIn->setIcon(m_zoomInNormal);
            }
        }
    } else if (obj == button_zoomOut) {
        if (event->type() == QEvent::Enter) {
            if (button_zoomOut->isEnabled()) {
                button_zoomOut->setIcon(m_zoomOutHover);
            }
        } else if (event->type() == QEvent::Leave) {
            if (button_zoomOut->isEnabled()) {
                button_zoomOut->setIcon(m_zoomOutNormal);
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

QPixmap Magnifier::loadSvgWithColor(const QString &resourcePath, const QString &oldColor,
                                    const QString &newColor)
{
    QFile file(resourcePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QPixmap();
    }
    QString svgContent = QString::fromUtf8(file.readAll());
    file.close();

    if (!oldColor.isEmpty() && !newColor.isEmpty()) {
        svgContent.replace(oldColor, newColor);
    }

    QByteArray data = svgContent.toUtf8();
    QPixmap pixmap;
    pixmap.loadFromData(data, "SVG");
    return pixmap;
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
