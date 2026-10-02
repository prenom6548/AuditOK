/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Yuan ShanShan   yuanshanshan@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "gpswindow.h"

void GPSWINDOW::pullUpWindow(const QString &msg)
{
    Q_UNUSED(msg)
    this->hide();
    this->show();
    showNormal();
}

GPSWINDOW::GPSWINDOW(QWidget *parent) : QDialog(parent)
{
    this->setFixedSize(720, 480);
    setWindowFlags(Qt ::Window | Qt ::WindowMinimizeButtonHint | Qt ::WindowCloseButtonHint);
    setWindowTitle(tr("virtualgps"));
    //搜索布局
    m_searchView = new SearchView;
    QGridLayout *LeftLayout = new QGridLayout;
    LeftLayout->addWidget(m_searchView);

    //当前位置显示布局
    QHBoxLayout *ButtomLayout = new QHBoxLayout;
    m_currentlocationView = new CurrentLocationView;
    ButtomLayout->addWidget(m_currentlocationView);

    //信号
    connect(m_searchView, &SearchView::sigcurrentShow, this, &GPSWINDOW::currentShow);
    connect(m_searchView, &SearchView::sigupdateCurrentShow, this, &GPSWINDOW::recoverCurrentShow);
    connect(m_currentlocationView, &CurrentLocationView::restoreMapToDefault, this, &GPSWINDOW::restoreMapToDefault);

    //整体布局调整
    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setMargin(15);
    mainLayout->setSpacing(10);
    mainLayout->setColumnStretch(0, 1);
    mainLayout->setColumnStretch(1, 2);
    mainLayout->addLayout(LeftLayout, 0, 0, 1, 1);
    mainLayout->addLayout(ButtomLayout, 1, 0, 1, 1);

    //将布局加入widget
    QWidget *widget = new QWidget(this);
    widget->setLayout(mainLayout);
}

void GPSWINDOW::currentShow()
{
    int i = m_searchView->m_locationlistWidget->num;
    QString data = LocationGet::getInstance()->m_locationArray[i].name;
    data.append("\n");
    data.append(LocationGet::getInstance()->m_locationArray[i].cityName);
    data.append(LocationGet::getInstance()->m_locationArray[i].adName);
    data.append(LocationGet::getInstance()->m_locationArray[i].address);
    data.append("（经纬度：");
    data.append(LocationGet::getInstance()->m_locationArray[i].location);
    data.append("）");
    m_searchView->m_locationlistWidget->hide();

    //更新地图当前显示
    QString tmp = LocationGet::getInstance()->m_locationArray[i].location;
    QStringList list1 = tmp.split(",");
    QString longitude = list1[0];
    QString latitude = list1[1];
    QString cmd = QString("addMarker(%0,%1)").arg(longitude).arg(latitude);
    m_searchView->m_view->page()->runJavaScript(cmd);
    //更新位置当前显示
    m_currentlocationView->m_label->setText(data);
    m_currentlocationView->m_label->show();
    m_currentlocationView->m_saveBtn->show();
    m_currentlocationView->m_saveBtn->setText(tr("uselocation"));

}

bool GPSWINDOW::event(QEvent *event)
{
    //点击别处，使搜索框失去焦点
    if (event->type() == QEvent::MouseButtonPress) {
        m_searchView->m_locationlistWidget->hide();
        m_searchView->m_searchBox->clearFocus();
    }
    return QWidget::event(event);
}


GPSWINDOW::~GPSWINDOW()
{
    if (m_searchView != nullptr) {
        delete m_searchView;
        m_searchView = nullptr;
    }
    if (m_currentlocationView != nullptr) {
        delete m_currentlocationView;
        m_currentlocationView = nullptr;
    }
}

void GPSWINDOW::recoverCurrentShow(){
    CurrentLocation currentlocation;
    QString data ="";
    if (currentlocation.getLocation()) {
        data = currentlocation.m_currentlocation.province;
        data.append("\n");
        data.append(currentlocation.m_currentlocation.cityName);
        data.append(currentlocation.m_currentlocation.adName);
        QString pos = tr("（经纬度：") + currentlocation.m_currentlocation.location + "）";
        data.append(pos);
        QString tmp = currentlocation.m_currentlocation.location;
        QStringList list1 = tmp.split(",");
        QString longitude = list1[0];
        QString latitude = list1[1];
        QString cmd = QString("addMarker(%0,%1)").arg(longitude).arg(latitude);
        m_searchView->m_view->page()->runJavaScript(cmd);
        m_currentlocationView->m_label->setText(data);
        m_currentlocationView->m_label->show();
        m_currentlocationView->m_saveBtn->show();
        m_currentlocationView->m_saveBtn->setText(tr("uselocation"));
    }else {
        m_currentlocationView->m_label->setText(tr("Unable to get the current location of the user"));
        m_currentlocationView->m_label->show();
        m_currentlocationView->m_saveBtn->hide();
    }
}

void GPSWINDOW::restoreMapToDefault(){
    m_searchView->m_view->page()->load(QUrl("qrc:/res/map.html"));
}
