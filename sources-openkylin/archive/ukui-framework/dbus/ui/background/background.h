/*
 * UKUI Framework
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <QObject>
#include <QDBusContext>
#include "qgsettings_define.h"
#include "gsettings_manager.h"

class Background : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.UI.Background")
public:
    explicit Background(QObject *parent = nullptr);
    ~Background();

public slots:
    /**
     * @brief GetBackgroundFile
     * 获取当前壁纸所在路径
     * @return
     */
    Q_INVOKABLE const QString GetBackgroundFile() const;

    /**
     * @brief SetBackgroundFile
     * 设置壁纸路径
     * @param filePath
     * @return
     */
    Q_INVOKABLE bool SetBackgroundFile(const QString &filePath);

    /**
     * @brief GetSupportedBackgroundOptions
     * 获取当前支持的壁纸选项
     * @return
     */
    Q_INVOKABLE const QStringList GetSupportedBackgroundOptions();

    /**
     * @brief GetBackgroundOption
     * 获取当前壁纸选项
     * @return
     */
    Q_INVOKABLE const QString GetBackgroundOption() const;

    /**
     * @brief SetBackgroundOption
     * 设置壁纸选项
     * @param option
     * @return
     */
    Q_INVOKABLE bool SetBackgroundOption(const QString &option);

signals:
    void BackgroundFileChanged();

    void BackgroundOptionChanged();

private:
    // 错误string
    const QString C_ERROR_STRING = "";
    const int C_ERROR_NUMBER = 0;
    QHash<QString, OnKeyChanged> setChangedHandler();
    QHash<QString, OnKeyChanged> m_settingsChangedHandlers;
    QString BackgroundFile;
    QString BackgroundOption;
    Q_PROPERTY(QString BackgroundFile READ GetBackgroundFile WRITE SetBackgroundFile NOTIFY BackgroundFileChanged)
    Q_PROPERTY(QString BackgroundOption READ GetBackgroundOption WRITE SetBackgroundOption NOTIFY BackgroundOptionChanged)
    GsettingsManager *m_pGsettingsManager = nullptr;
};

#endif // BACKGROUND_H
