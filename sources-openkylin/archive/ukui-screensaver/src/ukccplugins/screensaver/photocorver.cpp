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
#include "photocorver.h"
#include <QPainter>
#include <QPainterPath>
#include <QIcon>
#include <QDebug>

PhotoCorver::PhotoCorver(QWidget *parent, bool isSystemCorver, bool isScreensaver)
    : QLabel{parent},
    m_bSystemCorver(isSystemCorver),
    m_bScreensaver(isScreensaver)
{
    if (isScreensaver){
        setFixedSize(201, 113);
    } else {
        setFixedSize(213, 145);
    }
    installEventFilter(this);
    m_maskLabel = new MaskLable(this, true);
    m_maskLabel->setGeometry(0, 0, this->width(), this->height());
    m_maskLabel->hide();
}

void PhotoCorver::setBackground(QPixmap background)
{
    m_background = background;
    repaint();
}

void PhotoCorver::setClickStatus(bool state)
{
    m_bSelectStatus = state;
    m_maskLabel->setVisible(m_bSelectStatus);
}

bool PhotoCorver::eventFilter(QObject *watched, QEvent *event)
{
    if (parentWidget() && !parentWidget()->isEnabled()) {
        event->ignore();
        return QWidget::eventFilter(watched, event);
    }
    if (event->type() == QEvent::Enter) {
        m_maskLabel->show();
    } else if (event->type() == QEvent::Leave) {
        if (!m_bSelectStatus) {
            m_maskLabel->hide();
        }
    }
    return QWidget::eventFilter(watched, event);
}

void PhotoCorver::mouseReleaseEvent(QMouseEvent *event)
{
    if (parentWidget() && !parentWidget()->isEnabled()) {
        event->ignore();
        return;
    }
    if (event->button() == Qt::LeftButton) {
        m_bSelectStatus = true;
        m_maskLabel->show();
        Q_EMIT photoCorverLeftClicked();
    }
    QWidget::mouseReleaseEvent(event);
}

void PhotoCorver::contextMenuEvent(QContextMenuEvent *event)
{
    if (parentWidget() && !parentWidget()->isEnabled()) {
        event->ignore();
        return;
    }
    if (!m_bSelectStatus) {
        m_maskLabel->hide();
    }
    if (!m_bSystemCorver) {
        if (!m_rightClickMenu) {
            m_rightClickMenu = new QMenu(this);
            QAction *openFileAction = new QAction(tr("Right open the file location"), m_rightClickMenu);
            connect(openFileAction, &QAction::triggered, this, [&]{
                Q_EMIT openFileFolderClicked();
            });
            QAction *deleteAction = new QAction(tr("Remove the file"), m_rightClickMenu);
            connect(deleteAction, &QAction::triggered, this, [&]{
                Q_EMIT removePathClicked();
            });
            m_rightClickMenu->addActions({openFileAction, deleteAction});
        }
        m_rightClickMenu->exec(event->globalPos());
    }
}

void PhotoCorver::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    painter.fillRect(rect(), palette().color(QPalette::Base));

    QRect bgRect = QRect(4, 4, width() - 8, height() - 8);
    QPainterPath path;
    path.addRoundedRect(bgRect, 8, 8);
    painter.setClipPath(path);
    painter.drawPixmap(bgRect, m_background, m_background.rect());
}
