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
#include "shortcutui.h"
#include "realizeshortcutwheel.h"
#include <QCoreApplication>
#include <QLabel>
#include <QShowEvent>
#include <QTimer>
#include <QToolButton>
#include <functional>
#include <QPointer>
#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

#define UKCC_SETTING "org.ukui.control-center"

QList<KeyEntry> g_systemEntryList;
QList<KeyEntry> g_windowEntryList;
QList<KeyEntry> g_customEntryList;

static const char kShortcutNameProp[] = "ukccShortcutName";

// 读屏：accessibleName 放快捷键名称；按键内容由编辑框文本接口朗读（避免 Qt 用当前文本覆盖 name 后名称丢失）
static void refreshShortcutEditAccessibleName(QLineEdit *edit)
{
    if (!edit) {
        return;
    }
    const QString shortcutName = edit->property(kShortcutNameProp).toString().trimmed();
    QString content = edit->text().trimmed();
    if (content.isEmpty()) {
        content = QCoreApplication::translate("ShortcutUi", "Null");
    }
    if (!shortcutName.isEmpty()) {
        edit->setAccessibleName(shortcutName);
        edit->setAccessibleDescription(content);
    } else {
        edit->setAccessibleName(content);
        edit->setAccessibleDescription(QString());
    }
}

static void bindShortcutEditAccessibleName(QLineEdit *edit, const QString &shortcutName)
{
    if (!edit) {
        return;
    }
    edit->setProperty(kShortcutNameProp, shortcutName);
    refreshShortcutEditAccessibleName(edit);
}

ShortcutUi::ShortcutUi()
{
    setAttribute(Qt::WA_DeleteOnClose);
    uiLayout     = new QVBoxLayout(this);
    systemLayout = new QVBoxLayout(this);
    m_windowLayout = new QVBoxLayout(this);
    customLayout = new QVBoxLayout(this);
    m_defaultLayout = new QHBoxLayout(this);;
    uiLayout->addLayout(systemLayout);
    uiLayout->addLayout(m_windowLayout);
    uiLayout->addLayout(m_defaultLayout);
    uiLayout->addLayout(customLayout);
    uiLayout->addStretch();
    uiLayout->setMargin(0);
    initUi();

    m_ukccSetting = new QGSettings(UKCC_SETTING);
    m_shortcutInterface = new QDBusInterface("org.ukui.ukcc.session",
                                            "/Shortcut",
                                            "org.ukui.ukcc.session.Shortcut",
                                            QDBusConnection::sessionBus(), this);
}

ShortcutUi::~ShortcutUi()
{
    if (m_ukccSetting) {
        delete m_ukccSetting;
        m_ukccSetting = nullptr;
    }

    if (addButton) {
        addButton->deleteLater();
    }
}

void ShortcutUi::initShortEntry()
{
    QDBusReply<KeyEntryList> listEntrySystem = m_shortcutInterface->call("getSystemShortcutEntry");
    QDBusReply<KeyEntryList> listEntryCustom = m_shortcutInterface->call("getCustomShortcutEntry");
    QDBusReply<KeyEntryList> listEntryWindow = m_shortcutInterface->call("getWindowShortcutEntry");

    if (listEntrySystem.isValid()) {
        g_systemEntryList = listEntrySystem.value();
    }
    g_windowEntryList.clear();
    if (listEntryWindow.isValid()) {
        g_windowEntryList = listEntryWindow.value();
        if (g_windowEntryList.empty()) {
            qDebug() << Q_FUNC_INFO << "window shortcut is empty";
            m_windowShortcutValid = false;
        }
    } else {
        m_windowShortcutValid = false;
        qDebug() << Q_FUNC_INFO << "window shortcut invalid";
    }

    if (listEntryCustom.isValid()) {
        g_customEntryList = listEntryCustom.value();
    }

    // 已与系统/自定义占用同一按键时，窗口项（如显示或隐藏桌面）置为无
    clearConflictingWindowShortcutsOnLoad();

    loadDefaultShortcut();
}

QMap<QString, QString> ShortcutUi::MergerOfTheSamekind(QMap<QString, QString> desktopMap) {
    QMap<QString, QString>::iterator it = desktopMap.begin();
    for (; it != desktopMap.end(); it++) {
        QString name = it.key().at(it.key().size() - 1);
        QString name_modification = it.key().left(it.key().length() - 1);
        if (name == '2') {
            desktopMap[name_modification] = desktopMap[name_modification]+" or " + it.value();
            qDebug() << Q_FUNC_INFO << it.key() << name << name_modification << desktopMap[name_modification];
            desktopMap.erase(it);
            it = desktopMap.begin()+1;// 除之后要将指针指向后面一个
        }
    }
    return desktopMap;
}

void ShortcutUi::initSystem()
{
    QDBusReply<QStringPairList> list = m_shortcutInterface->call("getSystemShortcut");
    // 界面白名单外的系统快捷键（如 ukui-clipboard）也会参与冲突检测，
    // 需预先写入显示名，否则冲突提示会出现空的 “”。
    if (list.isValid()) {
        for (const QStringPair &s : list.value()) {
            if (!s.mediaKey.isEmpty() && !s.name.isEmpty()) {
                mAllKeyI18nName.insert(s.mediaKey, s.name);
            }
        }
    }
    QMap<QString, QString> desktopMap;
    for (int i = 0; i < g_systemEntryList.count(); i++) {
        if ((g_systemEntryList[i].gsSchema == KEYBINDINGS_DESKTOP_SCHEMA) &&
                !g_systemEntryList[i].keyStr.contains("ukui-window-switch")) {
            desktopMap.insert(g_systemEntryList[i].keyStr, g_systemEntryList[i].valueStr);
        }
    }
    desktopMap = MergerOfTheSamekind(desktopMap);
    QList<QString> generalSortKeyList = {"area-screenshot", "screenshot", "window-screenshot", "kylin-display-switch",
                                     "logout", "nm-connection-editor", "peony-qt", "screensaver", "terminal", "ukui-control-center", "ukui-search", "ukui-sidebar",
                                     "ukui-system-monitor", "ukui-window-switch",
                                     "kylin-service-support"};
    int systemIndex = 0;
    foreach (const auto &key, generalSortKeyList) {
        QMap<QString, QString>::iterator iter = desktopMap.find(key);
        if (iter != desktopMap.end()) {
            for (QStringPair s : list.value()) {
                if (s.mediaKey != key) {
                    continue;
                }
                QString name = s.name;
                addSystemShortcut(name, iter.value(), iter.key(), systemIndex);
                ++systemIndex;
                break;
            }
            desktopMap.erase(iter);
        }
    }
}

void ShortcutUi::initWindow()
{
    if (!m_windowShortcutValid) {
        setWindowValid(false);
        setWindowHide();
        addDefaultBtn();
        return;
    }
    QMap<QString, QString> windowMap;
    for (int i = 0; i < g_windowEntryList.count(); i++) {
        windowMap.insert(g_windowEntryList[i].keyStr, g_windowEntryList[i].valueStr);
        qDebug() << Q_FUNC_INFO << g_windowEntryList[i].keyStr << g_windowEntryList[i].valueStr;
    }
    windowMap = MergerOfTheSamekind(windowMap);
    int windowIndex = 0;
    for (const auto& entries : g_windowEntryList) {
        QString key = entries.keyStr;
        if (!windowMap.keys().contains(key)) {
            continue;
        }
        QString name = entries.nameStr;
        addWindowShortcut(name, windowMap[key], key, windowIndex);
        ++windowIndex;
    }
    addDefaultBtn();
}

void ShortcutUi::initCustom()
{
    int customIndex = 0;
    for (KeyEntry s : g_customEntryList) {
        addCustomShortcut(s, customIndex);
        ++customIndex;
    }
    // addButton 在 initUi 里早于各快捷键行创建，默认会排到 Tab 链最前
    fixShortcutTabOrder();
}

void ShortcutUi::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // 页面真正显示后再钉一次 Tab，避免堆叠页未显示时链未建好
    QTimer::singleShot(0, this, [this]() {
        fixShortcutTabOrder();
    });
}

void ShortcutUi::fixShortcutTabOrder()
{
    QList<QWidget *> order;
    auto appendFocusable = [&order](QWidget *w) {
        if (!w) {
            return;
        }
        // 初始化时页面常在堆叠页里未 show，isVisibleTo 会误判为不可见，导致链为空、添加仍排第一
        if (!(w->focusPolicy() & Qt::TabFocus)) {
            return;
        }
        if (order.contains(w)) {
            return;
        }
        order.append(w);
    };

    // 系统/窗口：Tab 落在常驻编辑框上（与添加对话框一致），不再经灰框跳板
    for (DoubleClickShortCut *edit : mGeneralLineEdit) {
        appendFocusable(edit);
    }
    for (DoubleClickShortCut *edit : m_windowLineEdit) {
        appendFocusable(edit);
    }
    appendFocusable(m_defaultButton);

    for (QFrame *frame : m_customFrameList) {
        if (!frame) {
            continue;
        }
        // 明确顺序：名称编辑框 → 快捷键编辑框 → 「…」更多按钮（不依赖布局遍历偶然顺序）
        DoubleClickLineEdit *nameEdit = frame->findChild<DoubleClickLineEdit *>();
        DoubleClickShortCut *bindEdit = frame->findChild<DoubleClickShortCut *>();
        QToolButton *moreBtn = frame->findChild<QToolButton *>();
        appendFocusable(nameEdit);
        appendFocusable(bindEdit);
        appendFocusable(moreBtn);
    }
    // 添加按钮始终放最后（创建最早，默认会在 Tab 链最前）
    if (addButton) {
        order.removeAll(addButton);
        appendFocusable(addButton);
    }

    for (int i = 0; i + 1 < order.size(); ++i) {
        QWidget::setTabOrder(order.at(i), order.at(i + 1));
    }

    // 「自定义快捷键」标题关联第一个名称编辑框（与添加对话框 label+buddy 一致）
    bindCustomTitleToFirstNameEdit();
}

void ShortcutUi::bindCustomTitleToFirstNameEdit()
{
    if (!customTitle) {
        return;
    }
    const QString titleText = customTitle->text().trimmed();
    // 标题自身可读名（勿留空/调试串，否则 buddy 即使建上也不播）
    if (!titleText.isEmpty()) {
        customTitle->setAccessibleName(titleText);
    }

    bool boundFirst = false;
    for (QFrame *frame : m_customFrameList) {
        if (!frame) {
            continue;
        }
        DoubleClickLineEdit *nameEdit = frame->findChild<DoubleClickLineEdit *>();
        if (!nameEdit) {
            continue;
        }
        // 清掉行内隐藏名称标签对名称框的 buddy，避免 LABELLED_BY 抢走标题朗读
        for (ClickFixLabel *rowNameLab : frame->findChildren<ClickFixLabel *>()) {
            if (rowNameLab && rowNameLab->buddy() == nameEdit) {
                rowNameLab->setBuddy(nullptr);
            }
        }
        // 仅首个名称框：setBuddy + accessibleName=「自定义快捷键」（与添加对话框一致）
        if (!boundFirst) {
            customTitle->setBuddy(nameEdit);
            if (!titleText.isEmpty()) {
                bindShortcutEditAccessibleName(nameEdit, titleText);
            }
            boundFirst = true;
        } else {
            bindShortcutEditAccessibleName(nameEdit, tr("Name"));
        }
    }
}

void ShortcutUi::updateDefaultButtonStatus(bool status) {
    if (m_ukccSetting && m_ukccSetting->keys().contains("shortcutDefaultEnabled")) {
          m_ukccSetting->set("shortcutDefaultEnabled", status);
          m_defaultButton->setEnabled(status);
          qDebug() << Q_FUNC_INFO << "set shortcut default button enabled to false" << status;
    } else {
        qDebug() << Q_FUNC_INFO << "has not key shortcutDefaultEnabled";
    }
}

void ShortcutUi::initUi()
{
    initSystemUi();
    initWindowUi();
    initCustomUi();
}

