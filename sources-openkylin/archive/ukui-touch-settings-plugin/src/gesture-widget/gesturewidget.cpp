/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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

#include "gesturewidget.h"
#include "textlabel.h"
#include "playiconlabel.h"

#include <QString>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPixmap>
#include <QIcon>
#include <QFocusEvent>
#include <QPalette>
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include <QGSettings>


static const char* VIDEO_ICON_NAME = "folder-videos-symbolic";
static const char* UKUI_STYLE_SCHEMAS = "org.ukui.style";

static QGSettings* globalStyleGSettings() {
    if (!QGSettings::isSchemaInstalled(UKUI_STYLE_SCHEMAS)) {
        qWarning() << "Don't install schema" << UKUI_STYLE_SCHEMAS;
        return nullptr;
    }

    static QGSettings s_styleGSettings(UKUI_STYLE_SCHEMAS);
    return &s_styleGSettings;
}

GestureWidget::GestureWidget(QWidget *parent)
    : QPushButton(parent)
{
    initProperty();

    initUI();

    initConnections();
}

void GestureWidget::initUI()
{
    m_gestureDescription = new TextLabel(this);
    m_gestureAction = new TextLabel(this);
    updateLabelTextColor(false);
    m_playIcon = new PlayIconLabel(VIDEO_ICON_NAME, getCurrentStyleName(), this);

    QHBoxLayout* gestureDescriptionLayout = createGestureNameLayout();
    QVBoxLayout* mainLayout = createMainLayout();
    mainLayout->addLayout(gestureDescriptionLayout);
    mainLayout->addWidget(m_gestureAction);
    setLayout(mainLayout);
}

void GestureWidget::setGestureDescription(const QString &text)
{
    m_gestureDescription->setText(text);
    m_gestureDescription->setRealText(text);
    updateActionMaxWidth();
}

void GestureWidget::setGestureAction(const QString &text)
{
    m_gestureAction->setText(text);
    m_gestureAction->setRealText(text);
}

void GestureWidget::focusOutEvent(QFocusEvent *e)
{
    if(e->reason() == Qt::FocusReason::MouseFocusReason) {
        emit loseFocus();
    }

    QPushButton::focusOutEvent(e);
}

void GestureWidget::updateLabelTextColor(bool checked)
{
    //! \todo Encapsulate all the logic instead of using 'setStyleSheet'

    QPalette descriptionPal = m_gestureDescription->palette();
    QPalette actionPal = m_gestureAction->palette();
    if (checked == true) {
        descriptionPal.setColor(QPalette::Text, qApp->property("kfont-white").value<QBrush>().color());
        actionPal.setColor(QPalette::Text, qApp->property("kfont-white-secondary").value<QBrush>().color());
    } else {
        descriptionPal.setColor(QPalette::Text, qApp->property("kfont-primary").value<QBrush>().color());
        actionPal.setColor(QPalette::Text, qApp->property("kfont-secondary").value<QBrush>().color());
    }
    m_gestureDescription->setPalette(descriptionPal);
    m_gestureAction->setPalette(actionPal);
}

void GestureWidget::updateActionMaxWidth()
{
    QFontMetrics metrics(qApp->font());
    int textWidth = metrics.width(m_gestureDescription->realText());
    m_gestureDescription->setMaximumWidth(textWidth);
    m_gestureDescription->updateElidedText();
}

void GestureWidget::initProperty()
{
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    setCheckable(true);
    setAutoExclusive(true);
    setProperty("useButtonPalette", true);
    //统一使用主题的处理机制
    setFlat(true);
}

void GestureWidget::initConnections()
{
    //由于button中增加的QLabel没有办法实现反白效果，需要自己单独处理设置颜色
    connect(this, &GestureWidget::toggled, this, &GestureWidget::updateLabelTextColor);
    connect(this, &GestureWidget::toggled, m_playIcon, &PlayIconLabel::onToggled);

    connect(qApp, &QApplication::fontChanged, this, [this](){
        updateActionMaxWidth();
        m_gestureAction->updateElidedText();
    });

    initStyleConnection();
}

void GestureWidget::initStyleConnection()
{
    connect(globalStyleGSettings(), &QGSettings::changed, this, [this](const QString& key) {
        if (key == "styleName") {
            onStyleChanged();
        }
        else if (key == "iconThemeName") {
            onIconThemeChanged();
        }
    });
}

void GestureWidget::onStyleChanged()
{
    m_playIcon->onStyleChanged(getCurrentStyleName());
    updateLabelTextColor(isChecked());
}

void GestureWidget::onIconThemeChanged()
{
    m_playIcon->onIconThemeChanged();
}

QString GestureWidget::getCurrentStyleName() const
{
    QGSettings* styleGSettings = globalStyleGSettings();
    if (!styleGSettings)
        return QString();

    return styleGSettings->get("styleName").toString();
}

QHBoxLayout* GestureWidget::createGestureNameLayout()
{
    Q_ASSERT(m_playIcon);
    Q_ASSERT(m_gestureDescription);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setContentsMargins(0, 0, 0, 0);
    hlayout->setSpacing(8);
    hlayout->addWidget(m_gestureDescription);
    hlayout->addWidget(m_playIcon, 0, Qt::AlignVCenter);

    return hlayout;
}

QVBoxLayout* GestureWidget::createMainLayout()
{
    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(16, 10, 16, 7);
    vlayout->setSpacing(0);

    return vlayout;
}

QSize GestureWidget::sizeHint() const
{
    Q_ASSERT(m_gestureAction);
    Q_ASSERT(m_gestureDescription);

    int width = QPushButton::sizeHint().width();
    int height = m_gestureAction->sizeHint().height()
                 + m_gestureDescription->sizeHint().height()
                 + layout()->margin()
                 + layout()->contentsMargins().bottom()
                 + layout()->contentsMargins().top();

    return QSize(width, height);
}

QSize GestureWidget::minimumSizeHint() const
{
    QSize sizeHint = this->sizeHint();
    sizeHint.setHeight(qMax(sizeHint.height(), 64));

    return sizeHint;
}
