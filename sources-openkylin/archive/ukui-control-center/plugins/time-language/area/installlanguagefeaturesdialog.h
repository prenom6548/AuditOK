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
#ifndef INSTALLLANGUAGEFEATURESDIALOG_H
#define INSTALLLANGUAGEFEATURESDIALOG_H

#include <QCheckBox>
#include <QVBoxLayout>
#include "kdialog.h"
#include "klabel.h"
#include "kpushbutton.h"
#include "languagefeatureconfig.h"

using namespace kdk;

struct InstallLanguageResult {
    QString languageCode;
    QString languageName;
    bool setCurrentLanguage = false;
    bool installLanguageFont = false;
    bool installInputMethod = false;
    QString inputMethodId;
    QString inputMethodPackageName;
    QString fontPackageName;
};

class InstallLanguageFeaturesDialog : public KDialog
{
    Q_OBJECT

public:
    // 构造语言特性安装弹窗
    explicit InstallLanguageFeaturesDialog(QWidget *parent = nullptr);
    // 析构语言特性安装弹窗
    ~InstallLanguageFeaturesDialog();

    // 设置待安装特性的语言信息
    void setupLanguage(const QString &languageCode, const QString &languageName);

Q_SIGNALS:
    // 发送语言特性安装确认结果
    void installConfirmed(const InstallLanguageResult &result);

protected:
    // 弹窗显示时刷新复选框状态
    void showEvent(QShowEvent *event) override;

private:
    // 初始化语言特性安装弹窗界面
    void initUi();
    // 初始化语言特性安装弹窗信号连接
    void initConnect();
    // 按配置重置特性复选框
    void resetCheckboxes(const LanguageFeatureOption &option);

    QVBoxLayout *m_mainLayout = nullptr;
    KLabel *m_installLanguageTitleLabel = nullptr;
    KLabel *m_installLanguageNameLabel = nullptr;
    KLabel *m_optionalFeatureTitleLabel = nullptr;
    QCheckBox *m_setCurrentLanguageCheckBox = nullptr;
    QCheckBox *m_languageFontCheckBox = nullptr;
    QCheckBox *m_inputMethodCheckBox = nullptr;
    KPushButton *m_cancelButton = nullptr;
    KPushButton *m_confirmButton = nullptr;
    QString m_languageCode;
    QString m_languageName;
    LanguageFeatureOption m_option;
};

#endif // INSTALLLANGUAGEFEATURESDIALOG_H
