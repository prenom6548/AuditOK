/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#include "searchwidget.h"
#include "pinyin.h"
#include "homepagewidget.h"

#include <QDebug>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QToolButton>
#include <QXmlStreamReader>
#include <QCompleter>
#include <QRegularExpression>
#include <QTimer>
#include <QItemSelectionModel>
#include <mainwindow.h>
#include "utils/modulefactory.h"
#include "type.h"

extern "C" {
#include <glib.h>
}

const QStringList filterPathList = {"/About/Status", "/About/Active", "/About/<<Protocol>>", "/About/Serial"};

const QStringList g_powerFilterPathList = {"/Power/Notebook cover", "/Power/Using battery", "/Power/Battery saving plan",
                                        "/Power/Low battery notification", "/Power/\"Automatically run saving mode when low battery"};

const QStringList yangtzeFilterPathList = {"/Shortcut/Add", "/Shortcut/Customize Shortcut",
                                          "/Display/Color Temperature And Eye Care", "/Display/Eye protection mode",
                                          "/Keyboard/Input Method"};

const QStringList nileFilterPathList = {"/Display/Night Mode", "/Keyboard/Input settings", "/Keyboard/Show icon on tray",
                                       "/Keyboard/Text size", "/Keyboard/Input sound effect"};

const QStringList g_grubFilterPathList = {"/Boot/Grub Boot Delay", "/Boot/Grub Default Boot Item"};

const QStringList tabletFilterPathList = {"/Display/Color Temperature And Eye Care", "/Display/Eye protection mode"};

// 非wayland过滤词条
const QStringList g_nWaylandVinoFilterPathList = {"/Vino/The number of remote clients allowed to connect",
                                                  "/Vino/Displays that allow remote access",
                                                  "/Vino/Remote Connection Protocol"};

// wayland 过滤词条
const QStringList g_waylandVinoFilterPathList = {"/Vino/Allow others to connect to your desktop remotely using VNC",
                                                "/Vino/Allow others to connect to your desktop remotely using RDP"};

const QStringList kSystemShortcutAllowList = {"area-screenshot", "screenshot", "window-screenshot", "kylin-display-switch",
    "logout", "nm-connection-editor","peony-qt", "screensaver", "terminal", "ukui-control-center", "ukui-search","ukui-sidebar",
    "ukui-system-monitor", "ukui-window-switch", "kylin-service-support"};

SearchWidget::SearchWidget(QWidget *parent)
    : KSearchLineEdit(parent)
    , m_xmlExplain("")
    , m_bIsChinese(false)
    , m_searchValue("")
    , m_bIstextEdited(false)
{
    initExcludeSearch();
    m_model = new QStandardItemModel(this);
    m_completer = this->completer();
    this->completer()->setModel(m_model);
    QAbstractItemView *popup = m_completer->popup();
    popup->setWindowFlag(Qt::WindowDoesNotAcceptFocus, true);

    this->reloadStyle();
    // KSearchLineEdit 默认为 ClickFocus，Tab 键无法聚焦；StrongFocus 支持键盘 Tab 切换
    setFocusPolicy(Qt::StrongFocus);
    // 无障碍名称与 SDK placeholder 对齐，避免 KDK_ALL_INFO_FORMAT 覆盖为英文机器名
    setAccessibleName(placeholderText());
    this->setClearButtonEnabled(true);
    setupClearButtonKeyboardAccess();
    m_completer->popup()->setAttribute(Qt::WA_InputMethodEnabled);
    m_completer->setFilterMode(Qt::MatchContains);//设置QCompleter支持匹配字符搜索
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);//这个属性可设置进行匹配时的大小写敏感性
    m_completer->setCompletionRole(Qt::UserRole); //设置ItemDataRole
    m_completer->setWrapAround(false);
    // QLineEdit 默认连接 highlighted→改写输入框；方向键换项时会把「关于 --> …」写回搜索框，
    // 读屏会读「下拉选中」+「搜索框全文」两遍。仅保留 activated（回车/点击）写回。
    disconnect(m_completer,
               QOverload<const QString &>::of(&QCompleter::highlighted),
               this, nullptr);
    qRegisterMetaType<QStringPair>("QStringPair");
    qDBusRegisterMetaType<QStringPair>();
    qRegisterMetaType<QStringPairList>("QStringPairList");
    qDBusRegisterMetaType<QStringPairList>();
    m_shortcutInterface = new QDBusInterface("org.ukui.ukcc.session",
                                             "/Shortcut",
                                             "org.ukui.ukcc.session.Shortcut",
                                             QDBusConnection::sessionBus(), this);
    if (!m_shortcutInterface->isValid()) {
        qDebug() << Q_FUNC_INFO << "org.ukui.ukcc.session.Shortcut interface invalid";
        m_shortcutInterfaceValid = false;
    }

    connect(this, &QLineEdit::textEdited, this, [ = ] {
        if (text().isEmpty()) {
            m_bIstextEdited = false;
            return;
        }
        m_bIstextEdited = true;
        // 仅用户键入时默认选中第一项；上下键换项会改 text 触发 textChanged，不可在此重置选中
        QTimer::singleShot(0, this, [this]() {
            selectFirstCompletionResult();
        });
    });

    connect(this, &QLineEdit::textChanged, this, [ = ] {
        QString retValue = text();

        if (popup->model()->rowCount() == 0) {
            UkccCommon::buriedSettings(QString("SearchWidget"), nullptr, QString("No search results"), retValue);
            if (m_model->data(m_model->index(m_model->rowCount() - 1, 0)) != tr("No search results"))
                m_model->appendRow(new QStandardItem(tr("No search results")));
            m_model->setData(m_model->index(m_model->rowCount() - 1, 0), text(), Qt::UserRole);
            m_completer->setCompletionRole(Qt::UserRole);

        } else {
            if (!m_bIsChinese)
                m_completer->setCompletionRole(Qt::DisplayRole);
            if (m_model->data(m_model->index(m_model->rowCount() - 1, 0)) == tr("No search results")) {
                UkccCommon::buriedSettings(QString("SearchWidget"), nullptr, QString("No search results"), retValue);
                while (m_model->data(m_model->index(m_model->rowCount() - 1, 0)) == tr("No search results")) {
                    m_model->clearItemData(m_model->index(m_model->rowCount() - 1, 0));
                    m_model->removeRow(m_model->rowCount() - 1);
                }
                if (popup->model()->rowCount() == 0) {
                    m_model->appendRow(new QStandardItem(tr("No search results")));
                    m_model->setData(m_model->index(m_model->rowCount() - 1, 0), text(), Qt::UserRole);
                    m_completer->setCompletionRole(Qt::UserRole);
                }
            }
        }

        if (m_bIstextEdited) {
            m_bIstextEdited = false;
            return ;
        }

        //避免输入单个字符，直接匹配到第一个完整字符(导致不能匹配正确的字符)
        if ("" == retValue || m_searchValue.contains(retValue, Qt::CaseInsensitive)) {
            m_searchValue = retValue;
            return ;
        }

        retValue = transPinyinToChinese(text());

        //拼音转化没找到，再搜索字符包含关联字符
        if (retValue == text()) {
            retValue = containTxtData(retValue);
        }

        m_searchValue = retValue;
        this->setText(retValue);
    });

    connect(this, &QLineEdit::returnPressed, this, [ = ] {

        if (!text().isEmpty()) {
            //enter defalt set first
            if (!jumpContentPathWidget(text())) {
                const QString &currentCompletion = this->completer()->currentCompletion();
                qDebug() << Q_FUNC_INFO << " [SearchWidget] currentCompletion : " << currentCompletion;

                //中文遍历一遍,若没有匹配再遍历将拼音转化为中文再遍历
                //解决输入拼音时,有配置数据后,直接回车无法进入第一个匹配数据页面的问题
                if (!jumpContentPathWidget(currentCompletion)) {
                    jumpContentPathWidget(transPinyinToChinese(currentCompletion));
                }
            }
        }
    });
