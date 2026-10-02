/* iconedit.h
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 **/

#ifndef ICONEDIT_H
#define ICONEDIT_H

#include "iconbutton.h"
#include "statusbutton.h"
#include "lock-dialog/lockdialogmodel.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QGSettings>
#include <QPixmap>

enum EDITTYPE
{
    NORMALPWDEDIT,    //密码框
    CONFIDENTIALEDIT, //密保输入框
    RESETPWDEDIT,     //重置密码输入框
};

extern float scale;
class IconEdit : public QWidget
{
    Q_OBJECT
public:
    IconEdit(LockDialogModel *model, QWidget *parent = 0, int editType = NORMALPWDEDIT);

    void setIcon(const QIcon &icon, const QSize &size);
    void clear();
    void clearText();
    void setPrompt(const QString &);
    const QString text();
    void setType(QLineEdit::EchoMode type = QLineEdit::Password);
    void readOnly(bool enabled);
    void setLocked(bool lock);
    void setModeBtnVisible(bool visible);
    void startWaiting();
    void stopWaiting();
    void updateUI();
    void setFontSize(double fontSize);

protected:
    void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);

private:
    void updatePixmap();
    void resetFocus();
    void refreshTranslate();

Q_SIGNALS:
    void clicked(const QString &);
    void focusOut();
    void clickedPassword();
    void textChanged(const QString &);
public Q_SLOTS:
    void clicked_cb();
    void onCapsChanged(bool state);

private:
    LockDialogModel *m_modelLockDialog = nullptr;
    QLineEdit *m_edit = nullptr;
    QPushButton *m_iconButton = nullptr;
    QLabel *m_capsIcon = nullptr;
    IconButton *m_modeButton = nullptr;

    QString m_iconText; //文字作为图标
    QIcon m_icon;
    QTimer *m_timer = nullptr;
    QPixmap m_waitingPixmap;
    bool m_capsState = false;
    QSize m_sizeIconBtn;

    QString systemLang;
    int m_editType;
};

#endif // ICONEDIT_H
