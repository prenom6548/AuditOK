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

#ifndef TIMEZONECHOOSERDIALOG_H
#define TIMEZONECHOOSERDIALOG_H

#include <QtCore/QVariant>
#include <QApplication>
#include <QDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QListView>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QGSettings>
#include <QShowEvent>
#include "ksearchlineedit.h"
#include "kdialog.h"
#include "kpushbutton.h"

using namespace kdk;

class TimeZoneChooserDialog : public KDialog
{
    Q_OBJECT
public:
    TimeZoneChooserDialog(QWidget *parent = nullptr);
    ~TimeZoneChooserDialog();
    void setTitle(uint32_t type = 0);

protected:
    void showEvent(QShowEvent *e) override;

private:
    void initUi();
    void initModel();
    void initConnect();
    void styleSettingChangeSlot(const QString &key);
    QString getLocalTimezoneName(QString timezone, QString locale);
    QString readRile(const QString& filepath);
    void searchLineEditTextChangedSlot(const QString& search);
    void confirmButtonClickSlot();
    QString removeDigital(QString input);
    void selectTimezoneRow(int row);

Q_SIGNALS:
    void confirmed(QString zone, QString title);

private:
    QVBoxLayout* m_timezoneLayout = nullptr;
    QFrame* m_timezoneChooserframe = nullptr;
    QListView* m_timezoneListView = nullptr;
    QStandardItemModel* m_timezoneMode = nullptr;
    KPushButton *m_cancelButton = nullptr;
    KPushButton *m_confirmButton = nullptr;
    KSearchLineEdit* m_searchLineEdit = nullptr;
    QSortFilterProxyModel* m_proxyModel = nullptr;
    QGSettings *m_fontSetting = nullptr;
    int m_currentRow = 0;
    bool m_changeTimezone = true;
};

#endif // TIMEZONECHOOSERDIALOG_H
