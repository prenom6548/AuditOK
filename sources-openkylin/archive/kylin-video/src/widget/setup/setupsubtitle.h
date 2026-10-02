/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SETUPSUBTITLE_H
#define SETUPSUBTITLE_H

#include "../themewidget.h"

class CheckBox;

class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;
class QHBoxLayout;

class SetupSubtitle : public ThemeWidget
{
    Q_OBJECT

public:
    explicit SetupSubtitle(QWidget *parent = nullptr);
    ~SetupSubtitle();

    void initData();
    void setBlackTheme();
    void setDefaultTheme();
    void setWidgetFont(QString family, int size);

private:
    QLabel *m_title1Label;
    CheckBox *m_sameName;
    CheckBox *m_allName;

    QHBoxLayout *m_hboxLayout1;
    QLabel *m_label1;
    QLineEdit *m_subPathEdit;
    QPushButton *m_browserButton;

    QLabel *m_title2Label;

    QHBoxLayout *m_hboxLayout2;
    QLabel *m_label2;
    QComboBox *m_familyComboBox;

    QHBoxLayout *m_hboxLayout3;
    QLabel *m_label3;
    QComboBox *m_sizeComboBox;

    QString subPath;

    void initLayout();
    void initConnect();

    void paintEvent(QPaintEvent *e);
};

#endif // SETUPSUBTITLE_H
