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
#ifndef LANGUAGEFEATUREITEM_H
#define LANGUAGEFEATUREITEM_H

#include "ukccframe.h"
#include "fixlabel.h"
#include "kprogressbar.h"
#include "kpushbutton.h"
#include "lightlabel.h"

#include <QToolButton>

using namespace kdk;

enum class LanguageFeatureType {
    Font,
    InputMethod
};

enum class LanguageFeatureState {
    NotInstalled,
    Installing,
    Installed,
    Error
};

Q_DECLARE_METATYPE(LanguageFeatureType)

class LanguageFeatureItem : public UkccFrame
{
    Q_OBJECT

public:
    // 构造语言特性展示项
    explicit LanguageFeatureItem(LanguageFeatureType type,
                                 const QString &featureName,
                                 LanguageFeatureState state = LanguageFeatureState::Installing,
                                 QWidget *parent = nullptr);

    // 获取语言特性类型
    LanguageFeatureType featureType() const;
    // 获取语言特性安装状态
    LanguageFeatureState state() const;
    // 设置语言特性安装状态
    void setState(LanguageFeatureState state, int progress = 0, const QString &errorText = QString());

Q_SIGNALS:
    // 请求安装指定语言特性
    void installRequested(LanguageFeatureType type);

private:
    // 按安装状态刷新控件显示
    void updateUi();
    // 刷新语言特性项布局
    void updateFeatureLayout();

    LanguageFeatureType m_type;
    LanguageFeatureState m_state = LanguageFeatureState::Installing;
    FixLabel *m_nameLabel = nullptr;
    QWidget *m_rightColumn = nullptr;
    QWidget *m_installingWidget = nullptr;
    QWidget *m_installButtonContainer = nullptr;
    QWidget *m_errorWidget = nullptr;
    KProgressBar *m_progressBar = nullptr;
    QLabel *m_progressLabel = nullptr;
    QToolButton *m_retryButton = nullptr;
    LightLabel *m_statusLabel = nullptr;
    LightLabel *m_errorLabel = nullptr;
    KPushButton *m_installButton = nullptr;
};

#endif // LANGUAGEFEATUREITEM_H
