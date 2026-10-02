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
#include "shortcutinterface.h"
#include "../../universalinterface.h"


/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
#include <dconf/dconf.h>
#include <dconf/common/dconf-paths.h>
}

ShortcutInterface::ShortcutInterface():UKUI::ServiceObject(nullptr)
{
    qRegisterMetaType<QStringPair>("QStringPair");
    qDBusRegisterMetaType<QStringPair>();
    qRegisterMetaType<QStringPairList>("QStringPairList");
    qDBusRegisterMetaType<QStringPairList>();

    qRegisterMetaType<KeyEntry>("KeyEntry");
    qDBusRegisterMetaType<KeyEntry>();
    qRegisterMetaType<KeyEntryList>("KeyEntryList");
    qDBusRegisterMetaType<KeyEntryList>();
    getShortcutThread();
}

ShortcutInterface::~ShortcutInterface() {
    m_systemEntries.clear();
    m_windowEntries.clear();
    m_customEntries.clear();
    m_shortKeyValue.clear();
    m_systemShortcutList.clear();
    m_windowShortcutList.clear();
}

void ShortcutInterface::setDefaultShortcut(QString key, QString value) {
    QGSettings ukcc(DEFAULT_SHORTCUT_SCHEMA);
    if (!ukcc.keys().contains(DEFAULT_SHORTCUT)) {
        qDebug() << Q_FUNC_INFO << "org.ukui.control-center has no key defaultShortcut";
        return;
    }
    QString defaultShortcut = ukcc.get(DEFAULT_SHORTCUT).toString();
    QList<QString> shortcutKey;
    for (const auto& shortcutPair : defaultShortcut.split(";")) {
        QStringList shortcutPairList = shortcutPair.split(":");
        if (shortcutPairList.size() != 2) {
            continue;
        }
        QString pairKey = shortcutPairList.at(0);
        QString pairValue = shortcutPairList.at(1);
        shortcutKey.append(pairKey);
        //qDebug() << Q_FUNC_INFO << "gsetting shortcut" << pairKey << pairValue;
    }
    if (shortcutKey.contains(key)) {
        //qDebug() << Q_FUNC_INFO << "skip gsetting set default shortcut" << key << value;
        return;
    }
    if (defaultShortcut.isEmpty()) {
        defaultShortcut = key + ":" + value;
    } else {
        defaultShortcut = defaultShortcut + ";" + key + ":" + value;
    }
    ukcc.set("defaultShortcut", defaultShortcut);
}

void ShortcutInterface::generalShortcutSlot(QString schema, QString key, QString value) {
    KeyEntry generalKeyEntry;
    generalKeyEntry.gsSchema = schema;
    generalKeyEntry.keyStr = key;
    generalKeyEntry.valueStr = value;
    m_systemEntries.append(generalKeyEntry);
    int size = key.size();
    if (!key.isEmpty() && key[size - 1] == '2') {
        m_shortKeyValue.insert(key, " " + value);
        //qDebug() << Q_FUNC_INFO << "add system shortcut" << key << value;
    } else {
        m_shortKeyValue.insert(key, value);
        //qDebug() << Q_FUNC_INFO << "add system shortcut" << key << value;
    }
    setDefaultShortcut(key, value);
}

void ShortcutInterface::windowShortcutSlot(QString name, QString key, QString value) {
    KeyEntry windowKeyEntry;
    windowKeyEntry.nameStr = name;
    windowKeyEntry.bindingStr = key;
    windowKeyEntry.keyStr = key;
    windowKeyEntry.valueStr = value;
    m_windowEntries.append(windowKeyEntry);
    int size = key.size();
    if (!key.isEmpty() && key[size - 1] == '2') {
        m_shortKeyValue.insert(key, " " + value);
        //qDebug() << Q_FUNC_INFO << "add window shortcut" << key << value << name;
    } else {
        m_shortKeyValue.insert(key, value);
        //qDebug() << Q_FUNC_INFO << "add window shortcut" << key << value << name;
    }
    setDefaultShortcut(key, value);
}

