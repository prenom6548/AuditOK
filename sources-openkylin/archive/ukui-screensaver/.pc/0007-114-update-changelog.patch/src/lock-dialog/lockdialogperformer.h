/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
#ifndef LOCKDIALOGWORKER_H
#define LOCKDIALOGWORKER_H

#include <QObject>

class LockDialogModel;
class BackendDbusHelper;

/**
 * @brief 执行者类（承接程序内外的接口类）
 *
 */
class LockDialogPerformer : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造
     *
     * @param model 模型
     * @param parent 父指针
     */
    explicit LockDialogPerformer(LockDialogModel *model, QObject *parent = nullptr);

private:
    /**
     * @brief 初始化连接信号槽
     *
     */
    void initConnections();
    /**
     * @brief 初始化数据
     *
     */
    void initData();

private:
    LockDialogModel *m_modelLockDialog = nullptr; /**< 锁屏数据模型 */
    BackendDbusHelper *m_bdHelper = nullptr;      /**< 后端服务客户端类 */
};

#endif // LOCKDIALOGWORKER_H
