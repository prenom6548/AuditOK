/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef COMPUTERPROPERTIESPAGE_H
#define COMPUTERPROPERTIESPAGE_H

#include <QWidget>

#include "properties-window-tab-iface.h"
#include "peony-core_global.h"
#include "volume-manager.h"

class QFormLayout;

namespace Peony {

class PEONYCORESHARED_EXPORT ComputerPropertiesPage : public PropertiesWindowTabIface
{
    Q_OBJECT
public:
    explicit ComputerPropertiesPage(const QString &uri, QWidget *parent = nullptr);

protected:
    void addSeparator();

    /**
     * \brief dbus获取文件系统函数
     * \param uri
     * \return
     */
    QString getFileSystemType(QString uri);

    /*!
     * \brief 通过给定的targetUri从全部卷中获取匹配的卷。
     * \param targetUri 需要匹配的目标Uri (挂载路径)
     * \return 具有给定targetUri的卷，如果不存在则返回 nullptr
     */
    static std::shared_ptr<Volume> EnumerateOneVolumeByTargetUri(QString targetUri);

    /**
     * \brief 统一格式化容量字符串
     * \param capacityNum
     * \return
     */
    QString formatCapacityString(quint64 capacityNum);

    /**
     * \brief 把任意 设备映射路径（/dev/mapper/xxx 或 /dev/dm-N）转换成内核打印风格
     * (若输入/dev/sdc1非加密则不处理)
     * 例： 对于输入的加密设备的映射路径： /dev/mapper/luks-uuidxxx  -> "dm_2d3"
     *                                   /dev/dm-0                -> "dm_2d0"
     * @paramm apperAlias 可能是 /dev/mapper/xxx、 /dev/dm-N、dev/sdc1(默认不处理)
     * @return 转义后的 dm 名字，形如: /devdm_2d0
     */
    static void resolveKernelDmName(QString& mapperAlias);

private:
    QString m_uri;
    QFormLayout *m_layout;

    quint64 m_home_counted_size = 0;
    quint64 m_root_counted_size = 0;
    quint64 m_usershare_counted_size = 0;
    quint64 m_others_counted_size = 0;

    bool m_count_home_done = false;
    bool m_count_usershare_done = false;
    bool m_count_others_done = false;

    QTimer *m_timer = nullptr;

    // PropertiesWindowTabIface interface
public:
    void saveAllChange();
};

}

#endif // COMPUTERPROPERTIESPAGE_H
