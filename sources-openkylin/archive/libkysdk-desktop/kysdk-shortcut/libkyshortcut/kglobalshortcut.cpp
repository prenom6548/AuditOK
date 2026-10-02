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

#include "kglobalshortcut.h"
#include "../utils/kshortcututil.h"
#include "libkyshortcut.h"

#include <KGlobalAccel>
#include <KStandardShortcut>
#include <QGSettings>
#include <QDebug>
#include <QProcess>
#include <QDBusInterface>
#include <QDBusReply>
#include <QFile>
#include <QDBusArgument>

#define KDE_GLOBALACCEL_SERVER "org.kde.kglobalaccel"
#define KDE_GLOBALACCEL_PATH "/kglobalaccel"
#define KDE_GLOBALACCEL_INTERFACE "org.kde.KGlobalAccel"
#define KDE_ALL_COMPONENTS_METHOD "allComponents"
#define KDE_GLOBALACCEL_COMPONENT_INTERFACE "org.kde.kglobalaccel.Component"
#define KDE_ALL_SHORTCUT_INFOS_METHOD "allShortcutInfos"
#define KYSDK_COMPONET_NAME "kysdk-keybindings"

// 定义结构体来匹配返回的数据类型
struct DShortcutInfo
{
    QString contextUniqueName;
    QString contextFriendlyName;
    QString componentUniqueName;
    QString componentFriendlyName;
    QString uniqueName;
    QString friendlyName;
    QList<int> keys;
    QList<int> defaultKeys;
};

// 重载 QDBusArgument 的 >> 操作符以解析结构体
const QDBusArgument &operator>>(const QDBusArgument &argument, DShortcutInfo &myStruct)
{
    argument.beginStructure();
    argument >> myStruct.contextUniqueName >> myStruct.contextFriendlyName >> myStruct.componentUniqueName >> myStruct.componentFriendlyName >> myStruct.uniqueName >> myStruct.friendlyName >> myStruct.keys >> myStruct.defaultKeys;
    argument.endStructure();
    return argument;
}

kdk::KGlobalShortcut::KGlobalShortcut(QObject *parent) : QObject(parent)
{
}

int kdk::KGlobalShortcut::creatGlobalShortcut(QString name, QString key, QString action)
{
    if (name.isEmpty() || name.isNull())
        return KYSDK_SHORTCUT_NAME_ERROR;

    KShortcutUtil util;
    QList<KeyEntry> kysdkEntries = util.getKysdkShortcuts();
    for (KeyEntry &entry : kysdkEntries)
    {
        if (0 == entry.name.compare(name))
        {
            return KYSDK_SHORTCUT_EXISTED;
        }
    }

    key = convertKeyWithSeqString(key);
    int ret = checkShortcutIsAvailable(key);
    if (KYSDK_SUCCESS != ret)
    {
        return ret;
    }

    if (!isActionAvailable(action))
    {
        return KYSDK_SHORTCUT_ACTION_ERROR;
    }

    QString usdKey = convertKeyWithUsdString(key);

    QString availablepath = util.findFreePath(KYSDK_KEYBINDINGS_DIR);
    QGSettings setting(KYSDK_KEYBINDING_SCHEMA, availablepath.toLatin1(), this);
    setting.set(NAME_KEY, name);
    setting.set(BINDING_KEY, usdKey);
    setting.set(ACTION_KEY, action);

    // 创建快捷键
    return KYSDK_SUCCESS;
}

int kdk::KGlobalShortcut::deleteGlobalShortcut(QString name)
{
    KShortcutUtil util;
    QString targetpath = util.findTargetPath(name, KYSDK_KEYBINDINGS_DIR);
    if (targetpath.isEmpty())
        return -1;

    QProcess p(0);
    QStringList args;

    char *fullpath = targetpath.toLatin1().data();
    QString cmd = "dconf";

    args.append("reset");
    args.append("-f");
    args.append(fullpath);
    p.execute(cmd, args);
    p.waitForFinished(-1);
    return KYSDK_SUCCESS;
}

int kdk::KGlobalShortcut::globalShortcutIsAvailable(QString key)
{
    key = convertKeyWithSeqString(key);
    return checkShortcutIsAvailable(key);
}

