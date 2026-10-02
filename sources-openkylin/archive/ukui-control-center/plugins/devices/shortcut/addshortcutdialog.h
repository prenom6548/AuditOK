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
#ifndef ADDSHORTCUTDIALOG_H
#define ADDSHORTCUTDIALOG_H

#include <QFileDialog>
#include <QPainter>
#include <QPainterPath>
#include <QRegExpValidator>
#include <KActionCollection>
#include <KGlobalAccel>
#include <QDebug>
#include "shortcutline.h"
#include <QLabel>
#include <QVBoxLayout>
#include <klabel.h>
#include <kpushbutton.h>
#include <kdialog.h>
using namespace kdk;

class AddShortcutDialog : public KDialog
{
    Q_OBJECT

public:
    explicit AddShortcutDialog(QList<KeyEntry> generalEntries,
                               QList<KeyEntry> windowEntries,
                               QList<KeyEntry> customEntries,
                               QWidget *parent = nullptr);
    ~AddShortcutDialog();

public:
    void initUi();
    void initSetup();
    void slotsSetup();
    void setTitleText(QString text);
    void openProgramFileDialog();

    void setUpdateEnv(QString path, QString name, QString exec);

    void refreshCertainChecked(int triggerFlag);   //1:输入了程序，2:输入了名字， 3:输入了快捷键
    void limitInput();
    bool conflictWithStandardShortcuts(const QKeySequence &seq);
    bool conflictWithGlobalShortcuts(const QKeySequence &seq);
    bool conflictWithSystemShortcuts(const QKeySequence &seq);
    bool conflictWithCustomShortcuts(const QKeySequence &seq);
    bool isKeyAvailable(const QKeySequence &seq);
    QString keyToLib(QString key);
    QString keyToSequence(QString key);

    void setExecText(const QString &text);
    void setNameText(const QString &text);
    void setKeyText(const QString  &text);
    void setSourceEnable(bool enabled);
    void setKeyIsAvailable(const int key);
    void setIcon(const QString &iconname);

protected:
    bool eventFilter(QObject *o, QEvent *e);
    void showEvent(QShowEvent *e) override;
    bool focusNextPrevChild(bool next) override;
    bool event(QEvent *e) override;

private:
    void shortCutAvailableSlot(const int &flag);
    // Tab：选择 → 名称 → 按键 → 取消 → 确定
    void fixAddDialogTabOrder();
    QList<QWidget *> addDialogTabOrderList() const;
    void focusOpenOrName();

    // show/激活后钉「选择」为首焦；用户 Tab 后清掉
    bool m_pendingInitialOpenFocus = false;
    QString gsPath;
    QString selectedfile;
    QList<KeyEntry> m_systemEntry;
    QList<KeyEntry> m_windowEntry;
    QList<KeyEntry> m_customEntry;
    ShortcutLine* m_shortcutLine = nullptr;
    QString      editName;
    QKeySequence editSeq;
    int keyIsAvailable;         //快捷键有冲突/不可用/正常三种情况，1：冲突，2：不可用，3：正常
    bool nameIsAvailable;
    bool execIsAvailable;
    QLabel *execIcon = nullptr;
    QString mExec = "";

    QVBoxLayout *m_verticalLayout3;
    QVBoxLayout *m_verticalLayout2;
    QVBoxLayout *m_verticalLayout1;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *m_execHorizontalLayout;
    KLabel *m_execLabel;
    QLineEdit *m_execLineEdit;
    KPushButton *m_openBtn;
    QSpacerItem *verticalSpacer;
    QHBoxLayout *m_nameHorizontalLayout;
    KLabel *m_nameLabel;
    QLineEdit *m_nameLineEdit;
    QHBoxLayout *m_infoHorizontalLayout;
    KLabel *m_infoLabel;
    QHBoxLayout *m_keyHorizontalLayout;
    KLabel *m_keyLabel;
    QHBoxLayout *m_info2HorizontalLayout;
    KLabel *m_info2Label;
    QHBoxLayout *m_bottomHorizontalLayout;
    QSpacerItem *horizontalSpacer_2;
    KPushButton *m_cancelBtn;
    KPushButton *m_certainBtn;
    QString m_oriShortcutLineText;

Q_SIGNALS:
    void shortcutInfoSignal(QString path, QString name, QString exec, QString key);
};

#endif // ADDSHORTCUTDIALOG_H
