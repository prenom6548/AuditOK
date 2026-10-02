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

#include "netcheckwidget.h"
#include <QVBoxLayout>
#include "gsettings.hpp"
using namespace kdk::kabase;

NetCheckWidget::NetCheckWidget(QWidget *parent) : QWidget(parent)
{
    stacked_widget = new QStackedWidget(this);
    mHomeWidget = new NetCheckHomePage(stacked_widget);
    mMainWidget = new MainWindow(stacked_widget);
    stacked_widget->addWidget(mHomeWidget);
    stacked_widget->addWidget(mMainWidget);
    connect(mHomeWidget, &NetCheckHomePage::sigChangeStackWidgetHome, this,
            &NetCheckWidget::slotChangeCurrentStackWidget);
    connect(mMainWidget, &MainWindow::sigChangeStackWidgetMain, this, &NetCheckWidget::slotChangeCurrentStackWidget);
    connect(mHomeWidget, &NetCheckHomePage::sigStartCheck, mMainWidget, &MainWindow::startCheckProcess);
    connect(mHomeWidget, &NetCheckHomePage::sigUpdateMainConfig, mMainWidget, &MainWindow::slotUpdateConfigFile);

    QVBoxLayout *layout1 = new QVBoxLayout;
    layout1->addWidget(stacked_widget);
    layout1->setSpacing(0);
    layout1->setMargin(0);
    layout1->setContentsMargins(0, 0, 0, 0);

    this->setLayout(layout1);
    stacked_widget->setCurrentIndex(0);
    initThemeGetting();
}
void NetCheckWidget::slotChangeCurrentStackWidget(int page)
{
    stacked_widget->setCurrentIndex(page);
}
void NetCheckWidget::initThemeGetting()
{
    QString themeStr = kdk::kabase::Gsettings::getSystemTheme().toString();
    if (themeStr == QString("ukui-dark") || themeStr == QString("ukui-black")) {
        themeColor = 1;
    } else {
        themeColor = 0;
    }
    changeThemeColor(themeColor);
    connect(Gsettings::getPoint(), &Gsettings::systemThemeChange, this, [this]() {
        QString themeStr = kdk::kabase::Gsettings::getSystemTheme().toString();
        qDebug() << "ToolBoxWidget::initThemeGetting theme change :" << themeStr;
        if (themeStr == QString("ukui-dark") || themeStr == QString("ukui-black")) {
            themeColor = 1;
        } else {
            themeColor = 0;
        }
        changeThemeColor(themeColor);
    });

    //字体的
    mFontSize = Gsettings::getSystemFontSize().toInt();
    connect(Gsettings::getPoint(), &Gsettings::systemFontSizeChange, this, [this]() {
        mFontSize = Gsettings::getSystemFontSize().toInt();
        changeSystemFontSize(mFontSize);
    });
}

void NetCheckWidget::changeThemeColor(int color)
{
    qDebug() << "CleanerWidget::changeThemeColor" << color;
    mHomeWidget->changeThemeColor(color);
    mMainWidget->changeThemeColor(color);
}

void NetCheckWidget::changeSystemFontSize(int size)
{
    mHomeWidget->changeSystemSize(size);
    mMainWidget->changeSystemSize(size);
}