#if QT_VERSION <= QT_VERSION_CHECK(5, 12, 0)
    connect(m_completer, static_cast<void(QCompleter::*)(const QString &)>(&QCompleter::activated),
            [=](const QString &text) {
#else
    //鼠标点击后直接页面跳转(https://doc.qt.io/qt-5/qcompleter.html#activated-1)
    connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated),
            [=](const QString &text) {
#endif
        qDebug() << "clicked text = " << text;
        // 在textChanged里边appendRow之后，通过点击popup的选项之后调用returnPressed不会触发textChanged
        this->setText(text);
        Q_EMIT returnPressed();

    });
}

SearchWidget::~SearchWidget()
{
    // 显式清空 searchMap，避免程序退出时 QJsonObject 析构导致的内存问题
    searchMap.clear();
    // 清空其他可能包含 JSON 相关数据的容器
    m_EnterNewPagelist.clear();
    searchItemList.clear();
    m_inputList.clear();
}

QWidget *SearchWidget::clearButton() const
{
    return m_clearButton;
}

void SearchWidget::setupClearButtonKeyboardAccess()
{
    // KSearchLineEdit 内清空按钮默认 NoFocus；未升级 SDK 时在此覆盖，与文件管理器一致
    const auto buttons = findChildren<QToolButton *>(QString(), Qt::FindChildrenRecursively);
    for (QToolButton *btn : buttons) {
        // 自定义按钮为 checkable；清空按钮固定 16×16、非 checkable
        if (btn->isCheckable()) {
            continue;
        }
        if (btn->iconSize() != QSize(16, 16) && btn->size() != QSize(16, 16)) {
            continue;
        }
        btn->setFocusPolicy(Qt::TabFocus);
        btn->setAccessibleName(tr("Clear"));
        // KToolButton Flat 焦点框为 highlight 蓝色；暂用 Frameless 去掉焦点框，
        // 等 SDK 将焦点框改为 WindowText 后再恢复 Flat。
        if (auto *kbtn = qobject_cast<KToolButton *>(btn)) {
            kbtn->setType(KToolButtonType::Frameless);
        }
        // 清空后按钮隐藏，把焦点还给输入框
        connect(btn, &QToolButton::clicked, this, [this]() {
            setFocus(Qt::TabFocusReason);
        });
        m_clearButton = btn;
        break;
    }
}