void ShortcutUi::initSystemUi()
{
    systemTitle         = new TitleLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(systemTitle, "Shortcut", "systemShortcutTitle", "");
    systemTitle->setContentsMargins(16, 0, 0, 0);
    systemShortcutGroup = new SettingGroup(this);
    //~ contents_path /Shortcut/System Shortcut
    systemTitle->setText(tr("System Shortcut"));
    systemLayout->addWidget(systemTitle);
    systemLayout->setSpacing(8);
    systemLayout->addWidget(systemShortcutGroup);
}

void ShortcutUi::initWindowUi()
{
    m_windowTitle         = new TitleLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_windowTitle, "Shortcut", "windowShortcutTitle", "");
    m_windowTitle->setContentsMargins(16, 0, 0, 0);
    m_windowShortcutGroup = new SettingGroup(this);
    //~ contents_path /Shortcut/Window Shortcut
    m_windowTitle->setText(tr("Window Shortcut"));
    m_windowLayout->addWidget(m_windowTitle);
    m_windowLayout->setSpacing(8);
    m_windowLayout->addWidget(m_windowShortcutGroup);
}

AddButton* ShortcutUi::getAddButton() {
    return addButton;
}

TitleLabel* ShortcutUi::getCustomTitle() {
    return customTitle;
}

void ShortcutUi::initCustomUi()
{
    customTitle         = new TitleLabel(this);
    // 勿 ALL_INFO：会冲掉 setBuddy 的 LABELLED_BY，且 accessibleName 变成调试串
    KDK_EXTEND_OBJ_INFO_FORMAT(customTitle, "Shortcut", "customShortcutTitle");
    customShortcutGroup = new SettingGroup(this);
    addButton           = new AddButton(this);
    // 仅写 objectName 供自动化；ALL_INFO 会把 accessibleName 写成英文调试串
    KDK_EXTEND_OBJ_INFO_FORMAT(addButton, "Shortcut", "addButton");
    QHBoxLayout *layout = new QHBoxLayout(this);
    //~ contents_path /Shortcut/Customize Shortcut
    customTitle->setText(tr("Customize Shortcut"));
    customTitle->setContentsMargins(16, 0, 0, 0);
    customLayout->setSpacing(0);
    customLayout->addWidget(customTitle);
    customLayout->addSpacing(8);
    customLayout->addWidget(customShortcutGroup);
    customLayout->addSpacing(1);
    customLayout->addLayout(layout);
    layout->addWidget(addButton);

    addButton->setRadiusType(UkccFrame::BorderRadiusStyle::Around);
    customShortcutGroup->setOneRadiusStyle(UkccFrame::BorderRadiusStyle::Top);
    customShortcutGroup->setLastRadiusStyle(UkccFrame::BorderRadiusStyle::None);

    connect(addButton, &AddButton::clicked, this, [=] {
        AddShortcutDialog *addDialog = new AddShortcutDialog(g_systemEntryList, g_windowEntryList, g_customEntryList, this);
        addDialog->setTitleText(QObject::tr("Customize Shortcut"));

        connect(addDialog, &AddShortcutDialog::shortcutInfoSignal,
                [=](QString path, QString name, QString exec, QString key){
            createNewShortcut(path, name, exec, key, true, true);
            m_shortcutInterface->call("updateShortcut");
        });
        UkccCommon::buriedSettings("Shortcut", QString("AddBtn"), QString("clicked"));
        addDialog->exec();
    });

    // AddButton 上下文已有「添加」翻译；勿用 ShortcutUi::tr("Add")（可能未收录）
    addButton->setAccessibleName(QCoreApplication::translate("AddButton", "Add"));
    addButton->setAccessibleDescription(QString());
}

void ShortcutUi::createNewShortcut(QString path, QString name, QString exec, QString key, bool buildFlag, bool convertFlag, DoubleClickShortCut *shortCut)
{
    if (key.contains("Meta")) {
        key.replace("Meta", "Win");
    }
    if (key.contains("Print")) {
        key.replace("Print", "PrtSc");
    }
    if (key.contains("Prtsc")) {
        key.replace("Prtsc", "PrtSc");
    }
    qDebug() << "createNewShortcut" << path << name << exec << key;
    QString availablepath;
    if (path.isEmpty()) {
        availablepath = findFreePath(); // 创建快捷键

        // 更新数据
        KeyEntry nKeyentry;
        nKeyentry.gsPath = availablepath;
        nKeyentry.nameStr = name;
        if (convertFlag == true) {
            nKeyentry.bindingStr = keyToLib(key);
        } else {
            nKeyentry.bindingStr = key;
        }
        nKeyentry.actionStr = exec;

        g_customEntryList.append(nKeyentry);
        if (true == buildFlag) {
            addCustomShortcut(nKeyentry, 0);
        }
    } else {
        availablepath = path; // 更新快捷键

        if (convertFlag) {
            UkccCommon::buriedSettings(name, exec, QString("settings"), key);
        }
        // 更新数据
        for (int i = 0; i < g_customEntryList.count(); i++) {
            if (g_customEntryList[i].gsPath == availablepath) {
                g_customEntryList[i].nameStr = name;
                g_customEntryList[i].actionStr = exec;
                if (convertFlag == true) {
                     g_customEntryList[i].bindingStr = keyToLib(key);
                } else {
                     g_customEntryList[i].bindingStr = key;
                }

                if (shortCut) {
                    shortCut->setProperty("shortKey", g_customEntryList[i].bindingStr);
                    qDebug() << Q_FUNC_INFO << "custom update property shortKey to" << g_customEntryList[i].bindingStr << exec;
                } else {
                    qDebug() << Q_FUNC_INFO << "custom update shortCut is null" << g_customEntryList[i].bindingStr;
                }
                break;
            }
        }
    }

    const QByteArray id(KEYBINDINGS_CUSTOM_SCHEMA);
    const QByteArray idd(availablepath.toLatin1().data());
    QGSettings *settings = new QGSettings(id, idd, this);
    if (convertFlag == true) {
         settings->set(BINDING_KEY, keyToLib(key));
    } else {
         settings->set(BINDING_KEY, key);
    }

    settings->set(NAME_KEY, name);
    settings->set(ACTION_KEY, exec);

    delete settings;
    settings = nullptr;
}

void ShortcutUi::updateGeneralShort(DoubleClickShortCut *shortCutEdit, QString key, QString value, bool isConflict)
{
    QString convertValue = keyToLib(value);
    qDebug() << Q_FUNC_INFO << key << value << isConflict << convertValue;
    if (isConflict) {
        for (int i = 0; i < g_systemEntryList.count(); i++) {
            QString systemValue = keyToLib(g_systemEntryList[i].valueStr);
            qDebug() << Q_FUNC_INFO << "update systemEntries detect" << systemValue << convertValue << key;
            if (systemValue.toLower() == convertValue.toLower() && !value.isEmpty()) {
                for (int j = 0; j < mGeneralLineEdit.size(); j++) {
                    QString systemLineEditShort = keyToLib(mGeneralLineEdit[j]->property("shortKey").toString());
                    qDebug() << Q_FUNC_INFO << "update systemLineEdit detect" << systemLineEditShort << convertValue << key;
                    if (systemLineEditShort.toLower() == convertValue.toLower() && shortCutEdit != mGeneralLineEdit[j]) {
                        mGeneralLineEdit[j]->blockSignals(true);
                        mGeneralLineEdit[j]->setText(tr("Null"));
                        mGeneralLineEdit[j]->blockSignals(false);
                        mGeneralLineEdit[j]->setProperty("shortKey", "Null");
                        refreshShortcutEditAccessibleName(mGeneralLineEdit[j]);

                        mGeneralName[j]->blockSignals(true);
                        mGeneralName[j]->setText(tr("Null"));
                        mGeneralName[j]->blockSignals(false);
                        updateDefaultButtonStatus(true);
                        qDebug() << Q_FUNC_INFO << "set systemLineEdit Null" << key;
                    }
                }
                qDebug() << Q_FUNC_INFO << "update system shortcut" << g_systemEntryList[i].keyStr << "from" << g_systemEntryList[i].valueStr << "to Null";
                g_systemEntryList[i].valueStr = "Null";
                shortCutEdit->setProperty("shortKey", value);
                m_shortcutInterface->call("setSystemShortcut", g_systemEntryList[i].keyStr, "Null");
            }
        }

        for (int i = 0; i < g_windowEntryList.count(); i++) {
            QString windowValue = keyToLib(g_windowEntryList[i].valueStr);
            qDebug() << Q_FUNC_INFO << "update windowEntries detect" << windowValue << convertValue << key;
            if (windowValue.toLower() == convertValue.toLower() && !value.isEmpty()) {
                for (int j = 0; j < m_windowLineEdit.size(); j++) {
                    QString windowLineEditShort = keyToLib(m_windowLineEdit[j]->property("shortKey").toString());
                    qDebug() << Q_FUNC_INFO << "update windowLineEdit detect" << windowLineEditShort << convertValue << key;
                    if (windowLineEditShort.toLower() == convertValue.toLower() && shortCutEdit != m_windowLineEdit[j]) {
                        m_windowLineEdit[j]->blockSignals(true);
                        m_windowLineEdit[j]->setText(tr("Null"));
                        m_windowLineEdit[j]->blockSignals(false);
                        m_windowLineEdit[j]->setProperty("shortKey", "Null");
                        refreshShortcutEditAccessibleName(m_windowLineEdit[j]);

                        m_windowName[j]->blockSignals(true);
                        m_windowName[j]->setText(tr("Null"));
                        m_windowName[j]->blockSignals(false);
                        updateDefaultButtonStatus(true);
                        qDebug() << Q_FUNC_INFO << "set windowLineEdit Null" << key;
                    }
                }
                QString oldShortCut = g_windowEntryList[i].valueStr;
                g_windowEntryList[i].valueStr = "Null";
                m_shortcutInterface->call("setWindowShortcut", g_windowEntryList[i].keyStr, oldShortCut, "");
                qDebug() << Q_FUNC_INFO << "update window shortcut" << g_windowEntryList[i].keyStr << "from" << oldShortCut << "to Null";
            }
        }

        for (int i = 0; i < g_customEntryList.count(); i++) {
            QString customValue = keyToLib(g_customEntryList[i].bindingStr);
            qDebug() << Q_FUNC_INFO << "update customEntries detect" << customValue << convertValue << key;
            if (customValue.toLower() == convertValue.toLower()) {
                for (int j = 0; j < mCustomLineEdit.size(); j++) {
                    QString customLineEditShort = keyToLib(mCustomLineEdit[j]->property("shortKey").toString());
                    qDebug() << Q_FUNC_INFO << "update customLineEdit detect" << customLineEditShort << g_customEntryList[i].bindingStr << key;
                    if (mCustomLineEdit[j] && customLineEditShort.toLower() == convertValue.toLower() && shortCutEdit != mCustomLineEdit[j]) {
                        mCustomLineEdit[j]->blockSignals(true);
                        mCustomLineEdit[j]->setText(tr("Null"));
                        mCustomLineEdit[j]->blockSignals(false);
                        mCustomLineEdit[j]->setProperty("shortKey", "Null");
                        refreshShortcutEditAccessibleName(mCustomLineEdit[j]);

                        mCustomName[j]->blockSignals(true);
                        mCustomName[j]->setText(tr("Null"));
                        mCustomName[j]->blockSignals(false);
                        qDebug() << Q_FUNC_INFO << "set customLineEdit Null" << key;
                    }
                }
                qDebug() << Q_FUNC_INFO << "update custom shortcut" << g_customEntryList[i].nameStr << "from" << g_customEntryList[i].bindingStr << "to Null";
                g_customEntryList[i].bindingStr = "Null";
                m_shortcutInterface->call("setCustomShortcut", g_customEntryList[i].gsPath, "Null");
            }
        }
    }
    int g_customEntriesSize = g_customEntryList.count();
    for (int i = 0; i < g_customEntriesSize; i++) { //bug#205721
        qDebug() << Q_FUNC_INFO << "update customEntries detect" << key << g_customEntryList[i].nameStr;
        if (key == g_customEntryList[i].nameStr) {
            qDebug() << Q_FUNC_INFO << "update custom shortcut" << key << "from" << g_customEntryList[i].bindingStr << "to" << convertValue;
            g_customEntryList[i].bindingStr = convertValue;
            shortCutEdit->setProperty("shortKey", value);
            if (convertValue.contains("Super")) {
                convertValue.replace("Super", "Win");
            }
            m_shortcutInterface->call("setCustomShortcut", g_customEntryList[i].gsPath, convertValue);
        }
    }
    for (int i = 0; i < g_systemEntryList.count(); i++) {
        qDebug() << Q_FUNC_INFO << "update systemEntries detect" << key << g_systemEntryList[i].keyStr;
        if (key == g_systemEntryList[i].keyStr) {
            qDebug() << Q_FUNC_INFO << "update system shortcut" << key << "from" << g_systemEntryList[i].valueStr << "to" << convertValue;
            g_systemEntryList[i].valueStr = convertValue;
            shortCutEdit->setProperty("shortKey", value);
            if (convertValue.contains("Super")) {
                convertValue.replace("Super", "Win");
            }
            m_shortcutInterface->call("setSystemShortcut", key, convertValue);
        }
    }
    for (int i = 0; i < g_windowEntryList.count(); i++) {
        qDebug() << Q_FUNC_INFO << "update windowEntries detect" << key << g_windowEntryList[i].keyStr;
        if (key == g_windowEntryList[i].keyStr) {
            QString oldValue = g_windowEntryList[i].valueStr;
            g_windowEntryList[i].valueStr = value;
            shortCutEdit->setProperty("shortKey", value);
            if (oldValue == "Null") {
                oldValue = "";
            }
            if (value == "Null") {
                value = "";
            }
            if (value.contains("↑")) {
                value.replace("↑", "Up");
            }
            if (value.contains("→")) {
                value.replace("→", "Right");
            }
            if (value.contains("←")) {
                value.replace("←", "Left");
            }
            if (value.contains("↓")) {
                value.replace("↓", "Down");
            }
            m_shortcutInterface->call("setWindowShortcut", key, oldValue, value);
            qDebug() << Q_FUNC_INFO << "update window shortcut" << key << "from" << oldValue << "to" << value;
        }
    }
}

