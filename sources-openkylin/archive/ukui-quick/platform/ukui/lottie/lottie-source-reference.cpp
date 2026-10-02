/*
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "lottie-source-reference.h"

#include <QDir>
#include <QFileInfo>

#include <utility>

namespace UkuiQuick {

namespace {

const QString kMotionIconRoot = QStringLiteral("/usr/share/icons/ukui-motion-icons");

bool hasSuffix(const QString &source, const QString &suffix)
{
    return QFileInfo(source).suffix().compare(suffix, Qt::CaseInsensitive) == 0;
}

bool isBareMotionIconName(const QString &source)
{
    if (source.isEmpty() || source.startsWith(QLatin1Char(':')) ||
        QDir::isAbsolutePath(source) || source.contains(QLatin1Char('/')) ||
        source.contains(QLatin1Char('\\'))) {
        return false;
    }

    const QFileInfo sourceInfo(source);
    const QString suffix = sourceInfo.suffix();
    if (!suffix.isEmpty() &&
        suffix.compare(QStringLiteral("lottie"), Qt::CaseInsensitive) != 0) {
        return false;
    }

    const QString stem = suffix.isEmpty()
        ? sourceInfo.fileName()
        : sourceInfo.completeBaseName();
    return stem.endsWith(QStringLiteral("-motion"), Qt::CaseInsensitive);
}

QString canonicalBaseUrl(const QUrl &baseUrl)
{
    return baseUrl.adjusted(QUrl::NormalizePathSegments).toString(QUrl::FullyEncoded);
}

QString sourceIdentityKey(const QString &kind,
                          const QString &locator,
                          const QString &base = QString())
{
    return kind + QLatin1Char('|') + QString::number(locator.size()) + QLatin1Char(':') +
        locator + QLatin1Char('|') + base;
}

QString existingRegularFile(const QString &path)
{
    if (path.startsWith(QLatin1Char(':'))) {
        const QFileInfo info(path);
        return info.exists() && info.isFile() ? QDir::cleanPath(path) : QString();
    }

    const QFileInfo info(path);
    return info.exists() && info.isFile() && !info.isSymLink()
        ? info.canonicalFilePath()
        : QString();
}

QString resolveAgainstBase(const QString &locator, const QUrl &baseUrl)
{
    if (!baseUrl.isValid() || baseUrl.isEmpty()) {
        return QDir::cleanPath(locator);
    }

    const QUrl resolved = baseUrl.resolved(QUrl(locator));
    if (resolved.isLocalFile()) {
        return QDir::cleanPath(resolved.toLocalFile());
    }
    if (resolved.scheme() == QStringLiteral("qrc")) {
        return QDir::cleanPath(QStringLiteral(":") + resolved.path());
    }
    return {};
}

QString lexicalSourcePath(const LottieSourceReference &source)
{
    switch (source.kind()) {
    case LottieSourceKind::RelativeFile:
        return resolveAgainstBase(source.canonicalLocator(), source.baseUrl());
    case LottieSourceKind::LocalFile:
    case LottieSourceKind::Resource:
        return source.canonicalLocator();
    case LottieSourceKind::MotionIcon:
    case LottieSourceKind::Invalid:
        return {};
    }
    return {};
}

LottieSourceFormat sourceFormat(const QString &path)
{
    return hasSuffix(path, QStringLiteral("lottie"))
        ? LottieSourceFormat::DotLottie
        : LottieSourceFormat::Json;
}

} // namespace

LottieSourceReference LottieSourceReference::fromText(const QString &source,
                                                       const QUrl &baseUrl)
{
    LottieSourceReference reference;

    const QString trimmedSource = source.trimmed();
    if (trimmedSource.isEmpty()) {
        return reference;
    }

    if (trimmedSource.startsWith(QLatin1Char(':'))) {
        reference.m_kind = LottieSourceKind::Resource;
        reference.m_canonicalLocator = QDir::cleanPath(trimmedSource);
        return reference;
    }

    if (QDir::isAbsolutePath(trimmedSource)) {
        reference.m_kind = LottieSourceKind::LocalFile;
        reference.m_canonicalLocator = QDir::cleanPath(trimmedSource);
        return reference;
    }

    const QUrl sourceUrl(trimmedSource);
    if (sourceUrl.isLocalFile()) {
        reference.m_kind = LottieSourceKind::LocalFile;
        reference.m_canonicalLocator = QDir::cleanPath(sourceUrl.toLocalFile());
        return reference;
    }
    if (sourceUrl.scheme() == QStringLiteral("qrc")) {
        reference.m_kind = LottieSourceKind::Resource;
        reference.m_canonicalLocator =
            QDir::cleanPath(QStringLiteral(":") + sourceUrl.path());
        return reference;
    }
    if (!sourceUrl.scheme().isEmpty()) {
        return reference;
    }

    reference.m_baseUrl = baseUrl.adjusted(QUrl::NormalizePathSegments);
    if (isBareMotionIconName(trimmedSource)) {
        const QFileInfo sourceInfo(trimmedSource);
        reference.m_kind = LottieSourceKind::MotionIcon;
        reference.m_canonicalLocator = sourceInfo.suffix().isEmpty()
            ? sourceInfo.fileName() + QStringLiteral(".lottie")
            : sourceInfo.fileName();
        return reference;
    }

    reference.m_kind = LottieSourceKind::RelativeFile;
    reference.m_canonicalLocator = QDir::cleanPath(trimmedSource);
    return reference;
}

bool LottieSourceReference::isLottieSourceText(const QString &source)
{
    const LottieSourceReference reference = fromText(source);
    if (!reference.isValid()) {
        return false;
    }
    return reference.kind() == LottieSourceKind::MotionIcon ||
        hasSuffix(reference.canonicalLocator(), QStringLiteral("lottie")) ||
        hasSuffix(reference.canonicalLocator(), QStringLiteral("json"));
}

bool LottieSourceReference::isValid() const
{
    return m_kind != LottieSourceKind::Invalid && !m_canonicalLocator.isEmpty();
}

LottieSourceKind LottieSourceReference::kind() const
{
    return m_kind;
}

QString LottieSourceReference::canonicalLocator() const
{
    return m_canonicalLocator;
}

QUrl LottieSourceReference::baseUrl() const
{
    return m_baseUrl;
}

QString LottieSourceReference::jobKey() const
{
    if (!isValid()) {
        return {};
    }

    if (m_kind == LottieSourceKind::MotionIcon) {
        return sourceIdentityKey(QStringLiteral("motion"),
                                 m_canonicalLocator,
                                 canonicalBaseUrl(m_baseUrl));
    }

    const QString path = lexicalSourcePath(*this);
    if (path.startsWith(QLatin1Char(':'))) {
        return sourceIdentityKey(QStringLiteral("resource"), path);
    }
    if (QDir::isAbsolutePath(path)) {
        return sourceIdentityKey(QStringLiteral("local"), path);
    }
    return sourceIdentityKey(QStringLiteral("relative"),
                             m_canonicalLocator,
                             canonicalBaseUrl(m_baseUrl));
}

bool LottieResolvedSource::isValid() const
{
    return !path.isEmpty();
}

LottieSourceResolver::LottieSourceResolver(QString motionIconRoot)
    : m_motionIconRoot(QDir::cleanPath(std::move(motionIconRoot)))
{
}

LottieResolvedSource LottieSourceResolver::resolve(
    const LottieSourceReference &source) const
{
    if (!source.isValid()) {
        return {};
    }

    QString resolvedPath;
    if (source.kind() == LottieSourceKind::MotionIcon) {
        // 约定的系统动画目录优先；系统目录没有时，才按当前 QML 文件的
        // 位置解析同名动画。即使两个位置都没有，也把最后一个候选路径
        // 交给后台加载线程，让“文件不存在”从同一个异步出口返回。
        const QString installedCandidate =
            QDir(m_motionIconRoot).filePath(source.canonicalLocator());
        resolvedPath = existingRegularFile(installedCandidate);
        if (resolvedPath.isEmpty()) {
            const QString fallback =
                resolveAgainstBase(source.canonicalLocator(), source.baseUrl());
            const QString existingFallback = existingRegularFile(fallback);
            resolvedPath = existingFallback.isEmpty() ? fallback : existingFallback;
        }
    } else {
        const QString candidate = lexicalSourcePath(source);
        const QString existingCandidate = existingRegularFile(candidate);
        resolvedPath = existingCandidate.isEmpty() ? candidate : existingCandidate;
    }

    return {resolvedPath, sourceFormat(resolvedPath)};
}

QString LottieSourceResolver::defaultMotionIconRoot()
{
    return kMotionIconRoot;
}

} // namespace UkuiQuick
