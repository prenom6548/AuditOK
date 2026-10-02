/*
 * libkysdk-qtwidgets's Library
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
 * Authors: Yaxian Liu <liuyaxian@kylinos.cn>
 *
 */

#ifndef KDIRTABBAR_H
#define KDIRTABBAR_H

#include <QToolButton>
#include <QWidget>

namespace kdk
{

class KDirTabBarPrivate;

/**
 * @brief 目录层级控件
 * @since 2.5.2.0
 */
class KDirTabBar : public QWidget
{
    Q_OBJECT
public:
    KDirTabBar(QWidget *parent = nullptr);
    ~KDirTabBar();

    /**
     * @brief 设置目标路径
     * @param path
     */
    void setCurrentPath(const QString Dirpath);

    /**
     * @brief 获取目标路径
     * @return QString
     */
    QString currentPath() const;

    /**
     * @brief 设置tabbar显示个数
     * @param int
     */
    void setVisibleDepth(const int depth);

    /**
     * @brief 获取tabbar显示个数
     * @return int
     */
    int visibleDepth();

    /**
     * @brief 设置图标
     * @param icon
     */
    void setIcon(int index, const QIcon &icon);

    /**
     * @brief 获取图标
     * @return QIcon
     */
    QIcon icon(int index);

    /**
     * @brief 设置Root
     * @param
     */
    void setRootPath(const QString targetPath);

    /**
     * @brief 获取Root路径
     * @return QString
     */
    QString RootPath();

    /**
     * @brief 设置按钮显示字体长度
     * @param int
     */
    void setTextLength(int length);

    /**
     * @brief 返回字体显示长度
     * @return int
     */
    int textLength();

    /**
     * @brief 返回button列表
     * @return
     */
    QList<QToolButton *> buttons();
    //    QToolButton * buttonAt(int index);

signals:
    void tabBarClicked(const QString uri);

protected:
    //    void paintEvent(QPaintEvent *event);//有图标和无图标，三态
    //    void mousePressEvent(QMouseEvent *e) override;

private:
    Q_DECLARE_PRIVATE(KDirTabBar);
    KDirTabBarPrivate *const d_ptr;
};

}
#endif // KDIRTABBAR_H
