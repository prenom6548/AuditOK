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
#include <QDebug>
#include <QList>
#include <QStandardPaths>
#include "fontinterface.h"

#define INTERFACE_SCHEMA   "org.mate.interface"
#define DOC_FONT_KEY       "document-font-name"  // 用于阅读文档的默认字体的名称
#define GTK_FONT_KEY       "font-name"           // gkt+使用的默认字体
#define MONOSPACE_FONT_KEY "monospace-font-name" // 用于终端等处的等宽字体

#define STYLE_FONT_SCHEMA  "org.ukui.style"
#define SYSTEM_FONT_EKY    "system-font-size"
#define SYSTEM_NAME_KEY    "system-font"
#define FONT_STYLE         "system-font-style"
#define FONT_STYLE_KEY     "systemFontStyle"

#define UKCC_SCHEMA        "org.ukui.control-center"
#define GNOME_SCHEMA       "org.gnome.desktop.interface"

FontInterface::FontInterface():UKUI::ServiceObject(nullptr)
{
    qRegisterMetaType<FontInformation>("FontInformation");
    qDBusRegisterMetaType<FontInformation>();
    qRegisterMetaType<FontInformationList>("FontInformationList");
    qDBusRegisterMetaType<FontInformationList>();

    qRegisterMetaType<ViewData>("ViewData");
    qDBusRegisterMetaType<ViewData>();
    qRegisterMetaType<ViewDataList>("ViewDataList");
    qDBusRegisterMetaType<ViewDataList>();

    clear();
    if (!init()) {
        qCritical() << "FontInterface init fail";
    }
}

FontInterface::~FontInterface()
{
    clear();
}

void FontInterface::clear()
{
    m_installPath.clear();
    m_fontData.clear();
    m_viewDataList.clear();
    m_installViewDataList.clear();
    m_gtkfontStrList.clear();
    m_docfontStrList.clear();
    m_monospacefontStrList.clear();
}

bool FontInterface::init(void)
{
    const QByteArray styleID(STYLE_FONT_SCHEMA);
    m_styleSettings = new QGSettings(styleID, QByteArray(), this);

    const QByteArray id(INTERFACE_SCHEMA);
    m_ifsettings = new QGSettings(id, QByteArray(), this);

    const QByteArray ukcc(UKCC_SCHEMA);
    m_ukccSettings = new QGSettings(ukcc, QByteArray(), this);

    m_installPath = QStandardPaths::writableLocation(QStandardPaths::FontsLocation) + "/";

    getCurrentFontInfo();

    LibFun::updateFontList(); // 刷新字体库缓存

    connect(this, &FontInterface::updateFontSignal, this, [=]() {
        updateFontList();
    });

    return true;
}

QStringList FontInterface::splitFontNameSize(QString value) {
    QStringList valueStringList;
    QString str = value.right(1);
    QString ch0 = static_cast<QString>('0');
    QString ch9 = static_cast<QString>('9');
    if (str >= ch0 && str <= ch9) {
        QStringList tmpStringList = value.split(' ');
        QString::SectionFlag flag = QString::SectionSkipEmpty;
        valueStringList << value.section(' ', 0, tmpStringList.length() - 2, flag);
        valueStringList << value.section(' ', tmpStringList.length() - 1, tmpStringList.length() - 1, flag);
    } else {
        valueStringList << value << "10";
    }
    return valueStringList;
}

void FontInterface::getCurrentFontInfo()
{
    m_gtkfontStrList = splitFontNameSize(m_ifsettings->get(GTK_FONT_KEY).toString());
    m_docfontStrList = splitFontNameSize(m_ifsettings->get(DOC_FONT_KEY).toString());
    m_monospacefontStrList = splitFontNameSize(m_ifsettings->get(MONOSPACE_FONT_KEY).toString());
}

