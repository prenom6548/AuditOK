/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *          hxf <hewenfei@kylinos.cn>
 *
 */
#include "status-info.h"

#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

using namespace UkuiShortcut;

class UkuiShortcut::StatusInfoPrivate : private QJsonObject
{
public:
    explicit StatusInfoPrivate(UkuiShortcut::StatusInfo::InfoType infoType);

    virtual ~StatusInfoPrivate();

    UkuiShortcut::StatusInfo::InfoType getInfoType();
    void setInfoType(UkuiShortcut::StatusInfo::InfoType infoType);

    bool isDisabled();
    void setDisable(bool disable);

    bool isIconClickable();
    void setIconClickable(bool iconClickable);

    void setName(const QString &name);
    QString getName();

    void setStatusName(const QString &statusName);
    QString getStatusName();

    void setWidgetId(const QString &widgetId);
    QString getWidgetId();

    void setIcon(const QString &icon);
    QString getIcon();

    void setToolTip(const QString &tooltip);
    QString getToolTip();

    /**
     * @param color a hex string
     */
    void setColor(const QColor &color);
    void setColor(Color::ColorRole colorRole);
    Color::ColorRole getColor();

    void setValue(int value);
    int getValue();

    void setMaxValue(int maxValue);
    int getMaxValue();

    void setMenu(const QList<UkuiShortcut::StatusInfo::MenuItem> &menu);
    QList<UkuiShortcut::StatusInfo::MenuItem> getMenu();

    QString toJsonString();
};

StatusInfoPrivate::StatusInfoPrivate(UkuiShortcut::StatusInfo::InfoType infoType)
{
    setInfoType(infoType);
}

void StatusInfoPrivate::setInfoType(UkuiShortcut::StatusInfo::InfoType infoType)
{
    insert(QLatin1String("infoType"), {infoType});
}

void StatusInfoPrivate::setName(const QString &name)
{
    insert(QString::number(ValueType::Name), {name});
}

void StatusInfoPrivate::setStatusName(const QString &statusName)
{
    insert(QString::number(ValueType::StatusName), {statusName});
}

void StatusInfoPrivate::setWidgetId(const QString &widgetId)
{
    insert(QString::number(ValueType::WidgetId), {widgetId});
}

void StatusInfoPrivate::setIcon(const QString &icon)
{
    insert(QString::number(ValueType::Icon), {icon});
}

void StatusInfoPrivate::setToolTip(const QString &tooltip)
{
    insert(QString::number(ValueType::ToolTip), {tooltip});
}

void StatusInfoPrivate::setColor(const QColor &color)
{
    Q_UNUSED(color)
    insert(QString::number(ValueType::Color), {Color::ColorRole::BaseColor});
}

void StatusInfoPrivate::setColor(Color::ColorRole colorRole)
{
    insert(QString::number(ValueType::Color), {colorRole});
}

void StatusInfoPrivate::setMenu(const QList<UkuiShortcut::StatusInfo::MenuItem> &menu)
{
    QJsonArray array;
    for (const auto &item : menu) {
        QJsonObject object;
        object.insert(QString::number(ValueType::MenuItemIcon), item.icon);
        object.insert(QString::number(ValueType::MenuItemText), item.text);
        object.insert(QString::number(ValueType::MenuItemStatusIcon), item.statusIcon);

        array.push_back(object);
    }

    insert(QString::number(ValueType::Menu), array);
}

QString StatusInfoPrivate::toJsonString() {
    return {QJsonDocument(*this).toJson(QJsonDocument::Compact)};
}

UkuiShortcut::StatusInfo::InfoType StatusInfoPrivate::getInfoType()
{
    QJsonValue infoType = value(QLatin1String("infoType"));
    return infoType.type() == QJsonValue::Undefined
            ? UkuiShortcut::StatusInfo::Normal
            : static_cast<StatusInfo::InfoType>(infoType.toInt());
}

QString StatusInfoPrivate::getName() {
    QJsonValue name = value(QString::number(ValueType::Name));
    return name.type() == QJsonValue::Undefined ? "" : name.toString();
}

QString StatusInfoPrivate::getStatusName()
{
    QJsonValue statusName = value(QString::number(ValueType::StatusName));
    return statusName.type() == QJsonValue::Undefined ? "" : statusName.toString();
}

QString StatusInfoPrivate::getWidgetId()
{
    QJsonValue widgetId = value(QString::number(ValueType::WidgetId));
    return widgetId.type() == QJsonValue::Undefined ? "" : widgetId.toString();
}

QString StatusInfoPrivate::getIcon() {
    QJsonValue icon = value(QString::number(ValueType::Icon));
    return icon.type() == QJsonValue::Undefined ? "" : icon.toString();
}

QString StatusInfoPrivate::getToolTip() {
    QJsonValue tooltip = value(QString::number(ValueType::ToolTip));
    return tooltip.type() == QJsonValue::Undefined ? "" : tooltip.toString();
}

Color::ColorRole StatusInfoPrivate::getColor() {
    QJsonValue color = value(QString::number(ValueType::Color));
    return color.type() == QJsonValue::Undefined
           ? UkuiShortcut::Color::BaseColor
           : static_cast<Color::ColorRole>(color.toInt());
}