void SearchWidget::focusInEvent(QFocusEvent *event)
{
    // 仅拦截窗口激活时的自动抢焦点；PopupFocusReason（补全弹窗关闭后回焦）、
    // OtherFocusReason（输入法/setText 重入）等必须放行，否则连续输入或清空后会退出编辑态
    if (event->reason() == Qt::ActiveWindowFocusReason) {
        clearFocus();
        return;
    }
    KSearchLineEdit::focusInEvent(event);
}

void SearchWidget::selectFirstCompletionResult()
{
    if (!m_completer || text().isEmpty()) {
        return;
    }

    // 刷新补全弹层，无匹配时「无搜索结果」行才能出现在 popup 模型里
    m_completer->complete();

    QAbstractItemView *popup = m_completer->popup();
    if (!popup || !popup->model()) {
        return;
    }

    QAbstractItemModel *model = popup->model();
    int selectRow = -1;
    for (int r = 0; r < model->rowCount(); ++r) {
        const QModelIndex idx = model->index(r, m_completer->completionColumn());
        const QString display = idx.data(Qt::DisplayRole).toString();
        // 跳过 loadxml 的空占位行；「无搜索结果」也要选中，供读屏跟 selected 事件朗读
        if (!display.isEmpty()) {
            selectRow = r;
            break;
        }
    }

    if (selectRow < 0 && m_model) {
        for (int r = 0; r < m_model->rowCount(); ++r) {
            if (m_model->index(r, 0).data(Qt::DisplayRole).toString() == tr("No search results")) {
                if (model->rowCount() > 0) {
                    selectRow = 0;
                }
                break;
            }
        }
    }

    if (selectRow < 0) {
        return;
    }

    // QLineEdit 会连接 completer::highlighted → 改写输入框；此处只默认选中第一项，
    // 不得把「x」自动替换成完整条目。朗读由 Orca 跟 list item 的 selected 事件处理。
    const QString typed = text();
    m_selectingFirstCompletion = true;
    const bool blocked = m_completer->blockSignals(true);
    m_completer->setCurrentRow(selectRow);
    const QModelIndex first = model->index(selectRow, m_completer->completionColumn());
    if (first.isValid()) {
        popup->setCurrentIndex(first);
        if (QItemSelectionModel *sel = popup->selectionModel()) {
            sel->select(first, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        }
    }
    m_completer->blockSignals(blocked);
    m_selectingFirstCompletion = false;

    if (text() != typed) {
        setText(typed);
    }
    // 焦点留在搜索框；读屏通过 selected 朗读高亮项，不写 accessibleDescription
    if (!hasFocus()) {
        setFocus(Qt::OtherFocusReason);
    }
}

bool SearchWidget::jumpContentPathWidget(QString path) {
    qDebug() << Q_FUNC_INFO << path;
    bool bResult = false;

    if (m_EnterNewPagelist.count() > 0) {
        SearchBoxStruct data = getModuleBtnString(path);
        if (data.translateContent != "" && data.fullPagePath != "") {
            for (int i = 0; i < m_EnterNewPagelist.count(); i++) {
                if (m_EnterNewPagelist[i].translateContent == data.fullPagePath) {//getModuleBtnString解析SearchBoxStruct.fullPagePath，满足此处判断
#if DEBUG_XML_SWITCH
                    qDebug() << " [SearchWidget] m_EnterNewPagelist[i].translateContent : " << m_EnterNewPagelist[i].translateContent << " , fullPagePath : " << m_EnterNewPagelist[i].fullPagePath << " , actualModuleName: " << m_EnterNewPagelist[i].actualModuleName;
                    qDebug() << " [SearchWidget] data.translateContent : " << data.translateContent << " , data.fullPagePath : " << data.fullPagePath << " , data.actualModuleName: " << data.actualModuleName;
#endif
                    //the data.actualModuleName had translate to All lowercase
                    qDebug() <<" actulaModuleName is:" << data.translateContent << " " << m_EnterNewPagelist[i].fullPagePath << m_EnterNewPagelist[i].fullPagePath.section('/', 1, 1);
                    Q_EMIT notifyModuleSearch(data.translateContent, m_EnterNewPagelist[i].fullPagePath.section('/', 1, 1), data.fullPagePath);//fullPagePath need delete moduleName
                    bResult = true;

                    // 埋点搜索插件
                    UkccCommon::buriedSettings(QString("SearchWidget"), nullptr, QString("search"), m_EnterNewPagelist[i].fullPagePath);

                    break;
                }
            }
        } else {
            qWarning() << "[SearchWidget] translateContent : " << data.translateContent << " , fullPagePath : " << data.fullPagePath;
        }
    } else {
        qWarning() << " [SearchWidget] QList is nullptr.";
    }

    return bResult;
}

void SearchWidget::loadxml()
{
    QDBusInterface *hostLightStripDbus = new QDBusInterface("com.control.center.qt.systemdbus",
                                        "/",
                                        "com.control.center.interface",
                                        QDBusConnection::systemBus(), this);

    bool isSupportHostLight = false;
    if (!hostLightStripDbus->isValid()) {
        qCritical() << "Create Client Interface Failed:" << QDBusConnection::systemBus().lastError();
    } else {
        QDBusReply<QVariant> ret = hostLightStripDbus->call("isSupportHostLightStrip");
        isSupportHostLight = ret.value().toBool();
    }

    bool isSupportRescue = false;
    QFile file("/etc/default/grub.d/minimalos.cfg");
    if (file.exists()) {
        isSupportRescue = true;
    }

    if (!m_EnterNewPagelist.isEmpty()) {
        m_EnterNewPagelist.clear();
        searchItemList.clear();
        searchMap.clear();
    }

    if (!m_inputList.isEmpty()) {
        m_inputList.clear();
    }

    if (m_model->rowCount() > 0) {
        QStandardItem *item = nullptr;
        for (int i = 0; i < m_model->rowCount(); i++) {
            item = m_model->takeItem(i);
            delete item;
            item = nullptr;
        }
        m_model->clear();
    }

    //添加一项空数据，为了防止使用setText输入错误数据时直接跳转到list中正确的第一个页面
    m_searchBoxStruct.fullPagePath = "";
    m_searchBoxStruct.actualModuleName = "";
    m_searchBoxStruct.translateContent = "";
    m_searchBoxStruct.childPageName = "";
    m_EnterNewPagelist.append(m_searchBoxStruct);
    m_inputList.append(SearchDataStruct());
    m_model->appendRow(new QStandardItem(""));

    auto isChineseFunc = [](const QString &str)->bool {
        QRegularExpression rex_expression(R"(^[^a-zA-Z]+$)");
        return rex_expression.match(str).hasMatch();
    };

    bool isOpenkylinYangtze = UkccCommon::isOpenkylinYangtze();
    bool isOpenkylinNile = UkccCommon::isOpenkylinNile();
    bool isWayland = UkccCommon::isWayland();
    bool isOpenkylin = UkccCommon::isOpenkylin();
    bool isV11 = UkccCommon::isV11();
    bool isCommunity = UkccCommon::isCommunity();
    bool isKscDefender = g_file_test("/usr/sbin/ksc-defender", G_FILE_TEST_EXISTS);
    bool isKylinProxyProcessRunning = UkccCommon::isProcessRunning("kylin-proxy");
    QString cpu = UkccCommon::getCpuArchitecture();

    QDBusInterface iface("com.ksc.defender",
                             "/securitycenter",
                             "com.ksc.defender.securitycenter",
                             QDBusConnection::systemBus());

    QDBusReply<QStringList> reply = iface.call("getUnsupportTranslation");

    QStringList securityFilterPathList;
    if (reply.isValid()){
        securityFilterPathList = reply.value();
       qDebug() << "filter value = " << reply.value();
    }

    QJsonArray searchItems;
    QJsonObject rootobj;
    bool hasBat = UkccCommon::isExitBattery();
    for (const QString i : m_xmlFilePath) {

        const QString xmlPath = translationFileForLocale(i);
        QFile file(xmlPath);

        if (!file.exists()) {
            qWarning() << "[SearchWidget] translation file not exist:" << xmlPath;
            continue;
        }

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << " [SearchWidget] File open failed";
            continue;
        }

        QXmlStreamReader xmlRead(&file);
        QStringRef dataName;
        QXmlStreamReader::TokenType type = QXmlStreamReader::Invalid;
        bool vanishedTrans = false;
        QString typeStr;
        //遍历XML文件,读取每一行的xml数据都会
        //先进入StartElement读取出<>中的内容;
        //再进入Characters读取出中间数据部分;
        //最后进入时进入EndElement读取出</>中的内容
        while (!xmlRead.atEnd()) {
            type = xmlRead.readNext();

            switch (type) {
            case QXmlStreamReader::StartElement:
                m_xmlExplain = xmlRead.name().toString();
                if (m_xmlExplain == "translation") {
                    typeStr = xmlRead.attributes().value("type").toString();
                    if (typeStr == "vanished") {
                        vanishedTrans = true;
                    } else {
                        vanishedTrans = false;
                    }
                }
                break;
            case QXmlStreamReader::Characters:
                if (!xmlRead.isWhitespace()) {
                    if (m_xmlExplain == XML_Source) { // get xml source date
                        m_searchBoxStruct.translateContent = xmlRead.text().toString();
                    } else if (m_xmlExplain == XML_Title) {
                        if (xmlRead.text().toString() != "") // translation not nullptr can set it
                            m_searchBoxStruct.translateContent = xmlRead.text().toString();
                    } else if (m_xmlExplain == XML_Numerusform) {
                        if (xmlRead.text().toString() != "") // translation not nullptr can set it
                            m_searchBoxStruct.translateContent = xmlRead.text().toString();
                    } else if (m_xmlExplain == XML_Explain_Path) {
                        m_searchBoxStruct.fullPagePath = xmlRead.text().toString();
                        if (vanishedTrans) {
                            continue;
                        }

                        // 不支持救援模式，过滤搜索词条
                        if (!isSupportRescue && (m_searchBoxStruct.fullPagePath.contains("/Boot/After the system fails to start, it automatically enters the rescue system")
                                                 || m_searchBoxStruct.fullPagePath.contains("/Boot/Number of system boot failures"))) {
                            continue;
                        }

                        // 安全过滤词条
                        if (securityFilterPathList.contains(m_searchBoxStruct.fullPagePath)) {
                            continue;
                        }

                        // openkylin yangtze 过滤掉的搜索项
                        if (isOpenkylinYangtze && yangtzeFilterPathList.contains(m_searchBoxStruct.fullPagePath)) {
                            continue;
                        }

                        // openkylin Nile 过滤掉的搜索项
                        if (isOpenkylinNile && nileFilterPathList.contains(m_searchBoxStruct.fullPagePath)) {
                            continue;
                        }

                        // 非 Wayland 搜索词过滤
                        if (!isWayland && g_nWaylandVinoFilterPathList.contains(m_searchBoxStruct.fullPagePath)) {
                            continue;
                        }

                        // Wayland 远程桌面搜索词过滤
                        if (isWayland && g_waylandVinoFilterPathList.contains(m_searchBoxStruct.fullPagePath)) {
                            continue;
                        }

                        // 不支持主机灯带设置，搜索隐藏
                        if (!isSupportHostLight && m_searchBoxStruct.fullPagePath.contains("/Power/Host light strip")) {
                            continue;
                        }

                        // 非笔记本搜索词条过滤
                        if (!hasBat && g_powerFilterPathList.contains(m_searchBoxStruct.fullPagePath)) {
                            qDebug() << Q_FUNC_INFO << "filter power pathlist" << m_searchBoxStruct.fullPagePath;
                            continue;
                        }

                        // xc 平板 过滤掉的搜索项
                        if (!isOpenkylinNile && !isOpenkylin && tabletFilterPathList.contains(m_searchBoxStruct.fullPagePath)) {
                            continue;
                        }


                        if (!isV11 && filterPathList.contains(m_searchBoxStruct.fullPagePath)) {
                            continue;
                        }

                        // sw与mips不支持词条过滤
                        if ((cpu.contains("sw") || cpu.contains("mips")) && g_grubFilterPathList.contains(m_searchBoxStruct.fullPagePath)) {
                            qDebug() << Q_FUNC_INFO << "filter boot pathlist" << m_searchBoxStruct.fullPagePath;
                            continue;
                        }

                        // 无kylin-proxy则过滤应用代理词条
                        if (!isKylinProxyProcessRunning && ("/Proxy/App Proxy" == m_searchBoxStruct.fullPagePath)) {
                            continue;
                        }

                        // follow path module name to get actual module name  ->  Left module dispaly can support
                        // mulLanguages
                        m_searchBoxStruct.actualModuleName =
                                getModulesName(m_searchBoxStruct.fullPagePath.section('/', 1, 1));

                        // 插件不存在，跳过该插件的搜索项
                        if ( m_searchBoxStruct.actualModuleName.isEmpty())
                            continue;

                        if (!isChineseFunc(m_searchBoxStruct.translateContent)) {
                            if (!m_TxtList.contains(m_searchBoxStruct.translateContent)) {
                                m_TxtList.append(m_searchBoxStruct.translateContent);
                            }
                        }

                        if ((!isKscDefender && m_searchBoxStruct.fullPagePath.contains("securitycenter",Qt::CaseInsensitive))
                                || (!isCommunity && m_searchBoxStruct.fullPagePath.contains("update")) ) {
                            break;
                        }
#ifndef __sw_64__
                        if(m_searchBoxStruct.fullPagePath.contains("Change valid",Qt::CaseInsensitive)) {
                            break;
                        }
#endif
                        m_EnterNewPagelist.append(m_searchBoxStruct);
                        searchItemList.append(m_searchBoxStruct);

                        // Add search result content
                        if (!m_bIsChinese) {
                            if ("" == m_searchBoxStruct.childPageName) {
                                m_model->appendRow(new QStandardItem(
                                        QString("%1 --> %2")
                                                .arg(m_searchBoxStruct.actualModuleName)
                                                .arg(m_searchBoxStruct.translateContent)));
                            } else {
                                m_model->appendRow(new QStandardItem(
                                        QString("%1 --> %2 / %3")
                                                .arg(m_searchBoxStruct.actualModuleName)
                                                .arg(m_searchBoxStruct.childPageName)
                                                .arg(m_searchBoxStruct.translateContent)));
                            }
                        } else {
                            appendChineseData(m_searchBoxStruct);
                        }
                        QJsonObject bodyJson;
                        QString pluginName = m_searchBoxStruct.fullPagePath.section('/', 1, 1);
                        bodyJson.insert("plugin", pluginName);
                        bodyJson.insert("plugin" + m_lang, m_searchBoxStruct.actualModuleName);
                        bodyJson.insert("subEnglish", m_searchBoxStruct.fullPagePath.section('/', 2, 2));
                        bodyJson.insert("sub" + m_lang, m_searchBoxStruct.translateContent);
                        bodyJson.insert("superordinate", ModulesFactory::getModuleNamebyName(pluginName));
                        bodyJson.insert("superordinate" + m_lang, ModulesFactory::getModuleLocaleNamebyName(pluginName));
                        searchItems.append(bodyJson);
                        clearSearchData();
                    } else {
                        // donthing
                    }
                } else {
                    // qDebug() << "  QXmlStreamReader::Characters with whitespaces.";
                }
                break;
            case QXmlStreamReader::EndElement:
#if DEBUG_XML_SWITCH
                qDebug() << " [SearchWidget] -::EndElement: " << xmlRead.name();
#endif
                // if (m_xmlExplain != "") {
                //     m_xmlExplain = "";
                // }
                break;
            default:
                break;
            }
        }
        m_xmlExplain = "";
        clearSearchData();
        qDebug() << " [SearchWidget] m_EnterNewPagelist.count : " << m_EnterNewPagelist.count();

        file.close();
    }

    if (m_shortcutInterfaceValid) {
        QTime timer;
        timer.start();
        appendShortcutSearch();
        qDebug() << Q_FUNC_INFO << "appendShortcutSearch elapsed time:" << timer.elapsed();
    }
    rootobj.insert("ukcc", searchItems);
    searchMap = rootobj.toVariantMap();
}

