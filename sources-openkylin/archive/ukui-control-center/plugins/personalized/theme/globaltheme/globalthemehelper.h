/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef GLOBALTHEMEHELPER_H
#define GLOBALTHEMEHELPER_H

#include <QObject>
#include "globalthemehelper_p.h"

class GlobalThemeHelper : public QObject
{
    friend class GlobalTheme;
    Q_OBJECT
public:
    static GlobalThemeHelper* getInstance();

    /*!
     * \brief getCurrentGlobalTheme
     * \return 当前全局主题名称，不为空
     */
    QString getCurrentGlobalThemeName();

    /*!
     * \brief getCurrentGlobalTheme
     * \return
     * 当前全局主题的globaltheme实例
     */
    GlobalTheme *getCurrentGlobalTheme();

    /*!
     * \brief getCurrentGlobalThemeModified
     * \return 如果做了自定义设置，则为true
     */
    bool getCurrentGlobalThemeModified();

    QList<GlobalTheme *> getAllGlobalThemes();
    QStringList getAllGlobalThemeNames();
    QPixmap getThemeThumbnail(const QString &theme);

    /*!
     * \brief setGlobalTheme
     * 设置全局主题
     * \param theme
     * \return true表示设置成功，false失败，一般可能是theme不存在，或者重复设置
     */
    bool setGlobalTheme(const QString &theme);

    /*!
     * \brief invalidateCurrentGlobalTheme
     * 设置自定义子项时应该触发，表示全局主题被修改
     */
    void invalidateCurrentGlobalTheme();

    /*!
     * \brief getCustomTheme
     * \return 自定义主题
     */
    GlobalTheme *getCustomTheme();

    /*!
     * \brief syncCustomThemeFromCurrentTheme
     * 从当前全局主题同步自定义主题，需要在切换到自定义主题之前做这一个操作
     * 然后可能需要调用updateCustomThemeSetting()更新自定义的主题
     */
    void syncCustomThemeFromCurrentTheme();

    /*!
     * \brief updateCustomThemeSetting
     * 用于更新自定义主题，key值是对应方法的name，value是需要更新的值
     * \param functionName
     * \param value
     */
    void updateCustomThemeSetting(const QString &functionName, const QVariant &value);

    /*!
     * \brief loadThumbnail
     * \param theme
     * 异步加载全局主题的缩略图，globaltheme的thumbnailLoaded信号触发后可以获取
     */
    void loadThumbnail(GlobalTheme *theme);

Q_SIGNALS:
    /*!
     * \brief globalThemesChanged
     * 全局主题列表改变
     */
    void globalThemesChanged();

private:
    explicit GlobalThemeHelper(QObject *parent = nullptr);

    Q_DECLARE_PRIVATE(GlobalThemeHelper)
    GlobalThemeHelperPrivate *d_ptr;
};

#endif // GLOBALTHEMEHELPER_H