void ShortcutInterface::customShortcutSlot(QString path, QString name,
                                           QString binding, QString action) {
    KeyEntry customKeyEntry;
    customKeyEntry.gsSchema = KEYBINDINGS_CUSTOM_SCHEMA;
    customKeyEntry.gsPath = path;
    customKeyEntry.nameStr = name;
    customKeyEntry.bindingStr = binding;
    customKeyEntry.actionStr = action;
    bool exist = false;
    for (auto& entry : m_customEntries) {
        if (entry.gsPath == path) {
            exist = true;
            qDebug() << Q_FUNC_INFO << "custom repeated" << name << customKeyEntry.nameStr << path << binding;
        }
    }
    if (!exist) {
        m_customEntries.append(customKeyEntry);
    }
//    qDebug() << Q_FUNC_INFO << "add custom shortcut" << name << binding << action << m_customEntries.size();
}

QMap<QString, QString> ShortcutInterface:: MergerOfTheSamekind(QMap<QString, QString> desktopMap) {
    QMap<QString, QString>::iterator it = desktopMap.begin();
    for (; it != desktopMap.end(); it++) {
        QString name = it.key().at(it.key().size() - 1);
        QString name_modification = it.key().left(it.key().length() - 1);
        if (name == '2') {
            desktopMap[name_modification] = desktopMap[name_modification]+" or "+it.value();
            desktopMap.erase(it);
            it = desktopMap.begin()+1;// 除之后要将指针指向后面一个
        }
    }
    return desktopMap;
}

void ShortcutInterface::buildGeneralWidget(QString schema, QMap<QString, QString> subShortcutsMap) {
    GSettingsSchema *pSettings;
    QString domain;
    if (schema == "Desktop") {
        domain = "ukui-settings-daemon";
        pSettings = g_settings_schema_source_lookup(g_settings_schema_source_new_from_directory(
                                                    "/usr/share/glib-2.0/schemas/",
                                                    g_settings_schema_source_get_default(),
                                                    FALSE, NULL),
                                                    KEYBINDINGS_DESKTOP_SCHEMA,
                                                    FALSE);
    } else {
        return;
    }
    QMap<QString, QString>::iterator it = subShortcutsMap.begin();
    for (int i = 0; it != subShortcutsMap.end(); it++,i++) {
        QByteArray ba = domain.toLatin1();
        QByteArray ba1 = it.key().toLatin1();

        GSettingsSchemaKey *keyObj = g_settings_schema_get_key(pSettings, ba1.data());
        char *i18nKey;
        i18nKey = const_cast<char *>(g_dgettext(ba.data(), g_settings_schema_key_get_summary(keyObj)));
        QStringPair shortcut;
        shortcut.name = QString(i18nKey);
        shortcut.key  = it.value();
        shortcut.mediakey = it.key();
        m_systemShortcutList.append(shortcut);
        g_settings_schema_key_unref(keyObj);
    }

    g_settings_schema_unref(pSettings);
    return;
}

void ShortcutInterface::workerCompleteSlot() {
    QMap<QString, QString> desktopMap;
    for (int i = 0; i < m_systemEntries.count(); i++) {
        if (m_systemEntries[i].gsSchema == KEYBINDINGS_DESKTOP_SCHEMA) {
            desktopMap.insert(m_systemEntries[i].keyStr, m_systemEntries[i].valueStr);
        }
    }
    desktopMap = MergerOfTheSamekind(desktopMap);
    QMap<QString, QMap<QString, QString>> generalMaps;
    if (desktopMap.count() != 0) {
        generalMaps.insert("Desktop", desktopMap);
    }

    QMap<QString, QMap<QString, QString>>::iterator it = generalMaps.begin();
    for (; it != generalMaps.end(); it++) {
        buildGeneralWidget(it.key(), it.value());
    }

    for (KeyEntry ckeyEntry : m_windowEntries) {
        QStringPair sq;
        sq.name = ckeyEntry.nameStr;
        sq.key  = ckeyEntry.keyStr;
        m_windowShortcutList.append(sq);
//        qDebug() << Q_FUNC_INFO << "add window shortcut" << sq.key << sq.name;
    }

    for (KeyEntry ckeyEntry : m_customEntries) {
        QStringPair sq;
        sq.name = ckeyEntry.nameStr;
        sq.key  = getShowShortcutString(ckeyEntry.bindingStr);
        m_customShortcutList.append(sq);
    }
}