QList<ViewData> FontInterface::getViewData()
{
    qDebug() << Q_FUNC_INFO << "viewdata size" << m_viewDataList.size();
    return m_viewDataList;
}

QList<ViewData> FontInterface::getInstallViewData()
{
    qDebug() << Q_FUNC_INFO << "install viewdata size" << m_installViewDataList.size();
    return m_installViewDataList;
}

bool FontInterface::resetDefault()
{
    m_ifsettings->reset(GTK_FONT_KEY);
    m_ifsettings->reset(DOC_FONT_KEY);
    m_ifsettings->reset(MONOSPACE_FONT_KEY);
    // 字体页设置等宽时同步写了 gnome，恢复时一并重置以免 UI/应用仍读旧值
    const QByteArray gnomeID(GNOME_SCHEMA);
    if (QGSettings::isSchemaInstalled(gnomeID)) {
        QGSettings gnomeSettings(gnomeID);
        gnomeSettings.reset(MONOSPACE_FONT_KEY);
    }
    if (m_styleSettings->keys().contains(FONT_STYLE_KEY)) {
        m_styleSettings->reset(FONT_STYLE);
    } else {
        qDebug() << Q_FUNC_INFO << "org.ukui.style systemFontStyle not exist";
    }
    m_styleSettings->reset(SYSTEM_FONT_EKY);
    m_styleSettings->reset(SYSTEM_NAME_KEY);
    fontKwinSlot();
    return true;
}

bool FontInterface::installFontList(QVariantList pathList)
{
    bool installSuc = true;
    qDebug()  << Q_FUNC_INFO << "installFontList" << pathList.size();
    Q_FOREACH(QVariant variant, pathList) {
        QString path = variant.toString();
        if (!installFont(path)) {
            installSuc = false;
        }
    }
    if (installSuc) {
        Q_EMIT updateFontSignal();
    }
    return installSuc;
}

double FontInterface::getFontSize()
{
    float fontSize = m_styleSettings->get(SYSTEM_FONT_EKY).toFloat();
    return double(fontSize);
}

void FontInterface::setFontSize(double size)
{
    float fontsize = size;
    getCurrentFontInfo();
    m_ifsettings->set(GTK_FONT_KEY, QVariant(QString("%1 %2").arg(m_gtkfontStrList.at(0)).arg(fontsize)));
    m_ifsettings->set(DOC_FONT_KEY, QVariant(QString("%1 %2").arg(m_docfontStrList.at(0)).arg(fontsize)));
    m_ifsettings->set(MONOSPACE_FONT_KEY, QVariant(QString("%1 %2").arg(m_monospacefontStrList.at(0)).arg(fontsize)));
    m_styleSettings->set(SYSTEM_FONT_EKY, QVariant(QString("%1").arg(fontsize)));

    fontKwinSlot();
}

void FontInterface::fontKwinSlot() {
    const int fontSize = int(getFontSize());
    const QString fontType = m_styleSettings->get(SYSTEM_NAME_KEY).toString();
    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "slotFontChange");
    QList<QVariant> args;
    args.append(fontSize);
    args.append(fontType);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
    qDebug() << "send message to KGlobalSettings" << fontSize << fontType;
}

FontInformation FontInterface::getFontDetails(QString path)
{
    FontInformation ret;
    for (int i = 0 ; i < this->m_fontData.count(); i++) {
        if (this->m_fontData.at(i).path == path) {
            ret = this->m_fontData.at(i);
            break;
        }
    }
    qDebug() << Q_FUNC_INFO << "get font detail" << path << ret.name << ret.path;
    return ret;
}

bool FontInterface::installFont(QString path)
{
    if (!LibFun::installFont(path)) {
        qCritical() << "font install fail" << path;
        return false;
    }
    return true;
}

