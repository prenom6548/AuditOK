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
#include "installlanguagefeaturesdialog.h"

#include <QCoreApplication>
#include <QShowEvent>
#include <kysdk/applications/accessinfohelper.h>

// 构造语言特性安装弹窗
InstallLanguageFeaturesDialog::InstallLanguageFeaturesDialog(QWidget *parent) :
    KDialog(parent)
{
    initUi();
    initConnect();
}

// 析构语言特性安装弹窗
InstallLanguageFeaturesDialog::~InstallLanguageFeaturesDialog()
{
}

// 初始化语言特性安装弹窗界面
void InstallLanguageFeaturesDialog::initUi()
{
    setWindowIcon("ukui-control-center");
    setWindowTitle(tr("Install Language Features"));
    setWindowModality(Qt::ApplicationModal);
    setFixedSize(424, 520);

    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setSpacing(8);
    m_mainLayout->setContentsMargins(24, 6, 24, 24);

    m_installLanguageTitleLabel = new KLabel(this);
    m_installLanguageTitleLabel->setText(tr("Install Language"));

    m_installLanguageNameLabel = new KLabel(this);
    m_installLanguageNameLabel->setWordWrap(true);

    m_optionalFeatureTitleLabel = new KLabel(this);
    m_optionalFeatureTitleLabel->setText(tr("Optional Language Features"));

    m_setCurrentLanguageCheckBox = new QCheckBox(tr("Set as current system language"), this);
    m_languageFontCheckBox = new QCheckBox(tr("Corresponding language system font"), this);
    m_languageFontCheckBox->setChecked(true);
    m_inputMethodCheckBox = new QCheckBox(tr("Basic input method"), this);
    m_inputMethodCheckBox->setChecked(true);

    QVBoxLayout *featureLayout = new QVBoxLayout();
    featureLayout->setSpacing(4);
    featureLayout->setContentsMargins(0, 0, 0, 0);
    featureLayout->addWidget(m_setCurrentLanguageCheckBox);
    featureLayout->addWidget(m_languageFontCheckBox);
    featureLayout->addWidget(m_inputMethodCheckBox);

    m_cancelButton = new KPushButton(this);
    m_cancelButton->setText(QCoreApplication::translate("AddLanguageDialog", "No"));
    m_cancelButton->setMinimumSize(96, 36);
    m_cancelButton->setMaximumWidth(96);

    m_confirmButton = new KPushButton(this);
    m_confirmButton->setText(QCoreApplication::translate("AddLanguageDialog", "Yes"));
    m_confirmButton->setMinimumSize(96, 36);
    m_confirmButton->setMaximumWidth(96);
    m_confirmButton->setBackgroundColorHighlight(true);
    m_confirmButton->setDefault(true);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(16);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelButton);
    buttonLayout->addWidget(m_confirmButton);

    m_mainLayout->addWidget(m_installLanguageTitleLabel);
    m_mainLayout->addWidget(m_installLanguageNameLabel);
    m_mainLayout->addSpacing(8);
    m_mainLayout->addWidget(m_optionalFeatureTitleLabel);
    m_mainLayout->addLayout(featureLayout);
    m_mainLayout->addStretch();
    m_mainLayout->addLayout(buttonLayout);

    mainWidget()->setLayout(m_mainLayout);

    KDK_EXTEND_ALL_INFO_FORMAT(m_installLanguageTitleLabel, "Area", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_installLanguageNameLabel, "Area", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_optionalFeatureTitleLabel, "Area", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_setCurrentLanguageCheckBox, "Area", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_languageFontCheckBox, "Area", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_inputMethodCheckBox, "Area", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_cancelButton, "Area", "", "");
    KDK_EXTEND_ALL_INFO_FORMAT(m_confirmButton, "Area", "", "");

    m_setCurrentLanguageCheckBox->setAccessibleName(tr("Set as current system language"));
    m_languageFontCheckBox->setAccessibleName(tr("Corresponding language system font"));
    m_inputMethodCheckBox->setAccessibleName(tr("Basic input method"));
    m_cancelButton->setAccessibleName(QCoreApplication::translate("AddLanguageDialog", "No"));
    m_confirmButton->setAccessibleName(QCoreApplication::translate("AddLanguageDialog", "Yes"));
}

// 初始化语言特性安装弹窗信号连接
void InstallLanguageFeaturesDialog::initConnect()
{
    connect(m_confirmButton, &KPushButton::clicked, this, [=]() {
        InstallLanguageResult result;
        result.languageCode = m_languageCode;
        result.languageName = m_languageName;
        result.setCurrentLanguage = m_setCurrentLanguageCheckBox->isChecked();
        result.installLanguageFont = m_languageFontCheckBox->isChecked();
        result.installInputMethod = m_inputMethodCheckBox->isChecked();
        result.inputMethodId = m_option.inputMethodId;
        result.inputMethodPackageName = m_option.inputMethodPackageName;
        result.fontPackageName = m_option.fontPackageName;
        Q_EMIT installConfirmed(result);
        hide();
    });

    connect(m_cancelButton, &KPushButton::clicked, this, [=]() {
        hide();
    });
}

// 设置待安装特性的语言信息
void InstallLanguageFeaturesDialog::setupLanguage(const QString &languageCode,
                                                  const QString &languageName)
{
    m_languageCode = languageCode;
    m_languageName = languageName;
    m_option = LanguageFeatureConfig::optionForLanguage(languageCode);
    m_installLanguageNameLabel->setText(languageName);
    resetCheckboxes(m_option);
}

// 按配置重置特性复选框
void InstallLanguageFeaturesDialog::resetCheckboxes(const LanguageFeatureOption &option)
{
    m_setCurrentLanguageCheckBox->setVisible(option.showSetCurrentLanguage);
    m_languageFontCheckBox->setVisible(option.showLanguageFont);
    m_inputMethodCheckBox->setVisible(option.showInputMethod);

    m_setCurrentLanguageCheckBox->setChecked(option.checkSetCurrentLanguage);
    m_languageFontCheckBox->setChecked(option.checkLanguageFont);
    m_inputMethodCheckBox->setChecked(option.checkInputMethod);

    if (option.showInputMethod) {
        QString inputMethodText = tr("Basic input method");
        if (!option.inputMethodName.isEmpty()) {
            inputMethodText = option.inputMethodName;
        }
        m_inputMethodCheckBox->setText(inputMethodText);
    }

    const bool hasOptionalFeature = option.showSetCurrentLanguage
            || option.showLanguageFont
            || option.showInputMethod;
    m_optionalFeatureTitleLabel->setVisible(hasOptionalFeature);
}

// 弹窗显示时刷新复选框状态
void InstallLanguageFeaturesDialog::showEvent(QShowEvent *event)
{
    if (!m_languageCode.isEmpty()) {
        resetCheckboxes(m_option);
    }
    KDialog::showEvent(event);
}
