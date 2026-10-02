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

#ifndef LOGO_ICON_TITLE_BAR_H
#define LOGO_ICON_TITLE_BAR_H

#include <QObject>
#include <QLayout>
#include <QPushButton>
#include <QLabel>

class LogoIconTitleBar : public QWidget
{
    Q_OBJECT
public:
    LogoIconTitleBar(QWidget *parent = nullptr);
    void setTitleName(QString titleName);
    void setFontSize(QFont ft);

private:
    QPushButton *m_titleIcon;
    QLabel *m_titleLabel;

    QHBoxLayout *m_HLayoutLogoName;
};

#endif // LOGO_ICON_TITLE_BAR_H
