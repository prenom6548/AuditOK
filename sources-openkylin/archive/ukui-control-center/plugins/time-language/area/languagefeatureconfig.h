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
#ifndef LANGUAGEFEATURECONFIG_H
#define LANGUAGEFEATURECONFIG_H

#include <QString>

struct LanguageFeatureOption {
    bool showSetCurrentLanguage = true;
    bool showLanguageFont = false;
    bool showInputMethod = false;
    bool checkSetCurrentLanguage = false;
    bool checkLanguageFont = false;
    bool checkInputMethod = false;
    QString inputMethodId;
    QString inputMethodName;
    QString inputMethodPackageName;
    QString fontName;
    QString fontPath;
    QString fontStyle;
    QString fontPackageName;
};

class LanguageFeatureConfig
{
public:
    // 获取指定语言的特性配置
    static LanguageFeatureOption optionForLanguage(const QString &languageCode);

private:
    // 确保语言特性配置已加载
    static void ensureLoaded();
    // 从文件加载语言特性配置
    static bool loadFromFile(const QString &filePath);
    // 合并默认配置和指定语言配置
    static LanguageFeatureOption mergeOption(const QString &languageCode);

    static bool s_loaded;
};

#endif // LANGUAGEFEATURECONFIG_H