void ShortcutUi::lineEditFocusOutSlot(DoubleClickLineEdit* lineEdit,
                                      ClickFixLabel *fixLabel)
{
    if (!lineEdit || !fixLabel) {
        return;
    }
    // 常驻编辑框失焦后仍显示，保证 Tab 顺序：名称 → 快捷键 → 更多
    if (lineEdit->property("ukccAlwaysShowEdit").toBool()) {
        refreshShortcutEditAccessibleName(lineEdit);
        return;
    }
    fixLabel->show();
    lineEdit->hide();
    // 隐藏后勿再进 Tab 链；勿在此抢焦点（否则会打断点到其它控件）
    lineEdit->setFocusPolicy(Qt::NoFocus);
}

void ShortcutUi::labelDoubleClickSlot(ClickFixLabel *fixLabel,
                                      DoubleClickShortCut* lineEdit)
{
    if (!fixLabel || !lineEdit) {
        return;
    }
    // 常驻编辑框：灰框仅作双击入口时的兼容，焦点直接给编辑框
    if (lineEdit->property("ukccAlwaysShowEdit").toBool()) {
        refreshShortcutEditAccessibleName(lineEdit);
        if (QLabel *nameLab = qobject_cast<QLabel *>(
                    lineEdit->property("ukccNameLabel").value<QWidget *>())) {
            nameLab->setBuddy(lineEdit);
        }
        lineEdit->setFocus(Qt::MouseFocusReason);
        return;
    }
    fixLabel->hide();
    lineEdit->setProperty("ukccFixLabel", QVariant::fromValue(static_cast<QWidget *>(fixLabel)));
    lineEdit->setFocusPolicy(Qt::StrongFocus);
    refreshShortcutEditAccessibleName(lineEdit);
    if (QLabel *nameLab = qobject_cast<QLabel *>(
                lineEdit->property("ukccNameLabel").value<QWidget *>())) {
        nameLab->setBuddy(lineEdit);
    }
    lineEdit->show();
    lineEdit->setFocus(Qt::TabFocusReason);
}

void ShortcutUi::labelDoubleClickSlot(ClickFixLabel *fixLabel,
                                      DoubleClickLineEdit* lineEdit)
{
    if (!fixLabel || !lineEdit) {
        return;
    }
    if (lineEdit->property("ukccAlwaysShowEdit").toBool()) {
        refreshShortcutEditAccessibleName(lineEdit);
        lineEdit->setFocus(Qt::MouseFocusReason);
        return;
    }
    fixLabel->hide();
    lineEdit->setProperty("ukccFixLabel", QVariant::fromValue(static_cast<QWidget *>(fixLabel)));
    lineEdit->setFocusPolicy(Qt::StrongFocus);
    refreshShortcutEditAccessibleName(lineEdit);
    lineEdit->show();
    lineEdit->setFocus(Qt::TabFocusReason);
}

void ShortcutUi::lineEditFocusOutSlot(DoubleClickShortCut* lineEdit,
                                      ClickFixLabel *fixLabel)
{
    if (!lineEdit || !fixLabel) {
        return;
    }
    // 常驻编辑框失焦后仍显示，与添加对话框「按键」一致，保证 setBuddy 持续有效
    if (lineEdit->property("ukccAlwaysShowEdit").toBool()) {
        refreshShortcutEditAccessibleName(lineEdit);
        return;
    }
    fixLabel->show();
    lineEdit->hide();
    lineEdit->setFocusPolicy(Qt::NoFocus);
}

void ShortcutUi::lineEditChangeSlot(DoubleClickShortCut* lineEdit,
                                    ClickFixLabel *fixLabel,
                                    QString key)
{
    if (!lineEdit || !fixLabel) {
        return;
    }
    QString text = lineEdit->text();
    QString shortCutValue = lineEdit->keySequence().toString();
    if (text.contains("Start") && shortCutValue.contains("Meta")) {
        shortCutValue.replace("Meta", "Start");
        text.replace("Start", "Super");
    }
    updateGeneralShort(lineEdit, key, shortCutValue, false);
    lineEdit->blockSignals(true);
    lineEdit->setText(getShowShortcutString(lineEdit->text()));
    lineEdit->blockSignals(false);
    fixLabel->setText(text);
    refreshShortcutEditAccessibleName(lineEdit);
    updateDefaultButtonStatus(true);
}

void ShortcutUi::customLineEditStrChangeSlot(DoubleClickLineEdit* lineEdit,
                                             ClickFixLabel *fixLabel,
                                             KeyEntry* keyEntry)
{
    if (!lineEdit || !fixLabel || !keyEntry) {
        return;
    }
    QString currentBinding = keyEntry->bindingStr;
    for (const auto &entry : g_customEntryList) {
        if (entry.gsPath == keyEntry->gsPath || entry.actionStr == keyEntry->actionStr) {
            currentBinding = entry.bindingStr;
            break;
        }
    }
    createNewShortcut(keyEntry->gsPath, lineEdit->text(), keyEntry->actionStr, currentBinding, false, false);
    m_shortcutInterface->call("updateShortcut");
    mAllKeyI18nName.insert(keyEntry->actionStr, lineEdit->text());
    fixLabel->setText(lineEdit->text());
    refreshShortcutEditAccessibleName(lineEdit);
    // 名称变更后，绑定编辑框读屏前缀同步为新名称
    for (DoubleClickShortCut *bindingEdit : mCustomLineEdit) {
        if (!bindingEdit) {
            continue;
        }
        if (bindingEdit->property("shortcutId").toString() == keyEntry->actionStr) {
            bindShortcutEditAccessibleName(bindingEdit, lineEdit->text());
            break;
        }
    }
}

void ShortcutUi::customLineEditChangeSlot(DoubleClickShortCut* lineEdit,
                                          ClickFixLabel *fixLabel,
                                          KeyEntry* keyEntry)
{
    if (!lineEdit || !fixLabel || !keyEntry) {
        return;
    }
    QString currentName = keyEntry->nameStr;
    for (const auto &entry : g_customEntryList) {
        if (entry.gsPath == keyEntry->gsPath || entry.actionStr == keyEntry->actionStr) {
            currentName = entry.nameStr;
            break;
        }
    }
    createNewShortcut(keyEntry->gsPath, currentName, keyEntry->actionStr, lineEdit->keySequence().toString(), false, true, lineEdit);
    m_shortcutInterface->call("updateShortcut");
    lineEdit->blockSignals(true);
    QString text = lineEdit->text();
    if (text.contains("Start")) {
        text.replace("Start", "Super");
    }
    lineEdit->setText(getShowShortcutString(text));
    lineEdit->blockSignals(false);
    fixLabel->setText(lineEdit->text());
    refreshShortcutEditAccessibleName(lineEdit);
}

void ShortcutUi::customLineEditConflictSlot(DoubleClickShortCut* lineEdit,
                                ClickFixLabel *fixLabel,
                                KeyEntry* keyEntry) {
    if (!lineEdit || !fixLabel || !keyEntry)
    {
        return;
    }
    QPointer<DoubleClickShortCut> lineEditGuard(lineEdit);
    QPointer<ClickFixLabel> fixLabelGuard(fixLabel);
    QString tipValue = conflictTip(lineEdit->m_conflictKey, lineEdit->m_conflictValue);
    QString text = lineEdit->text();
    QString shortCutValue = lineEdit->keySequence().toString();
    if (text.contains("Start")) {
        text.replace("Start", "Super");
    }
    if (text.contains("Meta")) {
        text.replace("Meta", "Super");
    }
    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Warning);
    msg.addButton(tr("Cancel"), QMessageBox::NoRole);
    msg.addButton(tr("Continue"), QMessageBox::YesRole);
    msg.setText(tr("Shortcut key conflict, use it?"));
    msg.setInformativeText(QString(tr("\"%1\" occuied, using this combination will invalidate \"%2\".")).
                           arg(text).arg(tipValue));
    int ret = msg.exec();
    if (!lineEditGuard || !fixLabelGuard) {
        return;
    }
    if (ret) {
        if (text.contains("Start") && shortCutValue.contains("Meta")) {
            shortCutValue.replace("Meta", "Super");
            text.replace("Start", "Super");
        }
        QString toName = keyEntry->nameStr;
        for (int i = 0; i < g_customEntryList.size(); i++) {
            if (keyEntry->actionStr == g_customEntryList[i].actionStr) {
                toName = g_customEntryList[i].nameStr;
                qDebug() << Q_FUNC_INFO << "get new customEntries nameStr" << g_customEntryList[i].actionStr << "from" << keyEntry->nameStr << "to" << g_customEntryList[i].nameStr;
                break;
            }
        }
        updateGeneralShort(lineEditGuard.data(), toName, shortCutValue, true);
        lineEditGuard->blockSignals(true);
        lineEditGuard->setText(getShowShortcutString(text));
        lineEditGuard->blockSignals(false);
        fixLabelGuard->setText(getShowShortcutString(text));
    } else {
        Q_EMIT fixLabelGuard->doubleClicked();
    }
}

void ShortcutUi::lineEditConflictSlot(DoubleClickShortCut* lineEdit,
                                      ClickFixLabel *fixLabel,
                                      QString key)
{
    if (!lineEdit || !fixLabel) {
        return;
    }
    QPointer<DoubleClickShortCut> lineEditGuard(lineEdit);
    QPointer<ClickFixLabel> fixLabelGuard(fixLabel);
    QString tipValue = conflictTip(lineEdit->m_conflictKey, lineEdit->m_conflictValue);
    QString text = lineEdit->text();
    // 弹窗前先保存，避免 keyPressEvent 重入后对象状态异常
    QString shortCutValue = lineEdit->keySequence().toString();
    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Warning);
    msg.addButton(tr("Cancel"), QMessageBox::NoRole);
    msg.addButton(tr("Use"), QMessageBox::ApplyRole);
    msg.setText(tr("Shortcut key conflict, use it?"));
    msg.setInformativeText(QString(tr("%1 occuied, using this combination will invalidate %2")).
                           arg(text).arg(tipValue));
    int ret = msg.exec();

    if (!lineEditGuard || !fixLabelGuard) {
        return;
    }
    if (ret == 1) {
        if (text.contains("Start") && shortCutValue.contains("Meta")) {
            shortCutValue.replace("Meta", "Start");
            text.replace("Start", "Super");
        }
        updateGeneralShort(lineEditGuard.data(), key, shortCutValue, true);
        lineEditGuard->blockSignals(true);
        lineEditGuard->setText(getShowShortcutString(text));
        lineEditGuard->blockSignals(false);
        fixLabelGuard->setText(getShowShortcutString(text));
        updateDefaultButtonStatus(true);
    } else {
        Q_EMIT fixLabelGuard->doubleClicked();
    }
}

