/*
    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>
    SPDX-FileCopyrightText: 2023 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QDBusArgument>

#include "typedefs.h"

const QDBusArgument &operator<<(QDBusArgument &argument, const DbusImageStruct &icon);
const QDBusArgument &operator>>(const QDBusArgument &argument, DbusImageStruct &icon);

const QDBusArgument &operator<<(QDBusArgument &argument, const DbusImageVector &iconVector);
const QDBusArgument &operator>>(const QDBusArgument &argument, DbusImageVector &iconVector);

const QDBusArgument &operator<<(QDBusArgument &argument, const DbusToolTipStruct &toolTip);
const QDBusArgument &operator>>(const QDBusArgument &argument, DbusToolTipStruct &toolTip);
