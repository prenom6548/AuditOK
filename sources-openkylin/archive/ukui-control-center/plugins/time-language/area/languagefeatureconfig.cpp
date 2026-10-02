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
#include "languagefeatureconfig.h"

#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QDebug>

namespace {
const char *kConfigPath = "/etc/ukui-i18n-management/language-feature-config.json";

// 解析单个语言特性配置项
LanguageFeatureOption parseOption(const QJsonObject &object, const LanguageFeatureOption &fallback)
{
    LanguageFeatureOption option = fallback;
    if (object.contains("showSetCurrentLanguage")) {
        option.showSetCurrentLanguage = object.value("showSetCurrentLanguage").toBool();
    }
    if (object.contains("showLanguageFont")) {
        option.showLanguageFont = object.value("showLanguageFont").toBool();
    }
    if (object.contains("showInputMethod")) {
        option.showInputMethod = object.value("showInputMethod").toBool();
    }
    if (object.contains("checkSetCurrentLanguage")) {
        option.checkSetCurrentLanguage = object.value("checkSetCurrentLanguage").toBool();
    }
    if (object.contains("checkLanguageFont")) {
        option.checkLanguageFont = object.value("checkLanguageFont").toBool();
    }
    if (object.contains("checkInputMethod")) {
        option.checkInputMethod = object.value("checkInputMethod").toBool();
    }
    if (object.contains("inputMethodId")) {
        option.inputMethodId = object.value("inputMethodId").toString();
    }
    if (object.contains("inputMethodName")) {
        option.inputMethodName = object.value("inputMethodName").toString();
    }
    if (object.contains("inputMethodPackageName")) {
        option.inputMethodPackageName = object.value("inputMethodPackageName").toString();
    }
    if (object.contains("fontFamily")) {
        option.fontName = object.value("fontFamily").toString();
    }
    if (object.contains("fontName")) {
        option.fontName = object.value("fontName").toString();
    }
    if (object.contains("fontPath")) {
        option.fontPath = object.value("fontPath").toString();
    }
    if (object.contains("fontStyle")) {
        option.fontStyle = object.value("fontStyle").toString();
    }
    if (object.contains("fontPackageName")) {
        option.fontPackageName = object.value("fontPackageName").toString();
    }
    return option;
}

// 判断是否配置了字体特性
bool hasLanguageFontFeature(const LanguageFeatureOption &option)
{
    return !option.fontPackageName.trimmed().isEmpty()
            || !option.fontName.trimmed().isEmpty()
            || !option.fontPath.trimmed().isEmpty();
}

// 判断是否配置了输入法特性
bool hasInputMethodFeature(const LanguageFeatureOption &option)
{
    return !option.inputMethodPackageName.trimmed().isEmpty()
            || !option.inputMethodId.trimmed().isEmpty();
}

// 规范化语言特性显示和勾选状态
LanguageFeatureOption normalizeOption(LanguageFeatureOption option)
{
    option.showLanguageFont = option.showLanguageFont && hasLanguageFontFeature(option);
    option.checkLanguageFont = option.showLanguageFont && option.checkLanguageFont;
    option.showInputMethod = option.showInputMethod && hasInputMethodFeature(option);
    option.checkInputMethod = option.showInputMethod && option.checkInputMethod;
    return option;
}

// 获取内置默认配置
LanguageFeatureOption fallbackOption()
{
    LanguageFeatureOption option;
    option.showSetCurrentLanguage = true;
    option.showLanguageFont = false;
    option.showInputMethod = false;
    option.checkSetCurrentLanguage = false;
    option.checkLanguageFont = false;
    option.checkInputMethod = false;
    return option;
}
} // namespace

bool LanguageFeatureConfig::s_loaded = false;
static LanguageFeatureOption s_defaultOption;
static QJsonObject s_languageOptions;
static QDateTime s_lastModified;

// 重置为内置默认配置
static void resetToFallbackConfig()
{
    s_defaultOption = fallbackOption();
    s_languageOptions = QJsonObject();
    s_lastModified = QDateTime();
}

// 从文件加载语言特性配置
bool LanguageFeatureConfig::loadFromFile(const QString &filePath)
{
    const QFileInfo configInfo(filePath);
    if (!configInfo.exists()) {
        resetToFallbackConfig();
        return false;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << Q_FUNC_INFO << "failed to open config:" << filePath;
        resetToFallbackConfig();
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject()) {
        qWarning() << Q_FUNC_INFO << "invalid config format:" << filePath;
        resetToFallbackConfig();
        return false;
    }

    const QJsonObject root = document.object();
    LanguageFeatureOption defaultOption;
    QJsonObject languageOptions;
    if (root.contains("default") && root.value("default").isObject()) {
        defaultOption = parseOption(root.value("default").toObject(), LanguageFeatureOption());
    }
    if (root.contains("languages") && root.value("languages").isObject()) {
        languageOptions = root.value("languages").toObject();
    }

    s_defaultOption = defaultOption;
    s_languageOptions = languageOptions;
    s_lastModified = configInfo.lastModified();
    return true;
}

// 确保语言特性配置已加载
void LanguageFeatureConfig::ensureLoaded()
{
    const QString configPath = QString::fromUtf8(kConfigPath);
    const QDateTime lastModified = QFileInfo(configPath).lastModified();
    if (s_loaded
            && ((!lastModified.isValid() && !s_lastModified.isValid())
                || (lastModified.isValid() && lastModified == s_lastModified))) {
        return;
    }
    s_loaded = true;
    if (!loadFromFile(configPath)) {
        qInfo() << Q_FUNC_INFO << "use built-in default language feature config";
    }
}

// 合并默认配置和指定语言配置
LanguageFeatureOption LanguageFeatureConfig::mergeOption(const QString &languageCode)
{
    LanguageFeatureOption option = s_defaultOption;
    if (!s_languageOptions.contains(languageCode)) {
        return normalizeOption(option);
    }
    return normalizeOption(parseOption(s_languageOptions.value(languageCode).toObject(), option));
}

// 获取指定语言的特性配置
LanguageFeatureOption LanguageFeatureConfig::optionForLanguage(const QString &languageCode)
{
    ensureLoaded();
    return mergeOption(languageCode);
}