void ShortcutInterface::getShortcutThread() {
    m_toGetShortcutThread = new GetShortcutWorker(this);
    if (nullptr == m_toGetShortcutThread) {
        qWarning() << Q_FUNC_INFO << "get shortcut worker failed";
        return;
    }

    m_shortKeyValue.clear();
    m_systemEntries.clear();
    m_systemShortcutList.clear();
    connect(m_toGetShortcutThread, &GetShortcutWorker::generalShortcutGenerate, this,
            [=](QString schema, QString key, QString value) {
        generalShortcutSlot(schema, key, value);
    });

    m_windowEntries.clear();
    m_windowSortEntries.clear();
    m_windowShortcutList.clear();
    connect(m_toGetShortcutThread, &GetShortcutWorker::windowShortcutGenerate, this,
            [=](QString name, QString key, QString value) {
        windowShortcutSlot(name, key, value);
    });

    m_customEntries.clear();
    connect(m_toGetShortcutThread, &GetShortcutWorker::customShortcutGenerate, this,
            [=](QString path, QString name, QString binding, QString action) {
        customShortcutSlot(path, name, binding, action);
    });

    connect(m_toGetShortcutThread, &GetShortcutWorker::workerComplete, this, [=]() {
        workerCompleteSlot();
    });

    connect(m_toGetShortcutThread, &GetShortcutWorker::finished, this, [=]() {
        m_toGetShortcutThread->deleteLater();
    });
    m_toGetShortcutThread->start();
}

QList<KeyEntry> ShortcutInterface::getSystemShortcutEntry() {
    qDebug() << Q_FUNC_INFO << m_systemEntries.size();
    return m_systemEntries;
}

QList<KeyEntry> ShortcutInterface::getWindowShortcutEntry() {
    qDebug() << Q_FUNC_INFO << m_windowEntries.size();
    return m_windowEntries;
}

QList<KeyEntry> ShortcutInterface::getCustomShortcutEntry() {
    qDebug() << Q_FUNC_INFO << m_customEntries.size();
    return m_customEntries;
}

QVariantMap ShortcutInterface::getShortKeyValue() {
    qDebug() << Q_FUNC_INFO << m_shortKeyValue.size();
    return m_shortKeyValue;
}

QList<QStringPair> ShortcutInterface::getSystemShortcut() {
    qDebug() << Q_FUNC_INFO << m_systemShortcutList.size();
    return m_systemShortcutList;
}

QList<QStringPair> ShortcutInterface::getWindowShortcut() {
    qDebug() << Q_FUNC_INFO << m_windowShortcutList.size();
    if (m_windowShortcutList.empty()) {
        qDebug() << Q_FUNC_INFO << "start getShortcutThread";
        getShortcutThread();
    }
    return m_windowShortcutList;
}

bool ShortcutInterface::updateShortcut() {
    getShortcutThread();
    return true;
}

