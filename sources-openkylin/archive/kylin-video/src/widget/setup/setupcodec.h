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
#ifndef SETUPCODEC_H
#define SETUPCODEC_H

#include "../themewidget.h"

class QLabel;
class QLineEdit;
class QComboBox;

class SetupCodec : public ThemeWidget
{
    Q_OBJECT

public:
    explicit SetupCodec(QWidget *parent = nullptr);
    ~SetupCodec();

    void setBlackTheme();
    void setDefaultTheme();
    void setWidgetFont(QString family, int size);

private:
    QLabel *m_label1;
    QComboBox *m_comboBoxOutput;
    QLabel *m_label2;
    QComboBox *m_comboBoxDecoder;
    QLabel *m_label3;
    QLineEdit *m_threadEdit;

    void initLayout();
    void initData();
    void initConnect();
};

#endif // SETUPCODEC_H
