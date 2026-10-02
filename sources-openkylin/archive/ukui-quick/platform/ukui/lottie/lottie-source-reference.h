/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef UKUI_QUICK_LOTTIE_SOURCE_REFERENCE_H
#define UKUI_QUICK_LOTTIE_SOURCE_REFERENCE_H

#include "lottie-core-visibility_p.h"

#include <QString>
#include <QUrl>

namespace UkuiQuick {

enum class LottieSourceKind : quint8 {
    Invalid,
    MotionIcon,
    RelativeFile,
    LocalFile,
    Resource
};

enum class LottieSourceFormat : quint8 {
    Json,
    DotLottie
};

class UKUI_QUICK_LOTTIE_CORE_PRIVATE_EXPORT LottieSourceReference
{
public:
    static LottieSourceReference fromText(const QString &source,
                                          const QUrl &baseUrl = QUrl());
    static bool isLottieSourceText(const QString &source);

    bool isValid() const;
    LottieSourceKind kind() const;
    QString canonicalLocator() const;
    QUrl baseUrl() const;
    QString jobKey() const;

private:
    QString m_canonicalLocator;
    QUrl m_baseUrl;
    LottieSourceKind m_kind = LottieSourceKind::Invalid;
};

struct UKUI_QUICK_LOTTIE_CORE_PRIVATE_EXPORT LottieResolvedSource
{
    QString path;
    LottieSourceFormat format = LottieSourceFormat::Json;

    bool isValid() const;
};

class UKUI_QUICK_LOTTIE_CORE_PRIVATE_EXPORT LottieSourceResolver
{
public:
    explicit LottieSourceResolver(QString motionIconRoot = defaultMotionIconRoot());

    LottieResolvedSource resolve(const LottieSourceReference &source) const;
    static QString defaultMotionIconRoot();

private:
    QString m_motionIconRoot;
};

} // namespace UkuiQuick

#endif // UKUI_QUICK_LOTTIE_SOURCE_REFERENCE_H