bool FontInterface::uninstallFont(QStringList pathList, QString fontName)
{
    qDebug() << "remove font" << pathList << fontName;
    if (!m_external.removeFontFormContro(fontName)) {
        return false;
    }
    for (const auto& path : pathList) {
        if (!LibFun::uninstallFont(path)) {
            qCritical() << "uninstall font fail" << path << fontName;
            return false;
        }
    }
    Q_EMIT updateFontSignal();
    return true;
}

bool FontInterface::copyFont(QString srcPath , QString dstPath)
{
    for (int i = 0 ; i < this->m_fontData.count() ; i++) {
        if (this->m_fontData.at(i).path == srcPath) {
            if (!LibFun::copyFont(this->m_fontData.at(i).path , dstPath)) {
                qCritical() << "copy font fail" << srcPath << dstPath;
                return false;
            }
            break;
        }
    }
    return true;
}

bool FontInterface::applyFont(QString family, QString path, QString style)
{
    qDebug() << Q_FUNC_INFO << "apply font" << family << path;
    QString foundryLast = ""; // 上个字体铸造商
    QString foundryCurrent = ""; // 当前字体铸造商
    QString foundryTarget = ""; // 目标字体铸造商
    bool firstFont = true;
    QString fontFamily = "";
    bool isDiffFontExist = false; // 存在名字相同，铸造厂商不同的字体（默认为false）

    for (int i = 0 ; i < this->m_fontData.count() ; i++) {
        FontInformation item = this->m_fontData.at(i);
        // 寻找目标字体，判断铸造商是否存在不同
        if (item.family != family) {
            continue;
        }
        qDebug() << item.foundry << item.family << item.path;
        foundryCurrent = item.foundry;
        // 判断当前是否为第一个被找到的字体文件
        if (firstFont) {
            firstFont = false;
            foundryLast = item.foundry;
        }
        // 比较上一个字体和当前的字体的铸造商是否存在不同
        if (QString::compare(foundryLast, foundryCurrent) != 0) {
            isDiffFontExist = true;
        }
        if (item.path == path) {
            foundryTarget = item.foundry;
        }
        foundryLast = foundryCurrent;
    }
    if (family != "Z003" && family != "C059" && !isDiffFontExist) {
        fontFamily = family;
    } else {
        fontFamily = family + " [" + foundryTarget + "]";
    }

    if (m_styleSettings->keys().contains(FONT_STYLE_KEY)) {
        m_styleSettings->set(FONT_STYLE, style);
    } else {
        qDebug() << Q_FUNC_INFO << "org.ukui.style systemFontStyle not exist";
    }
    return this->m_external.applyFont(fontFamily);
}


bool FontInterface::isUsingFont(ViewData* viewData) {
    QString fontName = m_styleSettings->get(SYSTEM_NAME_KEY).toString();
    QString fontStyle;
    if (m_styleSettings->keys().contains(FONT_STYLE_KEY)) {
        fontStyle = m_styleSettings->get(FONT_STYLE).toString();
    } else {
        qDebug() << Q_FUNC_INFO << "org.ukui.style system-font-style not exist";
    }
    if (viewData->family == "C059" || viewData->family == "Z003") {
        if (fontName.contains(viewData->family) && (fontStyle != viewData->style)) {
            if (viewData->styleList.contains(fontStyle)) {
                viewData->style = fontStyle;
                int index = viewData->styleList.indexOf(fontStyle);
                viewData->path = viewData->pathList.at(index);
                return true;
            }
        } else if (fontName.contains(viewData->family) && (fontStyle == viewData->style)) {
            return true;
        }
        return false;
    } else {
        if (fontName == viewData->family && (fontStyle != viewData->style)) {
            if (viewData->styleList.contains(fontStyle)) {
                viewData->style = fontStyle;
                int index = viewData->styleList.indexOf(fontStyle);
                viewData->path = viewData->pathList.at(index);
                return true;
            }
        } else if (fontName == viewData->family && (fontStyle == viewData->style)) {
            return true;
        }
        return false;
    }
}

