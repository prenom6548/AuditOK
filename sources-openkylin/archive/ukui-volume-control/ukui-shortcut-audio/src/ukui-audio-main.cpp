/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: zhouyong <zhouyong01@kylinos.cn>
 *
 */

#include "ukui-audio-main.h"

#include <QQmlEngine>
#include <QQmlContext>
#include "common/common.h"
#include "control/ukui-audio-control.h"
#include "control/ukui-audio-sink-control.h"
#include "control/ukui-audio-source-control.h"
#include "model/ukui-audio-sink-model.h"
#include "model/ukui-audio-sink-app-volume-model.h"
#include "model/ukui-audio-source-model.h"

void UKUIAudioMain::registerTypes(const char *uri)
{
    Q_ASSERT(QString(uri) == QLatin1String("org.ukui.shortcut.audio"));
    qDebug() << __func__ << uri;

    qmlRegisterSingletonType<Common>("org.ukui.audio.common", 1, 0, "Common", [] (QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> QObject* {
        Common* common = &Common::getInstance();
        return common;
    });
    qmlRegisterSingletonType<UKUIAudioSinkControl>("org.ukui.audio.sinkcontrol", 1, 0, "UKUIAudioSinkControl", [] (QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> QObject* {
        UKUIAudioSinkControl* sinkControl = &UKUIAudioSinkControl::getInstance();
        return sinkControl;
    });
    qmlRegisterSingletonType<UKUIAudioSourceControl>("org.ukui.audio.sourcecontrol", 1, 0, "UKUIAudioSourceControl", [] (QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> QObject* {
        UKUIAudioSourceControl* sourceControl = &UKUIAudioSourceControl::getInstance();
        return sourceControl;
    });
    qmlRegisterSingletonType<UKUIAudioSinkModel>("org.ukui.audio.sinkmodel", 1, 0, "UKUIAudioSinkModel", [] (QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> QObject* {
        Q_UNUSED(qmlEngine)
        Q_UNUSED(jsEngine)
        return &UKUIAudioSinkModel::getInstance();
    });
    qmlRegisterSingletonType<UKUIAudioSinkAppVolumeModel>("org.ukui.audio.sinkappvolumemodel", 1, 0, "UKUIAudioSinkAppVolumeModel", [] (QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> QObject* {
        Q_UNUSED(qmlEngine)
        Q_UNUSED(jsEngine)
        return &UKUIAudioSinkAppVolumeModel::getInstance();
    });

    qmlRegisterSingletonType<UKUIAudioControl>("org.ukui.shortcut.audio", 1, 0, "UKUIAudioControl", [] (QQmlEngine *qmlEngine, QJSEngine *jsEngine) -> QObject* {
        UKUIAudioControl* control = &UKUIAudioControl::getInstance();
        return control;
    });
}