int kdk::KGlobalShortcut::setGlobalShortcut(QString name, QString key, QString action)
{
    if (name.isEmpty() || name.isNull())
        return KYSDK_SHORTCUT_NAME_ERROR;

    KShortcutUtil util;
    QList<KeyEntry> kysdkEntries = util.getKysdkShortcuts();
    for (KeyEntry &entry : kysdkEntries)
    {
        // 修改快捷键
        if (0 == entry.name.compare(name))
        {
            QGSettings setting(KYSDK_KEYBINDING_SCHEMA, entry.gsPath.toLatin1(), this);
            if (key.length() > 0)
            {
                key = convertKeyWithSeqString(key);
                if (checkShortcutIsAvailable(key))
                    return KYSDK_SHORTCUT_KEY_ERROR;
                key = convertKeyWithUsdString(key);
                setting.set(BINDING_KEY, key);
            }
            if (action.length() > 0)
            {
                if (!isActionAvailable(action))
                    return KYSDK_SHORTCUT_ACTION_ERROR;
                setting.set(ACTION_KEY, action);
            }
            return KYSDK_SUCCESS;
        }
    }

    // 创建快捷键
    key = convertKeyWithSeqString(key);
    int ret = checkShortcutIsAvailable(key);
    if (KYSDK_SUCCESS != ret)
    {
        return ret;
    }

    if (!isActionAvailable(action))
    {
        return KYSDK_SHORTCUT_ACTION_ERROR;
    }

    QString usdKey = convertKeyWithUsdString(key);

    QString availablepath = util.findFreePath(KYSDK_KEYBINDINGS_DIR);
    QGSettings setting(KYSDK_KEYBINDING_SCHEMA, availablepath.toLatin1(), this);
    setting.set(NAME_KEY, name);
    setting.set(BINDING_KEY, usdKey);
    setting.set(ACTION_KEY, action);

    // 创建快捷键
    return KYSDK_SUCCESS;
}

KYSDKGlobalShortcutInfoList *kdk::KGlobalShortcut::getGlobalShortcuts()
{
    KYSDKGlobalShortcutInfoList *list = nullptr;
    KYSDKGlobalShortcutInfoList *prev = nullptr, *curr = nullptr;

    QDBusInterface interface(KDE_GLOBALACCEL_SERVER, KDE_GLOBALACCEL_PATH, KDE_GLOBALACCEL_INTERFACE);

    QDBusReply<QList<QDBusObjectPath>> reply = interface.call(KDE_ALL_COMPONENTS_METHOD);
    if (!reply.isValid())
    {
        qDebug() << reply.error().message();
        return nullptr;
    }

    QList<QDBusObjectPath> componentPaths = reply.value();
    for (QDBusObjectPath componentPath : componentPaths)
    {
        QDBusInterface componentInterface(KDE_GLOBALACCEL_SERVER, componentPath.path(), KDE_GLOBALACCEL_COMPONENT_INTERFACE);

        QDBusReply<QDBusArgument> componentReply = componentInterface.call(KDE_ALL_SHORTCUT_INFOS_METHOD, "default");
        if (!componentReply.isValid())
        {
            qDebug() << componentReply.error().name() << componentReply.error().message();
            return nullptr;
        }
        const QDBusArgument argument = componentReply.value();

        QList<DShortcutInfo> shortcuts;
        argument.beginArray();
        while (!argument.atEnd())
        {
            DShortcutInfo myStruct;
            argument >> myStruct;
            shortcuts.append(myStruct);
        }
        argument.endArray();

        for (DShortcutInfo shortcut : shortcuts)
        {
            curr = (KYSDKGlobalShortcutInfoList *)calloc(1, sizeof(KYSDKGlobalShortcutInfoList));
            if (nullptr == list)
            {
                list = curr;
            }
            else
                prev->next = curr;

            curr->data = (KYSDKGlobalShortcutInfo *)calloc(1, sizeof(KYSDKGlobalShortcutInfo));
            curr->data->compenteName = qstrdup(shortcut.componentUniqueName.toLatin1().data());
            curr->data->name = qstrdup(shortcut.uniqueName.toLatin1().data());
            int i = 0;
            for (int key : shortcut.keys)
            {
                curr->data->keys[i++] = key;
            }

            prev = curr;
        }
    }

    KShortcutUtil util;
    QList<KeyEntry> kysdkEntries = util.getKysdkShortcuts();
    for (KeyEntry &entry : kysdkEntries)
    {
        curr = (KYSDKGlobalShortcutInfoList *)calloc(1, sizeof(KYSDKGlobalShortcutInfoList));
        if (nullptr == list)
        {
            list = curr;
        }
        else
            prev->next = curr;

        curr->data = (KYSDKGlobalShortcutInfo *)calloc(1, sizeof(KYSDKGlobalShortcutInfo));
        curr->data->compenteName = qstrdup(KYSDK_COMPONET_NAME);
        curr->data->name = qstrdup(entry.name.toLatin1().data());
        curr->data->keys[0] = keyToNumber(entry.binding);
        curr->data->action = qstrdup(entry.action.toLatin1().data());

        prev = curr;
    }
    return list;
}

