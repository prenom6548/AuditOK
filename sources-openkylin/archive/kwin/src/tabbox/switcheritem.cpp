/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "switcheritem.h"
// KWin
#include "abstract_output.h"
#include "composite.h"
#include "screens.h"
#include "tabbox/tabbox.h"
#include "tabboxhandler.h"
#include "workspace.h"
// Qt
#include <QAbstractItemModel>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocale>

#define THEME_QT_SCHEMA "org.ukui.style"
#define MODE_QT_KEY "style-name"
#define UKUI_STYLE_SCHEMA_WIDGETNAME "widgetThemeName"

namespace KWin
{
namespace TabBox
{

SwitcherItem::SwitcherItem(QObject *parent)
    : QObject(parent)
    , m_model(nullptr)
    , m_item(nullptr)
    , m_visible(false)
    , m_allDesktops(false)
    , m_currentIndex(0)
    , m_themeSettings(nullptr)
{
    m_selectedIndexConnection = connect(tabBox, &TabBoxHandler::selectedIndexChanged, this, [this] {
        if (isVisible()) {
            setCurrentIndex(tabBox->currentIndex().row());
        }
    });
    const QByteArray id(THEME_QT_SCHEMA);
    if (QGSettings::isSchemaInstalled(id)) {
        m_themeSettings = new QGSettings(id);
        m_theme = m_themeSettings->get(MODE_QT_KEY).toString();
        Q_EMIT themeChanged();
    }
    connect(screens(), &Screens::changed, this, &SwitcherItem::screenGeometryChanged);
    connect(Compositor::self(), &Compositor::compositingToggled, this, &SwitcherItem::compositingChanged);
    initLayoutDirection();
}

SwitcherItem::~SwitcherItem()
{
    if (m_themeSettings)
        delete m_themeSettings;
    disconnect(m_selectedIndexConnection);
}

void SwitcherItem::setItem(QObject *item)
{
    if (m_item == item) {
        return;
    }
    m_item = item;
    Q_EMIT itemChanged();
}

void SwitcherItem::setModel(QAbstractItemModel *model)
{
    m_model = model;
    Q_EMIT modelChanged();
}

int SwitcherItem::getThemeRadius()
{
    // default to 8
    QString widgetName = "default";
    if (m_themeSettings && m_themeSettings->keys().contains(UKUI_STYLE_SCHEMA_WIDGETNAME)) {
        widgetName = m_themeSettings->get(UKUI_STYLE_SCHEMA_WIDGETNAME).toString();
    } else {
        qWarning() << QString("未查询到Gesetting的键值(%1)").arg(UKUI_STYLE_SCHEMA_WIDGETNAME);
        return 8;
    }

    QFile file(QString("/usr/share/qt5-ukui-platformtheme/themeconfig/%1.json").arg(widgetName));
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开JSON文件";
        return 8;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    // 将JSON数据转换为QJsonDocument对象
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &error);
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON格式错误:" << error.errorString();
        return 8;
    }

    // 获取属性
    QJsonObject rootObj = jsonDoc.object();
    QJsonObject radiusObj = rootObj["Radius"].toObject();
    QJsonObject radiusValueObj = radiusObj["Max_Radius"].toObject();

    return radiusValueObj["value"].toString().toInt();
}

void SwitcherItem::setVisible(bool visible)
{
    if (m_visible == visible) {
        return;
    }
    if (visible)
        Q_EMIT screenGeometryChanged();
    m_visible = visible;
    m_theme = m_themeSettings ? m_themeSettings->get(MODE_QT_KEY).toString() : QStringLiteral("ukui-default");
    Q_EMIT themeChanged();
    Q_EMIT visibleChanged();
}

QRect SwitcherItem::screenGeometry() const
{
    return workspace()->activeOutput()->geometry();
}

void SwitcherItem::setCurrentIndex(int index)
{
    if (m_currentIndex == index) {
        return;
    }
    m_currentIndex = index;
    if (m_model) {
        tabBox->setCurrentIndex(m_model->index(index, 0));
    }
    Q_EMIT currentIndexChanged(m_currentIndex);
}

void SwitcherItem::setAllDesktops(bool all)
{
    if (m_allDesktops == all) {
        return;
    }
    m_allDesktops = all;
    Q_EMIT allDesktopsChanged();
}

void SwitcherItem::setNoModifierGrab(bool set)
{
    if (m_noModifierGrab == set) {
        return;
    }
    m_noModifierGrab = set;
    Q_EMIT noModifierGrabChanged();
}

void SwitcherItem::selectWindow()
{
    TabBox::TabBox::self()->accept(false);
    TabBox::TabBox::self()->close(true);
}

bool SwitcherItem::compositing()
{
    return Compositor::compositing();
}

void SwitcherItem::initLayoutDirection()
{
    QString systemLang = QLocale::system().name();
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        m_isLayoutLeftToRight = false;
    } else {
        m_isLayoutLeftToRight = true;
    }
}

}
}