void SearchWidget::addShortcutSearch() {
    m_searchBoxStruct.actualModuleName = getModulesName(QString("Shortcut"));
    if (m_searchBoxStruct.actualModuleName.isEmpty()) {
        return;
    }
    auto isChineseFunc = [](const QString &str)->bool {
        QRegularExpression rex_expression(R"(^[^a-zA-Z]+$)");
        return rex_expression.match(str).hasMatch();
    };
    if (!isChineseFunc(m_searchBoxStruct.translateContent)) {
        if (!m_TxtList.contains(m_searchBoxStruct.translateContent)) {
            m_TxtList.append(m_searchBoxStruct.translateContent);
        } else {
            return;
        }
    }
    bool hitPage = false;
    for (const auto& page : m_EnterNewPagelist) {
        if (page.translateContent == m_searchBoxStruct.translateContent) {
            hitPage = true;
            break;
        }
    }
    if (hitPage) {
        return;
    }
    m_EnterNewPagelist.append(m_searchBoxStruct);

    // Add search result content
    if (!m_bIsChinese) {
        if ("" == m_searchBoxStruct.childPageName) {
            m_model->appendRow(new QStandardItem(QString("%1 --> %2")
                            .arg(m_searchBoxStruct.actualModuleName)
                            .arg(m_searchBoxStruct.translateContent)));
        } else {
            m_model->appendRow(new QStandardItem(QString("%1 --> %2 / %3")
                            .arg(m_searchBoxStruct.actualModuleName)
                            .arg(m_searchBoxStruct.childPageName)
                            .arg(m_searchBoxStruct.translateContent)));
        }
    } else {
        appendChineseData(m_searchBoxStruct);
    }
    clearSearchData();
}