void ShortcutUi::customAddDialogSlot(AddShortcutDialog *addDialog,
                                     DoubleClickLineEdit *lineEdit,
                                     DoubleClickShortCut *bingdLineEdit,
                                     ClickFixLabel* fixLabel,
                                     ClickFixLabel* bindFixLabel,
                                     KeyEntry* keyEntry,
                                     QString name, QString exec, QString key)
{
    if (!addDialog || !lineEdit || !bingdLineEdit || !fixLabel  || !bindFixLabel || !keyEntry)
    {
        return;
    }
    createNewShortcut(keyEntry->gsPath, name, exec, key, false);
    m_shortcutInterface->call("updateShortcut");
    lineEdit->setToolTip(name);
    lineEdit->blockSignals(true);
    lineEdit->setText(name);
    lineEdit->blockSignals(false);
    bingdLineEdit->blockSignals(true);
    QString mKey = getShowShortcutString(addDialog->keyToLib(key));
    if (mKey.contains("Meta")) {
        mKey.replace("Meta", "Super");
    }
    if (mKey.contains("Win")) {
        mKey.replace("Win", "Super");
    }
    if (mKey.contains("Print")) {
        mKey.replace("Print", "PrtSc");
    }
    if (mKey.contains("Prtsc")) {
        mKey.replace("Prtsc", "PrtSc");
    }
    if (mKey == "") {
        mKey = tr("Null");
    }

    QString toSeq = keyToSequence(key);
    bingdLineEdit->setKeySequence(toSeq);
    bingdLineEdit->setText(mKey);
    bingdLineEdit->setProperty("shortKey", toSeq);
    bingdLineEdit->blockSignals(false);
    fixLabel->setText(name);
    bindFixLabel->setText(bingdLineEdit->text());
    mAllKeyI18nName.insert(exec, name);
    bindShortcutEditAccessibleName(lineEdit, tr("Name"));
    bindShortcutEditAccessibleName(bingdLineEdit, name);
    qDebug() << Q_FUNC_INFO << "shortcutInfoSignal" << mKey << name << exec << bingdLineEdit->keySequence();
}

void ShortcutUi::lineEditOccupySlot(DoubleClickShortCut* lineEdit,
                                    ClickFixLabel *fixLabel)
{
    if (!lineEdit || !fixLabel) {
        return;
    }
    QPointer<ClickFixLabel> fixLabelGuard(fixLabel);
    QMessageBox msg(this);
    msg.setIcon(QMessageBox::Warning);
    QString text = lineEdit->m_conflictKey;
    if (text.contains("Start")) {
        text.replace("Start", "Super");
    }
    if (text.contains("Meta")) {
        text.replace("Meta", "Super");
    }
    msg.setText(QString(tr("Shortcut \"%1\" occuied, please change the key combination")).arg(text));
    msg.exec();
    if (fixLabelGuard) {
        Q_EMIT fixLabelGuard->doubleClicked();
    }
}

