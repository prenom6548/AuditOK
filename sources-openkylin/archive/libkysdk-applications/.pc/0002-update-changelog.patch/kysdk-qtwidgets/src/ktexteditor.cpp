/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#include "ktexteditor.h"
#include "themeController.h"

#include <QBoxLayout>
#include <QDebug>
#include <QEvent>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QTextEdit>

namespace kdk
{

class KTextEditorPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KTextEditor)
public:
    KTextEditorPrivate(KTextEditor *parent);
    virtual ~KTextEditorPrivate();

private:
    KTextEditor *q_ptr;
    QVBoxLayout *m_pVBoxLayout;
    QScrollArea *m_pScrollArea;
    QTextEdit *m_pTextEdit;
    bool m_focus;
};

KTextEditor::KTextEditor(QWidget *parent)
    : QWidget(parent)
    , d_ptr(new KTextEditorPrivate(this))
{
    Q_D(KTextEditor);
    this->setFocusPolicy(Qt::ClickFocus);
    installEventFilter(this);
    d->m_pTextEdit->installEventFilter(this);
    d->m_pScrollArea->installEventFilter(this);
}

QTextEdit *KTextEditor::textEdit()
{
    Q_D(KTextEditor);
    return d->m_pTextEdit;
}

void KTextEditor::paintEvent(QPaintEvent *event)
{
    Q_D(KTextEditor);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(this->rect(), ThemeController::getPaletteColorFromDT("window"));
    QPen pen;
    pen.setWidth(2);
    if (d->m_focus) {
        pen.setColor(ThemeController::getPaletteColorFromDT("highlight"));
        painter.setPen(pen);
    } else {
        pen.setColor(ThemeController::getPaletteColorFromDT("placeholdertext"));
        painter.setPen(pen);
    }

    painter.setBrush(ThemeController::getPaletteColorFromDT("base"));
    QRect rect = this->rect().adjusted(1, 1, -1, -1);
    int radius = ThemeController::getRadiusFromDT("kradius-menu");
    if (radius == -1)
        radius = 8;
    painter.drawRoundedRect(rect, radius, radius);
}

bool KTextEditor::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KTextEditor);
    if (watched == this) {
        switch (event->type()) {
        case QEvent::FocusIn:
            d->m_focus = true;
            update();
            break;
        case QEvent::FocusOut:
            d->m_focus = false;
            update();
            break;
        default:
            break;
        }
        return QWidget::eventFilter(watched, event);
    } else {
        switch (event->type()) {
        case QEvent::FocusIn:
            d->m_focus = true;
            update();
            break;
        case QEvent::FocusOut:
            d->m_focus = false;
            update();
            break;
        default:
            break;
        }
        return false;
    }
}

KTextEditorPrivate::KTextEditorPrivate(KTextEditor *parent)
    : q_ptr(parent)
{
    Q_Q(KTextEditor);

    m_pVBoxLayout = new QVBoxLayout();
    m_pScrollArea = new QScrollArea();
    m_pTextEdit = new QTextEdit();
    m_focus = false;

    q->setContentsMargins(8, 12, 0, 13); // 上下左右边距为20 ，因控件自带边距调整
    m_pScrollArea->setWidget(m_pTextEdit);
    m_pScrollArea->horizontalScrollBar()->setVisible(false);
    m_pScrollArea->setWidgetResizable(true);
    m_pScrollArea->setFocusPolicy(Qt::ClickFocus);

    m_pTextEdit->setFrameShape(QFrame::NoFrame);
    m_pScrollArea->setFrameShape(QFrame::NoFrame);

    m_pVBoxLayout->addWidget(m_pScrollArea);

    q->setLayout(m_pVBoxLayout);
}

KTextEditorPrivate::~KTextEditorPrivate()
{
}

}

#include "ktexteditor.moc"
#include "moc_ktexteditor.cpp"
