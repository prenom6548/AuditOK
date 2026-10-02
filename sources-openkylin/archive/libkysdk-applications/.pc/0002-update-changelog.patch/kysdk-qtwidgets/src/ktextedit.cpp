/*
 * libkysdk-qtwidgets's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "ktextedit.h"
#include "ktag.h"
#include "themeController.h"
#include <QDebug>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QProxyStyle>
#include <QTextCursor>
#include <QTimer>

namespace kdk
{

class KTextEditPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KTextEdit)
public:
    KTextEditPrivate(KTextEdit *parent);

protected:
    void changeTheme();

private:
    KTextEdit *q_ptr;
    KLabel *m_pLabel;
    QTextBlockFormat m_blockFormat;
    QTextCursor *m_pTextCursor;
};

KTextEdit::KTextEdit(QWidget *parent)
    : QTextEdit(parent)
    , d_ptr(new KTextEditPrivate(this))
{
    Q_D(KTextEdit);
    viewport()->setContentsMargins(0, 0, 0, 0);
    setFrameShape(QFrame::NoFrame);
}

void KTextEdit::setLabelVisible(bool flag)
{
    Q_D(KTextEdit);
    d->m_pLabel->setVisible(flag);
}

void KTextEdit::setPixmap(const QPixmap &pixmap)
{
    Q_D(KTextEdit);
    d->m_pLabel->setPixmap(pixmap);
}

const QPixmap *KTextEdit::pixmap() const
{
    Q_D(const KTextEdit);
    return d->m_pLabel->pixmap();
}

void KTextEdit::setLabelText(const QString &text)
{
    Q_D(KTextEdit);
    d->m_pLabel->setText(text);
}

void KTextEdit::setReadOnly(bool ro)
{
    Q_D(KTextEdit);
    QTextEdit::setReadOnly(ro);
    if (isReadOnly()) {
        QPalette pale = palette();
        pale.setColor(QPalette::Base, ThemeController::getPaletteColorFromDT("highlight"));
        pale.setColor(QPalette::Text, Qt::white);
        setPalette(pale);
        d->m_pLabel->setBackgroundColor(true, Qt::white);
    } else {
        QPalette pale;
        setPalette(pale);
        d->m_pLabel->setBackgroundColor(false);
    }
}

void KTextEdit::paintEvent(QPaintEvent *event)
{
    Q_D(KTextEdit);
    QTextEdit::paintEvent(event);

    if (d->m_pLabel->isVisible()) {
        d->m_blockFormat.setTextIndent(d->m_pLabel->width() + 4);
        d->m_blockFormat.setLineHeight(3, QTextBlockFormat::LineDistanceHeight);
        d->m_pTextCursor->setBlockFormat(d->m_blockFormat);
    } else {
        d->m_blockFormat.setTextIndent(0);
        d->m_blockFormat.setLineHeight(3, QTextBlockFormat::LineDistanceHeight);
        d->m_pTextCursor->setBlockFormat(d->m_blockFormat);
    }
}

KTextEditPrivate::KTextEditPrivate(KTextEdit *parent)
    : q_ptr(parent)
{
    Q_Q(KTextEdit);
    m_pLabel = new KLabel(q);
    m_pLabel->setAlignment(Qt::AlignCenter);
    m_pLabel->setFixedHeight(30);
    m_pLabel->setLabelType(KLabelType::DataType);
    // 调整位置使其不遮盖边框
    m_pLabel->move(2, 2);
    m_pLabel->setVisible(false);
    m_pLabel->setDataHightColor(true);
    m_pLabel->setFixedWidth(70);
    m_pTextCursor = new QTextCursor(q->textCursor());
    connect(m_gsetting, &QGSettings::changed, q, [=] {
        changeTheme();
        m_pLabel->setFixedWidth(q->fontMetrics().width(m_pLabel->text()) + m_pLabel->pixmap()->width() + 12);
        //        if(q->fontMetrics().height() > 30)
        //        m_pLabel->setFixedHeight(q->fontMetrics().height() + 4);
    });
}

void KTextEditPrivate::changeTheme()
{
    Q_Q(KTextEdit);
    initThemeStyle();
}

}

#include "ktextedit.moc"
#include "moc_ktextedit.cpp"