static const char *const kWlcomService = "com.kylin.Wlcom";
static const char *const kWlcomShortcutsManagerPath = "/com/kylin/Wlcom/GlobalShortcuts";
static const char *const kWlcomShortcutsManagerIface = "com.kylin.Wlcom.GlobalShortcuts.Manager";
static const char *const kWlcomShortcutsSessionPath = "/com/kylin/Wlcom/GlobalShortcuts/Session/kylin_wlcom";
static const char *const kWlcomShortcutsSessionIface = "com.kylin.Wlcom.GlobalShortcuts.Session";
static const char *const kWlcomSessionId = "kylin_wlcom";
// wlcom 拒绝空 current_key（解析失败返回 0），清除快捷键时改绑到几乎不会按下的组合，以释放原按键
static const char *const kWlcomClearedKey = "win+f24";

// 窗口应用区只展示「显示或隐藏桌面」（Win+D / desktop_toggle_visibility）
static bool isShowDesktopWindowShortcut(const QString &actionId, const QString &name)
{
    const QString id = actionId.toLower();
    if (id == QLatin1String("desktop_toggle_visibility")) {
        return true;
    }
    if (name.contains(QStringLiteral("显示或隐藏桌面"))) {
        return true;
    }
    const QString n = name.toLower();
    return n.contains(QLatin1String("show or hide desktop"));
}

static QVariantMap shortcutVariantToMap(const QVariant &var)
{
    if (var.canConvert<QVariantMap>()) {
        return var.toMap();
    }
    if (var.canConvert<QDBusArgument>()) {
        return qdbus_cast<QVariantMap>(var);
    }
    const QDBusArgument arg = var.value<QDBusArgument>();
    if (arg.currentType() != QDBusArgument::UnknownType) {
        return qdbus_cast<QVariantMap>(arg);
    }
    return QVariantMap();
}

static bool ensureWlcomShortcutSession()
{
    QDBusInterface session(kWlcomService,
                           kWlcomShortcutsSessionPath,
                           kWlcomShortcutsSessionIface,
                           QDBusConnection::sessionBus());
    if (session.isValid()) {
        return true;
    }

    QDBusInterface manager(kWlcomService,
                           kWlcomShortcutsManagerPath,
                           kWlcomShortcutsManagerIface,
                           QDBusConnection::sessionBus());
    if (!manager.isValid()) {
        return false;
    }
    QDBusMessage reply = manager.call(QStringLiteral("CreateSession"),
                                      QString::fromLatin1(kWlcomSessionId));
    return reply.type() != QDBusMessage::ErrorMessage;
}

static bool kwinGetShortcutInfosAvailable()
{
    QDBusInterface *kwinInterface = UniversalInterface::self()->kwinDbusInterface();
    if (!kwinInterface || !kwinInterface->isValid()) {
        return false;
    }
    QDBusReply<QVariantList> res = kwinInterface->call("getShortcutInfos");
    return res.isValid();
}

static bool isUkccClearedBinding(const QString &binding)
{
    return binding.isEmpty()
        || binding.compare(QStringLiteral("Null"), Qt::CaseInsensitive) == 0
        || binding.compare(QStringLiteral("disable"), Qt::CaseInsensitive) == 0;
}

static QString ukccBindingToWlcomKey(const QString &binding)
{
    // 空串/Null 不能直接交给 ConfigureShortcuts，否则键仍占用且 UI 会误显示「无」
    if (isUkccClearedBinding(binding)) {
        return QString::fromLatin1(kWlcomClearedKey);
    }
    if (binding.contains(QLatin1Char('+')) && !binding.contains(QLatin1Char('<'))) {
        return binding.toLower();
    }

    QString s = binding;
    s.replace(QStringLiteral("<Super>"), QStringLiteral("win+"), Qt::CaseInsensitive);
    s.replace(QStringLiteral("<Meta>"), QStringLiteral("win+"), Qt::CaseInsensitive);
    s.replace(QStringLiteral("<Win>"), QStringLiteral("win+"), Qt::CaseInsensitive);
    s.replace(QStringLiteral("<Start>"), QStringLiteral("win+"), Qt::CaseInsensitive);
    s.replace(QStringLiteral("<Ctrl>"), QStringLiteral("ctrl+"), Qt::CaseInsensitive);
    s.replace(QStringLiteral("<Control>"), QStringLiteral("ctrl+"), Qt::CaseInsensitive);
    s.replace(QStringLiteral("<Alt>"), QStringLiteral("alt+"), Qt::CaseInsensitive);
    s.replace(QStringLiteral("<Shift>"), QStringLiteral("shift+"), Qt::CaseInsensitive);
    while (s.contains(QStringLiteral("++"))) {
        s.replace(QStringLiteral("++"), QStringLiteral("+"));
    }
    if (s.endsWith(QLatin1Char('+'))) {
        s.chop(1);
    }
    return s.toLower();
}

