/*
 * Copyright (C) 2020 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Kobe Lee    xiangli@ubuntukylin.com/kobe24_lixiang@126.com
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SELECTWIDGET_H
#define SELECTWIDGET_H

#include <QVBoxLayout>
#include <QMouseEvent>
#include <QGSettings>

#include <kysdk/applications/kdialog.h>
#include <kysdk/applications/klabel.h>
#include <kysdk/applications/kslider.h>
#include <kysdk/applications/kpushbutton.h>
using namespace kdk;

class QLabel;
class QSlider;
class QPushButton;

class ReniceDialog : public KDialog
{
    Q_OBJECT
public:
    ReniceDialog(const QString &procName = "", const QString &procId = "", QWidget *parent = 0);
    ~ReniceDialog();

    void loadData(int nice);

signals:
    void resetReniceValue(int value);

private:
    QHBoxLayout *h_layout = nullptr;
    QHBoxLayout *btn_layout = nullptr;
    KLabel *m_titleLabel = nullptr;
    KSlider *m_slider = nullptr;
    KLabel *m_valueLabel = nullptr;
    KLabel *m_valueStrLabel = nullptr;
    KLabel *m_tipLabel = nullptr;
    QString m_strProcName;
    QString m_strProcId;

    KPushButton *m_cancelbtn = nullptr;
    KPushButton *m_changeBtn = nullptr;
};

#endif // SELECTWIDGET_H
