/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "AppManagerSelectComboxItem.h"

#include <QApplication>
#include <QIcon>
#include <QGSettings>
#include <QDebug>

AppManagerSelectComboxItem::AppManagerSelectComboxItem(std::shared_ptr<ISelectBox> box, QLabel* label, QWidget* parent)
    : ISelectComboxItem(box, label),
      QFrame(parent)
{
    initUi();
    initSlots();
}

void AppManagerSelectComboxItem::initUi()
{
    setFixedSize(512, 60);
    setFrameShape(QFrame::Shape::Box);

    m_pDisplayLabel->setFixedSize(120, 36);
    m_pDisplayLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_pSelectBox->getWidget()->setFixedSize(347, 36);

    auto hLayout = new QHBoxLayout;
    hLayout->setSpacing(0);
    hLayout->setContentsMargins(0, 0, 0, 0);
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    hLayout->addWidget(m_pDisplayLabel);
    hLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));
    hLayout->addWidget(m_pSelectBox->getWidget());
    hLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));

    setLayout(hLayout);

    auto pal = palette();
    QColor color = qApp->property("kcontain-secondary-normal").value<QBrush>().color();
    pal.setBrush(QPalette::Base, color);
    setPalette(pal);
}

void AppManagerSelectComboxItem::initSlots()
{
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings* themeName = new QGSettings("org.ukui.style");
        QObject::connect(themeName, &QGSettings::changed, this, [this](const QString& key){
            if (key == "styleName" || key == "widgetThemeName" || key == "themeColor"
                    || key == "style-name" || key == "widget-theme-name" || key == "theme-color") {
                auto pal = palette();
                QColor color = qApp->property("kcontain-secondary-normal").value<QBrush>().color();
                pal.setBrush(QPalette::Base, color);
                setPalette(pal);
            }
        });
    }
}

void AppManagerSelectComboxItem::setEnabled(bool enable)
{
    m_pSelectBox->setEnabled(enable);
}

QWidget* AppManagerSelectComboxItem::getWidget()
{
    return this;
}

AppSelectComboxWithDescItem::AppSelectComboxWithDescItem(std::shared_ptr<ISelectBox> box, QLabel* label, QLabel* hintLabel, QWidget* parent)
    : ISelectComboxItem(box, label),
      m_pHintLabel(hintLabel),
      QFrame(parent)
{
    initUi();
    initSlots();
}

void AppSelectComboxWithDescItem::initUi()
{
    setFixedSize(512, 90);
    setFrameShape(QFrame::Shape::Box);

    m_pDisplayLabel->setFixedSize(120, 36);
    m_pDisplayLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_pSelectBox->getWidget()->setFixedSize(347, 36);
    m_pSelectBox->setEnabled(false);

    auto itemHLayout = new QHBoxLayout();
    itemHLayout->setSpacing(0);
    itemHLayout->setContentsMargins(0, 0, 0, 0);
    itemHLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));
    itemHLayout->addWidget(m_pDisplayLabel);
    itemHLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Fixed));
    itemHLayout->addWidget(m_pSelectBox->getWidget());
    itemHLayout->addItem(new QSpacerItem(16, 20, QSizePolicy::Fixed));

    m_pHintIconLabel = new QLabel;
    m_pHintIconLabel->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(16, 16)));

    m_pHintLabel->setFixedSize(325, 28);
    m_pHintLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    auto hintPl = m_pHintLabel->palette();
    QColor color = hintPl.color(QPalette::PlaceholderText);
    hintPl.setColor(QPalette::Text, color);
    m_pHintLabel->setPalette(hintPl);

    m_pHintHLayout = new QHBoxLayout();
    m_pHintHLayout->setSpacing(0);
    m_pHintHLayout->setContentsMargins(0, 0, 0, 0);
    m_pHintHLayout->addSpacerItem(new QSpacerItem(148, 10, QSizePolicy::Fixed));
    m_pHintHLayout->addWidget(m_pHintIconLabel);
    m_pHintHLayout->addSpacerItem(new QSpacerItem(3, 10, QSizePolicy::Fixed));
    m_pHintHLayout->addWidget(m_pHintLabel);
    m_pHintHLayout->addSpacerItem(new QSpacerItem(16, 10, QSizePolicy::Expanding));

    m_pVLayout = new QVBoxLayout();
    m_pVLayout->setSpacing(5);
    m_pVLayout->addLayout(itemHLayout);
    m_pVLayout->addLayout(m_pHintHLayout);
    m_pVLayout->setContentsMargins(0, 5, 0, 16);

    setLayout(m_pVLayout);

    auto pal = palette();
    QColor cl = qApp->property("kcontain-secondary-normal").value<QBrush>().color();
    pal.setBrush(QPalette::Base, cl);
    setPalette(pal);
}

void AppSelectComboxWithDescItem::initSlots()
{
    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings* themeName = new QGSettings("org.ukui.style");
        QObject::connect(themeName, &QGSettings::changed, this, [this](const QString& key){
            if (key == "styleName" || key == "widgetThemeName" || key == "themeColor"
                    || key == "style-name" || key == "widget-theme-name" || key == "theme-color") {
                auto pal = palette();
                QColor cl = qApp->property("kcontain-secondary-normal").value<QBrush>().color();
                pal.setBrush(QPalette::Base, cl);
                setPalette(pal);
            }
        });
    }
}

void AppSelectComboxWithDescItem::setEnabled(bool enable)
{
    if (!enable) {
        m_pVLayout->removeItem(m_pHintHLayout);
        m_pVLayout->addItem(m_pHintHLayout);
        setFixedHeight(90);
        m_pVLayout->setContentsMargins(0, 5, 0, 16);
    }
    else {
        m_pVLayout->removeItem(m_pHintHLayout);
        setFixedHeight(60);
        m_pVLayout->setContentsMargins(0, 12, 0, 16);
    }

    m_pSelectBox->setEnabled(enable);
    m_pHintLabel->setVisible(!enable);
    m_pHintIconLabel->setVisible(!enable);
    m_pVLayout->setSpacing(5);
    update();

    qDebug() << QString("Set Appliaction combox enable to %1").arg(enable);
}

QWidget* AppSelectComboxWithDescItem::getWidget()
{
    return this;
}