void SearchWidget::appendShortcutSearch() {
    QDBusReply<QStringPairList> systemEntryReply = m_shortcutInterface->call("getSystemShortcut");
    if (systemEntryReply.isValid()) {
        clearSearchData();
        QList<QStringPair> systemEntryList = systemEntryReply.value();
        qDebug() << Q_FUNC_INFO << systemEntryList.size();
        for (const auto& systemEntry : systemEntryList) {
            if (!kSystemShortcutAllowList.contains(systemEntry.mediaKey)) {
                qDebug() << Q_FUNC_INFO << "skip system shortcut not in allow list:"
                         << systemEntry.mediaKey << systemEntry.name;
                continue;
            }
            QString shortcutName = systemEntry.name;
            m_searchBoxStruct.fullPagePath = "/Shortcut/System Shortcut";
            m_searchBoxStruct.translateContent = shortcutName;
            addShortcutSearch();
        }
    }
    QDBusReply<QStringPairList> windowEntryReply = m_shortcutInterface->call("getWindowShortcut");
    if (windowEntryReply.isValid()) {
        clearSearchData();
        QSet<QString> windowNameSet;
        QList<QStringPair> windowEntryList = windowEntryReply.value();
        qDebug() << Q_FUNC_INFO << windowEntryList.size();
        for (const auto& windowEntry : windowEntryList) {
            QString shortcutName = windowEntry.name;
            if (windowNameSet.find(shortcutName) != windowNameSet.end()) {
                continue;
            } else {
                windowNameSet.insert(shortcutName);
            }
            m_searchBoxStruct.fullPagePath = "/Shortcut/Window Shortcut";
            m_searchBoxStruct.translateContent = shortcutName;
            addShortcutSearch();
        }
    }
}

