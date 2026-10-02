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

#include "gamelistitem.h"

#include <QMessageBox>
#include <QDir>

GameListItem::GameListItem(QWidget *parent, QString pkgName, QString appName) :
    QWidget(parent)
    , m_pkgName(pkgName)
    , m_appName(appName)
    , m_mainLyout(new QHBoxLayout(this))
    , m_pkgLabel(new QLabel(this))
    , m_appLabel(new QLabel(this))
    , m_clearBtn(new QPushButton(this))
{
    m_item = new QListWidgetItem();
    m_clearBtn->setFixedSize(70, 32);
    m_clearBtn->setFocusPolicy(Qt::NoFocus);
    m_clearBtn->setVisible(false);

    m_appLabel->setFixedWidth(150);
    m_appLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    m_pkgLabel->setFixedWidth(240);
    m_pkgLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_pkgLabel->setWordWrap(true);

    m_mainLyout->setMargin(0);
    m_mainLyout->setSpacing(5);
    m_mainLyout->setAlignment(Qt::AlignLeft);
    m_mainLyout->addWidget(m_appLabel);
    m_mainLyout->addWidget(m_pkgLabel);
    m_mainLyout->addStretch();
//    m_mainLyout->addWidget(m_clearBtn);

    QFont ft;
    QFontMetrics fm(ft);
    QString elided_text = fm.elidedText(m_appName, Qt::ElideMiddle, this->m_appLabel->maximumWidth());
    this->m_appLabel->setText(elided_text);

    QString pkg_elided_text = fm.elidedText(m_pkgName, Qt::ElideMiddle, this->m_pkgLabel->maximumWidth());
    this->m_pkgLabel->setText(pkg_elided_text);

}

GameListItem::~GameListItem()
{
    QLayoutItem *child;
    while ((child = m_mainLyout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }
}

QListWidgetItem* GameListItem::getItem()
{
    return m_item;
}

void GameListItem::onRestoreBtnStatus(const QString &pkgName)
{
    if (m_pkgName == pkgName) {
        m_clearBtn->setEnabled(true);
        m_clearBtn->setText(tr("Clear"));
    }
}
