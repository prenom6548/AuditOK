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

#include "appitem.h"
#include "switchbutton.h"
#include "utils.h"

#include <QWidget>
#include <QEvent>
#include <QPainter>
#include <QFile>

using namespace kmre;
static QColor getBlendColor(const QColor &color1, const QColor &color2)
{
    QColor rgbColor1 = color1.toRgb();
    QColor rgbColor2 = color2.toRgb();

    if (rgbColor2.alpha() >= 255) {
        return rgbColor2;
    }

    qreal ratio = 1 - rgbColor2.alphaF();
    int r = ratio * rgbColor1.red() + rgbColor2.alphaF() * rgbColor2.red();
    int g = ratio * rgbColor1.green() + rgbColor2.alphaF() * rgbColor2.green();
    int b = ratio * rgbColor1.blue() + rgbColor2.alphaF() * rgbColor2.blue();

    return QColor(r, g, b);
}

AppItem::AppItem(QWidget *parent) : QWidget(parent)
    , m_isEntered(false)
{
    this->installEventFilter(this);
    this->setMouseTracking(true);
    this->setFixedHeight(50);

    m_item = new QListWidgetItem();
    m_layout = new QHBoxLayout();
    m_infoLayout = new QHBoxLayout();
    m_btnLayout = new QHBoxLayout();

    m_iconLabel = new QLabel();
    m_appNameLabel = new QLabel();
    m_switchBtn = new SwitchButton();

    m_iconLabel->setFixedSize(36,36);
    m_appNameLabel->setFixedWidth(300);

    m_infoLayout->addWidget(m_iconLabel);
    m_infoLayout->addSpacing(16);
    m_infoLayout->addWidget(m_appNameLabel);
//    m_infoLayout->addWidget(m_pkgNameLabel);
    m_btnLayout->addWidget(m_switchBtn);

    m_layout->addLayout(m_infoLayout);
    m_layout->addStretch();
    m_layout->addLayout(m_btnLayout);
    m_layout->setContentsMargins(16, 0, 16, 0);
    this->setLayout(m_layout);

    //connect(m_switchBtn, &SwitchButton::checkedChanged, this, &TrayAppItem::checkedChanged);
    connect(m_switchBtn, &SwitchButton::checkedChanged, this, [=](const bool checked) {
        emit this->checkedChanged(m_pkgName, checked);
    });
}

void AppItem::setInfo(const QString &appName, const QString &pkgName, bool checked)
{
    QString iconPath = kmre::utils::getIconPath(pkgName);
    if (!QFile::exists(iconPath)) {
        m_iconLabel->setPixmap(QPixmap(":/res/kmre.svg").scaled(m_iconLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else {
        m_iconLabel->setPixmap(QPixmap(iconPath).scaled(m_iconLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    m_pkgName = pkgName;
    QFont ft;
    QFontMetrics fm(ft);
    QString elided_text = fm.elidedText(appName, Qt::ElideMiddle, m_appNameLabel->maximumWidth());
    m_appNameLabel->setText(elided_text);

//    QString pkg_elided_text = fm.elidedText(pkgName, Qt::ElideMiddle, m_pkgNameLabel->maximumWidth());
//    m_pkgNameLabel->setText(pkg_elided_text);
    this->setChecked(checked);
}

QString AppItem::getPkgName()
{
    return m_pkgName;
}

QListWidgetItem* AppItem::getItem()
{
    return m_item;
}

void AppItem::setChecked(const bool checked)
{
    m_switchBtn->blockSignals(true);
    m_switchBtn->setChecked(checked);
    m_switchBtn->blockSignals(false);
}

void AppItem::highlight()
{
    m_isEntered = true;
    repaint();
}

void AppItem::unhighlight()
{
    m_isEntered = false;
    repaint();
}

void AppItem::enterEvent(QEvent *event)
{
    emit enter();

    QWidget::enterEvent(event);
}

void AppItem::paintEvent(QPaintEvent *event)
{
    if (m_isEntered) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QPainterPath path;
        path.addRoundedRect(QRectF(rect()), 6, 6);
        painter.fillPath(path, getBlendColor(window()->palette().color(QPalette::Background), QColor(0, 0, 0, 0.05 * 255)));
    }

    QWidget::paintEvent(event);
}
