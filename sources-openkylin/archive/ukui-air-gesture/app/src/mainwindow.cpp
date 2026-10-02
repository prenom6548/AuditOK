/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "mainwindow.h"
#include "airgestureicons.h"

#include <QDebug>
#include <QPainterPath>

namespace {
    constexpr int WINDOW_SIZE = 40;
    constexpr int ICON_SIZE = 24;
    constexpr int WINDOW_Y = 37;
    constexpr int WINDOW_RADIUS = 6;
    const QByteArray STYLE_GSETTINGS_SCHEMA = QByteArrayLiteral("org.ukui.style");
    const QByteArray TRANSPARENCY_KEY = QByteArrayLiteral("menuTransparency");
    const QByteArray THEME_KEY = QByteArrayLiteral("styleName");

    const QMap<QString, QString> directionToIcon = {
        {"up", "hand-palm-up-24-symbolic"},
        {"down", "hand-back-down-24-symbolic"},
        {"left", "hand-palm-right-24-symbolic"},
        {"right", "hand-back-left-24-symbolic"}
    };
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    initGSettings();
    initWindow();
    initUI();
}

void MainWindow::setIcon(const QString &palmDirection)
{
    const QString iconName = directionToIcon.value(palmDirection, QString());
    QPixmap pixmap = airGestureIconPixmap(iconName, ICON_SIZE);

    if (m_pStyleGSettings->get(THEME_KEY).toString() != "ukui-light") {
        pixmap = invertPixmapColors(pixmap);
    }

    m_pIconLabel->setPixmap(pixmap);
    m_currentDirection = palmDirection;
}

void MainWindow::initGSettings()
{
    if (!QGSettings::isSchemaInstalled(STYLE_GSETTINGS_SCHEMA)) {
        qWarning() << "GSettings schema: " << STYLE_GSETTINGS_SCHEMA << "is not installed.";
        return;
    }

    m_pStyleGSettings = new QGSettings(STYLE_GSETTINGS_SCHEMA, "", this);

    m_transparency = m_pStyleGSettings->get(TRANSPARENCY_KEY).toInt();

    connect(m_pStyleGSettings, &QGSettings::changed, this, [this](const QString &key) {
        if (key == TRANSPARENCY_KEY) {
            m_transparency = m_pStyleGSettings->get(TRANSPARENCY_KEY).toInt();
            update();
        }
        if (key == THEME_KEY && this->isVisible()) {
            setIcon(m_currentDirection);
        }
    });
}

void MainWindow::initWindow()
{
    UkuiWindowHelper *windowHelper = new UkuiWindowHelper(this);
    windowHelper->setWindowState(UkuiWindowHelper::State::KeepAbove);
    windowHelper->removeTitleBar();
    windowHelper->setSkipTaskBar(true);
    windowHelper->setSkipSwitcher(true);
    windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::OnScreenDisplay);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setFixedSize(WINDOW_SIZE, WINDOW_SIZE);

    QPainterPath path;
    path.addRoundedRect(rect(), WINDOW_RADIUS, WINDOW_RADIUS);
    QRegion roundedRegion(path.toFillPolygon().toPolygon());
    windowHelper->setBlurEffect(roundedRegion);

    int x = (QApplication::primaryScreen()->geometry().width() - WINDOW_SIZE) / 2;
    move(x, WINDOW_Y);
}

void MainWindow::initUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *layout = new QVBoxLayout(centralWidget);
    layout->setContentsMargins(8, 8, 8, 8);

    m_pIconLabel = new QLabel(centralWidget);
    m_pIconLabel->setFixedSize(ICON_SIZE, ICON_SIZE);
    layout->addWidget(m_pIconLabel);
}

// 深色主题下将图标非透明部分颜色绘制为白色
QPixmap MainWindow::invertPixmapColors(const QPixmap& pixmap) {
    if (pixmap.isNull()) {
        return pixmap;
    }

    QPixmap result(pixmap.size());
    result.setDevicePixelRatio(pixmap.devicePixelRatio());
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.drawPixmap(0, 0, pixmap);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(result.rect(), Qt::white);
    painter.end();

    return result;
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QColor color = palette().color(QPalette::Window);
    color.setAlpha(m_transparency * 255 / 100);
    QBrush brush(color);
    painter.setBrush(brush);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), WINDOW_RADIUS, WINDOW_RADIUS);
}