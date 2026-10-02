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

#include "currentlocationview.h"
using namespace kmre_dataformat;

namespace kmre_gps
{
CurrentLocationView::CurrentLocationView(QWidget *parent) : QWidget(parent)
{
    setFixedSize(670, 100);
    initWidget();
}

void CurrentLocationView::initWidget()
{
    QFont font;
    font.setPointSize(10);

    m_tip = new QLabel(this);
    m_tip->setFont(font);
    m_tip->resize(70, 20);
    m_tip->move(5, 10);
    m_tip->setText(tr("currentlocation:\n"));

    m_label = new QLabel(this);
    m_label->setFont(font);
    m_label->resize(500, 60);
    m_label->move(5, 27);
    currentLocationShow();
    m_label->setText(DEFAULTLOCATION);

    m_saveBtn = new QPushButton(this);
    m_saveBtn->setText(tr("cancellocation"));
    m_saveBtn->move(550, 40);
    if (m_label->text() == DEFAULTLOCATION) {
        m_saveBtn->hide();
    }
    connect(m_saveBtn, &QPushButton::clicked, this, &CurrentLocationView::connectServer);
}

void CurrentLocationView::connectServer()
{
    QString msg;
    QGSettings *m_settings = new QGSettings(GPS_GSETTINGS_SERVICENAME.toLocal8Bit());
    if (m_saveBtn->text() == tr("uselocation") && m_label->text() != DEFAULTLOCATION) {
        m_data = m_label->text();
        QStringList list = m_data.split("：");
        msg = list[1];
        msg.chop(1);
        m_saveBtn->setText(tr("cancellocation"));
        m_settings->set("current-show", m_data); //写入配置文件
    } else if (m_saveBtn->text() == tr("cancellocation")){
        //点击取消定位时，当前地图恢复至默认状态
        Q_EMIT restoreMapToDefault();
        msg = DEFAULTGPSDATA;
        //点击取消定位时，当前文字恢复至默认位置北京并隐藏按钮
        m_label->setText(DEFAULTLOCATION);
        m_saveBtn->hide();
        m_settings->set("current-show", m_label->text());
    }
    QString gpsdata = FormatConversion::dataConversion(msg);
    if (gpsdata != "") {
        DbusClient::sendGpsData(gpsdata);
    }
    if (m_settings) {
        delete m_settings;
        m_settings = nullptr;
    }
}

void CurrentLocationView::currentLocationShow()
{
    CurrentLocation currentlocation;
    QGSettings *m_settings = new QGSettings(GPS_GSETTINGS_SERVICENAME.toLocal8Bit());
    QString currentShow = m_settings->get("current-show").toString();
    if (currentShow != "") {
        m_data = currentShow;
    } else {
        if (currentlocation.getLocation()) {
            m_data = m_data.append(currentlocation.m_currentlocation.province);
            m_data = m_data.append("\n");
            m_data = m_data.append(currentlocation.m_currentlocation.cityName);
            m_data = m_data.append(currentlocation.m_currentlocation.adName);
            QString pos = tr("（经纬度：") + currentlocation.m_currentlocation.location + "）";
            m_data = m_data.append(pos);
        }
    }
    if (m_settings) {
        delete m_settings;
        m_settings = nullptr;
    }
}

} // namespace kmre_gps
