/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2026 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TIPDIALOG_H
#define TIPDIALOG_H

#include <QMessageBox>

class QAbstractButton;
class QCloseEvent;

/** 基于 QMessageBox 的提示弹窗（关闭即销毁，下次显示时再创建） */
class TipDialog : public QMessageBox
{
    Q_OBJECT

public:
    explicit TipDialog(QWidget *parent = nullptr);

    /** 通用展示：正文、操作按钮文案、控制中心 -m 模块名 */
    void showTip(const QString &message,
                 const QString &actionButtonText,
                 const QString &controlCenterModule = QString());

    void showMuted();
    void showUnavailable();
    void showNoVoiceModel();

private Q_SLOTS:
    void onButtonClicked(QAbstractButton *button);

private:
    void updatePosition();
    void openControlCenter(const QString &module);
    void showCentered(const QString &message,
                      const QString &actionButtonText,
                      const QString &controlCenterModule);
    void closeEvent(QCloseEvent *event) override;

    QAbstractButton *m_cancelButton = nullptr;
    QAbstractButton *m_actionButton = nullptr;
    QString m_controlCenterModule;
};

#endif // TIPDIALOG_H
