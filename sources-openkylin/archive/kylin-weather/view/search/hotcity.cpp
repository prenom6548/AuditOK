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
#include "hotcity.h"

HotCity::HotCity(QWidget *parent) : QWidget(parent)
{
    this->setFixedSize(ViewVar::HOT);
    this->setAttribute(Qt::WA_TranslucentBackground);

    m_title = new QLabel(this);
    m_title->setAccessibleName("kylin-weather_QLabel_HotCity_title");
    m_title->setText(tr("Hot City"));
    m_title->setFixedWidth(this->width());
    m_title->move(15, ViewVar::HOT_TITLE.y());

    m_hotWid = new QWidget(this);
    m_hotWid->setAccessibleName("kylin-weather_QWidget_HotCity_hotwid");
    m_hotWid->setFixedSize(ViewVar::HOT_LIST);
    m_hotWid->move(10, m_title->y() + m_title->height());
    m_hotWid->setAttribute(Qt::WA_TranslucentBackground);
    //得到热门城市map
    m_btnList = ViewVar::saveHotCity();
    //初始化每个按钮
    QMap<int, QStringList>::iterator iter = m_btnList.begin();
    while (iter != m_btnList.end()) {
        HotcityBtn *m_addcityBtn = new HotcityBtn(m_hotWid, iter.value().at(0), iter.value().at(1));
        m_addcityBtn->setAccessibleName("kylin-weather_HotcityBtn_addcityBtn");
        QString fullText = m_addcityBtn->text();
        QString smpText = m_addcityBtn->text();
        //长度超过3，...显示省略部分
        if (smpText.length() > ViewVar::ITEM_LENGTHM) {
            smpText = smpText.mid(0, 2) + "…";
            m_addcityBtn->setText(smpText);
            m_addcityBtn->setToolTip(fullText);
        } else {
            m_addcityBtn->setToolTip("");
        }
        m_list.append(m_addcityBtn);
        iter++;
    }

    initPostion();
    initConnect();
}
//设置每个按钮的位置
void HotCity::initPostion()
{
    // 计算按钮间距，使其与收藏城市保持一致
    // 收藏城市使用spacing(10)，我们需要调整按钮间距
    int buttonWidth = ViewVar::HOT_BTN.width();  // 96
    int spacing = 10; // 与收藏城市的spacing保持一致
    int itemSpacing = buttonWidth + spacing; // 106
    int startX = 0; // 起始位置，已经通过容器位置调整了边距

    for (int i = 0; i < m_list.length(); i++) {
        int row = i / 6; // 每行6个按钮
        int col = i % 6; // 列位置

        int x = startX + col * itemSpacing;
        int y = row * (ViewVar::HOT_BTN.height() + spacing);

        m_list.at(i)->move(x, y);
    }

}
//点击发送对应的id值
void HotCity::initConnect()
{
    for (int i = 0; i < m_list.length(); i++) {
        connect(m_list.at(i), &HotcityBtn::hotCityClick, this, &HotCity::setHotCity);
    }
}
