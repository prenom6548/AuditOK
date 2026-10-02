/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef POWERITEMWIDGET_H
#define POWERITEMWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include "customlabel.h"

#include <QMouseEvent>
#include <QResizeEvent>

class StatusButton;

class PowerItemWidget : public QFrame
{
    Q_OBJECT

public:
    explicit PowerItemWidget(QString powerName, QWidget *parent = 0);
    ~PowerItemWidget();
    void setText(QString text);
    void setToolTip(const QString tip);
    void setIcon(const QIcon &icon);
    void setUpgradeTimeText(QString upgradetime);
    /**
     * @brief 设置按钮当前状态
     * @param selected 是否选中
     * @return
     */
    void setSelected(int status);

    void setFontFamily(QString fontFamily);

    void setFontSize(double fontSize);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

Q_SIGNALS:
    /**
     * @brief 响应点击事件
     * @return
     */
    void clicked();

    void iconClicked();           // 点击 icon 时
    void nonIconAreaClicked();    // 点击非 icon 区域时

private:
    StatusButton *itemFace = nullptr;
    CustomLabel *textLabel = nullptr;
    CustomLabel *upgradeTextLabel = nullptr;
    QVBoxLayout *itemlayout = nullptr;
    QHBoxLayout *upgradeLayout = nullptr;
    QString m_powerName = "";

    QRect m_iconRect;
    bool m_pressedInIcon;
    bool m_pressedInWidget;  // 记录是否按在整个 widget 上
};
#endif // POWERITEMWIDGET_H
