/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "checkedlineedit.h"

#include <QEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <QToolButton>

namespace kdk {

namespace {

static void makeBaseTransparent(QPalette *pal)
{
    const Qt::GlobalColor transparent = Qt::transparent;
    for (auto group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
        pal->setBrush(group, QPalette::Base, transparent);
        pal->setBrush(group, QPalette::Window, transparent);
        pal->setBrush(group, QPalette::Button, transparent);
        pal->setBrush(group, QPalette::AlternateBase, transparent);
        pal->setBrush(group, QPalette::Highlight, QColor(255, 255, 255, 60));
        pal->setBrush(group, QPalette::HighlightedText, Qt::white);
    }
}

static void makeButtonTextWhite(QPalette *pal)
{
    const QColor white(255, 255, 255);
    for (auto group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
        pal->setColor(group, QPalette::ButtonText, white);
    }
}

} // namespace

CheckedLineEdit::CheckedLineEdit(QWidget *parent)
    : KPasswordEdit(parent)
    , m_iconsWhite(true)
    , m_iconsPainted(false)
    , m_transparentInputBg(true)
    , m_inPaletteResync(false)
{
    setUseCustomPalette(true);
    setAutoFillBackground(false);
    setAttribute(Qt::WA_NoSystemBackground, true);

    setStyleSheet(R"(
        QLineEdit {
            background: transparent;
        }
        QLineEdit:focus {
            border: 2px solid #3790FA;
            background: transparent;
        }
        QLineEdit:!focus {
            border: 2px solid #3790FA;
            background: transparent;
        }  
    )");

    connect(this, &CheckedLineEdit::textChanged, this, &CheckedLineEdit::onTextChanged);

    QTimer::singleShot(0, this, &CheckedLineEdit::forceRepaintWhiteIcons);

    connect(this, &QLineEdit::textChanged, this, [this]() {
        QTimer::singleShot(0, this, &CheckedLineEdit::forceRepaintWhiteIcons);
    });

    QPalette pal = palette();
    applyOobePalette(&pal);
    KPasswordEdit::setPalette(pal);
    hookChildToolButtons();
}

void CheckedLineEdit::setInputBackgroundTransparent(bool transparent)
{
    m_transparentInputBg = transparent;
    setStyleSheet(R"(
        QLineEdit {
            background: #3D3D3D;
        }
        QLineEdit:focus {
            border: 2px solid #3790FA;
            background: transparent;
        }
    )");
    if (!m_iconsWhite) {
        return;
    }
    m_inPaletteResync = true;
    QPalette pal = palette();
    applyOobePalette(&pal);
    KPasswordEdit::setPalette(pal);
    m_inPaletteResync = false;
    QTimer::singleShot(0, this, &CheckedLineEdit::forceRepaintWhiteIcons);
}

void CheckedLineEdit::applyOobePalette(QPalette *pal)
{
    if (m_transparentInputBg) {
        makeBaseTransparent(pal);
    }
    makeButtonTextWhite(pal);
}

void CheckedLineEdit::hookChildToolButtons()
{
    const QList<QToolButton *> buttons = findChildren<QToolButton *>();
    for (QToolButton *button : buttons) {
        if (!button) {
            continue;
        }
        connect(button, &QToolButton::clicked, this, [this]() {
            QTimer::singleShot(0, this, &CheckedLineEdit::forceRepaintWhiteIcons);
        }, Qt::QueuedConnection);
    }
}

void CheckedLineEdit::setIconsWhite(bool white)
{
    m_iconsWhite = white;
    forceRepaintWhiteIcons();
}

void CheckedLineEdit::forceRepaintWhiteIcons()
{
    if (!m_iconsWhite) {
        return;
    }

    const QList<QToolButton *> buttons = findChildren<QToolButton *>();

    for (QToolButton *button : buttons) {
        if (!button) {
            continue;
        }

        QIcon currentIcon = button->icon();
        if (currentIcon.isNull()) {
            continue;
        }

        QIcon whiteIcon;
        QList<QSize> sizes = currentIcon.availableSizes();

        if (sizes.isEmpty()) {
            sizes.append(QSize(16, 16));
        }

        for (const QSize &size : sizes) {
            if (!size.isValid() || size.isNull()) {
                continue;
            }

            QPixmap originalPixmap = currentIcon.pixmap(size);
            if (originalPixmap.isNull()) {
                continue;
            }

            QPixmap tinted = originalPixmap.copy();
            QPixmap whitePixmap(tinted.size());
            whitePixmap.fill(Qt::transparent);

            QPainter tintPainter(&tinted);
            tintPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            tintPainter.fillRect(tinted.rect(), Qt::white);
            tintPainter.end();

            QPainter painter(&whitePixmap);
            painter.drawPixmap(0, 0, tinted);
            painter.end();

            whiteIcon.addPixmap(whitePixmap);
        }

        if (!whiteIcon.isNull()) {
            button->setIcon(whiteIcon);
        }
    }

    m_iconsPainted = true;
    update();
}

bool CheckedLineEdit::eventFilter(QObject *watched, QEvent *event)
{
    bool result = KPasswordEdit::eventFilter(watched, event);

    if (watched == this) {
        if (event->type() == QEvent::FocusIn || event->type() == QEvent::FocusOut) {
            QTimer::singleShot(0, this, &CheckedLineEdit::forceRepaintWhiteIcons);
        }
    }

    return result;
}

bool CheckedLineEdit::event(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange && m_iconsWhite && !m_inPaletteResync) {
        const bool r = KPasswordEdit::event(e);
        QTimer::singleShot(0, this, [this]() {
            if (!m_iconsWhite) {
                return;
            }
            m_inPaletteResync = true;
            QPalette p = palette();
            applyOobePalette(&p);
            KPasswordEdit::setPalette(p);
            m_inPaletteResync = false;
            forceRepaintWhiteIcons();
        });
        return r;
    }
    return KPasswordEdit::event(e);
}

void CheckedLineEdit::resizeEvent(QResizeEvent *event)
{
    KPasswordEdit::resizeEvent(event);
    QTimer::singleShot(50, this, &CheckedLineEdit::forceRepaintWhiteIcons);
}

void CheckedLineEdit::paintEvent(QPaintEvent *event)
{
    KPasswordEdit::paintEvent(event);

    if (!m_iconsPainted) {
        forceRepaintWhiteIcons();
    }
}

void CheckedLineEdit::onEchoModeChanged()
{
    QTimer::singleShot(0, this, &CheckedLineEdit::forceRepaintWhiteIcons);
}

void CheckedLineEdit::onTextChanged(const QString &text)
{
    Q_UNUSED(text);
    QTimer::singleShot(0, this, &CheckedLineEdit::forceRepaintWhiteIcons);
}

} // namespace kdk
