/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
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
 */

#ifndef UKUI_QUICK_PLATFORM_UKUI_LOTTIE_DOT_LOTTIE_PARSER_JSON_P_H
#define UKUI_QUICK_PLATFORM_UKUI_LOTTIE_DOT_LOTTIE_PARSER_JSON_P_H

#include "dot-lottie-parser.h"

#include <QByteArray>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QString>
#include <QtGlobal>

namespace UkuiQuick::LottieParserInternal {

using CancellationCheck = DotLottieParser::CancellationCheck;

struct Q_DECL_HIDDEN JsonReadResult
{
    QByteArray data;
    QString error;
    bool cancelled = false;
};

enum class JsonParseStatus
{
    NoInput,
    Parsed,
    CancelledDuringRead,
    CancelledBeforeParse,
    CancelledAfterParse
};

struct Q_DECL_HIDDEN JsonParseResult
{
    QByteArray inputData;
    QJsonDocument document;
    QJsonParseError parseError{};
    QString readError;
    bool hasInput = false;
    JsonParseStatus status = JsonParseStatus::NoInput;

    bool wasCancelled() const
    {
        return status == JsonParseStatus::CancelledDuringRead ||
               status == JsonParseStatus::CancelledBeforeParse ||
               status == JsonParseStatus::CancelledAfterParse;
    }
};

// JSON 必须从这里读取，调用方不能绕过统一的文件大小上限。
static inline JsonReadResult readFileData(const QString &path,
                                          const CancellationCheck &isCancelled,
                                          qint64 maxBytes)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return {{}, file.errorString(), false};
    }

    QByteArray data;
    while (true) {
        if (isCancelled && isCancelled()) {
            return {{}, QStringLiteral("asset work was cancelled"), true};
        }
        const QByteArray chunk = file.read(64 * 1024);
        if (chunk.isEmpty()) {
            if (file.error() != QFileDevice::NoError) {
                return {{}, file.errorString(), false};
            }
            break;
        }
        // 先检查再追加，确保内存里从来不会暂存超过上限的数据。
        if (maxBytes >= 0 && data.size() + chunk.size() > maxBytes) {
            return {{}, QStringLiteral("input exceeds the size budget"), false};
        }
        data.append(chunk);
    }
    return {data, {}, false};
}

// 解析调用方已经限额读取的 JSON。取消检查必须紧贴 fromJson 前后。
static inline JsonParseResult parseJsonData(const QByteArray &data,
                                            const CancellationCheck &isCancelled = {})
{
    JsonParseResult result;
    result.inputData = data;
    result.hasInput = !data.isEmpty();
    if (isCancelled && isCancelled()) {
        // 真正解析前检查一次取消，避免已经取消的任务继续占用 CPU。
        result.status = JsonParseStatus::CancelledBeforeParse;
        return result;
    }
    if (!result.hasInput) {
        return result;
    }

    result.document = QJsonDocument::fromJson(data, &result.parseError);
    // 解析后无条件再检查一次取消，并且必须早于任何错误或字段判断。
    // 这样即使 JSON 格式错误，解析期间到达的取消请求也仍然优先返回。
    if (isCancelled && isCancelled()) {
        result.status = JsonParseStatus::CancelledAfterParse;
        return result;
    }

    result.status = JsonParseStatus::Parsed;
    return result;
}

// DotLottieParser 的所有磁盘 JSON 文件都通过这个可取消入口读取和解析。
static inline JsonParseResult parseJsonFile(const QString &path,
                                            const CancellationCheck &isCancelled = {})
{
    const JsonReadResult readResult = readFileData(path,
                                                   isCancelled,
                                                   DotLottieParser::maxJsonInputBytes());
    if (readResult.cancelled) {
        JsonParseResult result;
        result.readError = readResult.error;
        result.status = JsonParseStatus::CancelledDuringRead;
        return result;
    }

    JsonParseResult result = parseJsonData(readResult.data, isCancelled);
    result.readError = readResult.error;
    return result;
}

} // namespace UkuiQuick::LottieParserInternal

#endif // UKUI_QUICK_PLATFORM_UKUI_LOTTIE_DOT_LOTTIE_PARSER_JSON_P_H