void FontInterface::updateFontList(bool firstLoad)
{
    if (firstLoad) {
        if (m_firstLoad) {
            m_firstLoad = false;
        } else {
            qDebug() << Q_FUNC_INFO << "already load font list, updateFontApplyList";
            updateFontApplyList();
            return;
        }
    }
    m_fontData.clear();
    m_fontData = LibFun::getAllFontInformation();
    m_viewDataList.clear();
    m_installViewDataList.clear();
    QStringList fontNameList;
    for (int i = 0; i < m_fontData.count(); i++) {
        FontInformation tmp = m_fontData.at(i);
        if (std::find(fontNameList.begin(), fontNameList.end(), tmp.name) != fontNameList.end()) {
            for (auto& viewData : m_viewDataList) {
                if (viewData.name == tmp.name) {
                    viewData.styleList.append(tmp.style);
                    viewData.pathList.append(tmp.path);
                    isUsingFont(&viewData);
                    qDebug() << Q_FUNC_INFO << "font name" << tmp.name << viewData.styleList.size();
                    break;
                }
            }
            // 安装的字体
            if (tmp.path.contains(m_installPath)) {
                for (auto& viewData : m_installViewDataList) {
                    if (viewData.name == tmp.name) {
                        viewData.styleList.append(tmp.style);
                        viewData.pathList.append(tmp.path);
                        isUsingFont(&viewData);
                        qDebug() << Q_FUNC_INFO << "font name" << tmp.name << viewData.styleList.size() << m_installViewDataList.at(0).styleList.size();
                        break;
                    }
                }
            }
            continue;
        } else {
            fontNameList.append(tmp.name);
        }
        ViewData item;
        item.path = tmp.path;
        item.family = tmp.family;
        item.style = tmp.style;
        item.name = tmp.name;
        item.styleList.append(tmp.style);
        item.pathList.append(tmp.path);
        if (item.path.contains(m_installPath)) {
            m_installViewDataList << item;
        }
        m_viewDataList << item;
    }
    qDebug() << Q_FUNC_INFO << "updateFontList allFontData.size" << m_fontData.size() << "viewDataList.size" << m_viewDataList.size() << "installViewDataList.size" << m_installViewDataList.size();

    if (firstLoad) {
        updateFontApplyList();
        qDebug() << Q_FUNC_INFO << "after update updateFontApplyList";
    }
}

void FontInterface::updateFontApplyList()
{
    QString fontName = m_styleSettings->get(SYSTEM_NAME_KEY).toString();
    QString fontStyle;
    if (m_styleSettings->keys().contains(FONT_STYLE_KEY)) {
        fontStyle = m_styleSettings->get(FONT_STYLE).toString();
    } else {
        qDebug() << Q_FUNC_INFO << "org.ukui.style systemFontStyle not exist";
    }

    int applyIndex = -1;
    for (int idx = 0; idx < m_viewDataList.count(); idx++) {
        QString family = m_viewDataList.at(idx).family;
        QString style = m_viewDataList.at(idx).style;
        ViewData* viewData = &m_viewDataList[idx];
        bool isApplyFont = isUsingFont(viewData);
        qDebug() << Q_FUNC_INFO << "updateFontApplyList index" << idx << family << style << fontName << fontStyle << isApplyFont;
        if (isApplyFont) {
            applyIndex = idx;
            break;
        }
    }
    if (applyIndex > 0 && applyIndex < m_viewDataList.count()) {
        ViewData viewdata = m_viewDataList.at(applyIndex);
        m_viewDataList.removeAt(applyIndex);
        m_viewDataList.insert(0, viewdata);
    }
    qDebug() << Q_FUNC_INFO << "updateFontApplyList applyIndex" << applyIndex;
}

bool FontInterface::judgeBadFontFile(QString path)
{
    // 判断文件是否为损坏文件
    if (LibFun::getFontInfo(path) == path) {
        return true;
    }
    return false;
}
