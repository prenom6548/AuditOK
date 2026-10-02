/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Sisheng He <hesisheng@kylinos.cn>
 *
 */
#ifndef SETTINGGROUP_H
#define SETTINGGROUP_H

#include "kbaseframe.h"
#include <QDebug>
#include <QEvent>
#include <QFrame>
#include <QVBoxLayout>
#include <QWidget>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

/**
 * @defgroup ContainerModules
 * @brief 创建一个 Spacing 为 1 的垂直布局 Frame
 * @since 3.0.1.0
 * {
 */

namespace kdk
{

class KSettingGroupPrivate;

class KSettingGroup : public KBaseFrame
{
    Q_OBJECT
public:
    /**
     * @brief 创建一个 Spacing 为 1 的垂直布局 Frame
     *
     * @param Shape: 设置frameShap
     * @param style: 设置圆角（None:无圆角,Top:上圆角,Bottom:下圆角,Around:四圆角）
     * @param parent
     */
    explicit KSettingGroup(QWidget *parent = nullptr, Shape shape = QFrame::NoFrame, BorderRadiusStyle style = BorderRadiusStyle::None);
    ~KSettingGroup();

    /**
     * @brief 为 mainLayout 添加控件并更新圆角
     *
     * @param item:  需要添加的控件
     * @param autoUpdateShape:  是否自动更新item圆角样式
     * @param visible 实际未show控件时isVisible总是false，导致圆角无法正常，因此重新设置，应为Qt的BUG
     */
    void addWidget(QFrame *item, bool autoUpdateShape = true, bool visible = true);

    /**
     * @brief 为 mainLayout 添加控件并更新圆角
     *
     * @param index: 序号
     * @param item:  需要添加的控件
     * @param autoUpdateShape:  是否自动更新item圆角样式
     * @param visible 实际未show控件时isVisible总是false，导致圆角无法正常，因此重新设置，应为Qt的BUG
     */
    void insertWidget(int index, QFrame *item, bool autoUpdateShape = true, bool visible = true);

    /**
     * @brief 为 mainLayout 移除控件并更新圆角
     *
     * @param w:  需要移除的控件
     * @param autoUpdateShape:  是否自动更新item圆角样式
     */
    void removeWidget(QWidget *w, bool autoUpdateShape = true);

    /**
     * @brief 更新圆角
     *
     */
    void updateShape();

    /**
     * @brief 获取 mainLayout
     *
     * @return QVBoxLayout mainLayout
     */
    QVBoxLayout *layout();

    /**
     * @brief 设置最后一个frame的圆角风格
     *
     * @param style 圆角风格
     */
    void setLastRadiusStyle(BorderRadiusStyle style);

    /**
     * @brief 设置仅一个frame时的圆角风格
     *
     * @param style
     */
    void setOneRadiusStyle(BorderRadiusStyle style);

    /**
     * @brief 获取显示的子控件个数
     *
     * @return int 显示的子控件个数
     */
    int showItemsCount();

    /**
     * @brief 移除并删除所有子控件
     *
     */
    void removeAndDeleteAllWidget();

protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Q_DECLARE_PRIVATE(KSettingGroup)
    KSettingGroupPrivate *const d_ptr;
};

}

#endif // SETTINGGROUP_H
