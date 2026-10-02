/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#ifndef ALLOWLOCATION_H
#define ALLOWLOCATION_H

#include <QObject>
#include <QDialog>
#include <QLabel>
#include <QButtonGroup>
#include <QRadioButton>
#include <QPushButton>
#include <QDebug>
#include "controller/Interaction.h"
#include "global/viewvar.h"
class AllowLocation : public QDialog
{
    Q_OBJECT
    friend class TestAllowLocation;
public:
    explicit AllowLocation(QWidget *parent = nullptr);

private:
    QLabel *m_label;
    QLabel *m_text;
    QButtonGroup *m_allowLocation;
    QRadioButton *m_allow;
    QRadioButton *m_noAllow;
    QLabel *m_allowText;
    QLabel *m_noAllowText;
    QPushButton *m_cancel;
    QPushButton *m_certain;

    void initLayout();
    void initConnect();

private slots:
    void rejectLocate();
    void allowLocate();

signals:
    void allowLocationYes();
    void setMenuModuleAllowLocationYes();
};

#endif // ALLOWLOCATION_H
