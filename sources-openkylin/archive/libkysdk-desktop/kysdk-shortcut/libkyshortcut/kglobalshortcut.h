/*
 * libkysdk-shortcut's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#ifndef KGLOBALSHORTCUT_H
#define KGLOBALSHORTCUT_H

#include <QObject>
#ifdef _WIN32
#define KYSDK_EXPORT __declspec(dllexport)
#else
#define KYSDK_EXPORT __attribute__((visibility("default")))
#endif

typedef struct _KYSDKGlobalShortcutInfoList KYSDKGlobalShortcutInfoList;
namespace kdk
{
    /**
     * @defgroup ShortcutModule
     */

    /**
     * @brief 快捷键控制类， 提供快捷键管理功能
     * @since 3.0.1.0
     */
    class KYSDK_EXPORT KGlobalShortcut : public QObject
    {
        Q_OBJECT
    public:
        explicit KGlobalShortcut(QObject *parent = nullptr);

    public:
        int creatGlobalShortcut(QString name, QString key, QString action);
        int deleteGlobalShortcut(QString name);
        int globalShortcutIsAvailable(QString key);
        int setGlobalShortcut(QString name, QString key, QString action);
        KYSDKGlobalShortcutInfoList *getGlobalShortcuts();
        KYSDKGlobalShortcutInfoList *getGlobalShortcutByKey(QString key);
        QString numberToKey(uint number);
        uint keyToNumber(QString key);

    private:
        bool isKeyAvailable(const QKeySequence seq);
        bool isActionAvailable(const QString &action);
        QString convertKeyWithSeqString(QString key);
        QString convertKeyWithUsdString(QString key);

    private:
        int checkShortcutIsAvailable(QString key);
        int conflictWithGlobalShortcuts(const QKeySequence &keySequence);
        int conflictWithStandardShortcuts(const QKeySequence &keySequence);
        int conflictWithKysdkShortcuts(const QKeySequence &keySequence);
    };
}
#endif // KGLOBALSHORTCUT_H