KYSDKGlobalShortcutInfoList *kdk::KGlobalShortcut::getGlobalShortcutByKey(QString key)
{
    KYSDKGlobalShortcutInfoList *list = nullptr;
    KYSDKGlobalShortcutInfoList *prev = nullptr, *curr = nullptr;

    key = convertKeyWithSeqString(key);
    QKeySequence seq = QKeySequence::fromString(key);
    if (false == isKeyAvailable(seq))
    {
        return nullptr;
    }

    QList<KGlobalShortcutInfo> shortcuts = KGlobalAccel::getGlobalShortcutsByKey(seq);
    for (KGlobalShortcutInfo shortcut : shortcuts)
    {
        curr = (KYSDKGlobalShortcutInfoList *)calloc(1, sizeof(KYSDKGlobalShortcutInfoList));
        if (nullptr == list)
        {
            list = curr;
        }
        else
            prev->next = curr;

        curr->data = (KYSDKGlobalShortcutInfo *)calloc(1, sizeof(KYSDKGlobalShortcutInfo));
        curr->data->compenteName = qstrdup(shortcut.componentUniqueName().toLatin1().data());
        curr->data->name = qstrdup(shortcut.contextUniqueName().toLatin1().data());
        if (0 == qstrcmp(curr->data->compenteName,"kysdk-keybindings"))
        {
            KShortcutUtil util;
            QString schema_path = util.findTargetPath(curr->data->name, KYSDK_KEYBINDINGS_DIR);
            QGSettings setting(KYSDK_KEYBINDING_SCHEMA, schema_path.toLatin1(), this);
            curr->data->action = qstrdup(setting.get(ACTION_KEY).toByteArray().data());
        }
        int i = 0;
        for (QKeySequence key : shortcut.keys())
        {
            curr->data->keys[i++] = key[0];
        }

        prev = curr;
    }

    return list;
}

QString kdk::KGlobalShortcut::numberToKey(uint number)
{
    QKeySequence seq(number);
    QString key = seq.toString();
    if (key.contains("Meta"))
    {
        key.replace("Meta", "Win");
    }
    if (key.contains("Print", Qt::CaseInsensitive))
    {
        key.replace("Print", "PrtSc", Qt::CaseInsensitive);
    }
    return key;
}

uint kdk::KGlobalShortcut::keyToNumber(QString key)
{
    key = convertKeyWithSeqString(key);
    QKeySequence seq(key);
    return seq[0];
}

bool kdk::KGlobalShortcut::isKeyAvailable(const QKeySequence seq)
{
    QString key = seq.toString();
    if (!key.contains("+"))
    {
        return false;
    }
    else if (key.contains("Num") ||
             key.contains("Space") ||
             key.contains("Meta") ||
             key.contains("Ins") ||
             key.contains("Home") ||
             key.contains("PgUp") ||
             key.contains("Del") ||
             key.contains("End") ||
             key.contains("PgDown") ||
             key.contains("Print") ||
             key.contains("Backspace") ||
             key.contains("ScrollLock") ||
             key.contains("Return") ||
             key.contains("Enter") ||
             key.contains("Tab") ||
             key.contains("CapsLock") ||
             key.contains("Left") ||
             key.contains("Right") ||
             key.contains("Up") ||
             key.contains("Down") ||
             key.contains("Clear Grab"))
    {
        return false;
    }
    else
    {
        QStringList keys = key.split("+");
        if (keys.count() >= 4)
        {
            return false;
        }
        else
        {
            QString key = keys.at(keys.count() - 1);
            if (!key.contains(QRegExp("[A-Z]")) && !key.contains(QRegExp("[a-z]")) && !key.contains(QRegExp("[0-9]")))
            {
                return false;
            }
        }
    }

    return true;
}

bool kdk::KGlobalShortcut::isActionAvailable(const QString &action)
{
    return QFile::exists(action);
}

