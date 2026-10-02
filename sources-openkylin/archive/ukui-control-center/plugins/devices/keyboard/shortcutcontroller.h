/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef SHORTCUTCONTROLLER_H
#define SHORTCUTCONTROLLER_H

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <QDBusPendingCallWatcher>
#include <QKeySequence>
#include <QObject>
#include "kglobaldbustypes.h"

class FcitxDbusProvider;
class KGlobalAccelDbusProxy;

// QKeySequence哈希函数
struct QKeySequenceHash {
    std::size_t operator()(const QKeySequence &key) const { return qHash(key); }
};

// 系统快捷键/窗口快捷键信息结构（使用QKeySequence）
struct UnifiedShortcutInfo {
    QString name;             // 快捷键名称
    QString description;      // 描述
    QString source;           // 来源 (System/Window)
    QKeySequence keySequence; // Qt键序
    QString displayText;      // 显示文本
};

// 冲突信息结构体，提供给qml使用
struct ConflictInfo {
    QString name;             // 冲突项名称
    QString description;      // 冲突项描述
    QString source;           // 冲突项来源
    QString displayText;      // 显示文本
    QString shortcut;         // 快捷键字符串

    ConflictInfo() = default;
    ConflictInfo(const QString &n, const QString &desc, const QString &src,
                 const QString &display, const QString &shortcutKey)
        : name(n), description(desc), source(src),
          displayText(display), shortcut(shortcutKey) {}
};

// fcitx快捷键信息结构，包含快捷键列表
struct FcitxShortcutInfo {
    QString uniqueName;           // 唯一名称
    QString category;           // 分类 (Global/InputMethod)
    QString description;          // 描述
    QStringList shortcuts;        // 快捷键列表
    QStringList deduplicatedKeys; // 去重后的快捷键列表
    QStringList displayTexts;     // 显示文本列表
};

class ShortcutController : public QObject {
    Q_OBJECT

public:
    explicit ShortcutController(FcitxDbusProvider *provider,
                                QObject *parent = nullptr);
    ~ShortcutController();

    Q_INVOKABLE void init();

    // 冲突检测
    Q_INVOKABLE QVariantMap checkShortcutConflict(const QString &shortcut,
                                                  const QString &uniqueName) const;

    // 更新fcitx快捷键map
    Q_INVOKABLE void updateFcitxShortcutMap(const QString &name,
                                            const QString &oldShortcut,
                                            const QString &newShortcut);

    // 临时快捷键映射管理接口
    Q_INVOKABLE void updateTempFcitxShortcutMap(const QString &name,
                                                const QString &oldShortcut,
                                                const QString &newShortcut);
    Q_INVOKABLE void clearTempFcitxShortcutMap();
    Q_INVOKABLE void mergeTempFcitxShortcutMap();

public Q_SLOTS:
    void getSystemShortcuts();
    void getWindowShortcuts();
    void getFcitxShortcuts();
    // qml接收按键事件时，屏蔽系统的按键事件
    Q_INVOKABLE void blockSystemShortcut();
    Q_INVOKABLE void unblockSystemShortcut();

signals:
    void systemShortcutsUpdated();
    void windowShortcutsUpdated();
    void fcitxShortcutsUpdated();

private Q_SLOTS:
    void fetchSystemShortcutsFinished(QDBusPendingCallWatcher *watcher);
    void fetchWindowShortcutsFinished(QDBusPendingCallWatcher *watcher);
    void fetchFcitxShortcutsFinished(QDBusPendingCallWatcher *watcher);

private:
    QKeySequence parseFcitxShortcut(const QString &shortcut) const;

    /* 需要将fcitx的快捷键转为QT统一的键序
       "Control+Space" -> "Ctrl+Space"
       "Control_L" -> "Ctrl"
    */
    QString normalizeQtKeySequence(const QString &shortcut) const;
    QString beautifyQtKeyString(const QString &shortcut) const;
    QString keySequenceToDisplayText(const QKeySequence &keySequence) const;
    /* 去重，fcitx的快捷键中会包含最后释放的修饰键的左右值
       如： "Control+Shift+Shift_L"
    */
    QString deduplicateFcitxKeys(const QString &shortcut) const;

    // 设置冲突信息到结果map中
    void setConflictInfo(QVariantMap &result, const ConflictInfo &conflictInfo) const;

    bool checkSystemShortcutsConflict(const QKeySequence &keySeq,
                                      const QString &uniqueName,
                                      QVariantMap &result) const;
    bool checkWindowShortcutsConflict(const QKeySequence &keySeq,
                                      const QString &uniqueName,
                                      QVariantMap &result) const;
    bool checkFcitxShortcutsConflict(const QString &key,
                                     const QString &uniqueName,
                                     QVariantMap &result) const;
    bool checkFcitxShortcutsConflictWithTempMap(const QString &key,
                                                const QString &uniqueName,
                                                QVariantMap &result) const;
    void clearAllShortcuts();

private:
    FcitxDbusProvider *m_fcitxDbus;
    std::unique_ptr<KGlobalAccelDbusProxy> m_kglobalAccelDbus;
    // 系统快捷键 ukui-settings-daemon
    std::unordered_multimap<QKeySequence, UnifiedShortcutInfo, QKeySequenceHash>
        m_systemShortcuts;
    // 窗口快捷键 kylin-wlcom
    std::unordered_multimap<QKeySequence, UnifiedShortcutInfo, QKeySequenceHash>
        m_windowShortcuts;
    /* fcitx快捷键map：键是uniquename，值是包含快捷键列表的结构体
       配置工具只显示快捷键列表的第一个元素,在修改快捷键后,需要同步更新这个map,
       用来做后续的冲突判断,使用uniqueName查找修改的功能名
   */
    std::unordered_map<QString, FcitxShortcutInfo> m_fcitxShortcuts;

    /* 临时fcitx快捷键map：存储InputMethodConfig中未保存的快捷键更改
       用于冲突检测，但不影响真实的快捷键映射,只有保存时才会merge到真实映射中
   */
    std::unordered_map<QString, FcitxShortcutInfo> m_tempFcitxShortcuts;
    const QString m_systemSourceName = "System";
    const QString m_windowSourceName = "Window";

};

#endif // SHORTCUTCONTROLLER_H
