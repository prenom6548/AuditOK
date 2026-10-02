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
#include "languagefeatureitem.h"
#include "languageframe.h"

#include "fixlabel.h"

#include <QIcon>
#include <QLabel>
#include <QHBoxLayout>
#include <kysdk/applications/accessinfohelper.h>

namespace {
constexpr int kRowHorizontalSpacing = 24;
constexpr int kRowSideMargin = 16;
constexpr int kProgressPercentWidth = 40;
constexpr int kFeatureItemHeight = 48;
}

// 构造语言特性展示项
LanguageFeatureItem::LanguageFeatureItem(LanguageFeatureType type,
                                         const QString &featureName,
                                         LanguageFeatureState state,
                                         QWidget *parent)
    : UkccFrame(parent, BorderRadiusStyle::None, false)
    , m_type(type)
    , m_state(state)
{
    setMinimumHeight(kFeatureItemHeight);
    setMaximumHeight(kFeatureItemHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(kRowSideMargin, 0, kRowSideMargin, 0);
    layout->setSpacing(kRowHorizontalSpacing);

    m_nameLabel = new FixLabel(featureName, this);
    m_nameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    KDK_EXTEND_ALL_INFO_FORMAT(m_nameLabel, "Area", "", "");

    m_rightColumn = new QWidget(this);
    m_rightColumn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QHBoxLayout *rightLayout = new QHBoxLayout(m_rightColumn);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(8);

    m_installingWidget = new QWidget(m_rightColumn);
    m_installingWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QHBoxLayout *installingLayout = new QHBoxLayout(m_installingWidget);
    installingLayout->setContentsMargins(0, 0, 0, 0);
    installingLayout->setSpacing(8);
    m_progressBar = new KProgressBar(m_installingWidget);
    m_progressBar->setFixedHeight(8);
    m_progressBar->setTextVisible(false);
    m_progressBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_progressLabel = new QLabel(m_installingWidget);
    m_progressLabel->setFixedWidth(kProgressPercentWidth);
    m_progressLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    installingLayout->addWidget(m_progressBar, 1);
    installingLayout->addWidget(m_progressLabel, 0);

    m_statusLabel = new LightLabel(m_rightColumn);
    m_statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_errorWidget = new QWidget(m_rightColumn);
    m_errorWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QHBoxLayout *errorLayout = new QHBoxLayout(m_errorWidget);
    errorLayout->setContentsMargins(0, 0, 0, 0);
    errorLayout->setSpacing(8);

    m_retryButton = new QToolButton(m_errorWidget);
    m_retryButton->setAutoRaise(true);
    m_retryButton->setIcon(QIcon::fromTheme("view-refresh-symbolic"));
    m_retryButton->setProperty("useButtonPalette", true);
    m_retryButton->setToolTip(tr("Retry"));
    KDK_EXTEND_ALL_INFO_FORMAT(m_retryButton, "Area", "", "");
    connect(m_retryButton, &QToolButton::clicked, this, [=]() {
        Q_EMIT installRequested(m_type);
    });

    m_errorLabel = new LightLabel(m_errorWidget);
    m_errorLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    errorLayout->addStretch();
    errorLayout->addWidget(m_retryButton, 0, Qt::AlignVCenter);
    errorLayout->addWidget(m_errorLabel, 0, Qt::AlignRight | Qt::AlignVCenter);

    m_installButton = new KPushButton(m_rightColumn);
    m_installButton->setText(tr("Install"));
    m_installButton->setFixedSize(72, 28);
    KDK_EXTEND_ALL_INFO_FORMAT(m_installButton, "Area", "", "");
    connect(m_installButton, &KPushButton::clicked, this, [=]() {
        Q_EMIT installRequested(m_type);
    });

    m_installButtonContainer = new QWidget(m_rightColumn);
    m_installButtonContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QHBoxLayout *buttonLayout = new QHBoxLayout(m_installButtonContainer);
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->setSpacing(0);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_installButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    rightLayout->addWidget(m_installingWidget, 1);
    rightLayout->addWidget(m_statusLabel, 1);
    rightLayout->addWidget(m_errorWidget, 1);
    rightLayout->addWidget(m_installButtonContainer, 1);

    layout->addWidget(m_nameLabel, 1);
    layout->addWidget(m_rightColumn, 1);

    updateUi();
    setState(state);
}

// 获取语言特性类型
LanguageFeatureType LanguageFeatureItem::featureType() const
{
    return m_type;
}

// 获取语言特性安装状态
LanguageFeatureState LanguageFeatureItem::state() const
{
    return m_state;
}

// 设置语言特性安装状态
void LanguageFeatureItem::setState(LanguageFeatureState state, int progress, const QString &errorText)
{
    const LanguageFeatureState previousState = m_state;
    m_state = state;
    if (state == LanguageFeatureState::Installing) {
        m_progressBar->setValue(progress);
        m_progressLabel->setText(QString("%1%").arg(progress));
    } else if (state == LanguageFeatureState::Installed) {
        m_statusLabel->setText(tr("Installed"));
    } else if (state == LanguageFeatureState::Error) {
        m_errorLabel->setText(errorText.isEmpty()
                              ? tr("Installation failed. Please retry later.")
                              : errorText);
    }
    updateUi();
    if (previousState != state) {
        updateFeatureLayout();
    }
}

// 刷新语言特性项布局
void LanguageFeatureItem::updateFeatureLayout()
{
    if (QLayout *itemLayout = layout()) {
        itemLayout->invalidate();
    }
    updateGeometry();
    for (QWidget *w = parentWidget(); w; w = w->parentWidget()) {
        if (LanguageFrame *frame = qobject_cast<LanguageFrame *>(w)) {
            frame->updateLayoutHeight();
            break;
        }
    }
}

// 按安装状态刷新控件显示
void LanguageFeatureItem::updateUi()
{
    m_installingWidget->setVisible(m_state == LanguageFeatureState::Installing);
    m_installButtonContainer->setVisible(m_state == LanguageFeatureState::NotInstalled);
    m_statusLabel->setVisible(m_state == LanguageFeatureState::Installed);
    m_errorWidget->setVisible(m_state == LanguageFeatureState::Error);
    m_nameLabel->setVisible(true);
}
