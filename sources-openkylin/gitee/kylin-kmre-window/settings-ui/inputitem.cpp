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

#include "inputitem.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

InputItem::InputItem(QFrame *parent)
    : SettingsItem(parent)
    , m_appTitle(new QLabel)
    , m_pkgTitle(new QLabel)
    , m_appEdit(new QLineEdit)
    , m_pkgEdit(new QLineEdit)
{
    m_appEdit->setFocus();
    m_appEdit->setPlaceholderText(tr("For example: the glory of the king"));
    m_pkgEdit->setPlaceholderText(tr("For example: com.tencent.tmgp.sgame"));

    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_appTitle->setFixedWidth(100);
    m_pkgTitle->setFixedWidth(100);

    QHBoxLayout *hlayout1 = new QHBoxLayout();
    hlayout1->setSpacing(0);
    hlayout1->setMargin(0);
    hlayout1->addWidget(m_appTitle);
    hlayout1->addWidget(m_appEdit);

    QHBoxLayout *hlayout2 = new QHBoxLayout();
    hlayout2->setSpacing(0);
    hlayout2->setMargin(0);
    hlayout2->addWidget(m_pkgTitle);
    hlayout2->addWidget(m_pkgEdit);

    layout->addStretch();
    layout->addLayout(hlayout1);
    layout->addSpacing(16);
    layout->addLayout(hlayout2);
    layout->addStretch();

    setLayout(layout);
    setFixedHeight(74);
}

void InputItem::setTitle(const QString &title1, const QString &title2)
{
    m_appTitle->setText(title1);
    m_pkgTitle->setText(title2);
}

QPair<QString, QString> InputItem::getInformation()
{
    return QPair<QString, QString>(m_appEdit->text(), m_pkgEdit->text());
}

void InputItem::clear()
{
    m_appEdit->clear();
    m_pkgEdit->clear();
}
