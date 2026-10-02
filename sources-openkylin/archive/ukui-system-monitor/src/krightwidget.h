/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
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
#ifndef KRIGHTWIDGET_H
#define KRIGHTWIDGET_H

#include <QWidget>
#include <QToolButton>
#include <QMenu>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QLineEdit>
#include <QTimer>
#include <QGSettings>

#include "kaboutdialog.h"
#include "ksearchlineedit.h"
#include "kmenubutton.h"
using namespace kdk;

class KRightWidget : public QWidget
{
    Q_OBJECT
public:
    explicit KRightWidget(QWidget *parent = nullptr);
    virtual ~KRightWidget();

    void initUI();
    void addPanel(QWidget* pWidget, QString strName, QString strIcon, int nPanelIdx = -1);
    int currentIndex();

private:
    QStackedWidget *m_stackedWidget = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;
    QGSettings *m_styleSettings = nullptr;
    QGSettings *m_pSystemMonitorGSetting = nullptr;

public:
    static QString searchText;

public slots:
    void onSwichPanel(int nIndex);

signals:

};

#endif // KRIGHTWIDGET_H