// 读回时把清除占位键映射为 Null，供控制中心显示「无」
static QString wlcomKeyToUkccBinding(const QString &wlcomKey)
{
    if (wlcomKey.isEmpty()
        || wlcomKey.compare(QString::fromLatin1(kWlcomClearedKey), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("Null");
    }
    return wlcomKey;
}

// 向 wlcom 写入某动作的 current_key；成功返回 true
static bool configureWlcomCurrentKey(QDBusInterface *session,
                                     const QString &actionId,
                                     const QString &wlcomKey)
{
    if (!session || actionId.isEmpty() || wlcomKey.isEmpty()) {
        return false;
    }
    QVariantMap actionProps;
    actionProps.insert(QStringLiteral("current_key"), wlcomKey);
    QVariantMap request;
    request.insert(actionId, QVariant::fromValue(actionProps));
    QDBusReply<uint> reply = session->call(QStringLiteral("ConfigureShortcuts"),
                                           QVariant::fromValue(request));
    if (!reply.isValid()) {
        qWarning() << Q_FUNC_INFO << "ConfigureShortcuts failed" << actionId << reply.error();
        return false;
    }
    if (reply.value() == 0) {
        qWarning() << Q_FUNC_INFO << "ConfigureShortcuts rejected" << actionId << wlcomKey;
        return false;
    }
    return true;
}

// ResetShortcuts 不会撤销清除占位键，恢复默认时显式写回 default_key
static void restoreWlcomShowDesktopToDefault(QDBusInterface *session)
{
    if (!session) {
        return;
    }
    QDBusReply<QVariantMap> reply = session->call(QStringLiteral("ListShortcuts"));
    if (!reply.isValid()) {
        qWarning() << Q_FUNC_INFO << "ListShortcuts failed" << reply.error();
        return;
    }
    for (auto it = reply.value().constBegin(); it != reply.value().constEnd(); ++it) {
        const QString actionId = it.key();
        const QVariantMap info = shortcutVariantToMap(it.value());
        if (info.isEmpty()) {
            continue;
        }
        QString name = info.value(QStringLiteral("desc_l10n")).toString();
        if (name.isEmpty()) {
            name = info.value(QStringLiteral("desc")).toString();
        }
        if (!isShowDesktopWindowShortcut(actionId, name)) {
            continue;
        }
        const QString defaultKey = info.value(QStringLiteral("default_key")).toString().trimmed();
        if (defaultKey.isEmpty()) {
            continue;
        }
        const QString currentKey = info.value(QStringLiteral("current_key")).toString().trimmed();
        if (currentKey.compare(defaultKey, Qt::CaseInsensitive) == 0) {
            continue;
        }
        qDebug() << Q_FUNC_INFO << "restore" << actionId << "from" << currentKey << "to" << defaultKey;
        configureWlcomCurrentKey(session, actionId, defaultKey);
    }
}

static bool loadWlcomWindowEntries(QList<KeyEntry> *windowEntries, QVariantMap *shortKeyValue)
{
    if (!ensureWlcomShortcutSession()) {
        return false;
    }
    QDBusInterface session(kWlcomService,
                           kWlcomShortcutsSessionPath,
                           kWlcomShortcutsSessionIface,
                           QDBusConnection::sessionBus());
    if (!session.isValid()) {
        return false;
    }
    QDBusReply<QVariantMap> reply = session.call(QStringLiteral("ListShortcuts"));
    if (!reply.isValid() || reply.value().isEmpty()) {
        return false;
    }

    windowEntries->clear();
    for (auto it = reply.value().constBegin(); it != reply.value().constEnd(); ++it) {
        const QString actionId = it.key();
        const QVariantMap info = shortcutVariantToMap(it.value());
        if (info.isEmpty()) {
            continue;
        }
        QString name = info.value(QStringLiteral("desc_l10n")).toString();
        if (name.isEmpty()) {
            name = info.value(QStringLiteral("desc")).toString();
        }
        if (name.isEmpty()) {
            name = actionId;
        }
        if (!isShowDesktopWindowShortcut(actionId, name)) {
            continue;
        }
        QString value = info.value(QStringLiteral("current_key")).toString().trimmed();
        // 已停靠在清除占位键时显示为「无」，勿回落到 default_key（否则 Win+D 仍会显示为占用）
        if (value.compare(QString::fromLatin1(kWlcomClearedKey), Qt::CaseInsensitive) == 0) {
            value = QStringLiteral("Null");
        } else if (value.isEmpty()) {
            value = info.value(QStringLiteral("default_key")).toString().trimmed();
        }
        if (value.isEmpty()) {
            value = QStringLiteral("Null");
        } else {
            value = wlcomKeyToUkccBinding(value);
        }

        KeyEntry windowKeyEntry;
        windowKeyEntry.nameStr = name;
        windowKeyEntry.bindingStr = actionId;
        windowKeyEntry.keyStr = actionId;
        windowKeyEntry.valueStr = value;
        windowEntries->append(windowKeyEntry);
        shortKeyValue->insert(actionId, value);
    }
    return !windowEntries->isEmpty();
}

void ShortcutInterface::defaultWindowShortcut() {
    QDBusInterface* kwinInterface = UniversalInterface::self()->kwinDbusInterface();
    if (kwinInterface && kwinGetShortcutInfosAvailable()) {
        kwinInterface->call("resetShortcut");
        QDBusReply<QVariantList> res = kwinInterface->call("getShortcutInfos");
        if (!res.isValid()) {
            qDebug() << Q_FUNC_INFO << "get window shortcutInfos failed" << res.error();
            return;
        }
        const static int WIN_SHORTCUT_MIN_LEN = 3;
        const static int WIN_SHORTCUT_MAX_LEN = 4;
        m_windowEntries.clear();
        for (auto &shortcutInfo : res.value()) {
            QStringList kwinStringList = shortcutInfo.toStringList();
            int size = kwinStringList.size();
            if (size < WIN_SHORTCUT_MIN_LEN) {
                continue;
            }
            QString key = kwinStringList.at(0);
            QString name = kwinStringList.at(1);
            if (!isShowDesktopWindowShortcut(key, name)) {
                continue;
            }
            for (int index = 2; index < size && index < WIN_SHORTCUT_MAX_LEN; ++index) {
                if (index == WIN_SHORTCUT_MIN_LEN) {
                    key = key + "2";
                }
                QString value = kwinStringList.at(index);
                KeyEntry windowKeyEntry;
                windowKeyEntry.nameStr = name;
                windowKeyEntry.bindingStr = key;
                windowKeyEntry.keyStr = key;
                windowKeyEntry.valueStr = value;
                m_windowEntries.append(windowKeyEntry);
                int keySize = key.size();
                if (!key.isEmpty() && key[keySize - 1] == '2') {
                    QString newValue = " " + value;
                    m_shortKeyValue.insert(key, newValue);
                } else {
                    m_shortKeyValue.insert(key, value);
                }
            }
        }
        // kwin 无「显示桌面」项时继续走 wlcom 回落
        if (!m_windowEntries.isEmpty()) {
            return;
        }
    }

    qDebug() << Q_FUNC_INFO << "fallback to wlcom ResetShortcuts";
    if (!ensureWlcomShortcutSession()) {
        qWarning() << Q_FUNC_INFO << "wlcom shortcut session unavailable";
        return;
    }
    QDBusInterface session(kWlcomService,
                           kWlcomShortcutsSessionPath,
                           kWlcomShortcutsSessionIface,
                           QDBusConnection::sessionBus());
    if (!session.isValid()) {
        qWarning() << Q_FUNC_INFO << "wlcom session invalid" << session.lastError();
        return;
    }
    session.call(QStringLiteral("ResetShortcuts"));
    // 恢复默认前 UI 会先把窗口快捷键清成占位键；Reset 无法把它变回 default_key
    restoreWlcomShowDesktopToDefault(&session);
    if (!loadWlcomWindowEntries(&m_windowEntries, &m_shortKeyValue)) {
        qWarning() << Q_FUNC_INFO << "failed to reload wlcom window shortcuts";
    }
}

void ShortcutInterface::defaultGeneralShortcut() {
    if (!QGSettings::isSchemaInstalled(KEYBINDINGS_DESKTOP_SCHEMA)) {
        qWarning() << Q_FUNC_INFO << "org.ukui.SettingsDaemon.plugins.media-keys not install";
        return;
    }
    GSettings* desktopsettings = g_settings_new(KEYBINDINGS_DESKTOP_SCHEMA);
    char **dkeys = g_settings_list_keys(desktopsettings);
    m_systemEntries.clear();
    for (int i = 0; dkeys[i] != nullptr; i++) {
        // 跳过非快捷键
        if (!g_strcmp0(dkeys[i], "active") || !g_strcmp0(dkeys[i], "volume-step")
            || !g_strcmp0(dkeys[i], "priority") || !g_strcmp0(dkeys[i], "enable-osd")) {
            continue;
        }

        GVariant* variant = g_settings_get_default_value(desktopsettings, dkeys[i]);
        gsize size = g_variant_get_size(variant);
        char* str = const_cast<char *>(g_variant_get_string(variant, &size));

        // 保存桌面快捷键
        QString key = QString(dkeys[i]);
        QString value = QString(str);
        //qDebug() << Q_FUNC_INFO << "default system shortcut" << key << value;
        QGSettings generalShort(KEYBINDINGS_DESKTOP_SCHEMA);
        generalShort.set(key, value);
        if (value.contains("KP_Delete")) {
            value = "<Ctrl><Alt>Del";
        }
        if (value != "" && !value.contains("XF86") && !value.contains("KP_")) {
            KeyEntry generalKeyEntry;
            generalKeyEntry.gsSchema = KEYBINDINGS_DESKTOP_SCHEMA;
            generalKeyEntry.keyStr   = key;
            generalKeyEntry.valueStr = value;
            m_systemEntries.append(generalKeyEntry);
            int size = key.size();
            if (!key.isEmpty() && key[size - 1] == '2') {
                QString newValue = " " + value;
                m_shortKeyValue.insert(key, newValue);
                //qDebug() << Q_FUNC_INFO << "add default system shortcut" << key << newValue;
            } else {
                m_shortKeyValue.insert(key, value);
                //qDebug() << Q_FUNC_INFO << "add default system shortcut" << key << value;
            }
        }
    }
    g_strfreev(dkeys);
    g_object_unref(desktopsettings);
}

void ShortcutInterface::defaultShortcut() {
    // 1) default general shortcut
    defaultGeneralShortcut();

    // 2) default window shortcut
    defaultWindowShortcut();
}

void ShortcutInterface::getThreadStart()
{
    return m_toGetShortcutThread->run();
}

QList<QStringPair> ShortcutInterface::getCustomShortcut()
{
    return m_customShortcutList;
}

void ShortcutInterface::setSystemShortcut(const QString &key, const QString &value) {
    QGSettings generalShort(KEYBINDINGS_DESKTOP_SCHEMA);
    generalShort.set(key, value);
    for (auto& entries : m_systemEntries) {
        if (entries.keyStr == key) {
            qDebug() << Q_FUNC_INFO << "update system shortcut key" << key << "from" << entries.valueStr << "to" << value;
            entries.valueStr = value;
            break;
        }
    }
}

void ShortcutInterface::setWindowShortcut(const QString &key, const QString &oldValue, const QString &value) {
    int size = key.size();
    QString newKey = key;
    bool secondKey = (!key.isEmpty() && key[size - 1] == '2');
    if (secondKey) {
        newKey = key.left(size - 1);
    }

    const QString storedValue = isUkccClearedBinding(value)
        ? QStringLiteral("Null")
        : value;

    if (kwinGetShortcutInfosAvailable()) {
        QDBusInterface* kwinInterface = UniversalInterface::self()->kwinDbusInterface();
        qDebug() << Q_FUNC_INFO << "before modifyShortcut" << newKey << oldValue << value;
        kwinInterface->call("modifyShortcut", newKey, oldValue, value);
    } else if (ensureWlcomShortcutSession()) {
        QDBusInterface session(kWlcomService,
                               kWlcomShortcutsSessionPath,
                               kWlcomShortcutsSessionIface,
                               QDBusConnection::sessionBus());
        const QString wlcomKey = ukccBindingToWlcomKey(value);
        qDebug() << Q_FUNC_INFO << "before ConfigureShortcuts" << newKey << oldValue << value
                 << wlcomKey;
        // ConfigureShortcuts 参数为 a{sv}，value 应为内层 a{sv}；再包 QDBusVariant 会 ENXIO
        if (!configureWlcomCurrentKey(&session, newKey, wlcomKey)) {
            return;
        }
    } else {
        qWarning() << Q_FUNC_INFO << "no available window shortcut backend";
        return;
    }

    for (auto& entries : m_windowEntries) {
        if (entries.keyStr != key) {
            continue;
        }
        qDebug() << Q_FUNC_INFO << "update window shortcut key" << key << "from" << oldValue << "to"
                 << storedValue;
        entries.valueStr = storedValue;
        if (secondKey) {
            m_shortKeyValue[key] = QStringLiteral(" ") + storedValue;
        } else {
            m_shortKeyValue[key] = storedValue;
        }
        qDebug() << Q_FUNC_INFO << "update window shortcut keyValue" << key << m_shortKeyValue[key].toString();
        break;
    }
}

void ShortcutInterface::setCustomShortcut(const QString &key, const QString &value) {
    QGSettings customShort(KEYBINDINGS_CUSTOM_SCHEMA, key.toLatin1());
    customShort.set(BINDING_KEY, value);
    QString name = customShort.get(NAME_KEY).toString();
    for (auto& entries : m_customEntries) {
        if (entries.nameStr == name) {
            qDebug() << Q_FUNC_INFO << "update custom shortcut key" << name << "from" << entries.bindingStr << "to" << value;
            entries.bindingStr = value;
            break;
        }
    }
}

QString ShortcutInterface::getShowShortcutString(QString src) {
    if (src.contains("Meta")) {
        src.replace("Meta", "Win");
    }
    if (src.contains("Start")) {
        src.replace("Start", "Win");
    }
    src.replace("<","");
    src.replace(">","   ");
    QStringList temp_list = src.split(" ");
    QString str;
    for(qint32 i = 0; i < temp_list.count(); i++) {
        str += temp_list.at(i)
                .left(1).toUpper() +
                temp_list.at(i)
                .mid(1, temp_list.at(i).length() - 1);
        str += " ";
    }
    str.replace("Or","or");
    return  str;
}
