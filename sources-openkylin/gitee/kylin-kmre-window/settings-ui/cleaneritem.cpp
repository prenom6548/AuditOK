/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
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

#include "cleaneritem.h"
#include "global.h"
#include "messagebox.h"

#include <QJsonObject>
#include <QEvent>
#include <QPainter>
#include <QPushButton>
#include <QMessageBox>

using namespace Global;
CleanerItem::CleanerItem(const QJsonObject &value, const QString &currentImage, const QString &container, QWidget *parent) : QWidget(parent),
    m_active(false),
    m_iconLabel(new QLabel),
//    m_idLabel(new QLabel),
    m_nameLabel(new QLabel),
    m_timeLabel(new QLabel),
    m_szLabel(new QLabel),
//    m_containerLabel(new QLabel),
    m_tipLabel(new QLabel),
    m_removeButton(new QPushButton(this)),
    m_mainLayout(new QHBoxLayout),
    referencedContainer(container)
{
    m_item = new QListWidgetItem();
    m_iconLabel->setMargin(0);
//    m_idLabel->setMargin(0);
    m_nameLabel->setMargin(0);
    m_timeLabel->setMargin(0);
    m_szLabel->setMargin(0);
//    m_containerLabel->setMargin(0);
    m_tipLabel->setMargin(0);

    m_iconLabel->setFixedSize(16, 16);
//    m_idLabel->setFixedWidth(100);
    m_nameLabel->setFixedWidth(180);
    m_timeLabel->setFixedWidth(100);
    m_szLabel->setFixedWidth(50);
//    m_containerLabel->setFixedWidth(100);
    m_removeButton->setFixedSize(36, 36);

//    m_idLabel->setStyleSheet("font-size: 12px;font-style: normal;color: rgba(0, 0, 0, 0.6);");

//    this->setCustomFont(m_nameLabel, m_idLabel->font());//font()
//    this->setCustomFont(m_timeLabel, m_idLabel->font());
//    this->setCustomFont(m_szLabel, m_idLabel->font());
//    this->setCustomFont(m_containerLabel, m_idLabel->font());

    m_iconLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    m_iconLabel->setAutoFillBackground(false);
//    m_idLabel->setAttribute(Qt::WA_TranslucentBackground, true);
//    m_idLabel->setAutoFillBackground(false);
    m_nameLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    m_nameLabel->setAutoFillBackground(false);
    m_timeLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    m_timeLabel->setAutoFillBackground(false);
//    m_containerLabel->setAttribute(Qt::WA_TranslucentBackground, true);
//    m_containerLabel->setAutoFillBackground(false);
    m_szLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    m_szLabel->setAutoFillBackground(false);
    m_tipLabel->setAttribute(Qt::WA_TranslucentBackground, true);
    m_tipLabel->setAutoFillBackground(false);

//    m_removeButton->setAttribute(Qt::WA_TranslucentBackground, true);
//    m_removeButton->setAutoFillBackground(false);
    m_removeButton->setFocusPolicy(Qt::NoFocus);
    m_removeButton->setIcon(QIcon(":/res/delete.svg"));
    m_removeButton->setProperty("useIconHighligntEffect", 0x8);
    m_removeButton->setProperty("isWindowButton", 0x02);
    m_removeButton->setVisible(false);
    m_tipLabel->setVisible(false);

    QHBoxLayout *hboxlayout = new QHBoxLayout;
    hboxlayout->setMargin(0);
    hboxlayout->setSpacing(20);
//    hboxlayout->addWidget(m_idLabel, 0, Qt::AlignLeft);
    hboxlayout->addWidget(m_nameLabel, 0, Qt::AlignLeft);
//    hboxlayout->addStretch();
    hboxlayout->addWidget(m_szLabel, 0, Qt::AlignLeft);
    hboxlayout->addWidget(m_timeLabel, 0, Qt::AlignLeft);
//    hboxlayout->addWidget(m_containerLabel, 0, Qt::AlignRight);
    hboxlayout->addWidget(m_tipLabel, 0, Qt::AlignRight);
//    hboxlayout->addSpacing(20);
//    hboxlayout->addWidget(m_removeButton, 0, Qt::AlignRight);
//    hboxlayout->addSpacing(5);
//    hboxlayout->setContentsMargins(0, 0, 5, 0);

    m_mainLayout->setContentsMargins(0, 0, 10, 0);
    m_mainLayout->setSpacing(5);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
    m_mainLayout->addLayout(hboxlayout);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_removeButton, 0, Qt::AlignRight);
    this->setLayout(m_mainLayout);

    connect(m_removeButton, &QPushButton::clicked, [=] {
        if (KylinUI::MessageBox::question(this, tr("KMRE"), tr("After the image is deleted, KMRE will no longer be able to switch to the image. Are you sure you want to perform this operation?"))) {
            emit requestRemove(m_imageid, referencedContainer, m_imageName);
            emit itemRemove(m_item);
        }
    });

    this->setActiveStatus(false);
    if (value.contains("id")) {
        setId(value["id"].toString());
    }
    if (value.contains("name")) {
        setName(value["name"].toString());
        if (currentImage == value["name"].toString()) {
            this->setActiveStatus(true);
        }
    }
    if (value.contains("size")) {
        setSize(value["size"].toString());
    }
    if (value.contains("created")) {
        setCreatedTime(value["created"].toString());
    }

