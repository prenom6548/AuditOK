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
#ifndef TROUBLESHOOTING_INFO_WIDGET_H
#define TROUBLESHOOTING_INFO_WIDGET_H

#include <QWidget>

namespace Ui {
class TroubleshootingInfoWidget;
}

class TroubleshootingInfoWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TroubleshootingInfoWidget(QWidget *parent = nullptr);
    ~TroubleshootingInfoWidget();
    enum STATUS{
        SUCCESS=0,
        FAIL,
        PENDING,
    };
    void setMainLabel(const QString &str);
    void setInformativeLabel(const QString &str);
    void setStatus(STATUS status);

private:
    Ui::TroubleshootingInfoWidget *ui;
};

#endif // TROUBLESHOOTING_INFO_WIDGET_H
