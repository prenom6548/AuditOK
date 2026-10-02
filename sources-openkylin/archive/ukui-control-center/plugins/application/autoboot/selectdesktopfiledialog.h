/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2025 KylinSoft  Co., Ltd.
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

#ifndef SELECTDESKTOPFILEDIALOG_H
#define SELECTDESKTOPFILEDIALOG_H

#include <QFrame>
#include <QVBoxLayout>
#include <QListView>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include "kdialog.h"
#include "klabel.h"
#include "kpushbutton.h"
#include "kborderlessbutton.h"

using namespace kdk;

class SelectDesktopFileDialog : public KDialog
{
    Q_OBJECT
public:
    SelectDesktopFileDialog(QWidget *parent = nullptr);
    ~SelectDesktopFileDialog();

private:
    void initUi();
    void initModel();
    void initConnect();
    void confirmButtonClickSlot();
    void updateCurrentRow(const QModelIndex &index);

Q_SIGNALS:
    void confirmed(QString selectDesktopPath);
    void addOtherApp();

private:
    QVBoxLayout* m_selectDesktopFileLayout = nullptr;
    QFrame* m_selectDesktopFileframe = nullptr;
    QListView* m_appListView = nullptr;
    QStandardItemModel* m_selectDesktopFileMode = nullptr;
    KPushButton *m_cancelButton = nullptr;
    KPushButton *m_confirmButton = nullptr;
    KLabel* m_titleLable = nullptr;
    KBorderlessButton* m_toOtherDesktopButton = nullptr;
    QSortFilterProxyModel* m_proxyModel = nullptr;
    int m_currentRow = -1;
    bool m_changeTimezone = true;
    QList<QString> m_desktopPathList;
};

#endif // SELECTDESKTOPFILEDIALOG_H
