/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#include "kybutton.h"
#include "kydtbrush.h"

#include <QApplication>

#include "../private/styleparameter/tokenparameter.h"

namespace UKUISDKStyle {

KyButton::KyButton(QObject *parent)
    : QObject(parent)
{
    connectTheme();
    refreshPalette();

    if (qApp) {
        connect(qApp, &QApplication::paletteChanged, this, [this](const QPalette &) {
            refreshPalette();
        });
    }
}

void KyButton::setStyleType(int styleType)
{
    if (m_styleType == styleType)
        return;
    m_styleType = styleType;
    emit styleTypeChanged();
    updateContentColor();
}

void KyButton::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    emit enabledChanged();
    updateContentColor();
}

void KyButton::setHovered(bool hovered)
{
    if (m_hovered == hovered)
        return;
    m_hovered = hovered;
    emit hoveredChanged();
    updateContentColor();
}

void KyButton::setPressed(bool pressed)
{
    if (m_pressed == pressed)
        return;
    m_pressed = pressed;
    emit pressedChanged();
    updateContentColor();
}

void KyButton::setHasIcon(bool hasIcon)
{
    if (m_hasIcon == hasIcon)
        return;
    m_hasIcon = hasIcon;
    emit hasIconChanged();
    updateContentColor();
}

void KyButton::connectTheme()
{
    if (!qApp || !qApp->property("qqc2-globaltoken").isValid())
        return;

    auto *token = qApp->property("qqc2-globaltoken").value<UKUIQQC2Style::TokenParameter *>();
    if (!token)
        return;

    m_dt = token->getInstance();
    if (!m_dt)
        return;

    connect(m_dt, &UKUIGlobalDTConfig::GlobalDTConfig::tokenChanged, this, [this]() {
        refreshPalette();
    }, Qt::UniqueConnection);
}

void KyButton::refreshPalette()
{
    if (!m_dt)
        connectTheme();
    if (!m_dt)
        return;

    // DT 与 kborderlessbutton / kborderbutton 中 ThemeController 使用的 key 对应
    m_disableColor = DtBrush::brushColor(m_dt->kFontPrimaryDisable());
    m_iconNormalColor = DtBrush::brushColor(m_dt->kFontPrimary());
    m_textNormalColor = DtBrush::brushColor(m_dt->highLightActive());
    m_borderTextNormalColor = DtBrush::brushColor(m_dt->buttonTextActive());
    m_borderLineNormalColor = DtBrush::brushColor(m_dt->buttonActive());

    m_hoverColor = DtBrush::startColor(m_dt->kBrandHover());
    m_hoverEndColor = DtBrush::endColor(m_dt->kBrandHover());
    m_clickColor = DtBrush::startColor(m_dt->kBrandClick());
    m_clickEndColor = DtBrush::endColor(m_dt->kBrandClick());

    updateContentColor();
}

void KyButton::updateContentColor()
{
    QColor content;
    QColor border;

    if (m_styleType == StyleBorder) {
        // kdk::KBorderButton：hover/press 用渐变 endColor
        if (!m_enabled) {
            content = m_disableColor;
            border = m_disableColor;
        } else if (m_hovered) {
            if (m_pressed) {
                content = m_clickEndColor;
                border = content;
            } else {
                content = m_hoverEndColor;
                border = content;
            }
        } else if (m_pressed) {
            content = m_clickEndColor;
            border = content;
        } else {
            content = m_borderTextNormalColor;
            border = m_borderLineNormalColor;
        }
    } else {
        // kdk::KBorderlessButton：hover/press 用渐变 startColor
        if (!m_enabled) {
            content = m_disableColor;
        } else if (m_hovered) {
            content = m_pressed ? m_clickColor : m_hoverColor;
        } else if (m_hasIcon) {
            content = m_iconNormalColor;
        } else {
            content = m_textNormalColor;
        }
        border = QColor();
    }

    if (!content.isValid())
        content = Qt::black;
    if (m_styleType == StyleBorder && !border.isValid())
        border = content;

    if (m_contentColor != content) {
        m_contentColor = content;
        emit contentColorChanged();
    }
    if (m_borderColor != border) {
        m_borderColor = border;
        emit borderColorChanged();
    }
}

} // namespace UKUISDKStyle
