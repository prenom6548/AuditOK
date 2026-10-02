/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef LOCALUPDATENAME_H
#define LOCALUPDATENAME_H

#include <QWidget>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScreen>
#include <QGuiApplication>

#include "view/titlebar/titlebar.h"
#include "global/utils/inisetting.h"

class LocalUpdateName : public QWidget
{
    Q_OBJECT
    friend class TestLocalUpdateName;
public:
    explicit LocalUpdateName(int nameType, bool isAuto, QWidget *parent = nullptr);
    explicit LocalUpdateName(int nameType, int friendId, QWidget *parent = nullptr);

    ~LocalUpdateName();

    // 初始化数据
    void setInitData(int nameType, bool isAuto);
    void setInitData(int nameType, int friendId);

    // 初始化组件
    void setWidgetUi();

    // 设置组件样式
    void setWidgetStyle();

    QLabel *m_nameTypeLab;    // 目录名称
    QLineEdit *m_nameEdit;    // 名字
    QLabel *m_tipsLab;        // 提示
    QPushButton *m_cancelBtn; // 取消按钮
    QPushButton *m_sureBtn;   // 确定按钮

    QVBoxLayout *m_mainLayout;     // 设置页面标题布局
    QVBoxLayout *m_vControlLayout; // 总体控件纵向布局
    QHBoxLayout *m_hControlLayout; // 总体控件横向布局
    QHBoxLayout *m_hButtonLayout;  // 总体控件横向布局

    enum NameType {
        LocalName = 0,
        FriendName,
    };

    // 按Esc关闭弹窗
    virtual void keyPressEvent(QKeyEvent *event);

private:
    TitleBar *m_titleBar; // 标题栏


    IniSettings *m_settings;

    bool m_isAuto;
    int m_nameType;
    int m_friendId;

    // 确认按钮槽函数
    void slotConfirmBtn();

    // 取消按钮槽函数
    void slotCancelBtn();

    //确认按钮激活
    void slotEnableSureBtn(QString text);

signals:
};

#endif // LOCALUPDATENAME_H
