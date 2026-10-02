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
#ifndef VERIFYKYIDWIDGET_H
#define VERIFYKYIDWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QTimer>
#include <QDateTime>
#include "lock-dialog/lockdialogmodel.h"

class IconEdit;
class KyIconButton;
class KLabel;

class VerifyKyidWidget : public QWidget
{
    Q_OBJECT
public:
    explicit VerifyKyidWidget(LockDialogModel *model, QWidget *parent = nullptr);
    QString getKylinIdEditText();
    QString getPwdEditText();
    QString getCaptchaEditText();
    void clearText();
    void onCaptchaBtnclicked(bool success);
private:
    void initUi();

Q_SIGNALS:
    void textChanged(const QString &text);
    void captchaBtnclicked();

private:
    LockDialogModel *m_modelLockDialog = nullptr;
    IconEdit *m_pKylinIdEdit = nullptr;
    IconEdit *m_pPasswordEdit = nullptr;
    IconEdit *m_pCaptchaEdit = nullptr;
    KyIconButton *m_pGetCaptchaBtn = nullptr;
    KLabel *m_pMessageLabel = nullptr;
    QTimer *m_pCountdown = nullptr;
    QDateTime  m_lastDateTime;
};

#endif // VERIFYKYIDWIDGET_H
