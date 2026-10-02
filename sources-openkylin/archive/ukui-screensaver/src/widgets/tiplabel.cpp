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
#include "tiplabel.h"
#include "customlabel.h"
#include <QHBoxLayout>
#include <QIcon>
#include <QPainter>
#include <QStyleOption>
#include <QStylePainter>
#include <qtextdocument.h>


TipLabel::TipLabel(QWidget *parent) : QWidget(parent)
{
    initUI();
}

void TipLabel::initUI()
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(10);

    m_pIconLabel = new QLabel(this);
    m_pIconLabel->setPixmap(QIcon::fromTheme("dialog-info").pixmap(16, 16));

    m_pTextLabel = new CustomLabel(this);
    m_pTextLabel->setFontSize(14);

    layout->addWidget(m_pIconLabel);
    layout->addWidget(m_pTextLabel);
}

void TipLabel::setText(QString tip)
{
    m_pTextLabel->setText(tip);

    // Ensure that we get correct sizeHints by placing this window on the right screen.
    QFontMetrics fm(m_pTextLabel->font());
    QSize extra(1, 0);
    // Make it look good with the default ToolTip font on Mac, which has a small descent.
    if (fm.descent() == 2 && fm.ascent() >= 11)
        ++extra.rheight();
    m_pTextLabel->setWordWrap(Qt::mightBeRichText(m_pTextLabel->text()));
    QSize sh = sizeHint();
    if (parentWidget()) {
        const qreal screenWidth = parentWidget()->geometry().width();
        if (!m_pTextLabel->wordWrap() && sh.width() > screenWidth) {
            m_pTextLabel->setWordWrap(true);
            sh = m_pTextLabel->sizeHint();
        }
    }
    m_pTextLabel->resize(sh + extra);
}

void TipLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::NoBrush);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.setOpacity(0.25);
    QRect rect = this->rect();
    painter.drawRoundedRect(rect.adjusted(1, 1, -1, -1), 6, 6);
    painter.restore();
    return QWidget::paintEvent(event);
}
