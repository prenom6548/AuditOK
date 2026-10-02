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
#ifndef SETUPSCREENSHOT_H
#define SETUPSCREENSHOT_H

#include "../themewidget.h"
#include <QDir>

class CheckBox;

class QLabel;
class QLineEdit;
class QComboBox;
class QHBoxLayout;
class QPushButton;
class QRadioButton;

#define DEFAULT_SAVE_PATH QDir::homePath().append(tr("Pictures"))

class SetupScreenshot : public ThemeWidget
{
    Q_OBJECT

public:
    explicit SetupScreenshot(QWidget *parent = nullptr);
    ~SetupScreenshot();

    void initData();
    void setBlackTheme();
    void setDefaultTheme();
    void setWidgetFont(QString family, int size);

private:
    QRadioButton *m_save2clipRadioButton;
    QRadioButton *m_save2fileRadioButton;

    QHBoxLayout *m_hboxLayout1;
    QLabel *m_label1;
    QLineEdit *m_lineEdit;
    QPushButton *m_browserButton;

    QHBoxLayout *m_hboxLayout2;
    QLabel *m_label2;
    QComboBox *m_comboBox;

    CheckBox *m_checkBox;

    int m_format_index;

    void initLayout();
    void initConnect();

    void paintEvent(QPaintEvent *e);
};

#endif // SETUPSCREENSHOT_H