//Follow display content to Analysis SearchBoxStruct data
SearchWidget::SearchBoxStruct SearchWidget::getModuleBtnString(QString value)
{
    SearchBoxStruct data;

    data.translateContent = value.section('-', 0, 1).remove('-').trimmed();
    //follow actual module name to get path module name
    data.actualModuleName = getModulesName(data.translateContent, false);
    data.fullPagePath = value.section('>', 1, -1).remove('>').trimmed();

#if DEBUG_XML_SWITCH
    qDebug() << Q_FUNC_INFO << " [SearchWidget] data.translateContent : " << data.translateContent << "   ,  data.fullPagePath : " << data.fullPagePath;
#endif

    return data;
}

QVariantMap SearchWidget::getSearchItems() {
    return searchMap;
}

void SearchWidget::updateSearchItem(QString plugin_name, bool show,
                                    SearchWidget::SearchBoxStruct *data) {
    if (show && data == nullptr) {
        return;
    }
    if (data != nullptr) {
        searchItemList.append(*data);
    }
    for (int index = 0; index < searchItemList.size() && data == nullptr;) {
        QString pluginName = searchItemList.at(index).actualModuleName;
        if (pluginName.contains(plugin_name)) {
            searchItemList.removeAt(index);
        } else {
            ++index;
        }
    }
    QJsonArray searchItems;
    QJsonObject rootobj;
    for (auto &searchItem: searchItemList) {
        QJsonObject bodyJson;
        QString pluginName = searchItem.fullPagePath.section('/', 1, 1);
        bodyJson.insert("plugin", pluginName);
        bodyJson.insert("plugin" + m_lang, searchItem.actualModuleName);
        bodyJson.insert("subEnglish", searchItem.fullPagePath.section('/', 2, 2));
        bodyJson.insert("sub" + m_lang, searchItem.translateContent);
        bodyJson.insert("superordinate", ModulesFactory::getModuleNamebyName(pluginName));
        searchItems.append(bodyJson);
    }
    rootobj.insert("ukcc", searchItems);
    QJsonDocument document;
    document.setObject(rootobj);
    QByteArray byteArray = document.toJson(QJsonDocument::Indented);
    QString strJson(byteArray);
    searchMap = rootobj.toVariantMap();
}

