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
#ifndef ADDLANGUAGEDIALOG_H
#define ADDLANGUAGEDIALOG_H

#include <QFrame>
#include <QStandardItemModel>
#include <QItemSelection>
#include <QSortFilterProxyModel>
#include <QListView>
#include <QVBoxLayout>
#include "kdialog.h"
#include "ksearchlineedit.h"
#include "kpushbutton.h"

using namespace kdk;

class AddLanguageDialog : public KDialog
{
    Q_OBJECT

public:
    explicit AddLanguageDialog(QWidget *parent = nullptr);
    ~AddLanguageDialog();
    void initCompleter();
    void addRow(const QString &languageName, bool isAdded = false);

public:
    static const QList<QPair<QString, QPair<QString, QString>>> supportedLanguageList;
    void initDict();
    QString chineseToPinyin(const QString &words);
    QString removeDigital(QString input);
    void setConfirmEnable(bool enabled);
    void setRowFlags(const QString &languageName, bool flag);

protected:
    void showEvent(QShowEvent *e) override;

Q_SIGNALS:
    void addShowLanguage(QStringList selectedLanguageList);

private slots:
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    // 初始化添加语言弹窗界面
    void initUi();
    // 初始化添加语言弹窗信号连接
    void initConnect();
    // 默认选中首个可选语言项（与添加开机启动窗口一致，便于 Tab 进列表时读屏读到名称）
    void selectFirstEnabledLanguage();

    QVBoxLayout *m_mainLayout = nullptr;
    QFrame *m_searchFrame = nullptr;
    QListView *m_languageListView = nullptr;
    KPushButton *m_cancelButton = nullptr;
    KPushButton *m_confirmButton = nullptr;
    QStandardItemModel *mode = nullptr;
    kdk::KSearchLineEdit *mSearchLineEdit = nullptr;
    QHash<uint, QString> dict = {};
    const QString kDictFile = ":/dpinyin.dict";
    QSortFilterProxyModel *m_proxyModel = nullptr;
};

#endif // ADDLANGUAGEDIALOG_H
