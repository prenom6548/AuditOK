/*
 * Copyright © 2016-2018 Red Hat, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *       Jan Grulich <jgrulich@redhat.com>
 * Modifier:
 *       Yue Lan <lanyue@kylinos.cn>
 */

#include "ukuidtparameters.h"

#include <QDialogButtonBox>
#include <QDBusMetaType>
#include <QDBusArgument>
#include <QLoggingCategory>
#include <QFile>
#include <QPushButton>
#include <QVBoxLayout>
#include <QUrl>

#include <QStandardPaths>


Q_LOGGING_CATEGORY(UKUIDTParameters, "ukui-dt-parameters")


DTParameters::DTParameters(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{

    m_dtconfig = new UKUIThemeConfigDaemon::GlobalDTConfig();
}

DTParameters::~DTParameters()
{
}

QVariantMap DTParameters::GetDTThemeConfig(const QString key)
{
    qCDebug(UKUIDTParameters) << "dt key: " << key;

    QVariantMap results;
    DTThemeData dtDate;
    QByteArray color;
    QByteArray startColor;
    QByteArray endColor;

    if (key.isEmpty() || !m_dtconfig->getDTDateMap().contains(key)) {
        qWarning() << "Unvalue key!" << key;
        results.insert(QStringLiteral("type"), DTThemeData::None);
        return results;
    }

    dtDate = m_dtconfig->getDTDateMap().value(key);
    results.insert(QStringLiteral("type"), dtDate.dateType());
    switch (dtDate.dateType()) {
    case DTThemeData::Integer:
        results.insert(QStringLiteral("int_value"), dtDate.intValue());
        break;
    case DTThemeData::Color:
        color.resize(4);
        color[0] = dtDate.color().red();
        color[1] = dtDate.color().green();
        color[2] = dtDate.color().blue();
        color[3] = dtDate.color().alpha();
        results.insert(QStringLiteral("color"), color);
        break;
    case DTThemeData::Gradient:
        startColor.resize(4);
        startColor[0] = dtDate.startColor().red();
        startColor[1] = dtDate.startColor().green();
        startColor[2] = dtDate.startColor().blue();
        startColor[3] = dtDate.startColor().alpha();

        endColor.resize(4);
        endColor[0] = dtDate.endColor().red();
        endColor[1] = dtDate.endColor().green();
        endColor[2] = dtDate.endColor().blue();
        endColor[3] = dtDate.endColor().alpha();

        results.insert(QStringLiteral("start_color"), startColor);
        results.insert(QStringLiteral("end_color"), endColor);
        break;
    default:
        break;
    }

//    //QVariantList
//    QVariantList color;
//    QVariantList startColor;
//    QVariantList endColor;

//    results.insert(QStringLiteral("type"), dtDate.dateType());
//    switch (dtDate.dateType()) {
//    case DTThemeData::Color:
//        color<<dtDate.color().red();
//        color<<dtDate.color().green();
//        color<<dtDate.color().blue();
//        color<<dtDate.color().alpha();

//        results.insert(QStringLiteral("color"), color);
//        break;
//    case DTThemeData::Gradient:
//        startColor<<dtDate.startColor().red();
//        startColor<<dtDate.startColor().green();
//        startColor<<dtDate.startColor().blue();
//        startColor<<dtDate.startColor().alpha();

//        endColor<<dtDate.endColor().red();
//        endColor<<dtDate.endColor().green();
//        endColor<<dtDate.endColor().blue();
//        endColor<<dtDate.endColor().alpha();

//        results.insert(QStringLiteral("start_color"), startColor);
//        results.insert(QStringLiteral("end_color"), endColor);
//        break;
//    case DTThemeData::Integer:
//        results.insert(QStringLiteral("int_value"), dtDate.intValue());
//        break;
//    default:
//        break;
//    }

    qCDebug(UKUIDTParameters) << "map size"<<results.size() << "map"<<results;

    return results;
}