//tranlate the path name to tr("name")
QString SearchWidget::getModulesName(QString name, bool state) {
    QString strResult = "";

    for (auto it : m_moduleNameList) {
        if (state) { //true : follow first search second (use pathName translate to actual moduleName)
            if (it.first == name) {
                strResult = it.second;
                break;
            }
        } else { //false : follow second search first (use actual moduleName translate to pathName)
            if (it.second == name) {
                strResult = it.first;
                break;
            }
        }
    }

    return strResult;
}

QString SearchWidget::removeDigital(QString input) {
    if ("" == input)
        return "";

    QString value = "";
    QByteArray ba = input.toLocal8Bit();
    char *data = nullptr;
    data = ba.data();
    while (*data) {
        if (!(*data >= '0' && *data <= '9')) {
            value += *data;
        }
        data++;
    }

    return value;
}

QString SearchWidget::transPinyinToChinese(QString pinyin) {
    QString value = pinyin;

    //遍历"汉字-拼音"列表,将存在的"拼音"转换为"汉字"
    for (auto data : m_inputList) {
        if (value == data.pinyin) {
            value = data.chiese;
            break;
        }
    }

    return value;
}

QString SearchWidget::containTxtData(QString txt) {
    QString value = txt;

    //遍历"汉字-拼音"列表,将存在的"拼音"转换为"汉字"
    for (auto data : m_inputList) {
        if (data.chiese.contains(txt, Qt::CaseInsensitive) ||
               data.pinyin.contains(txt, Qt::CaseInsensitive)) {
            value = data.chiese;
            break;
        }
    }

    return value;
}

void SearchWidget::appendChineseData(SearchWidget::SearchBoxStruct data) {
    if ("" == data.childPageName) {
        //先添加使用appenRow添加Qt::EditRole数据(用于下拉框显示),然后添加Qt::UserRole数据(用于输入框搜索)
        //Qt::EditRole数据用于显示搜索到的结果(汉字)
        //Qt::UserRole数据用于输入框输入的数据(拼音/汉字 均可)
        //即在输入框搜索Qt::UserRole的数据,就会在下拉框显示Qt::EditRole的数据
        m_model->appendRow(new QStandardItem(//icon.value(),
                                             QString("%1 --> %2").arg(data.actualModuleName).arg(data.translateContent)));

        //设置汉字的Qt::UserRole数据
        m_model->setData(m_model->index(m_model->rowCount() - 1, 0),
                         QString("%1 --> %2")
                         .arg(data.actualModuleName)
                         .arg(data.translateContent),
                         Qt::UserRole);

        QString hanziTxt = QString("%1 --> %2").arg(data.actualModuleName).arg(data.translateContent);

        for (auto datas : m_TxtList) {
            for (int i = 0; i < datas.count(); i++) {
                if (data.translateContent == datas){
                    return;
                }
            }
        }

        QString pinyinTxt = QString("%1 --> %2")
                            .arg(removeDigital(Chinese2Pinyin(data.actualModuleName)))
                            .arg(removeDigital(Chinese2Pinyin(data.translateContent)));

        //添加显示的汉字(用于拼音搜索显示)
        m_model->appendRow(new QStandardItem(/*icon.value(),*/ hanziTxt));
        //设置Qt::UserRole搜索的拼音(即搜索拼音会显示上面的汉字)
        m_model->setData(m_model->index(m_model->rowCount() - 1, 0), pinyinTxt, Qt::UserRole);

        SearchDataStruct transdata;
        transdata.chiese = hanziTxt;
        transdata.pinyin = pinyinTxt;
        //存储 汉字和拼音 : 在选择对应的下拉框数据后,会将Qt::UserRole数据设置到输入框(即pinyin)
        //而在输入框发送 DSearchEdit::textChanged 信号时,会遍历m_inputList,根据pinyin获取到对应汉字,再将汉字设置到输入框
        m_inputList.append(transdata);
    } else {
        //先添加使用appenRow添加Qt::EditRole数据(用于下拉框显示),然后添加Qt::UserRole数据(用于输入框搜索)
        //Qt::EditRole数据用于显示搜索到的结果(汉字)
        //Qt::UserRole数据用于输入框输入的数据(拼音/汉字 均可)
        //即在输入框搜索Qt::UserRole的数据,就会在下拉框显示Qt::EditRole的数据
        m_model->appendRow(new QStandardItem(//icon.value(),
                                             QString("%1 --> %2 / %3").arg(data.actualModuleName).arg(data.childPageName).arg(data.translateContent)));

        //设置汉字的Qt::UserRole数据
        m_model->setData(m_model->index(m_model->rowCount() - 1, 0),
                         QString("%1 --> %2 / %3")
                         .arg(data.actualModuleName)
                         .arg(data.childPageName)
                         .arg(data.translateContent),
                         Qt::UserRole);

        QString hanziTxt = QString("%1 --> %2 / %3").arg(data.actualModuleName).arg(data.childPageName).arg(data.translateContent);
        QString pinyinTxt = QString("%1 --> %2 / %3")
                            .arg(removeDigital(Chinese2Pinyin(data.actualModuleName)))
                            .arg(removeDigital(Chinese2Pinyin(data.childPageName)))
                            .arg(removeDigital(Chinese2Pinyin(data.translateContent)));

        m_model->appendRow(new QStandardItem(/*icon.value(),*/ hanziTxt));
        //设置Qt::UserRole搜索的拼音(即搜索拼音会显示上面的汉字)
        m_model->setData(m_model->index(m_model->rowCount() - 1, 0), pinyinTxt, Qt::UserRole);

        SearchDataStruct transdata;
        transdata.chiese = hanziTxt;
        transdata.pinyin = pinyinTxt;
        //存储 汉字和拼音 : 在选择对应的下拉框数据后,会将Qt::UserRole数据设置到输入框(即pinyin)
        //而在输入框发送 DSearchEdit::textChanged 信号时,会遍历m_inputList,根据pinyin获取到对应汉字,再将汉字设置到输入框
        m_inputList.append(transdata);
    }
}

