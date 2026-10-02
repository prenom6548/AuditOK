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
#ifndef CONFIG_IP_ADDRESS_DIALOG_H
#define CONFIG_IP_ADDRESS_DIALOG_H

#include <QCoreApplication>
#include "custom_dialog.h"
#include <QString>
#include <QGSettings>
namespace Ui {
class ConfigIPAddressDialog;
}

class ConfigIPAddressDialog : public CustomDialog
{
    Q_OBJECT

public:
    explicit ConfigIPAddressDialog(QWidget *parent = nullptr);
    ~ConfigIPAddressDialog();
    bool getConfigStatus() const;
private slots:
    void onApplyBtnClicked();
    void onCancelBtnClicked();
private:
    QString m_oldIpAddress = QString();
    bool m_cupsDebug = false;

    void loadSettings();
    void saveSettings();
    void onSettingsChanged();
    Ui::ConfigIPAddressDialog *m_ui;
    bool m_isConfigStatusOK;
};

#endif // CONFIG_IP_ADDRESS_DIALOG_H
