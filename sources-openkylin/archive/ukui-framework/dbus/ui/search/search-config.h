/*
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 */

#ifndef SEARCHCONFIG_H
#define SEARCHCONFIG_H

#include <QObject>
#include <QDBusContext>
#include <QStringList>

#include "qgsettings.h"

class SearchConfig : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.UI.Search")
    Q_PROPERTY(QString DefaultSearchEngine READ GetDefaultSearchEngine WRITE SetDefaultSearchEngine NOTIFY DefaultSearchEngineChanged)
    Q_PROPERTY(bool FileNameSearchEnabled READ IsFileNameSearchEnabled WRITE SetFileNameSearchEnabled NOTIFY FileNameSearchEnabledChanged)

public:
    explicit SearchConfig(QObject *parent = nullptr);
    ~SearchConfig();

public slots:
    Q_INVOKABLE QString GetDefaultSearchEngine() const;
    Q_INVOKABLE bool SetDefaultSearchEngine(const QString &engine);
    Q_INVOKABLE QStringList GetSupportedDefaultSearchEngines() const;

    Q_INVOKABLE bool IsFileNameSearchEnabled() const;
    Q_INVOKABLE bool SetFileNameSearchEnabled(bool enabled);

signals:
    void DefaultSearchEngineChanged(const QString &engine);
    void FileNameSearchEnabledChanged(bool enabled);

private slots:
    void onSettingsChanged(const QString &key);

private:
    bool isAvailable() const;
    void sendUnavailableError() const;
    void sendInvalidEngineError(const QString &engine) const;

private:
    UKUI::QGSettings *m_settings = nullptr;
};

#endif // SEARCHCONFIG_H