QList<UkuiShortcut::StatusInfo::MenuItem> StatusInfoPrivate::getMenu() {
    QJsonValue menu = value(QString::number(ValueType::Menu));
    if (menu.type() == QJsonValue::Undefined) {
        return {};
    }

    QList<UkuiShortcut::StatusInfo::MenuItem> list;

    for (const auto &item: menu.toArray()) {
        QJsonObject itemObject = item.toObject();

        UkuiShortcut::StatusInfo::MenuItem menuItem {
            itemObject.value(QString::number(ValueType::MenuItemIcon)).toString(),
            itemObject.value(QString::number(ValueType::MenuItemText)).toString(),
            itemObject.value(QString::number(ValueType::MenuItemStatusIcon)).toString()
        };

        list.push_back(menuItem);
    }

    return list;
}

void StatusInfoPrivate::setValue(int value)
{
    insert(QString::number(ValueType::Value), value);
}

int StatusInfoPrivate::getValue()
{
    QJsonValue size = value(QString::number(ValueType::Value));
    return size.type() == QJsonValue::Undefined ? 0 : size.toInt();
}

void StatusInfoPrivate::setMaxValue(int maxValue)
{
    insert(QString::number(ValueType::MaxValue), maxValue);
}

int StatusInfoPrivate::getMaxValue()
{
    QJsonValue size = value(QString::number(ValueType::MaxValue));
    return size.type() == QJsonValue::Undefined ? 100 : size.toInt();
}

bool StatusInfoPrivate::isDisabled()
{
    QJsonValue disable = value(QString::number(ValueType::Disabled));
    return disable.type() != QJsonValue::Undefined && disable.toBool();
}

void StatusInfoPrivate::setDisable(bool disable)
{
    insert(QString::number(ValueType::Disabled), {disable});
}

bool StatusInfoPrivate::isIconClickable()
{
    const QJsonValue iconClickable = value(QString::number(ValueType::IconClickable));
    return iconClickable.type() != QJsonValue::Undefined && iconClickable.toBool();
}

void StatusInfoPrivate::setIconClickable(bool iconClickable)
{
    insert(QString::number(ValueType::IconClickable), iconClickable);
}

StatusInfoPrivate::~StatusInfoPrivate() = default;

//========== StatusInfo ==========

UkuiShortcut::StatusInfo::StatusInfo(InfoType infoType) : d(new StatusInfoPrivate(infoType))
{

}

StatusInfo::~StatusInfo() = default;


UkuiShortcut::StatusInfo::StatusInfo::StatusInfo(const StatusInfo& si) : d(new StatusInfoPrivate(*si.d))
{
//    qDebug() << "StatusInfo copy ready" << &si;
}

StatusInfo &UkuiShortcut::StatusInfo::StatusInfo::operator=(const StatusInfo &rhs)
{
    *d = *rhs.d;
    return *this;
}

StatusInfo::InfoType StatusInfo::getInfoType()
{
    return d->getInfoType();
}

void StatusInfo::setInfoType(UkuiShortcut::StatusInfo::InfoType infoType)
{
    d->setInfoType(infoType);
}

bool StatusInfo::isDisabled() const
{
    return d->isDisabled();
}

void StatusInfo::setDisable(bool disable)
{
    d->setDisable(disable);
}

bool StatusInfo::isIconClickable() const
{
    return d->isIconClickable();
}

void StatusInfo::setIconClickable(bool iconClickable)
{
    d->setIconClickable(iconClickable);
}

void StatusInfo::setName(const QString &name)
{
    d->setName(name);
}

QString StatusInfo::getName() const
{
    return d->getName();
}

void StatusInfo::setStatusName(const QString &statusName)
{
    d->setStatusName(statusName);
}

QString StatusInfo::getStatusName() const
{
    return d->getStatusName();
}

void StatusInfo::setWidgetId(const QString &widgetId)
{
    d->setWidgetId(widgetId);
}

QString StatusInfo::getWidgetId() const
{
    return d->getWidgetId();
}

void StatusInfo::setIcon(const QString &icon)
{
    d->setIcon(icon);
}

QString StatusInfo::getIcon() const
{
    return d->getIcon();
}

void StatusInfo::setToolTip(const QString &tooltip)
{
    d->setToolTip(tooltip);
}

QString StatusInfo::getToolTip() const
{
    return d->getToolTip();
}

void StatusInfo::setColor(const QColor &color)
{
    d->setColor(color);
}

void StatusInfo::setColor(Color::ColorRole colorRole)
{
    d->setColor(colorRole);
}

Color::ColorRole StatusInfo::getColor() const
{
    return d->getColor();
}

void StatusInfo::setMenu(const QList<UkuiShortcut::StatusInfo::MenuItem> &menu)
{
    d->setMenu(menu);
}

QList<UkuiShortcut::StatusInfo::MenuItem> StatusInfo::getMenu() const
{
    return d->getMenu();
}

QString StatusInfo::toJsonString() const
{
    return d->toJsonString();
}

void StatusInfo::setValue(int value)
{
    d->setValue(value);
}

int StatusInfo::getValue() const
{
    return d->getValue();
}

void StatusInfo::setMaxValue(int maxValue)
{
    d->setMaxValue(maxValue);
}

int StatusInfo::getMaxValue() const
{
    return d->getMaxValue();
}
