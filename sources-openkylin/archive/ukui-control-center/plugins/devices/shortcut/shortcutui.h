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
#ifndef SHORTCUTUI_H
#define SHORTCUTUI_H

#include "settinggroup.h"
#include "addbutton.h"
#include "doubleclicklineedit.h"
#include "clickfixlabel.h"
#include "addshortcutdialog.h"

#include <QHBoxLayout>
#include <QWidget>
#include <QMenu>
#include <QMessageBox>
#include <QGSettings>
#include <klabel.h>
#include "titlelabel.h"

using namespace kdk;

class ShortcutUi : public QWidget
{
    Q_OBJECT
public:
    ShortcutUi();
    ~ShortcutUi();
    void addSystemShortcut(const QString &name, const QString &shortcut, const QString &mediaKey, int systemIndex);
    void addWindowShortcut(const QString &name, const QString &shortcut, const QString &mediaKey, int windowIndex);
    void addCustomShortcut(KeyEntry keyEntry, int customIndex);
    void clearCustomShorcutFrame();
    void setWindowValid(bool isValid);
    void setWindowHide();
    void addDefaultBtn();
    void loadDefaultShortcut();
    AddButton* getAddButton();
    TitleLabel* getCustomTitle();
    QList<QFrame*> getCustomFrameList();
    void initShortEntry();
    void initSystem();
    void initWindow();
    void initCustom();
    // 按界面顺序重建 Tab 链（系统→窗口→恢复默认→自定义→添加）
    void fixShortcutTabOrder();
    // 「自定义快捷键」标题 setBuddy 到首个名称编辑框
    void bindCustomTitleToFirstNameEdit();

private:
    void initUi();
    void initSystemUi();
    void initWindowUi();
    void initCustomUi();
    void updateGeneralShort(DoubleClickShortCut *shortCutEdit, QString key, QString value, bool isConflict);
    void showEvent(QShowEvent *event) override;

    QString getShowShortcutString(QString src);
    QString keyToLib(QString key);
    QString conflictTip(const QString &key, const QString &value);
    // 加载时若与系统/自定义冲突，将窗口快捷键（显示或隐藏桌面）置为无
    void clearConflictingWindowShortcutsOnLoad();
    void defaultButtonSlot();
    void setWindowShortcutToNull();
    void defaultShortcut();
    void setCustomShortcutToNull();
    QString keyToSequence(QString key);
    void labelDoubleClickSlot(ClickFixLabel* fixLabel,
                              DoubleClickLineEdit* lineEdit);
    void labelDoubleClickSlot(ClickFixLabel* fixLabel,
                              DoubleClickShortCut *lineEdit);
    void lineEditFocusOutSlot(DoubleClickShortCut* lineEdit,
                              ClickFixLabel *fixLabel);
    void lineEditFocusOutSlot(DoubleClickLineEdit* lineEdit,
                              ClickFixLabel *fixLabel);
    void lineEditChangeSlot(DoubleClickShortCut* lineEdit,
                            ClickFixLabel *fixLabel,
                            QString key);
    void customLineEditStrChangeSlot(DoubleClickLineEdit* lineEdit,
                                     ClickFixLabel *fixLabel,
                                     KeyEntry* keyEntry);
    void customLineEditChangeSlot(DoubleClickShortCut* lineEdit,
                                  ClickFixLabel *fixLabel,
                                  KeyEntry* keyEntry);
    void lineEditConflictSlot(DoubleClickShortCut* lineEdit,
                              ClickFixLabel *fixLabel,
                              QString key);
    void customLineEditConflictSlot(DoubleClickShortCut* lineEdit,
                                    ClickFixLabel *fixLabel,
                                    KeyEntry* keyEntry);
    void lineEditOccupySlot(DoubleClickShortCut* lineEdit,
                            ClickFixLabel* fixLabel);
    void customAddDialogSlot(AddShortcutDialog *addDialog,
                             DoubleClickLineEdit *lineEdit,
                             DoubleClickShortCut *bingdLineEdit,
                             ClickFixLabel* fixLabeL,
                             ClickFixLabel* bindFixLabel,
                             KeyEntry* keyEntry,
                             QString name, QString exec, QString key);
    void updateDefaultButtonStatus(bool status = false);
    QMap<QString, QString> MergerOfTheSamekind(QMap<QString, QString> desktopMap);
    void createNewShortcut(QString path, QString name, QString exec, QString key, bool buildFlag, bool convertFlag = true, DoubleClickShortCut *shortCut = nullptr);
    bool conflictWithGlobalShortcuts(const QKeySequence &keySequence);
    bool conflictWithStandardShortcuts(const QKeySequence &seq);
    bool conflictWithSystemShortcuts(const QKeySequence &seq);
    bool conflictWithWindowShortcuts(const QKeySequence &seq);
    bool conflictWithCustomShortcuts(const QKeySequence &seq);
    void conflictDetectSlot(ShortcutLine* shortcutLineEdit, const QKeySequence &seq);

private:
    QVBoxLayout *uiLayout     = nullptr;
    QVBoxLayout *systemLayout = nullptr;
    QVBoxLayout *customLayout = nullptr;
    QVBoxLayout *m_windowLayout = nullptr;
    QHBoxLayout *m_defaultLayout = nullptr;

private:
    TitleLabel   *systemTitle         = nullptr;
    SettingGroup *systemShortcutGroup = nullptr;

    TitleLabel   *m_windowTitle         = nullptr;
    SettingGroup *m_windowShortcutGroup = nullptr;
    QPushButton* m_defaultButton = nullptr;

private:
    TitleLabel   *customTitle         = nullptr;
    SettingGroup *customShortcutGroup = nullptr;
    AddButton    *addButton           = nullptr;

    QList<DoubleClickShortCut *> mGeneralLineEdit;
    QList<ClickFixLabel *>       mGeneralName;
    QList<DoubleClickShortCut *> mCustomLineEdit;
    QList<ClickFixLabel *>       mCustomName;
    QList<DoubleClickShortCut *> m_windowLineEdit;
    QList<ClickFixLabel *>       m_windowName;

    QMap<QString, QString>       mAllKeyI18nName;
    bool m_windowShortcutValid = true;
    QDBusInterface* m_shortcutInterface = nullptr;
    QList<QString> m_defaultShortcutList;
    QGSettings* m_ukccSetting = nullptr;
    QList<QFrame*> m_customFrameList;
    QString m_conflictValue = "";
    QString m_conflictKey = "";

Q_SIGNALS:
    void deleteShortcut(const QString &path);
    void updateSystemShortcut(const QString &key, const QString &value);
    void updateCustomShortcut(const QByteArray &path, const QString &key, const QString &value);
};

#endif // SHORTCUTUI_H