//    if (!container.isEmpty()) {
//        this->setContainerName(container);
//    }
}

void CleanerItem::freeItem()
{
    m_animation = new QPropertyAnimation(this, "pos",this);
    m_animation->setDuration(300);
    m_animation->setStartValue(QPoint(this->x(), this->y()));
    m_animation->setEndValue(QPoint(this->width(), this->y()));
    m_animation->setEasingCurve(QEasingCurve::InOutCubic);
    m_animation->start();

    connect(m_animation, &QPropertyAnimation::finished, [=] {
        this->deleteLater();
    });
}

void CleanerItem::setCustomFont(QLabel *label, QFont referenceFont)
{
    QFont font(referenceFont);
    font.setPixelSize(11);
    label->setFont(font);
}

void CleanerItem::setIcon(const QString &iconName)
{
    //const QIcon &icon = QIcon::fromTheme(iconName, QIcon::fromTheme("application-x-desktop"));
    const QIcon &icon = QIcon(iconName);
    if (!icon.isNull()) {
        m_iconLabel->setPixmap(icon.pixmap(16, 16));
    }
}

void CleanerItem::setId(const QString &id)
{
    m_imageid = id;
//    QFontMetrics fm(m_idLabel->font());
//    QString text = fm.elidedText(id, Qt::ElideMiddle, m_idLabel->width());
//    m_idLabel->setText(text);
}

void CleanerItem::setName(const QString &name)
{
    m_imageName = name;
    QFontMetrics fm(m_nameLabel->font());
    QString text = fm.elidedText(name, Qt::ElideMiddle, m_nameLabel->width());
    m_nameLabel->setText(text);
}

void CleanerItem::setCreatedTime(const QString &time)
{
    QFontMetrics fm(m_timeLabel->font());
    QString text = fm.elidedText(time, Qt::ElideRight, m_timeLabel->width());
    m_timeLabel->setText(text);
}

void CleanerItem::setContainerName(const QString &name)
{
//    QFontMetrics fm(m_containerLabel->font());
//    QString text = fm.elidedText(name, Qt::ElideMiddle, m_containerLabel->width());
//    m_containerLabel->setText(text);
}

void CleanerItem::setActiveStatus(bool b)
{
    m_active = b;
    if (b) {
        this->setIcon(":/res/active.png");
    }
    else {
        this->setIcon(":/res/about_normal.png");
    }
    m_tipLabel->setText(b ? tr("Currently configured") : tr("Idle"));//m_tipLabel->setText(b ? tr("active") : tr("inactive"));
}

void CleanerItem::setSize(const QString &sz)
{
    m_szLabel->setText(sz);
}

void CleanerItem::onRemove()
{
    if (!m_active) {
        emit requestRemove(m_imageid, referencedContainer, m_imageName);//m_removeButton->clicked();
    }
}

QListWidgetItem* CleanerItem::getItem()
{
    return m_item;
}

void CleanerItem::enterEvent(QEvent *)
{
    if (!m_active) {
        m_removeButton->setVisible(true);
    }
    m_removeButton->setVisible(true);
    m_timeLabel->setVisible(false);
    m_tipLabel->setVisible(true);
}

void CleanerItem::leaveEvent(QEvent *)
{
    m_removeButton->setVisible(false);
    m_timeLabel->setVisible(true);
    m_tipLabel->setVisible(false);
}

void CleanerItem::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
