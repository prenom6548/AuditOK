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

#ifndef FONT_H
#define FONT_H

#include <QObject>
#include <QDBusContext>
#include "qgsettings_define.h"
#include "gsettings_manager.h"

class Font : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.UI.Font")
public:
    explicit Font(QObject *parent = nullptr);
    ~Font();

public slots:
    /**
     * @brief GetFontFamily
     * 获取当前字体
     * @return
     */
    Q_INVOKABLE const QString GetFontFamily() const;

    /**
     * @brief SetFontFamily
     * 设置当前字体
     * @param family
     * @return
     */
    Q_INVOKABLE bool SetFontFamily(const QString &family);

    /**
     * @brief GetFontSize
     * 获取当前字体大小
     * @return
     */
    Q_INVOKABLE double GetFontSize() const;

    /**
     * @brief SetFontSize
     * 设置当前字体大小
     * @param size
     * @return
     */
    Q_INVOKABLE bool SetFontSize(double size);

    /**
     * @brief SetFontSize
     * 使用类型设置当前字体大小
     * @param size
     * @return
     */
    Q_INVOKABLE bool SetFontSizeWithString(QString size);

    /**
     * @brief SetFontSize
     * 使用类型设置当前字体大小
     * @param size
     * @return
     */
    Q_INVOKABLE bool SetFontSizeWithType(QString size);

    /**
     * @brief GetFontSizeWithType
     * 获取字体大小的类型
     * @return
     */
    Q_INVOKABLE QString GetFontSizeWithType();

signals:
    void FontFamilyChanged();

    void FontSizeChanged();

private:
    QHash<QString, OnKeyChanged> setChangedHandler();
    QHash<QString, OnKeyChanged> m_settingsChangedHandlers;

    GsettingsManager *m_pGsettingsManager = nullptr;

    QString FontFamily = "Noto Sans CJK SC";
    double FontSize = 11;
    const QMap<QString,double> m_fontSizeMap = {{"s",10},
                                                {"m",12},
                                                {"l",13.5},
                                                {"xl",15}};
    Q_PROPERTY(QString FontFamily READ GetFontFamily WRITE SetFontFamily NOTIFY FontFamilyChanged)
    Q_PROPERTY(double FontSize READ GetFontSize WRITE SetFontSize NOTIFY FontSizeChanged)
};

#endif // FONT_H
