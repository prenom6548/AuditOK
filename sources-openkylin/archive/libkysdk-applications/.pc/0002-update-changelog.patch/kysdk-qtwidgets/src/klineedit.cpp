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

#include "klineedit.h"
#include "themeController.h"
#include <QBoxLayout>
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionButton>

namespace kdk
{

class Q_DECL_HIDDEN PushButton : public QPushButton, public ThemeController
{
    Q_OBJECT
public:
    PushButton(QWidget *parent = nullptr);

    void setFocusStyle(bool flag);

protected:
    void paintEvent(QPaintEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void leaveEvent(QEvent *e);
    void enterEvent(QEvent *e);

private:
    bool m_pressEnter;
    bool m_mouseEnter;
    bool m_foucusStyle;
};

class Q_DECL_HIDDEN KLineEditPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KLineEdit)
public:
    KLineEditPrivate(KLineEdit *parent);
    void adjustLayout();

protected:
    void changeTheme() override;

private:
    KLineEdit *q_ptr;
    QWidget *m_pWidget;
    PushButton *m_pPushButton;
};

KLineEdit::KLineEdit(QWidget *parent)
    : QLineEdit(parent)
    , d_ptr(new KLineEditPrivate(this))
{
    installEventFilter(this);
    setFocusPolicy(Qt::StrongFocus);
    clearFocus();
    connect(this, &KLineEdit::returnPressed, this, &KLineEdit::clearFocus);
}

QPushButton *KLineEdit::pushbutton()
{
    Q_D(KLineEdit);
    return d->m_pPushButton;
}

void KLineEdit::resizeEvent(QResizeEvent *event)
{
    Q_D(KLineEdit);
    QLineEdit::resizeEvent(event);
    d->adjustLayout();
}

bool KLineEdit::eventFilter(QObject *watched, QEvent *event)
{
    Q_D(KLineEdit);
    if (watched == this) {
        switch (event->type()) {
        case QEvent::FocusIn:
            d->m_pPushButton->setFocusStyle(true);
            break;
        case QEvent::FocusOut:
            d->m_pPushButton->setFocusStyle(false);
            break;
        default:
            break;
        }
    }
    return QLineEdit::eventFilter(watched, event);
}

void PushButton::mouseReleaseEvent(QMouseEvent *event)
{
    m_pressEnter = false;
    m_mouseEnter = true;
    QPushButton::mouseReleaseEvent(event);
}

void PushButton::mousePressEvent(QMouseEvent *event)
{
    m_pressEnter = true;
    m_mouseEnter = false;
    QPushButton::mousePressEvent(event);
}

void PushButton::leaveEvent(QEvent *e)
{
    m_mouseEnter = false;
    QPushButton::leaveEvent(e);
}

void PushButton::enterEvent(QEvent *e)
{
    m_mouseEnter = true;
    QPushButton::enterEvent(e);
}

KLineEditPrivate::KLineEditPrivate(KLineEdit *parent)
    : q_ptr(parent)
{
    Q_Q(KLineEdit);
    m_pPushButton = new PushButton();
    m_pPushButton->setFixedSize(24, 24);
    m_pWidget = new QWidget(q);
    QHBoxLayout *hlayout = new QHBoxLayout(m_pWidget);
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->addWidget(m_pPushButton);
    connect(m_gsetting, &QGSettings::changed, this, [=] {
        changeTheme();
    });
}

void KLineEditPrivate::adjustLayout()
{
    Q_Q(KLineEdit);
    int spacing = 5;
    int width = spacing;
    if (!m_pPushButton->isHidden())
        width += (spacing + m_pPushButton->width());
    m_pWidget->setFixedSize(width, q->cursorRect().height());
    m_pWidget->move(q->width() - m_pWidget->width(), (q->rect().height() - m_pWidget->height()) / 2);
    q->setTextMargins(0, 0, m_pWidget->width(), 0);
}

void KLineEditPrivate::changeTheme()
{
    initThemeStyle();
}

PushButton::PushButton(QWidget *parent)
{
    //    connect(m_ge)
    m_pressEnter = false;
    m_mouseEnter = false;
    m_foucusStyle = false;
}

void PushButton::setFocusStyle(bool flag)
{
    m_foucusStyle = flag;
    if (flag)
        setFocusPolicy(Qt::NoFocus);
    else
        setFocusPolicy(Qt::StrongFocus);
}

void PushButton::paintEvent(QPaintEvent *event)
{
    QStyleOptionButton option;
    initStyleOption(&option);
    QPainter p(this);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.setPen(Qt::NoPen);
    QPixmap pixmap = icon().pixmap(iconSize());

    if (m_foucusStyle) {
        if (ThemeController::themeMode() == DarkTheme)
            pixmap = ThemeController::drawSymbolicColoredPixmap(pixmap);
        if (m_mouseEnter || m_pressEnter) {
            pixmap = ThemeController::drawSymbolicColoredPixmap(pixmap);
            p.setBrush(ThemeController::getPaletteColorFromDT("highlight"));
        } else
            p.setBrush(Qt::NoBrush);
    } else {
        if (ThemeController::themeMode() == DarkTheme)
            pixmap = ThemeController::drawSymbolicColoredPixmap(pixmap);
        if (m_mouseEnter) {
            p.setBrush(QColor(218, 218, 218));
        } else if (m_pressEnter) {
            p.setBrush(QColor(184, 184, 184));
        } else
            //            p.setBrush(palette().button().color());
            p.setBrush(ThemeController::getPaletteColorFromDT("button"));
    }
    int radius = ThemeController::getRadiusFromDT("kradius-normal");
    if (radius == -1)
        radius = 6;
    p.drawRoundedRect(rect(), radius, radius);

    QRect iconRect((rect().width() - iconSize().width()) / 2, (rect().height() - iconSize().height()) / 2, iconSize().width(), iconSize().height());
    p.drawPixmap(iconRect, pixmap);
}

}

#include "klineedit.moc"
#include "moc_klineedit.cpp"
