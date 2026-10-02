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
#ifndef CHECKBOX_H
#define CHECKBOX_H

#include <QCheckBox>

class CheckBox : public QCheckBox
{
    Q_OBJECT
public:
    CheckBox(QWidget *parent = nullptr);
    ~CheckBox();

    void setText(const QString &text);
    void setFont(const QFont &font);

private:
    QString m_tip;

    void resetText();
    void resizeEvent(QResizeEvent *e) override;
};

#endif // CHECKBOX_H