void ShortcutUi::addSystemShortcut(const QString &name, const QString &shortcut, const QString &mediaKey, int systemIndex)
{
    int editWidth = 286;
    QString keyStr = mediaKey;
    QString value = shortcut;
    QStringList twiceValues;

    UkccFrame              *shortCutFrame         = new UkccFrame(this, UkccFrame::BorderRadiusStyle::Around, true);

    KLabel                 *nameLabel             = new KLabel(shortCutFrame);
    // 勿 KDK_EXTEND_ALL_INFO：会冲掉 setBuddy 的 LABELLED_BY，且 accessibleName 变成调试串
    KDK_EXTEND_OBJ_INFO_FORMAT(nameLabel, "Shortcut", "systemNameLabel" + QString(systemIndex));
    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        nameLabel->setAlignment(Qt::AlignTop|Qt::AlignRight);
    }
    ClickFixLabel          *bingdingLabel         = new ClickFixLabel(shortCutFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(bingdingLabel, "Shortcut", "bingdingLabel" + QString(systemIndex), "");
    ClickFixLabel          *bingdingLabelTwice    = nullptr;

    DoubleClickShortCut    *bingdingLineEditTwice = nullptr;
    DoubleClickShortCut    *bingdingLineEdit      = new DoubleClickShortCut(g_systemEntryList, g_windowEntryList, g_customEntryList, shortCutFrame);
    // 勿 ALL_INFO：会覆盖下方「名称 + 按键」accessibleName
    KDK_EXTEND_OBJ_INFO_FORMAT(bingdingLineEdit, "Shortcut", "systemLineEdit" + QString(systemIndex));
    connect(bingdingLineEdit, &DoubleClickShortCut::conflictDetect, this, [=](QKeySequence seq) {
        conflictDetectSlot(bingdingLineEdit, seq);
    });
    QHBoxLayout            *gHorLayout            = new QHBoxLayout(shortCutFrame);
    QHBoxLayout            *lineEditLayout        = new QHBoxLayout;
    QHBoxLayout            *lineEditLayoutTwice   = new QHBoxLayout;

#ifndef Nile
    KLabel *bindingLabel = new KLabel(shortCutFrame);
    bindingLabel->setText(getShowShortcutString(shortcut));
    bindingLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
#endif
    nameLabel->setText(name);
    // 与添加对话框一致：常驻编辑框 + setBuddy + accessibleName=快捷键名称
    bingdingLineEdit->setProperty("ukccAlwaysShowEdit", true);
    bindShortcutEditAccessibleName(bingdingLineEdit, name);
    bingdingLineEdit->setProperty("ukccNameLabel", QVariant::fromValue(static_cast<QWidget *>(nameLabel)));
    nameLabel->setBuddy(bingdingLineEdit);
    nameLabel->setFocusPolicy(Qt::NoFocus);
    bingdingLineEdit->setFocusPolicy(Qt::StrongFocus);

    gHorLayout->setContentsMargins(16, 8, 16, 8);

    bool isTwice = false;

    if (shortcut.contains("or")) {
        editWidth = 130;
        isTwice = true;
        twiceValues = shortcut.split("or");
        value = twiceValues.at(1);

        bingdingLineEditTwice = new DoubleClickShortCut(g_systemEntryList, g_windowEntryList, g_customEntryList, shortCutFrame);
        KDK_EXTEND_OBJ_INFO_FORMAT(bingdingLineEditTwice, "Shortcut", "systemLineEditTwice" + QString(systemIndex));
        connect(bingdingLineEditTwice, &DoubleClickShortCut::conflictDetect, this, [=](QKeySequence seq) {
            conflictDetectSlot(bingdingLineEditTwice, seq);
        });
        bingdingLineEditTwice->setAttribute(Qt::WA_InputMethodEnabled, false);
        bingdingLineEditTwice->setProperty("ukccAlwaysShowEdit", true);
        bingdingLineEditTwice->setFocusPolicy(Qt::StrongFocus);
        bingdingLabelTwice = new ClickFixLabel(shortCutFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(bingdingLabelTwice, "Shortcut", "systemLabelTwice" + QString(systemIndex), "");
        bingdingLabelTwice->setStyleSheet("QLabel{background-color:palette(button);border-radius: 4px}");
        bingdingLabelTwice->setFocusPolicy(Qt::NoFocus);

        bingdingLineEditTwice->setFixedSize(150, 36);

        bingdingLabelTwice->setFixedSize(150, 36);
        QString showText = getShowShortcutString(value).trimmed();
        if (showText == "Null") {
            showText = tr("Null");
        }
        bingdingLabelTwice->setText(showText);
        bingdingLabelTwice->setAccessibleName(QStringLiteral(" "));
        bindShortcutEditAccessibleName(bingdingLineEditTwice, name);
        bingdingLineEditTwice->setProperty("ukccNameLabel", QVariant::fromValue(static_cast<QWidget *>(nameLabel)));

        // 常驻编辑框：只放 LineEdit，灰框隐藏仅用于同步文案
        lineEditLayoutTwice->addWidget(bingdingLineEditTwice, 1);
        lineEditLayoutTwice->addWidget(bingdingLabelTwice, 1);
        bingdingLabelTwice->setVisible(false);
        bingdingLineEditTwice->setVisible(true);

        bingdingLineEditTwice->blockSignals(true);
        bingdingLineEditTwice->setText(showText);
        bingdingLineEditTwice->setKeySequence(QKeySequence(value));
        bingdingLineEditTwice->setProperty("shortKey", value);
        bingdingLineEditTwice->setProperty("shortcutId", keyStr + "2");
        bingdingLineEditTwice->blockSignals(false);
        refreshShortcutEditAccessibleName(bingdingLineEditTwice);

        mAllKeyI18nName.insert(keyStr + "2", QString(name));

        connect(bingdingLabelTwice, &ClickFixLabel::doubleClicked, this, [=]() {
            labelDoubleClickSlot(bingdingLabelTwice, bingdingLineEditTwice);
        });

        connect(bingdingLineEditTwice, &DoubleClickShortCut::focusOut, this, [=]() {
            lineEditFocusOutSlot(bingdingLineEditTwice, bingdingLabelTwice);
        });

        connect(bingdingLineEditTwice, &DoubleClickShortCut::shortcutChanged, this, [=]() {
            lineEditChangeSlot(bingdingLineEditTwice, bingdingLabelTwice, keyStr + "2");
        });

        connect(bingdingLineEditTwice, &DoubleClickShortCut::shortcutConflict, this, [=]() {
            lineEditConflictSlot(bingdingLineEditTwice, bingdingLabelTwice, keyStr + "2");
        });
    }

    if (isTwice) value = twiceValues.at(0);

    mGeneralLineEdit.append(bingdingLineEdit);
    mGeneralName.append(bingdingLabel);
    // 视觉顺序：第一个编辑框 → 第二个；入表顺序与布局一致
    if (isTwice && bingdingLineEditTwice && bingdingLabelTwice) {
        mGeneralLineEdit.append(bingdingLineEditTwice);
        mGeneralName.append(bingdingLabelTwice);
    }

    QString showText = getShowShortcutString(value).trimmed();
    if (showText == "Null") {
        showText = tr("Null");
    }
    bingdingLabel->setText(showText);
    bingdingLabel->setFixedSize(editWidth, 36);
    bingdingLabel->setStyleSheet("QLabel{background-color:palette(button);border-radius: 4px}");
    bingdingLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    bingdingLabel->setAccessibleName(QStringLiteral(" "));
    bingdingLabel->setFocusPolicy(Qt::NoFocus);

    bingdingLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    bingdingLineEdit->setFixedSize(editWidth, 36);
    bingdingLineEdit->setAlignment(Qt::AlignLeft);
    bingdingLineEdit->blockSignals(true);
    bingdingLineEdit->setText(showText);
    bingdingLineEdit->setKeySequence(QKeySequence(value));
    bingdingLineEdit->setProperty("shortKey", value);
    bingdingLineEdit->setProperty("shortcutId", keyStr);
    bingdingLineEdit->blockSignals(false);
    refreshShortcutEditAccessibleName(bingdingLineEdit);
    nameLabel->setBuddy(bingdingLineEdit);
    // 常驻显示编辑框，灰框隐藏
    bingdingLineEdit->setVisible(true);
    bingdingLabel->setVisible(false);

    QSizePolicy policy;
    policy = bingdingLabel->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    bingdingLabel->setSizePolicy(policy);

    policy = bingdingLineEdit->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    bingdingLineEdit->setSizePolicy(policy);

    policy = bingdingLabel->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    bingdingLabel->setSizePolicy(policy);

    mAllKeyI18nName.insert(keyStr, name);

    lineEditLayout->addWidget(bingdingLineEdit, 1);
    lineEditLayout->addWidget(bingdingLabel, 1);

    gHorLayout->addWidget(nameLabel, 1);

#ifdef Nile
    gHorLayout->addStretch();
    gHorLayout->addLayout(lineEditLayout, 1);
    gHorLayout->addLayout(lineEditLayoutTwice, 1);
#else
    bingdingLineEdit->setVisible(false);
    bingdingLabel->setVisible(false);
    if (bingdingLineEditTwice != nullptr) {
        bingdingLineEditTwice->setVisible(false);
        bingdingLabelTwice->setVisible(false);
    }
    gHorLayout->addWidget(bindingLabel, 1);
#endif

    shortCutFrame->setLayout(gHorLayout);

    systemShortcutGroup->addWidget(shortCutFrame);

    connect(bingdingLabel,&ClickFixLabel::doubleClicked,this,[=](){
        labelDoubleClickSlot(bingdingLabel, bingdingLineEdit);
    });

    connect(bingdingLineEdit,&DoubleClickShortCut::focusOut,this,[=](){
        lineEditFocusOutSlot(bingdingLineEdit, bingdingLabel);
    });

    connect(bingdingLineEdit, &DoubleClickShortCut::shortcutChanged, this, [=]() {
        lineEditChangeSlot(bingdingLineEdit, bingdingLabel, keyStr);
    });

    connect(bingdingLineEdit, &DoubleClickShortCut::shortcutConflict, this, [=]() {
        lineEditConflictSlot(bingdingLineEdit, bingdingLabel, keyStr);
    });

    connect(bingdingLineEdit, &DoubleClickShortCut::shortcutOccupy, this, [=]() {
        lineEditOccupySlot(bingdingLineEdit, bingdingLabel);
    });
}

void ShortcutUi::addWindowShortcut(const QString &name, const QString &shortcut, const QString &mediaKey, int windowIndex)
{
    int editWidth = 286;
    QString keyStr = mediaKey;
    QString value = shortcut;
    QStringList twiceValues;

    UkccFrame              *shortCutFrame         = new UkccFrame(this, UkccFrame::BorderRadiusStyle::Around, true);

    KLabel                 *nameLabel             = new KLabel(shortCutFrame);
    // 勿 KDK_EXTEND_ALL_INFO：会冲掉 setBuddy 的 LABELLED_BY，且 accessibleName 变成调试串
    KDK_EXTEND_OBJ_INFO_FORMAT(nameLabel, "Shortcut", "windowNameLabel" + QString(windowIndex));
    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        nameLabel->setAlignment(Qt::AlignTop|Qt::AlignRight);
    }
    ClickFixLabel          *bingdingLabel         = new ClickFixLabel(shortCutFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(bingdingLabel, "Shortcut", "bingdingLabel" + QString(windowIndex), "");
    ClickFixLabel          *bingdingLabelTwice    = nullptr;

    DoubleClickShortCut    *bingdingLineEditTwice = nullptr;
    DoubleClickShortCut    *bingdingLineEdit      = new DoubleClickShortCut(g_systemEntryList, g_windowEntryList, g_customEntryList, shortCutFrame);
    // 勿 ALL_INFO：会覆盖下方「名称 + 按键」accessibleName
    KDK_EXTEND_OBJ_INFO_FORMAT(bingdingLineEdit, "Shortcut", "windowLineEdit" + QString(windowIndex));
    connect(bingdingLineEdit, &DoubleClickShortCut::conflictDetect, this, [=](QKeySequence seq) {
        conflictDetectSlot(bingdingLineEdit, seq);
    });
    QHBoxLayout            *gHorLayout            = new QHBoxLayout(shortCutFrame);
    QHBoxLayout            *lineEditLayout        = new QHBoxLayout;
    QHBoxLayout            *lineEditLayoutTwice   = new QHBoxLayout;

#ifndef Nile
    FixLabel *bindingLabel = new FixLabel(shortCutFrame);
    bindingLabel->setText(getShowShortcutString(shortcut));
    bindingLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
#endif
    nameLabel->setText(name);
    // 与添加对话框一致：常驻编辑框 + setBuddy + accessibleName=快捷键名称
    bingdingLineEdit->setProperty("ukccAlwaysShowEdit", true);
    bindShortcutEditAccessibleName(bingdingLineEdit, name);
    bingdingLineEdit->setProperty("ukccNameLabel", QVariant::fromValue(static_cast<QWidget *>(nameLabel)));
    nameLabel->setBuddy(bingdingLineEdit);
    nameLabel->setFocusPolicy(Qt::NoFocus);
    bingdingLineEdit->setFocusPolicy(Qt::StrongFocus);

    gHorLayout->setContentsMargins(16, 8, 16, 8);

    bool isTwice = false;

    if (shortcut.contains("or")) {
        editWidth = 130;
        isTwice = true;
        twiceValues = shortcut.split("or");
        value = twiceValues.at(1);

        bingdingLineEditTwice = new DoubleClickShortCut(g_systemEntryList, g_windowEntryList, g_customEntryList, shortCutFrame);
        KDK_EXTEND_OBJ_INFO_FORMAT(bingdingLineEditTwice, "Shortcut", "windowLineEditTwice" + QString(windowIndex));
        connect(bingdingLineEditTwice, &DoubleClickShortCut::conflictDetect, this, [=](QKeySequence seq) {
            conflictDetectSlot(bingdingLineEditTwice, seq);
        });
        bingdingLineEditTwice->setAttribute(Qt::WA_InputMethodEnabled, false);
        bingdingLineEditTwice->setProperty("ukccAlwaysShowEdit", true);
        bingdingLineEditTwice->setFocusPolicy(Qt::StrongFocus);
        bingdingLabelTwice = new ClickFixLabel(shortCutFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(bingdingLabelTwice, "Shortcut", "windowLabelTwice" + QString(windowIndex), "");
        bingdingLabelTwice->setStyleSheet("QLabel{background-color:palette(button);border-radius: 4px}");
        bingdingLabelTwice->setFocusPolicy(Qt::NoFocus);

        bingdingLineEditTwice->setFixedSize(150, 36);

        bingdingLabelTwice->setFixedSize(150, 36);
        QString showText = getShowShortcutString(value).trimmed();
        if (showText == "Null") {
            showText = tr("Null");
        }
        bingdingLabelTwice->setText(showText);
        bingdingLabelTwice->setAccessibleName(QStringLiteral(" "));
        bindShortcutEditAccessibleName(bingdingLineEditTwice, name);
        bingdingLineEditTwice->setProperty("ukccNameLabel", QVariant::fromValue(static_cast<QWidget *>(nameLabel)));

        lineEditLayoutTwice->addWidget(bingdingLineEditTwice, 1);
        lineEditLayoutTwice->addWidget(bingdingLabelTwice, 1);
        bingdingLabelTwice->setVisible(false);
        bingdingLineEditTwice->setVisible(true);

        bingdingLineEditTwice->blockSignals(true);
        bingdingLineEditTwice->setText(showText);
        bingdingLineEditTwice->setKeySequence(QKeySequence(value));
        bingdingLineEditTwice->setProperty("shortKey", value);
        bingdingLineEditTwice->setProperty("shortcutId", keyStr + "2");
        bingdingLineEditTwice->blockSignals(false);
        refreshShortcutEditAccessibleName(bingdingLineEditTwice);

        mAllKeyI18nName.insert(keyStr + "2", QString(name));

        connect(bingdingLabelTwice, &ClickFixLabel::doubleClicked, this, [=]() {
            labelDoubleClickSlot(bingdingLabelTwice, bingdingLineEditTwice);
        });

        connect(bingdingLineEditTwice, &DoubleClickShortCut::focusOut, this, [=]() {
            lineEditFocusOutSlot(bingdingLineEditTwice, bingdingLabelTwice);
        });

        connect(bingdingLineEditTwice, &DoubleClickShortCut::shortcutChanged, this, [=]() {
            lineEditChangeSlot(bingdingLineEditTwice, bingdingLabelTwice, keyStr + "2");
        });

        connect(bingdingLineEditTwice, &DoubleClickShortCut::shortcutConflict, this, [=]() {
            lineEditConflictSlot(bingdingLineEditTwice, bingdingLabelTwice, keyStr + "2");
        });

        connect(bingdingLineEditTwice, &DoubleClickShortCut::shortcutOccupy, this, [=]() {
            lineEditOccupySlot(bingdingLineEditTwice, bingdingLabelTwice);
        });
    }

    if (isTwice) {
        value = twiceValues.at(0).trimmed();
    }

    m_windowLineEdit.append(bingdingLineEdit);
    m_windowName.append(bingdingLabel);
    // 视觉顺序：第一个编辑框 → 第二个；入表顺序与布局一致
    if (isTwice && bingdingLineEditTwice && bingdingLabelTwice) {
        m_windowLineEdit.append(bingdingLineEditTwice);
        m_windowName.append(bingdingLabelTwice);
    }

    QString showText = getShowShortcutString(value).trimmed();
    if (showText == "Null") {
        showText = tr("Null");
    }
    bingdingLabel->setText(showText);
    bingdingLabel->setFixedSize(editWidth, 36);
    bingdingLabel->setStyleSheet("QLabel{background-color:palette(button);border-radius: 4px}");
    bingdingLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    bingdingLabel->setAccessibleName(QStringLiteral(" "));
    bingdingLabel->setFocusPolicy(Qt::NoFocus);

    bingdingLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    bingdingLineEdit->setFixedSize(editWidth, 36);
    bingdingLineEdit->setAlignment(Qt::AlignLeft);
    bingdingLineEdit->blockSignals(true);
    bingdingLineEdit->setText(showText);
    bingdingLineEdit->setKeySequence(QKeySequence(value));
    bingdingLineEdit->setProperty("shortKey", value);
    bingdingLineEdit->setProperty("shortcutId", keyStr);
    bingdingLineEdit->blockSignals(false);
    refreshShortcutEditAccessibleName(bingdingLineEdit);
    nameLabel->setBuddy(bingdingLineEdit);
    bingdingLineEdit->setVisible(true);
    bingdingLabel->setVisible(false);

    QSizePolicy policy;
    policy = bingdingLabel->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    bingdingLabel->setSizePolicy(policy);

    policy = bingdingLineEdit->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    bingdingLineEdit->setSizePolicy(policy);

    policy = bingdingLabel->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    bingdingLabel->setSizePolicy(policy);

    mAllKeyI18nName.insert(keyStr, name);

    lineEditLayout->addWidget(bingdingLineEdit, 1);
    lineEditLayout->addWidget(bingdingLabel, 1);

    gHorLayout->addWidget(nameLabel, 1);

#ifdef Nile
    gHorLayout->addStretch();
    gHorLayout->addLayout(lineEditLayout, 1);
    gHorLayout->addLayout(lineEditLayoutTwice, 1);
#else
    bingdingLineEdit->setVisible(false);
    bingdingLabel->setVisible(false);
    if (bingdingLineEditTwice != nullptr) {
        bingdingLineEditTwice->setVisible(false);
        bingdingLabelTwice->setVisible(false);
    }
    gHorLayout->addWidget(bindingLabel, 1);
#endif

    shortCutFrame->setLayout(gHorLayout);

    m_windowShortcutGroup->addWidget(shortCutFrame);

    connect(bingdingLabel, &ClickFixLabel::doubleClicked, this, [=]() {
        labelDoubleClickSlot(bingdingLabel, bingdingLineEdit);
    });

    connect(bingdingLineEdit, &DoubleClickShortCut::focusOut, this, [=]() {
        lineEditFocusOutSlot(bingdingLineEdit, bingdingLabel);
    });

    connect(bingdingLineEdit, &DoubleClickShortCut::shortcutChanged, this, [=]() {
        lineEditChangeSlot(bingdingLineEdit, bingdingLabel, keyStr);
    });

    connect(bingdingLineEdit, &DoubleClickShortCut::shortcutConflict, this, [=]() {
        lineEditConflictSlot(bingdingLineEdit, bingdingLabel, keyStr);
    });

    connect(bingdingLineEdit, &DoubleClickShortCut::shortcutOccupy, this, [=]() {
        lineEditOccupySlot(bingdingLineEdit, bingdingLabel);
    });
}

QString ShortcutUi::keyToSequence(QString key) {
    QString oldKey = key;
    key.replace(" ", "");
    if (key.contains("Super")) {
        key.replace("Super", "Meta");
    }

    if (key.contains("Start")) {
        key.replace("Start", "Meta");
    }
    if (key.contains("<")) {
        key.replace("<", "");
    }

    if (key.contains(">")) {
        key.replace(">", "+");
    }
    //qDebug() << Q_FUNC_INFO << "update" << oldKey << "to" << key;
    return key;
}

QList<QFrame*> ShortcutUi::getCustomFrameList() {
    return m_customFrameList;
}

void ShortcutUi::addCustomShortcut(KeyEntry keyEntry, int customIndex)
{
    UkccFrame           *frame            = new UkccFrame(this, UkccFrame::BorderRadiusStyle::Around, true);
    QHBoxLayout         *layout           = new QHBoxLayout(frame);
    QHBoxLayout         *lineEditLayout   = new QHBoxLayout;
    DoubleClickLineEdit *nameLineEdit     = new DoubleClickLineEdit(g_customEntryList, frame);
    // 勿 ALL_INFO：会覆盖「自定义快捷键/名称」accessibleName
    KDK_EXTEND_OBJ_INFO_FORMAT(nameLineEdit, "Shortcut", "customNameLineEdit" + QString(customIndex));
    DoubleClickShortCut *bingdingLineEdit = new DoubleClickShortCut(g_systemEntryList, g_windowEntryList, g_customEntryList, frame);
    // 勿 ALL_INFO：会覆盖「名称 + 按键」accessibleName
    KDK_EXTEND_OBJ_INFO_FORMAT(bingdingLineEdit, "Shortcut", "customBindingLineEdit" + QString(customIndex));
    connect(bingdingLineEdit, &DoubleClickShortCut::conflictDetect, this, [=](QKeySequence seq) {
        conflictDetectSlot(bingdingLineEdit, seq);
    });
    ClickFixLabel       *nameLabel        = new ClickFixLabel(frame);
    // 勿 ALL_INFO：作绑定框 buddy 时勿把 accessibleName 写成调试串
    KDK_EXTEND_OBJ_INFO_FORMAT(nameLabel, "Shortcut", "customNameLabel" + QString(customIndex));
    QString systemLang = QLocale::system().name();
    if(systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ"){
        nameLabel->setAlignment(Qt::AlignTop|Qt::AlignRight);
    }
    ClickFixLabel       *bingdingLabel    = new ClickFixLabel(frame);
    KDK_EXTEND_OBJ_INFO_FORMAT(bingdingLabel, "Shortcut", "customBindingLabel" + QString(customIndex));
    KeyEntry            *mKeyEntry        = new KeyEntry;
    m_customFrameList.append(frame);

    mKeyEntry->gsSchema   = keyEntry.gsSchema;
    mKeyEntry->keyStr     = keyEntry.keyStr;
    mKeyEntry->valueStr   = keyEntry.valueStr;
    mKeyEntry->descStr    = keyEntry.descStr;
    mKeyEntry->gsPath     = keyEntry.gsPath;
    mKeyEntry->nameStr    = keyEntry.nameStr;
    mKeyEntry->bindingStr = keyEntry.bindingStr;
    mKeyEntry->actionStr  = keyEntry.actionStr;

    customShortcutGroup->addWidget(frame);
    addButton->setRadiusType(UkccFrame::BorderRadiusStyle::Bottom);
    mCustomLineEdit.append(bingdingLineEdit);
    mCustomName.append(bingdingLabel);

    // 大小写字母数字中文
    QRegExp rx("[a-zA-Z0-9\u4e00-\u9fa5]+");
    QRegExpValidator *regValidator = new QRegExpValidator(rx);
    // 输入限制
    nameLineEdit->setValidator(regValidator);
    nameLineEdit->setFixedHeight(36);
    static const int MAX_LENGTH = 256;
    nameLineEdit->setMaxLength(MAX_LENGTH);
    bingdingLineEdit->setFixedSize(234, 36);

    bingdingLineEdit->setAlignment(Qt::AlignLeft);
    bingdingLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    bingdingLineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);
    bingdingLabel->setFixedSize(234, 36);
    bingdingLabel->setStyleSheet("QLabel{background-color:palette(button);border-radius: 4px}");

    QToolButton *btn = new QToolButton(frame);
    btn->setAutoRaise(true);
    btn->setPopupMode(QToolButton::InstantPopup);
    QMenu *menu = new QMenu(btn);
    btn->setMenu(menu);
    QAction *edit = new QAction(btn);
    QAction *del = new QAction(btn);
    edit->setText(tr("Edit"));
    del->setText(tr("Delete"));
    menu->addAction(edit);
    menu->addAction(del);

    connect(del, &QAction::triggered, this, [=](){
        customShortcutGroup->removeWidget(frame);
        if (customShortcutGroup->showItemsCount() == 0) {
            addButton->setRadiusType(UkccFrame::BorderRadiusStyle::Around);
        }
        m_customFrameList.removeOne(frame);
        frame->deleteLater();
        Q_EMIT deleteShortcut(mKeyEntry->gsPath);
        for (int i = 0; i < g_customEntryList.size(); ++i) {
            if (g_customEntryList.at(i).nameStr == mKeyEntry->nameStr) {
                qDebug() << Q_FUNC_INFO << "remove shortcut" << mKeyEntry->nameStr;
                int index = 0;
                for (QList<DoubleClickShortCut *>::iterator iter = mCustomLineEdit.begin(); iter != mCustomLineEdit.end(); ++index) {
                    if (bingdingLineEdit == *iter) {
                        iter = mCustomLineEdit.erase(iter);
                        QList<ClickFixLabel *>::iterator mIter = mCustomName.begin();
                        mCustomName.erase(mIter + index);
                        qDebug() << Q_FUNC_INFO << "remove bingdingLineEdit from mCustomLineEdit" << bingdingLineEdit->text();
                    } else {
                        ++iter;
                    }
                }
                g_customEntryList.removeAt(i);
                break;
            }
        }
        delete mKeyEntry;
        fixShortcutTabOrder();
        UkccCommon::buriedSettings("Shortcut", QString("del"), QString("clicked"));
    });

    connect(edit, &QAction::triggered, this, [=](){
        AddShortcutDialog *addDialog = new AddShortcutDialog(g_systemEntryList, g_windowEntryList, g_customEntryList, this);
        addDialog->setAttribute(Qt::WA_DeleteOnClose);
        addDialog->setSourceEnable(false);
        addDialog->setWindowTitle(QObject::tr("Edit Shortcut"));

        addDialog->setExecText(mKeyEntry->actionStr);
        addDialog->setNameText(nameLineEdit->text());
        QString newBindingStr = mKeyEntry->bindingStr;
        for (int i = 0; i < g_customEntryList.size(); i++) {
            if (mKeyEntry->actionStr == g_customEntryList.at(i).actionStr) {
                newBindingStr = g_customEntryList.at(i).bindingStr;
                if (newBindingStr == "Null") {
                    newBindingStr = tr("Null");
                }
                qDebug() << Q_FUNC_INFO << "get new customEntries bindingStr" << g_customEntryList.at(i).actionStr << "from" << mKeyEntry->bindingStr << "to" << g_customEntryList.at(i).bindingStr;
                break;
            }
        }
        addDialog->setKeyText(newBindingStr);
        addDialog->setKeyIsAvailable(3);
        UkccCommon::buriedSettings("Shortcut", QString("edit"), QString("clicked"));
        connect(addDialog, &AddShortcutDialog::shortcutInfoSignal, this,
                [=](QString path, QString name, QString exec, QString key){
            customAddDialogSlot(addDialog, nameLineEdit, bingdingLineEdit,
                                nameLabel, bingdingLabel, mKeyEntry,
                                name, exec, key);
        });
        addDialog->exec();
    });

    btn->setIcon(QIcon::fromTheme("view-more-horizontal-symbolic"));
    btn->setProperty("useButtonPalette", true);
    btn->setFixedSize(36, 36);
    // 保证「更多」在名称、快捷键之后进入 Tab 链
    btn->setFocusPolicy(Qt::StrongFocus);
    btn->setAccessibleName(tr("菜单"));

    layout->setContentsMargins(8, 0, 16, 0);
    layout->setSpacing(16);
    layout->addLayout(lineEditLayout);
    layout->addWidget(btn);

    lineEditLayout->setMargin(0);
    lineEditLayout->setSpacing(140);

    QSizePolicy policy;
    policy = nameLineEdit->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    nameLineEdit->setSizePolicy(policy);

    policy = nameLabel->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    nameLabel->setSizePolicy(policy);

    policy = bingdingLineEdit->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    bingdingLineEdit->setSizePolicy(policy);

    policy = bingdingLabel->sizePolicy();
    policy.setHorizontalPolicy(QSizePolicy::Ignored);
    bingdingLabel->setSizePolicy(policy);

    lineEditLayout->addWidget(nameLineEdit, 1);
    lineEditLayout->addWidget(nameLabel, 1);
    lineEditLayout->addWidget(bingdingLineEdit, 1);
    lineEditLayout->addWidget(bingdingLabel, 1);
    // 与系统快捷键一致：名称/快捷键编辑框常驻，Tab 顺序为 名称 → 快捷键 → 更多
    nameLineEdit->setProperty("ukccAlwaysShowEdit", true);
    bingdingLineEdit->setProperty("ukccAlwaysShowEdit", true);
    nameLineEdit->setVisible(true);
    bingdingLineEdit->setVisible(true);
    nameLabel->setVisible(false);
    bingdingLabel->setVisible(false);

    nameLineEdit->blockSignals(true);
    nameLineEdit->setText(mKeyEntry->nameStr);
    nameLineEdit->blockSignals(false);
    bingdingLineEdit->blockSignals(true);
    QString displayText = getShowShortcutString(mKeyEntry->bindingStr);
    if (displayText == "Null") {
        displayText = tr("Null");
    }
    bingdingLineEdit->setText(displayText);
    bingdingLineEdit->setProperty("shortKey", mKeyEntry->bindingStr);
    bingdingLineEdit->setProperty("shortcutId", mKeyEntry->actionStr);
    bingdingLineEdit->blockSignals(false);

    nameLabel->setText(mKeyEntry->nameStr);
    bingdingLabel->setText(displayText);
    nameLabel->setAccessibleName(QStringLiteral(" "));
    // 名称框常驻后勿再让隐藏 nameLabel setBuddy：否则 LABELLED_BY 变成「用户手册」，
    // 读屏 labelOrName 优先读 buddy，首行就读不到「自定义快捷键」。
    // 首行由 bindCustomTitleToFirstNameEdit（标题 buddy + accessibleName）；其余行用「名称」。
    // 首行 accessibleName 在 fixShortcutTabOrder→bindCustomTitleToFirstNameEdit 中写成标题
    bindShortcutEditAccessibleName(nameLineEdit, tr("Name"));
    bindShortcutEditAccessibleName(bingdingLineEdit, mKeyEntry->nameStr);
    bingdingLineEdit->setProperty("ukccNameLabel", QVariant::fromValue(static_cast<QWidget *>(nameLabel)));
    bingdingLabel->setAccessibleName(QStringLiteral(" "));
    nameLineEdit->setFocusPolicy(Qt::StrongFocus);
    bingdingLineEdit->setFocusPolicy(Qt::StrongFocus);
    nameLabel->setFocusPolicy(Qt::NoFocus);
    bingdingLabel->setFocusPolicy(Qt::NoFocus);
    mAllKeyI18nName.insert(mKeyEntry->actionStr, mKeyEntry->nameStr);

    connect(nameLabel, &ClickFixLabel::doubleClicked, this, [=]() {
        labelDoubleClickSlot(nameLabel, nameLineEdit);
    });

    connect(bingdingLabel, &ClickFixLabel::doubleClicked, this, [=]() {
        labelDoubleClickSlot(bingdingLabel, bingdingLineEdit);
    });

    connect(nameLineEdit, &DoubleClickLineEdit::focusOut, this, [=]() {
        lineEditFocusOutSlot(nameLineEdit, nameLabel);
    });

    connect(bingdingLineEdit, &DoubleClickShortCut::focusOut, this, [=]() {
        lineEditFocusOutSlot(bingdingLineEdit, bingdingLabel);
    });

    connect(nameLineEdit, &DoubleClickLineEdit::strChanged, this, [=]() {
        customLineEditStrChangeSlot(nameLineEdit, nameLabel, mKeyEntry);
    });

    connect(bingdingLineEdit, &DoubleClickShortCut::shortcutChanged, this, [=](){
        customLineEditChangeSlot(bingdingLineEdit, bingdingLabel, mKeyEntry);
    });

    connect(bingdingLineEdit, &DoubleClickShortCut::shortcutConflict, this, [=]() {
        customLineEditConflictSlot(bingdingLineEdit, bingdingLabel, mKeyEntry);
    });
    connect(bingdingLineEdit, &DoubleClickShortCut::shortcutOccupy, this, [=]() {
        lineEditOccupySlot(bingdingLineEdit, bingdingLabel);
    });

    fixShortcutTabOrder();
}

bool ShortcutUi::conflictWithGlobalShortcuts(const QKeySequence &keySequence) {
    QHash<QKeySequence, QList<KGlobalShortcutInfo> > clashing;
    for (int i = 0; i < keySequence.count(); ++i) {
        QKeySequence keys(keySequence[i]);
        qDebug() << "全局快捷键冲突" << keySequence.count() << keySequence << keys;
        if (!KGlobalAccel::isGlobalShortcutAvailable(keySequence)) {
            clashing.insert(keySequence, KGlobalAccel::getGlobalShortcutsByKey(keys));
        }
    }

    if (clashing.isEmpty() || (clashing[keySequence].empty() && keySequence.toString() != "Alt+M")) {
        qDebug() << Q_FUNC_INFO << keySequence << "not conflict with Global Shortcuts key" << keySequence.toString();
        return false;
    } else {
        qDebug() << Q_FUNC_INFO << "conflict With Global Shortcuts key" << keySequence;
        if (clashing[keySequence].empty()) {
            m_conflictValue = "";
        } else {
            m_conflictValue = clashing[keySequence][0].friendlyName();
            qDebug() << Q_FUNC_INFO << "conflict With Global Shortcuts" << m_conflictValue;
        }
        m_conflictKey = keySequence.toString();
    }
    return true;
}

bool ShortcutUi::conflictWithStandardShortcuts(const QKeySequence &seq) {
    KStandardShortcut::StandardShortcut ssc = KStandardShortcut::find(seq);
    if (ssc != KStandardShortcut::AccelNone) {
        qDebug() << "conflict With Standard Shortcuts" << seq;
        m_conflictValue = KStandardShortcut::label(ssc);
        m_conflictKey = seq.toString();
        return true;
    }
    return false;
}

bool ShortcutUi::conflictWithSystemShortcuts(const QKeySequence &seq) {
    QString systemKeyStr = keyToLib(seq.toString());
    if (systemKeyStr.contains("Meta")) {
        systemKeyStr.replace("Meta", "Win");
    }
    if (systemKeyStr.contains("Start")) {
        systemKeyStr.replace("Start", "Win");
    }
    if (systemKeyStr.contains("PrtSc", Qt::CaseInsensitive)) {
        systemKeyStr.replace("PrtSc", "Print", Qt::CaseInsensitive);
    }

    for (KeyEntry& ckeyEntry : g_systemEntryList) {
        QString ckeyString = ckeyEntry.valueStr;
        if (ckeyString.contains("Control", Qt::CaseInsensitive)) {
            ckeyString.replace("Control", "Ctrl", Qt::CaseInsensitive);
        }

        if (ckeyString.contains("Start")) {
            ckeyString.replace("Start", "Win");
        }

        if (ckeyString.contains("PrtSc", Qt::CaseInsensitive)) {
            ckeyString.replace("PrtSc", "Print", Qt::CaseInsensitive);
        }
        if (!systemKeyStr.compare(ckeyString, Qt::CaseInsensitive)) {
            qDebug() << "conflictWithSystemShortcuts" << seq << ckeyEntry.keyStr;
            m_conflictKey = ckeyEntry.keyStr;
            m_conflictValue = mAllKeyI18nName.value(ckeyEntry.keyStr);
            return true;
        }
    }

    return false;
}

bool ShortcutUi::conflictWithWindowShortcuts(const QKeySequence &seq) {
    QString windowKeyStr = seq.toString();
    if (windowKeyStr.contains("Meta")) {
        windowKeyStr.replace("Meta", "Win");
    }
    if (windowKeyStr.contains("Start")) {
        windowKeyStr.replace("Start", "Win");
    }

    if (windowKeyStr.contains("PrtSc", Qt::CaseInsensitive)) {
        windowKeyStr.replace("PrtSc", "Print", Qt::CaseInsensitive);
    }
    if (windowKeyStr.contains("↑")) {
        windowKeyStr.replace("↑", "Up");
    }
    if (windowKeyStr.contains("→")) {
        windowKeyStr.replace("→", "Right");
    }
    if (windowKeyStr.contains("←")) {
        windowKeyStr.replace("←", "Left");
    }
    if (windowKeyStr.contains("↓")) {
        windowKeyStr.replace("↓", "Down");
    }
    for (KeyEntry& ckeyEntry : g_windowEntryList) {
        QString ckeyString = ckeyEntry.valueStr;
        if (ckeyString.contains("Control", Qt::CaseInsensitive)) {
            ckeyString.replace("Control", "Ctrl", Qt::CaseInsensitive);
        }
        if (ckeyString.contains("Meta")) {
            ckeyString.replace("Meta", "Win");
        }
        if (ckeyString.contains("Start")) {
            ckeyString.replace("Start", "Win");
        }

        if (ckeyString.contains("PrtSc", Qt::CaseInsensitive)) {
            ckeyString.replace("PrtSc", "Print", Qt::CaseInsensitive);
        }
        qDebug() << Q_FUNC_INFO << "window conflict detect" << windowKeyStr << ckeyString << ckeyEntry.keyStr;
        if (!windowKeyStr.compare(ckeyString, Qt::CaseInsensitive)) {
            qDebug() << "conflictWithWindowShortcuts" << windowKeyStr << ckeyEntry.keyStr;
            m_conflictKey = ckeyEntry.keyStr;
            m_conflictValue = ckeyEntry.nameStr.isEmpty()
                    ? mAllKeyI18nName.value(ckeyEntry.keyStr)
                    : ckeyEntry.nameStr;
            return true;
        }
    }

    return false;
}

bool ShortcutUi::conflictWithCustomShortcuts(const QKeySequence &seq) {
    QString customKeyStr = keyToLib(seq.toString());
    if (customKeyStr.contains("Meta")) {
        customKeyStr.replace("Meta", "Win");
    }
    for (KeyEntry& ckeyEntry : g_customEntryList) {
        QString ckeyString = ckeyEntry.bindingStr;
        if (ckeyString.contains("Control")) {
            ckeyString.replace("Control", "Ctrl");
        }
        if (ckeyString.contains("Meta")) {
            ckeyString.replace("Meta", "Win");
        }
        if (customKeyStr == ckeyString) {
            qDebug() << "conflictWithCustomShortcuts" << seq << ckeyEntry.bindingStr;
            m_conflictValue.clear();
            m_conflictKey = ckeyEntry.actionStr;
            return true;
        }
    }

    return false;
}

void ShortcutUi::conflictDetectSlot(ShortcutLine* shortcutLineEdit, const QKeySequence &seq) {
    if (!shortcutLineEdit) {
        return;
    }
    m_conflictValue.clear();
    m_conflictKey.clear();
    QPointer<ShortcutLine> guard(shortcutLineEdit);
    if (conflictWithSystemShortcuts(seq) ||
            conflictWithWindowShortcuts(seq) ||
            conflictWithCustomShortcuts(seq)) { //快捷键冲突
        shortcutLineEdit->setConflictInfo(m_conflictKey, m_conflictValue);
        // 延后弹窗，避免在 keyPressEvent 调用栈中同步 exec 导致重入崩溃
        QTimer::singleShot(0, this, [guard]() {
            if (!guard) {
                return;
            }
            Q_EMIT guard->shortCutAvailable(-2);
        });
    } else if (conflictWithGlobalShortcuts(seq) || conflictWithStandardShortcuts(seq)) {
        shortcutLineEdit->setShortCutObtainedFlag(false);
        shortcutLineEdit->initInputKeyAndText(true);
        shortcutLineEdit->setConflictInfo(m_conflictKey, m_conflictValue);
        QTimer::singleShot(0, this, [guard]() {
            if (!guard) {
                return;
            }
            Q_EMIT guard->shortCutAvailable(-1);
        });
    } else {
        Q_EMIT shortcutLineEdit->shortCutAvailable(0);
    }
}

QString ShortcutUi::getShowShortcutString(QString src)
{
    // wlcom 多为 win+d：须忽略大小写，否则会显示成 Win D 而非 Super D
    src.replace(QStringLiteral("Win"), QStringLiteral("Super"), Qt::CaseInsensitive);
    src.replace(QStringLiteral("Meta"), QStringLiteral("Super"), Qt::CaseInsensitive);
    src.replace(QStringLiteral("Start"), QStringLiteral("Super"), Qt::CaseInsensitive);
    src.replace("<","");
    src.replace(">","   ");
    src.replace(" or ",tr(" or "));
    src.replace("+", " ");
    QStringList temp_list = src.split(" ");
    QString str;
    int size = temp_list.count();
    for(qint32 i = 0; i < size; i++) {
        str += temp_list.at(i)
                .left(1).toUpper() +
                temp_list.at(i)
                .mid(1, temp_list.at(i).length() - 1);
        if (i != size - 1) {
            str += " ";
        }
    }
    str.replace("Or","or");
    str.replace(" Left", " \u2190");
    str.replace(" Right", " \u2192");
    str.replace(" Up", " \u2191");
    str.replace(" Down", " \u2193");
    return str;
}

QString ShortcutUi::keyToLib(QString key)
{
    // 与 getShowShortcutString 一致：win/meta/start 统一成 Super，便于和系统快捷键比冲突
    key.replace(QStringLiteral("Win"), QStringLiteral("Super"), Qt::CaseInsensitive);
    key.replace(QStringLiteral("Meta"), QStringLiteral("Super"), Qt::CaseInsensitive);
    key.replace(QStringLiteral("Start"), QStringLiteral("Super"), Qt::CaseInsensitive);

    if (key.contains("Print", Qt::CaseInsensitive)) {
        key.replace("Print", "PrtSc", Qt::CaseInsensitive);
    }

    if (key.contains("+")) {
        QStringList keys = key.split("+");
        if (keys.count() == 2) {
            QString lower = keys.at(1);
            QString keyToLib = "<" + keys.at(0) + ">" + lower.toLower();
            return keyToLib;
        } else if (keys.count() == 3) {
            QString lower = keys.at(2);
            QString keyToLib = "<" + keys.at(0) + ">" + "<" + keys.at(1) + ">" + lower.toLower();
            return keyToLib;
        } else if (keys.count() == 4) {
            QString lower = keys.at(3);
            QString keyToLib = "<" + keys.at(0) + ">" + "<" + keys.at(1) + ">" + "<" + keys.at(2) + ">" + lower.toLower();
            return keyToLib;
        }
    }
    return key;
}

QString ShortcutUi::conflictTip(const QString &key, const QString &value)
{
    if (!value.isEmpty()) {
        return value;
    } else {
        QMap<QString, QString>::Iterator it;
        for (it = mAllKeyI18nName.begin(); it != mAllKeyI18nName.end(); it++) {
            qDebug() << Q_FUNC_INFO << it.key() << it.value();
            if (it.key() == key) {
                return it.value();
            }
        }
    }
    return value;
}

void ShortcutUi::clearCustomShorcutFrame()
{
    customShortcutGroup->removeAndDeleteAllWidget();
}

void ShortcutUi::setWindowValid(bool isValid) {
    m_windowShortcutValid = isValid;
}

void ShortcutUi::setWindowHide() {
    m_windowTitle->setVisible(false);
    m_windowShortcutGroup->setVisible(false);
}

void ShortcutUi::loadDefaultShortcut() {
    QGSettings ukcc("org.ukui.control-center");
    m_defaultShortcutList.clear();
    if (!ukcc.keys().contains("defaultShortcut")) {
        qDebug() << Q_FUNC_INFO << "org.ukui.control-center has no key defaultShortcut";
        return;
    }
    QString defaultShortcut = ukcc.get("defaultShortcut").toString();
    for (const auto& shortcutPair : defaultShortcut.split(";")) {
        QStringList shortcutPairList = shortcutPair.split(":");
        if (shortcutPairList.size() != 2) {
            continue;
        }
        QString pairKey = shortcutPairList.at(0);
        QString pairValue = shortcutPairList.at(1);
        QString valueStr = keyToLib(pairValue).toLower();
        m_defaultShortcutList.append(valueStr);
        //qDebug() << Q_FUNC_INFO << "load default shortcut" << pairKey << pairValue << valueStr;
    }
}

void ShortcutUi::clearConflictingWindowShortcutsOnLoad()
{
    for (int i = 0; i < g_windowEntryList.count(); ++i) {
        const QString windowRaw = g_windowEntryList[i].valueStr;
        if (windowRaw.isEmpty()
                || windowRaw.compare(QStringLiteral("Null"), Qt::CaseInsensitive) == 0) {
            continue;
        }
        const QString windowNorm = keyToLib(windowRaw).toLower();
        if (windowNorm.isEmpty()) {
            continue;
        }

        bool conflict = false;
        for (const KeyEntry &sys : g_systemEntryList) {
            const QString sysNorm = keyToLib(sys.valueStr).toLower();
            if (!sysNorm.isEmpty()
                    && sysNorm.compare(QStringLiteral("null")) != 0
                    && sysNorm == windowNorm) {
                conflict = true;
                break;
            }
        }
        if (!conflict) {
            for (const KeyEntry &custom : g_customEntryList) {
                const QString customNorm = keyToLib(custom.bindingStr).toLower();
                if (!customNorm.isEmpty()
                        && customNorm.compare(QStringLiteral("null")) != 0
                        && customNorm == windowNorm) {
                    conflict = true;
                    break;
                }
            }
        }
        if (!conflict) {
            continue;
        }

        // 冲突时窗口侧让出按键，界面显示「无」
        const QString oldShortCut = g_windowEntryList[i].valueStr;
        g_windowEntryList[i].valueStr = QStringLiteral("Null");
        m_shortcutInterface->call("setWindowShortcut",
                                  g_windowEntryList[i].keyStr,
                                  oldShortCut,
                                  QString());
        qDebug() << Q_FUNC_INFO << "clear window shortcut on load"
                 << g_windowEntryList[i].keyStr << "from" << oldShortCut << "to Null";
    }
}

void ShortcutUi::setWindowShortcutToNull() {
    if (!m_windowShortcutValid) {
        return;
    }
    QDBusReply<QVariantMap> shortKeyValueReply = m_shortcutInterface->call("getShortKeyValue");
    QVariantMap shortKeyValueMap;
    if (shortKeyValueReply.isValid()) {
        shortKeyValueMap = shortKeyValueReply.value();
        qDebug() << Q_FUNC_INFO << "ShortKeyValue size:" << shortKeyValueMap.size();
    } else {
        qDebug() << Q_FUNC_INFO << "getShortKeyValue invalid" << shortKeyValueReply.error();
        return;
    }
    for (int j = 0; j < m_windowLineEdit.size(); j++) {
        QString shortKey = m_windowLineEdit.at(j)->property("shortcutId").toString();
        QString shortValue = m_windowLineEdit.at(j)->property("shortKey").toString();
        qDebug() << Q_FUNC_INFO << "before set window lineEdit to Null" << shortKey << shortValue << j << m_windowLineEdit.at(j)->text();
        if (shortKeyValueMap.contains(shortKey)) {
            QString oldValue = shortKeyValueMap[shortKey].toString().trimmed();
            m_shortcutInterface->call("setWindowShortcut", shortKey, oldValue, "");
            qDebug() << Q_FUNC_INFO << "set window shortcut" << shortKey << "from" << oldValue << "to Null";
        }
    }
}

void ShortcutUi::setCustomShortcutToNull() {
    for (int j = 0; j < mCustomLineEdit.size(); j++) {
        QString actionStr = mCustomLineEdit.at(j)->property("shortcutId").toString();
        for (int i = 0; i < g_customEntryList.count(); i++) {
            if (actionStr != g_customEntryList[i].actionStr) {
                qDebug() << Q_FUNC_INFO << "lineEdit" << actionStr << "not equal gsetting" << g_customEntryList[i].actionStr;
                continue;
            }
            QString shortcut = keyToLib(g_customEntryList[i].bindingStr).toLower();
            qDebug() << Q_FUNC_INFO << "before set custom lineEdit to Null" << actionStr << shortcut << g_customEntryList[i].gsPath;
            if (!m_defaultShortcutList.contains(shortcut)) {
                qDebug() << Q_FUNC_INFO << "shortcut" << shortcut << "not find in default shortcut";
                continue;
            }
            QString oldValue = mCustomLineEdit[j]->text().trimmed();
            mCustomLineEdit[j]->blockSignals(true);
            mCustomLineEdit[j]->setText(tr("Null"));
            mCustomLineEdit[j]->blockSignals(false);
            mCustomLineEdit[j]->setProperty("shortKey", "Null");
            refreshShortcutEditAccessibleName(mCustomLineEdit[j]);
            mCustomName[j]->blockSignals(true);
            mCustomName[j]->setText(tr("Null"));
            mCustomName[j]->blockSignals(false);
            qDebug() << Q_FUNC_INFO << "set customLineEdit" << g_customEntryList[i].nameStr << "from" << oldValue << "to null";
            g_customEntryList[i].bindingStr = "Null";
            m_shortcutInterface->call("setCustomShortcut", g_customEntryList[i].gsPath, "Null");
        }
    }
}

void ShortcutUi::defaultShortcut() {
    m_shortcutInterface->call("defaultShortcut");

    QDBusReply<QVariantMap> shortKeyValueReply = m_shortcutInterface->call("getShortKeyValue");
    QVariantMap shortKeyValueMap;
    if (shortKeyValueReply.isValid()) {
        shortKeyValueMap = shortKeyValueReply.value();
        qDebug() << Q_FUNC_INFO << "ShortKeyValue size:" << shortKeyValueMap.size();
    } else {
        qDebug() << Q_FUNC_INFO << "getShortKeyValue invalid" << shortKeyValueReply.error();
        return;
    }

    // update system shortcut lineEdit
    g_systemEntryList.clear();
    QDBusReply<KeyEntryList> systemEntryReply = m_shortcutInterface->call("getSystemShortcutEntry");
    if (systemEntryReply.isValid()) {
        g_systemEntryList = systemEntryReply.value();
    }
    for (int j = 0; j < mGeneralLineEdit.size(); j++) {
        QString shortKey = mGeneralLineEdit.at(j)->property("shortcutId").toString();
        QString shortValue = mGeneralLineEdit.at(j)->property("shortKey").toString();
        qDebug() << Q_FUNC_INFO << "before default system lineEdit" << shortKey << shortValue << j << mGeneralLineEdit.at(j)->text();
        if (shortKeyValueMap.keys().contains(shortKey)) {
            QString newValue = shortKeyValueMap[shortKey].toString();
            QString showText = getShowShortcutString(newValue);
            qDebug() << Q_FUNC_INFO << "default system lineEdit" << shortKey << "from" << mGeneralLineEdit[j]->text() << "to" << showText;
            mGeneralLineEdit[j]->blockSignals(true);
            mGeneralLineEdit[j]->setText(showText);
            mGeneralLineEdit[j]->blockSignals(false);
            mGeneralLineEdit[j]->setProperty("shortKey", newValue);
            refreshShortcutEditAccessibleName(mGeneralLineEdit[j]);
            mGeneralName[j]->blockSignals(true);
            mGeneralName[j]->setText(showText);
            mGeneralName[j]->blockSignals(false);
        }
    }

    // update window shortcut lineEdit
    g_windowEntryList.clear();
    QDBusReply<KeyEntryList> windowEntryReply = m_shortcutInterface->call("getWindowShortcutEntry");
    if (windowEntryReply.isValid()) {
        g_windowEntryList = windowEntryReply.value();
        if (g_windowEntryList.empty()) {
            qDebug() << Q_FUNC_INFO << "default window shortcut is empty";
            m_windowShortcutValid = false;
        }
    } else {
        m_windowShortcutValid = false;
        qDebug() << Q_FUNC_INFO << "default window shortcut invalid";
    }
    for (int j = 0; j < m_windowLineEdit.size(); j++) {
        QString shortKey = m_windowLineEdit.at(j)->property("shortcutId").toString();
        QString shortValue = m_windowLineEdit.at(j)->property("shortKey").toString();
        qDebug() << Q_FUNC_INFO << "before default window lineEdit" << shortKey << shortValue << j << m_windowLineEdit.at(j)->text();
        if (shortKeyValueMap.contains(shortKey)) {
            QString newValue = shortKeyValueMap[shortKey].toString();
            QString showText = getShowShortcutString(newValue).trimmed();
            // 与建行逻辑一致：英文 Null 显示为翻译后的「无」
            if (showText.compare(QStringLiteral("Null"), Qt::CaseInsensitive) == 0) {
                showText = tr("Null");
            }
            qDebug() << Q_FUNC_INFO << "default window lineEdit" << shortKey << "from" << m_windowLineEdit[j]->text() << "to" << showText;
            m_windowLineEdit[j]->blockSignals(true);
            m_windowLineEdit[j]->setText(showText);
            m_windowLineEdit[j]->blockSignals(false);
            m_windowLineEdit[j]->setProperty("shortKey", newValue);
            refreshShortcutEditAccessibleName(m_windowLineEdit[j]);
            m_windowName[j]->blockSignals(true);
            m_windowName[j]->setText(showText);
            m_windowName[j]->blockSignals(false);
        }
    }
}

void ShortcutUi::defaultButtonSlot() {
    // 1) window shortcut to null
    setWindowShortcutToNull();

    // 2) set conflict custom shortcut to null
    setCustomShortcutToNull();

    // 3) system shortcut to default, window shortcut to default
    defaultShortcut();

    // 4) reset m_defaultButton enable状态
    updateDefaultButtonStatus(false);

    UkccCommon::buriedSettings("Shortcut", QString("default"), QString("clicked"));
}

void ShortcutUi::addDefaultBtn() {
    //~ contents_path /Shortcut/Default
    m_defaultButton = new QPushButton(tr("Default"));
    KDK_EXTEND_ALL_INFO_FORMAT(m_defaultButton, "Shortcut", "shortcutDefaultButton", "");
    m_defaultLayout->addWidget(m_defaultButton);
    m_defaultLayout->addStretch();
    m_defaultLayout->setContentsMargins(0, 8, 16, 24);
    bool enabled = true;
    if (m_ukccSetting && m_ukccSetting->keys().contains("shortcutDefaultEnabled")) {
        enabled = m_ukccSetting->get("shortcutDefaultEnabled").toBool();
        qDebug() << Q_FUNC_INFO << "set shortcut default button enabled" << enabled;
    }
    updateDefaultButtonStatus(enabled);
    connect(m_defaultButton, &QPushButton::clicked, this, [=]() {
        defaultButtonSlot();
    });

    m_defaultButton->setAccessibleName(tr("Default"));
}