QString kdk::KGlobalShortcut::convertKeyWithSeqString(QString key)
{
    key.replace(" ", "");
    if (key.contains("Win"))
    {
        key.replace("Win", "Meta");
    }
    if (key.contains("Start"))
    {
        key.replace("Start", "Meta");
    }
    if (key.contains("PrtSc", Qt::CaseInsensitive))
    {
        key.replace("PrtSc", "Print", Qt::CaseInsensitive);
    }
    if (key.contains("Control", Qt::CaseInsensitive))
    {
        key.replace("Control", "Ctrl", Qt::CaseInsensitive);
    }
    if (key.contains("↑"))
    {
        key.replace("↑", "Up");
    }
    if (key.contains("→"))
    {
        key.replace("→", "Right");
    }
    if (key.contains("←"))
    {
        key.replace("←", "Left");
    }
    if (key.contains("↓"))
    {
        key.replace("↓", "Down");
    }

    if (key.contains("<"))
    {
        key.replace("<", "");
    }

    if (key.contains(">"))
    {
        key.replace(">", "+");
    }
    return key;
}

QString kdk::KGlobalShortcut::convertKeyWithUsdString(QString key)
{
    key.replace(" ", "");
    if (key.contains("Meta"))
    {
        key.replace("Meta", "Win");
    }
    if (key.contains("Start"))
    {
        key.replace("Start", "Win");
    }
    if (key.contains("Print", Qt::CaseInsensitive))
    {
        key.replace("Print", "PrtSc", Qt::CaseInsensitive);
    }

    if (key.contains("+"))
    {
        QStringList keys = key.split("+");
        if (keys.count() == 2)
        {
            QString upper = keys.at(1);
            QString usdKey = "<" + keys.at(0) + ">" + upper.toUpper();

            return usdKey;
        }
        else if (keys.count() == 3)
        {
            QString upper = keys.at(2);
            QString usdKey = "<" + keys.at(0) + ">" + "<" + keys.at(1) + ">" + upper.toUpper();

            return usdKey;
        }
        else if (keys.count() == 4)
        {
            QString upper = keys.at(2);
            QString usdKey = "<" + keys.at(0) + ">" + "<" + keys.at(1) + ">" + "<" + keys.at(2) + ">" + upper.toUpper();

            return usdKey;
        }
    }

    return key;
}

int kdk::KGlobalShortcut::checkShortcutIsAvailable(QString key)
{
    int ret = KYSDK_SUCCESS;

    QKeySequence seq = QKeySequence::fromString(key);
    if (false == isKeyAvailable(seq))
    {
        return KYSDK_SHORTCUT_KEY_ERROR;
    }

    do
    {
        ret = conflictWithGlobalShortcuts(seq);
        if (KYSDK_SUCCESS != ret)
            break;

        ret = conflictWithStandardShortcuts(seq);
        if (KYSDK_SUCCESS != ret)
            break;
    } while (0);

    return ret;
}

int kdk::KGlobalShortcut::conflictWithGlobalShortcuts(const QKeySequence &keySequence)
{
    QList<KGlobalShortcutInfo> shortcuts = KGlobalAccel::getGlobalShortcutsByKey(keySequence);

    if (!shortcuts.isEmpty())
    {
        KGlobalShortcutInfo &sfd = shortcuts.first();
        QString componentUniqueName = sfd.componentUniqueName();
        QString uniqueName = sfd.uniqueName();
        qDebug() << componentUniqueName << ":" << uniqueName;
        return KYSDK_SHORTCUT_EXISTED;
    }
    return KYSDK_SUCCESS ;
}

int kdk::KGlobalShortcut::conflictWithStandardShortcuts(const QKeySequence &keySequence)
{
    KStandardShortcut::StandardShortcut ssc = KStandardShortcut::find(keySequence);
    if (ssc != KStandardShortcut::AccelNone)
    {
        return KYSDK_SHORTCUT_EXISTED;
    }
    return KYSDK_SUCCESS;
}

int kdk::KGlobalShortcut::conflictWithKysdkShortcuts(const QKeySequence &keySequence)
{
    QString kysdkKey = convertKeyWithUsdString(keySequence.toString());

    KShortcutUtil util;
    QList<KeyEntry> customEntries = util.getKysdkShortcuts();
    for (KeyEntry &entry : customEntries)
    {
        QString key = entry.binding;
        if (!kysdkKey.compare(key, Qt::CaseInsensitive))
        {
            return KYSDK_SHORTCUT_EXISTED;
        }
    }

    return KYSDK_SUCCESS;
}
