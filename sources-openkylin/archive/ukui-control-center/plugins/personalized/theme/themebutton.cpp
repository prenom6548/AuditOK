/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "themebutton.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QPainter>
#include <QButtonGroup>
#include <QKeyEvent>
#include <QFocusEvent>
#include <algorithm>
#include <kysdk/applications/accessinfohelper.h>

using namespace kdk;

ThemeButton::ThemeButton(const QString &name, QWidget *parent):
    QPushButton(parent),
    themename(name)
{
    mWidth = 200;
    mHeigh = 164;
    setFixedWidth(mWidth);
    mIconLabel = new ThemeIconLabel(QSize(mWidth, mHeigh - 32), this);
    initUi();
    setConnect();
}

ThemeButton::ThemeButton(const QString &name, QPixmap pixmap, QWidget *parent):
    QPushButton(parent),
    themename(name)
{
    mIconLabel = new ThemeIconLabel(pixmap, QSize(mWidth, mHeigh - 32), this);
    initUi();
    setConnect();
}


ThemeButton::ThemeButton(QString name, QStringList iStringList, QWidget *parent):
    themename(name),
    QPushButton(parent)
{
    mIconLabel = new ThemeIconLabel( iStringList, QSize(mWidth, mHeigh - 32), this);
    initUi();
    setConnect();
}

ThemeButton::ThemeButton(QString name, QList<QPixmap> iconList, QWidget *parent):
    themename(name),
    QPushButton(parent)
{
    mIconLabel = new ThemeIconLabel( iconList, QSize(mWidth, mHeigh - 32), this);
    initUi();
    setConnect();
}

ThemeButton::ThemeButton(const QString &name, const QIcon &icon, QSize size, QWidget *parent):
    QPushButton(parent),
    themename(name)
{
    mIconLabel = new ThemeIconLabel(icon, size, this);

    initUi();
    setConnect();
}

void ThemeButton::setIconCenter()
{
    if (mIconLabel) {
        mIconLabel->setIconCenter();
    }
}

void ThemeButton::initUi()
{
    setCheckable(true);
    // 互斥由 QButtonGroup 在点击时处理；关闭 autoExclusive，避免方向键移焦时连带选中
    setAutoExclusive(false);
    setFocusPolicy(Qt::StrongFocus);
    setFixedWidth(mWidth);
    QVBoxLayout *baseVerLayout = new QVBoxLayout(this);
    baseVerLayout->setSpacing(8);
    baseVerLayout->setMargin(0);

    mIconLabel->changeClickedFlag(false);

    QHBoxLayout *bottomHorLayout = new QHBoxLayout;
    bottomHorLayout->setSpacing(8);
    bottomHorLayout->setMargin(0);

    mNameLabel = new QLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mNameLabel, "Theme", "", "theme name label of" + themename);
    KDK_EXTEND_ALL_INFO_FORMAT(mIconLabel, "Theme", "", "theme icon label of" + themename);
    mNameLabel->setText(themename);
    mNameLabel->setAlignment(Qt::AlignTop);

    bottomHorLayout->addStretch();
    bottomHorLayout->setContentsMargins(0, 0, 0, 0);
    bottomHorLayout->addWidget(mNameLabel);
    bottomHorLayout->addStretch();

    baseVerLayout->addWidget(mIconLabel);
    baseVerLayout->addLayout(bottomHorLayout);

    this->setLayout(baseVerLayout);

    // 主题模式按钮：KDK 之后覆盖 accessibleName，读屏朗读选项名称
    setAccessibleName(themename);

    // 选中变化后刷新 Tab 落点（仅当前选中项在 Tab 链中）
    connect(this, &QAbstractButton::toggled, this, [this](bool) {
        refreshGroupKeyboardNav();
    });
}

void ThemeButton::setAccessibleOptionName(const QString &sectionTitle)
{
    if (sectionTitle.isEmpty()) {
        setAccessibleName(themename);
        return;
    }
    // 与强调色「强调色 拂晓蓝」、特效开关嵌入标题一致，保证 Tab 进组能读行标题
    setAccessibleName(sectionTitle + QLatin1Char(' ') + themename);
}

void ThemeButton::btnCheckedSignalsSlot(bool status) {
    this->setChecked(status);
    if (!status) {
        mIconLabel->changeClickedFlag(false);
    }
}

void ThemeButton::enterWidgetSlot() {
    if (!this->isChecked()) {
        mIconLabel->changeClickedFlag(true);
    }
}

