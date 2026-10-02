/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "netmaskitem.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

NetMaskItem::NetMaskItem(QFrame *parent)
    : SettingsItem(parent)
    , m_titleLabel(new QLabel)
    , m_spinBox(new QSpinBox)
{
    //m_titleLabel->setFixedWidth(120);
    m_titleLabel->adjustSize();
    m_titleLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_titleLabel->setFixedWidth(150);

    m_spinBox->setFixedSize(220, 36);

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_titleLabel);
    layout->addSpacing(30);
    layout->addWidget(m_spinBox);
//    layout->addStretch();

    setLayout(layout);
    setFixedHeight(36);

    m_spinBox->setMinimum(1);
    m_spinBox->setMaximum(32);
    m_spinBox->setValue(16);

    setWhatsThis(m_spinBox, tr("Set the subnet mask of container docker"));
    connect(m_spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, [=]() {emit BtnEnable();});
}

void NetMaskItem::setTitle(const QString &title)
{
    QFont ft;
    QFontMetrics fm(ft);
    QString elided_text = fm.elidedText(title, Qt::ElideRight, 150);
    m_titleLabel->setText(elided_text);
    if (fm.width(title) > m_titleLabel->width()) {
        m_titleLabel->setToolTip(title);
    }
}

QString NetMaskItem::getNetmask() const
{
    return QString::number(m_spinBox->value());
}

void NetMaskItem::setWhatsThis(QWidget *w, const QString &text)
{
    w->setWhatsThis(text);
    w->setToolTip("<qt>"+ text +"</qt>" );
}

void NetMaskItem::clear()
{
    m_spinBox->setValue(16);
}
