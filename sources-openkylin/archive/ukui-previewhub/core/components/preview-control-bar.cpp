/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2026, KylinSoft Co, Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Wenjie Xiang <xiangwenjie@kylinos.cn>
 *
 */

#include "components/preview-control-bar.h"

#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QPalette>
#include <QTimer>

namespace {
constexpr int kHideDelayMs = 3000;
}

PreviewControlBar::PreviewControlBar(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("previewControlBar");
    setAttribute(Qt::WA_StyledBackground, true);
    setMouseTracking(true);
    setFixedHeight(48);

    m_opacity_effect = new QGraphicsOpacityEffect(this);
    m_opacity_effect->setOpacity(1.0);
    setGraphicsEffect(m_opacity_effect);

    m_opacity_animation = new QPropertyAnimation(m_opacity_effect, "opacity", this);
    m_opacity_animation->setDuration(180);
    m_opacity_animation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_opacity_animation, &QPropertyAnimation::finished, this, [this]() {
        if (!m_bar_visible)
            hide();
    });

    m_hide_timer = new QTimer(this);
    m_hide_timer->setSingleShot(true);
    m_hide_timer->setInterval(kHideDelayMs);
    connect(m_hide_timer, &QTimer::timeout, this, [this]() {
        if (isInteractionActive()) {
            scheduleHide();
            return;
        }
        showBar(false);
    });

    if (parent)
        parent->installEventFilter(this);

    // Defer the initial refresh until the most-derived control bar has
    // finished constructing its child widgets.
    scheduleThemeUpdate();
    connect(qApp, &QGuiApplication::paletteChanged, this, [this]() {
        scheduleThemeUpdate();
    });
}

void PreviewControlBar::setPreviewWidget(QWidget *previewWidget)
{
    if (m_preview_widget == previewWidget)
        return;

    if (m_preview_widget) {
        m_preview_widget->removeEventFilter(this);
        const auto oldChildren = m_preview_widget->findChildren<QWidget *>();
        for (QWidget *child : oldChildren) {
            if (child != this && !isAncestorOf(child))
                child->removeEventFilter(this);
        }
    }

    m_preview_widget = previewWidget;
    installPreviewEventFilters();
    showBar(true);
}

void PreviewControlBar::setFloating(bool floating)
{
    if (m_floating == floating)
        return;

    m_floating = floating;
    updateFloatingGeometry();
}

void PreviewControlBar::showBar(bool visible)
{
    if (visible) {
        if (m_preview_widget)
            scheduleHide();
        else
            m_hide_timer->stop();
    } else {
        m_hide_timer->stop();
    }

    if (m_bar_visible == visible)
        return;

    m_bar_visible = visible;
    m_opacity_animation->stop();
    m_opacity_animation->setStartValue(m_opacity_effect->opacity());
    m_opacity_animation->setEndValue(visible ? 1.0 : 0.0);
    if (visible) {
        QWidget::show();
        raise();
    }
    m_opacity_animation->start();
}

void PreviewControlBar::stop()
{
    m_hide_timer->stop();
    m_opacity_animation->stop();
    m_bar_visible = false;
    hide();
}

bool PreviewControlBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parentWidget() && event->type() == QEvent::Resize)
        updateFloatingGeometry();

    if (isPreviewObject(watched)) {
        switch (event->type()) {
        case QEvent::ChildAdded:
            QTimer::singleShot(0, this, &PreviewControlBar::installPreviewEventFilters);
            break;
        case QEvent::Enter:
        case QEvent::MouseMove:
        case QEvent::MouseButtonPress:
            showBar(true);
            break;
        case QEvent::Leave:
            if (!isCursorInPreview())
                scheduleHide();
            break;
        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void PreviewControlBar::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::PaletteChange ||
        event->type() == QEvent::ApplicationPaletteChange ||
        event->type() == QEvent::ThemeChange ||
        event->type() == QEvent::StyleChange) {
        scheduleThemeUpdate();
    }
}

bool PreviewControlBar::isDarkTheme() const
{
    const QPalette currentPalette = qApp ? qApp->palette() : palette();
    return currentPalette.color(QPalette::Window).lightness() < 128;
}

bool PreviewControlBar::isInteractionActive() const
{
    if (!m_bar_visible || !isVisible())
        return false;

    return rect().contains(mapFromGlobal(QCursor::pos()));
}

QString PreviewControlBar::themeStyleSheet(bool dark) const
{
    const QString background = dark ? "rgba(36, 36, 36, 210)" : "rgba(255, 255, 255, 220)";
    const QString border = dark ? "rgba(255, 255, 255, 32)" : "rgba(0, 0, 0, 18)";
    return QString(
        "#previewControlBar {"
        "background-color: %1;"
        "border: 1px solid %2;"
        "border-radius: 6px;"
        "}")
        .arg(background, border);
}

void PreviewControlBar::themeChanged(bool dark)
{
    Q_UNUSED(dark)
}

void PreviewControlBar::refreshThemeStyle()
{
    if (m_updating_theme)
        return;

    m_theme_update_pending = false;
    m_updating_theme = true;
    const bool dark = isDarkTheme();
    const QString style = themeStyleSheet(dark);
    if (styleSheet() != style)
        setStyleSheet(style);
    themeChanged(dark);
    m_updating_theme = false;
}

bool PreviewControlBar::isCursorInPreview() const
{
    return m_preview_widget &&
           m_preview_widget->rect().contains(
               m_preview_widget->mapFromGlobal(QCursor::pos()));
}

bool PreviewControlBar::isPreviewObject(QObject *object) const
{
    if (!m_preview_widget || !object)
        return false;

    QObject *current = object;
    while (current) {
        if (current == m_preview_widget)
            return true;
        if (current == this)
            return false;
        current = current->parent();
    }
    return false;
}

void PreviewControlBar::installPreviewEventFilters()
{
    if (!m_preview_widget)
        return;

    m_preview_widget->setMouseTracking(true);
    m_preview_widget->installEventFilter(this);
    const auto children = m_preview_widget->findChildren<QWidget *>();
    for (QWidget *child : children) {
        if (child != this && !isAncestorOf(child)) {
            child->setMouseTracking(true);
            child->installEventFilter(this);
        }
    }
}

void PreviewControlBar::scheduleHide()
{
    if (m_preview_widget)
        m_hide_timer->start();
}

void PreviewControlBar::updateFloatingGeometry()
{
    if (!m_floating || !parentWidget())
        return;

    const QRect rect = parentWidget()->rect();
    const int sideMargin = rect.width() > 340 ? 36 : 8;
    const int availableWidth = qMax(0, rect.width() - sideMargin * 2);
    const int controlWidth = qMin(availableWidth, maximumWidth());
    const int controlX = (rect.width() - controlWidth) / 2;
    const int controlY = qMax(8, rect.height() - height() - 20);
    setGeometry(controlX, controlY, controlWidth, height());
    raise();
}

void PreviewControlBar::scheduleThemeUpdate()
{
    if (m_updating_theme || m_theme_update_pending)
        return;

    m_theme_update_pending = true;
    QTimer::singleShot(0, this, &PreviewControlBar::refreshThemeStyle);
}
