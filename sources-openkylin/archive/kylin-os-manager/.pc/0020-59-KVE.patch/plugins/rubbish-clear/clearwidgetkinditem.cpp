/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#include "clearwidgetkinditem.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QIcon>
#include <QDebug>
#include "toolutils.h"

namespace KylinRubbishClear
{

ClearWidgetKindItem::ClearWidgetKindItem(QWidget *parent) : QWidget(parent)
{
    mBtn = new QPushButton(this);
    mBtn->setFocusPolicy(Qt::NoFocus);
    mBtn->setFlat(true);
    mBtn->resize(56, 24);
    connect(mBtn, &QPushButton::clicked, [=]() {
        Q_EMIT sigKindItemClick();
    });

    QHBoxLayout *mBtnLayout = new QHBoxLayout();
    mBtnLayout->setContentsMargins(0, 0, 0, 0);
    mBtnLayout->setSpacing(0);
    mBtnLayout->addWidget(mBtn);
    mBtnLayout->addStretch(1);

    mLabelIcon = new QLabel(this);
    mLabelIcon->setFixedSize(36, 36);
    //    mLabelIcon->setStyleSheet("QPushButton{background:#E6E6E6;border-radius:18px;}");
    //    QPalette iconPal = palette();
    //    iconPal.setBrush(QPalette::Background,iconPal.button());
    //    mLabelIcon->setPalette(iconPal);

    mLabelTips = new QLabel(this);
    mLabelTips->setFixedSize(358, 26);
    QPalette pal;
    //    QColor color = palette().color(QPalette::PlaceholderText);
    //    qDebug() << QString("rgb:%1").arg(color.HexRgb);
    pal.setColor(QPalette::WindowText, QColor("#8F9399"));
    mLabelTips->setPalette(pal);

    QVBoxLayout *mVLayout = new QVBoxLayout();
    mVLayout->setContentsMargins(0, 0, 0, 0);
    mVLayout->setSpacing(0);
    mVLayout->addLayout(mBtnLayout);
    mVLayout->addWidget(mLabelTips);

    QHBoxLayout *mHLayout = new QHBoxLayout(this);
    mHLayout->addWidget(mLabelIcon);
    mHLayout->addLayout(mVLayout);
    mHLayout->addStretch(1);
    mHLayout->setDirection(QBoxLayout::LeftToRight);

    this->setLayout(mHLayout);
}
void ClearWidgetKindItem::changeSystemFontSize(int size)
{
    float fontSize = 14 * size / 11;
    QFont font;
    font.setPixelSize(int(fontSize));
    mLabelTips->setFont(font);

    ToolUtils::setToolTipAfterChangeFont(mLabelTips, mStrTips);
}
void ClearWidgetKindItem::setWidgetContent(const QString iconName, const QString btnText, const QString tipText)
{

    QIcon icon;
    if (QIcon::hasThemeIcon(iconName)) {
        icon = QIcon::fromTheme(iconName);
        QPixmap pix = icon.pixmap(icon.actualSize(QSize(36, 36)));
        mLabelIcon->setPixmap(pix);
        //        mLabelIcon->setIcon(icon);
        //        mLabelIcon->setIconSize(QSize(14,14));
    } else {
        qWarning() << "ClearWidgetKindItem::setWidgetContent 系统中无该图标：" << icon;
    }
    mBtn->setText(btnText);
    mLabelTips->setText(tipText);
}

void ClearWidgetKindItem::setWidgetItemIcon(const QString str)
{
    QIcon icon(str);
    QPixmap pix = icon.pixmap(icon.actualSize(QSize(36, 36)));
    mLabelIcon->setPixmap(pix);
}
void ClearWidgetKindItem::setWidgetItemBtnText(const QString btnText)
{
    mBtn->setText(btnText);
}
void ClearWidgetKindItem::setWidgetItemTipText(const QString tipText)
{
    mStrTips = tipText;
    mLabelTips->setText(tipText);
}

} // namespace KylinRubbishClear