void SearchWidget::clearSearchData() {
    m_searchBoxStruct.translateContent = "";
    m_searchBoxStruct.actualModuleName = "";
    m_searchBoxStruct.childPageName = "";
    m_searchBoxStruct.fullPagePath = "";
}

void SearchWidget::hiddenSearchItem(QString name, bool show)
{
    if (show && m_model->rowCount() == count) {
            return;
    }
    bool exist = false;
    for (int index = 0 ; index < m_model->rowCount() ; index++) {
        if (m_model->item(index)->text().contains(name + " -->")) {
            exist = true;
            break;
        }
    }
    if (!show) {//去掉需隐藏的搜索项
         for (int index = 0 ; index < m_model->rowCount() ; index++) {
             if (m_model->item(index)->text().contains(name + " -->")) {
                 m_model->removeRow(index);
                 index--;
             }
         }
         updateSearchItem(name, show, nullptr);
    } else if (show && exist){
       return;
    } else {//加入需展示的搜索项
        for(SearchBoxStruct tmpStruct : m_EnterNewPagelist) {
            if (tmpStruct.actualModuleName == name) {
                appendChineseData(tmpStruct);
                updateSearchItem(name, show, &tmpStruct);
            }
        }
     }
}

void SearchWidget::initExcludeSearch() {
    if (!UkccCommon::isExistEffect()) {
        mExcludeList << "/Theme/Performance mode" << "/Theme/Transparency";
    }

    if (!UkccCommon::isExitBattery()) {
        mExcludeList << "/Power/Battery saving plan";
    }

    if (UkccCommon::isWayland() || !UkccCommon::isExistEffect()) {
        mExcludeList << "/Display/night mode";
    }
    if (!UkccCommon::isTablet()) {
        mExcludeList << "/UserinfoIntel/Change Tel" << "/Display/Dynamic light";
    }
}

QString SearchWidget::translationFileForLocale(const QString &pathTemplate) const
{
    if (pathTemplate.contains(QStringLiteral("%1"))) {
        return pathTemplate.arg(m_lang);
    }
    return pathTemplate;
}

void SearchWidget::setLanguage(QString type)
{
    if (m_searchLoadXml) {
        qDebug() << Q_FUNC_INFO << "already load xml, skip";
        return;
    }
    m_searchLoadXml = true;
    m_lang = type;

    if (type == "zh_CN" || type == "zh_HK" || type == "zh_TW" || type == "bo_CN" || type == "mn_MN") {
        m_bIsChinese = true;
        m_completer->setCompletionRole(Qt::UserRole); //设置ItemDataRole
    } else {
        m_completer->setCompletionRole(Qt::DisplayRole);
    }
    // QtConcurrent search dbus cannot get result. issue#I8W7WE
    loadxml();
    //记录搜索项总数
    count = m_model->rowCount();
}

//save all modules moduleInteface name and actual moduleName
//moduleName : moduleInteface name  (used to path module to translate searchName)
//searchName : actual module
void SearchWidget::addTranslationPath(const QString &translation)
{
    if (translation.isEmpty() || m_xmlFilePath.contains(translation)) {
        return;
    }

    m_xmlFilePath.insert(translation);
    if (m_searchLoadXml) {
        loadxml();
    }
}

void SearchWidget::addModulesName(QString moduleName, QString searchName, QString translation) {
    QPair<QString, QString> data;
    data.first = moduleName;
    data.second = searchName;
    m_moduleNameList.append(data);

    addTranslationPath(translation);
}
