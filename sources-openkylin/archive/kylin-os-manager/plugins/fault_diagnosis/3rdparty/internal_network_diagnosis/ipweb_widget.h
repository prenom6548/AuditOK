/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef IPWEB_WIDGET_H
#define IPWEB_WIDGET_H

#include <QWidget>

enum class IPWebWidgetType : int {
    IP_WIDGET = 0,
    WEB_WIDGET,
};

struct IpWeb {
    enum Level {
        kNone,
        kSystem,
        kUser
    };

    IpWeb()
        : text(""),
          level(Level::kNone)
    {

    }

    QString text;
    Level level;
};

namespace Ui {

class IPWebWidget;

}

class IPWebWidget : public QWidget {
    Q_OBJECT

public:
    explicit IPWebWidget(bool isFirst, IPWebWidgetType type, QWidget *parent = nullptr);
    ~IPWebWidget();

    void Disable(bool first_item);
    void SetIpWeb(const IpWeb &ip_web);
    IpWeb::Level GetLevel();
    void setLineText(QString str);
    QString getLineText();
    void setAddBtnEnable(bool);
    bool getFormatStatus() {return formatStatus;}

signals:
    void delPressed();
    void addPressed();
    void userSettingsChanged();

private slots:
    void slotTextChanged(QString text);

private:
    Ui::IPWebWidget *ui;
    bool m_isFirst = false;
    bool formatStatus = true;
    IPWebWidgetType m_curType;
    IPWebWidget *m_nextWidget = nullptr;
    IpWeb ip_web_;
};

#endif