void ThemeButton::leaveWidgetSlot() {
    if (!this->isChecked()) {
        mIconLabel->changeClickedFlag(false);
    }
}

void ThemeButton::setConnect()
{
    connect(this, &ThemeButton::clicked, this, [=]() {
        mIconLabel->changeClickedFlag(true);
    });

    connect(this, &ThemeButton::btnCheckedSignals, [=](bool status) {
        btnCheckedSignalsSlot(status);
    });

    connect(mIconLabel,&ThemeIconLabel::enterWidget, [=]() {
        enterWidgetSlot();
    });

    connect(mIconLabel,&ThemeIconLabel::leaveWidget, [=]() {
        leaveWidgetSlot();
    });
}

void ThemeButton::setRadius(int radius)
{
    mRadius = radius;
    mIconLabel->setRadius(radius);
}

void ThemeButton::setRect(QSize size)
{
    mWidth = size.width();
    mHeigh = size.height();
    setFixedWidth(mWidth);
    mIconLabel->setFixedSize(QSize(mWidth, mHeigh - 32));
    mIconLabel->setMaskSize(QSize(mWidth, mHeigh - 32));
}

void ThemeButton::setPixmap(QPixmap pixmap)
{    
    mIconLabel->setIcon(pixmap);
}

void ThemeButton::setBtnClicked(bool status)
{
    this->setChecked(status);
    mIconLabel->changeClickedFlag(status);
    refreshGroupKeyboardNav();
}

void ThemeButton::refreshGroupKeyboardNav()
{
    QButtonGroup *g = group();
    if (!g) {
        setAutoExclusive(false);
        setFocusPolicy(Qt::StrongFocus);
        return;
    }

    const QList<QAbstractButton *> buttons = g->buttons();
    QAbstractButton *checked = g->checkedButton();
    for (QAbstractButton *b : buttons) {
        if (!b) {
            continue;
        }
        b->setAutoExclusive(false);
        // 无选中时保留第一项可 Tab，避免整组从键盘不可达
        const bool tabStop = checked ? (b == checked)
                                     : (b == buttons.first());
        b->setFocusPolicy(tabStop ? Qt::StrongFocus : Qt::ClickFocus);
    }
}

void ThemeButton::moveFocusInGroup(int key)
{
    QButtonGroup *g = group();
    if (!g) {
        return;
    }

    QList<ThemeButton *> peers;
    for (QAbstractButton *b : g->buttons()) {
        auto *tb = qobject_cast<ThemeButton *>(b);
        if (!tb || !tb->isVisible() || !tb->isEnabled()) {
            continue;
        }
        peers.append(tb);
    }
    if (peers.size() < 2) {
        return;
    }

    std::sort(peers.begin(), peers.end(), [](ThemeButton *a, ThemeButton *b) {
        const QPoint pa = a->mapToGlobal(QPoint(0, 0));
        const QPoint pb = b->mapToGlobal(QPoint(0, 0));
        if (pa.y() != pb.y()) {
            return pa.y() < pb.y();
        }
        return pa.x() < pb.x();
    });

    const int idx = peers.indexOf(this);
    if (idx < 0) {
        return;
    }

    int next = idx;
    if (key == Qt::Key_Left || key == Qt::Key_Up) {
        next = (idx - 1 + peers.size()) % peers.size();
    } else if (key == Qt::Key_Right || key == Qt::Key_Down) {
        next = (idx + 1) % peers.size();
    } else {
        return;
    }

    // TabFocusReason 才能稳定画出键盘黑色焦点框
    peers.at(next)->setFocus(Qt::TabFocusReason);
}

void ThemeButton::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        // 只移动焦点框，不 click / 不改 checked（空格或鼠标再选中）
        moveFocusInGroup(event->key());
        event->accept();
        return;
    default:
        break;
    }
    QPushButton::keyPressEvent(event);
}

void ThemeButton::focusInEvent(QFocusEvent *event)
{
    QPushButton::focusInEvent(event);
    // 焦点框画在图标区（与选中框同大），需刷新 ThemeIconLabel
    if (mIconLabel) {
        mIconLabel->update();
    }
}

void ThemeButton::focusOutEvent(QFocusEvent *event)
{
    QPushButton::focusOutEvent(event);
    if (mIconLabel) {
        mIconLabel->update();
    }
}

void ThemeButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);

    painter.drawRoundedRect(this->rect(), mRadius, mRadius);

    if (height() != mIconLabel->height() + 8 + mNameLabel->height())
        this->setFixedHeight(mIconLabel->height() + 8 + mNameLabel->height());

}
