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
#include "perhour.h"
#include "onehourweather.h"
#include <QLabel>
#include <QPushButton>
#include <QGraphicsOpacityEffect>
#include <QDebug>
#include <QPainter>
#include <QStyleOption>
#include <QEvent>
#include <QResizeEvent>
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>

const QSize ICON_SIZE = QSize(72, 66);
const QString ICON_PATH = ":/res/weather-icon/PNG48/1x/";
const QString ICON_FILE = ":/res/weather-icon/PNG48/1x/100.png";

perhour::perhour(QWidget *parent) : QWidget(parent)
{
    this->initWidget();
    this->initConnect();
}

void perhour::initWidget()
{
    // 创建24个小时天气组件并添加到列表中
    m_onehourWeatherFirstFirst = new onehourweather(this);
    m_onehourWeatherFirstFirst->setAccessibleName("kylin-weather_onehourWeatherFirstFirst");
    m_onehourWeatherFirstSecond = new onehourweather(this);
    m_onehourWeatherFirstSecond->setAccessibleName("kylin-weather_onehourWeatherFirstSecond");
    m_onehourWeatherFirstThird = new onehourweather(this);
    m_onehourWeatherFirstThird->setAccessibleName("kylin-weather_onehourWeatherFirstThird");
    m_onehourWeatherFirstFourth = new onehourweather(this);
    m_onehourWeatherFirstFourth->setAccessibleName("kylin-weather_onehourWeatherFirstFourth");
    m_onehourWeatherFirstFifth = new onehourweather(this);
    m_onehourWeatherFirstFifth->setAccessibleName("kylin-weather_onehourWeatherFirstFifth");
    m_onehourWeatherFirstSixth = new onehourweather(this);
    m_onehourWeatherFirstSixth->setAccessibleName("kylin-weather_onehourWeatherFirstSixth");
    m_onehourWeatherFirstSeventh = new onehourweather(this);
    m_onehourWeatherFirstSeventh->setAccessibleName("kylin-weather_onehourWeatherFirstSeventh");
    m_onehourWeatherFirstEighth = new onehourweather(this);
    m_onehourWeatherFirstEighth->setAccessibleName("kylin-weather_onehourWeatherFirstEighth");

    m_onehourWeatherSecondFirst = new onehourweather(this);
    m_onehourWeatherSecondFirst->setAccessibleName("kylin-weather_onehourWeatherSecondFirst");
    m_onehourWeatherSecondSecond = new onehourweather(this);
    m_onehourWeatherSecondSecond->setAccessibleName("kylin-weather_onehourWeatherSecondSecond");
    m_onehourWeatherSecondThird = new onehourweather(this);
    m_onehourWeatherSecondThird->setAccessibleName("kylin-weather_onehourWeatherSecondThird");
    m_onehourWeatherSecondFourth = new onehourweather(this);
    m_onehourWeatherSecondFourth->setAccessibleName("kylin-weather_onehourWeatherSecondFourth");
    m_onehourWeatherSecondFifth = new onehourweather(this);
    m_onehourWeatherSecondFifth->setAccessibleName("kylin-weather_onehourWeatherSecondFifth");
    m_onehourWeatherSecondSixth = new onehourweather(this);
    m_onehourWeatherSecondSixth->setAccessibleName("kylin-weather_onehourWeatherSecondSixth");
    m_onehourWeatherSecondSeventh = new onehourweather(this);
    m_onehourWeatherSecondSeventh->setAccessibleName("kylin-weather_onehourWeatherSecondSeventh");
    m_onehourWeatherSecondEighth = new onehourweather(this);
    m_onehourWeatherSecondEighth->setAccessibleName("kylin-weather_onehourWeatherSecondEighth");

    m_onehourWeatherThirdFirst = new onehourweather(this);
    m_onehourWeatherThirdFirst->setAccessibleName("kylin-weather_onehourWeatherThirdFirst");
    m_onehourWeatherThirdSecond = new onehourweather(this);
    m_onehourWeatherThirdSecond->setAccessibleName("kylin-weather_onehourWeatherThirdSecond");
    m_onehourWeatherThirdThird = new onehourweather(this);
    m_onehourWeatherThirdThird->setAccessibleName("kylin-weather_onehourWeatherThirdThird");
    m_onehourWeatherThirdFourth = new onehourweather(this);
    m_onehourWeatherThirdFourth->setAccessibleName("kylin-weather_onehourWeatherThirdFourth");
    m_onehourWeatherThirdFifth = new onehourweather(this);
    m_onehourWeatherThirdFifth->setAccessibleName("kylin-weather_onehourWeatherThirdFifth");
    m_onehourWeatherThirdSixth = new onehourweather(this);
    m_onehourWeatherThirdSixth->setAccessibleName("kylin-weather_onehourWeatherThirdSixth");
    m_onehourWeatherThirdSeventh = new onehourweather(this);
    m_onehourWeatherThirdSeventh->setAccessibleName("kylin-weather_onehourWeatherThirdSeventh");
    m_onehourWeatherThirdEighth = new onehourweather(this);
    m_onehourWeatherThirdEighth->setAccessibleName("kylin-weather_onehourWeatherThirdEighth");

    // 将所有组件添加到列表中，便于管理
    m_hourWeatherList << m_onehourWeatherFirstFirst << m_onehourWeatherFirstSecond 
                      << m_onehourWeatherFirstThird << m_onehourWeatherFirstFourth
                      << m_onehourWeatherFirstFifth << m_onehourWeatherFirstSixth
                      << m_onehourWeatherFirstSeventh << m_onehourWeatherFirstEighth
                      << m_onehourWeatherSecondFirst << m_onehourWeatherSecondSecond
                      << m_onehourWeatherSecondThird << m_onehourWeatherSecondFourth
                      << m_onehourWeatherSecondFifth << m_onehourWeatherSecondSixth
                      << m_onehourWeatherSecondSeventh << m_onehourWeatherSecondEighth
                      << m_onehourWeatherThirdFirst << m_onehourWeatherThirdSecond
                      << m_onehourWeatherThirdThird << m_onehourWeatherThirdFourth
                      << m_onehourWeatherThirdFifth << m_onehourWeatherThirdSixth
                      << m_onehourWeatherThirdSeventh << m_onehourWeatherThirdEighth;

    // 创建导航按钮
    m_leftBtn = new QPushButton(this);
    m_leftBtn->setAccessibleName("kylin-weather_QPushButton_perhour_leftBtn");
    m_leftBtn->setIcon(QIcon::fromTheme("ukui-start-symbolic"));
    m_leftBtn->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
    m_rightBtn = new QPushButton(this);
    m_rightBtn->setAccessibleName("kylin-weather_QPushButton_perhour_rightBtn");
    m_rightBtn->setIcon(QIcon::fromTheme("ukui-end-symbolic"));
    m_rightBtn->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
    
    // 设置按钮样式和初始状态
    m_leftBtn->setFixedSize(24, 36);
    m_rightBtn->setFixedSize(24, 36);
    m_leftBtn->hide(); // 初始隐藏
    m_rightBtn->hide(); // 初始隐藏

    m_perhourLayout = new QHBoxLayout();
    m_perhourWid = new QWidget(this);
    m_perhourWid->setAccessibleName("kylin-weather_QWidget_perhour_perhourWid");
    m_Layout = new QHBoxLayout(this);
    this->initStyle();
    this->setFixedSize(856, 138);

    return;
}

void perhour::initStyle()
{
    for (int i = 0; i < m_hourWeatherList.size() && i < m_itemsPerPage; ++i) {
        m_perhourLayout->addWidget(m_hourWeatherList[i]);
        m_hourWeatherList[i]->show();
    }

    for (int i = m_itemsPerPage; i < m_hourWeatherList.size(); ++i) {
        m_hourWeatherList[i]->hide();
    }
    
    // 设置布局参数
    m_perhourLayout->setMargin(0);
    m_perhourLayout->setContentsMargins(25, 5, 25, 5);
    
    // 设置主布局
    m_perhourWid->setLayout(m_perhourLayout);
    m_Layout->addWidget(m_perhourWid);
    m_Layout->setMargin(0);
    m_Layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(m_Layout);
    
    // 设置按钮位置
    m_leftBtn->move(10, this->height() / 2 - 50);
    m_rightBtn->move(this->width() - 35, this->height() / 2 - 50);
    m_leftBtn->raise();
    m_rightBtn->raise();

    switch (ViewVar::getInstance()->m_currentMode)
    {
        case CurrentMode::PCMode:
            slotChangePCMode();
            break;
        case CurrentMode::PadHMode:
            slotChangePadHMode();
            break;
        case CurrentMode::PadVMode:
            slotChangePadVMode();
            break;
        default:
            slotChangePCMode();
            break;
    }
    return;
}

// 逐小时信息
void perhour::setHourlyInfor(QList<Hourly> tmp_list, int m_isFullScreen)
{
    tmpList = tmp_list;

    if (tmpList.isEmpty()) {

        return;
    }

    // 计算总页数
    m_totalPages = (tmpList.size() + m_itemsPerPage - 1) / m_itemsPerPage;
    m_currentPage = 0; // 重置到第一页
    
    // 为所有小时天气组件设置数据
    for (int i = 0; i < m_hourWeatherList.size() && i < tmpList.size(); ++i) {
        QString iconPath = ICON_PATH + tmpList.at(i).cond_code + ".png";
        QFileInfo iconFile(iconPath);
        if (!iconFile.exists()) {
            iconPath = ICON_FILE; // 使用默认图标
        }
        
        // 设置小时天气数据
        m_hourWeatherList[i]->m_time->setText(tmpList.at(i).time);
        m_hourWeatherList[i]->m_icon->setPixmap(QPixmap(iconPath));
        m_hourWeatherList[i]->m_temperature->setText(tmpList.at(i).tmp);
    }
    
    // 显示第一页并更新按钮状态
    updatePageDisplay();
    updateButtonVisibility();
    
}

// 更新当前页面显示
void perhour::updatePageDisplay()
{
    // 清空当前布局
    QLayoutItem *child;
    while ((child = m_perhourLayout->takeAt(0)) != nullptr) {
        // 不删除widget，只是从布局中移除
    }
    // 隐藏所有组件
    for (onehourweather* widget : m_hourWeatherList) {
        widget->hide();
    }

    // 计算实际显示的数据范围
    int actualStartIndex;
    int remainingItems = tmpList.size() - (m_currentPage * m_itemsPerPage);

    if (remainingItems >= m_itemsPerPage) {
        // 如果剩余数据足够显示一页，正常显示
        actualStartIndex = m_currentPage * m_itemsPerPage;
    } else {
        // 如果最后一页不够9个，显示倒数9个数据
        actualStartIndex = qMax(0, tmpList.size() - m_itemsPerPage);
    }

    // 始终显示9个组件
    for (int i = 0; i < m_itemsPerPage && i < m_hourWeatherList.size(); ++i) {

        int dataIndex = actualStartIndex + i;
        
        // 如果超出数据范围，从头开始循环
        if (dataIndex >= tmpList.size()) {
            dataIndex = dataIndex % tmpList.size();
        }
        
        if (dataIndex < tmpList.size() && i < m_hourWeatherList.size()) {
            // 更新组件数据
            QString iconPath = ICON_PATH + tmpList.at(dataIndex).cond_code + ".png";
            QFileInfo iconFile(iconPath);
            if (!iconFile.exists()) {
                iconPath = ICON_FILE;
            }
            
            m_hourWeatherList[i]->m_time->setText(tmpList.at(dataIndex).time);
            m_hourWeatherList[i]->m_icon->setPixmap(QPixmap(iconPath));
            m_hourWeatherList[i]->m_temperature->setText(tmpList.at(dataIndex).tmp);
            
            // 添加到布局并显示
            m_perhourLayout->addWidget(m_hourWeatherList[i]);
            m_hourWeatherList[i]->show();
        }
    }
    
    // 重新设置布局参数以保持间距
    int availableWidth = this->width() - 20; // 减去左右边距
    int space = (availableWidth - (9 * 80)) / 8; // 9个组件的间距计算
    m_perhourLayout->setSpacing(space);
}

// 更新按钮显示状态
void perhour::updateButtonVisibility()
{
    // 设置按钮的可用性
    bool canGoLeft = m_currentPage > 0;
    bool canGoRight = m_currentPage < m_totalPages - 1;
    
    m_leftBtn->setEnabled(canGoLeft);
    m_rightBtn->setEnabled(canGoRight);
    
    // 如果当前鼠标在组件内，根据可用性显示按钮
    if (this->underMouse()) {
        if (canGoLeft) {
            m_leftBtn->show();
        } else {
            m_leftBtn->hide();
        }
        
        if (canGoRight) {
            m_rightBtn->show();
        } else {
            m_rightBtn->hide();
        }
    }
}

// 显示指定页面
void perhour::showPage(int pageIndex)
{
    if (pageIndex >= 0 && pageIndex < m_totalPages) {
        m_currentPage = pageIndex;
        updatePageDisplay();
        updateButtonVisibility();
    }
}

// 左按钮点击事件
void perhour::onLeftButtonClicked()
{
    if (m_currentPage > 0) {
        showPage(m_currentPage - 1);
    }
}

// 右按钮点击事件
void perhour::onRightButtonClicked()
{
    if (m_currentPage < m_totalPages - 1) {
        showPage(m_currentPage + 1);
    }
}

// 鼠标进入事件
void perhour::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    // 更新按钮显示状态
    updateButtonVisibility();
    QWidget::enterEvent(event);
}

// 鼠标离开事件
void perhour::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    // 隐藏所有按钮
    m_leftBtn->hide();
    m_rightBtn->hide();
    QWidget::leaveEvent(event);
}

void perhour::setHourlyIcon(int m_isFullScreen)
{
    if (tmpList.isEmpty()) {

        return;
    }
    m_firstFirstIconPath = ICON_PATH + tmpList.at(0).cond_code + ".png";
    m_firstSecondIconPath = ICON_PATH + tmpList.at(1).cond_code + ".png";
    m_firstThirdIconPath = ICON_PATH + tmpList.at(2).cond_code + ".png";
    m_firstFourthIconPath = ICON_PATH + tmpList.at(3).cond_code + ".png";
    m_firstFifthIconPath = ICON_PATH + tmpList.at(4).cond_code + ".png";
    m_firstSixthIconPath = ICON_PATH + tmpList.at(5).cond_code + ".png";
    m_firstSeventhIconPath = ICON_PATH + tmpList.at(6).cond_code + ".png";
    m_firstEighthIconPath = ICON_PATH + tmpList.at(7).cond_code + ".png";

    m_secondFirstIconPath = ICON_PATH + tmpList.at(8).cond_code + ".png";
    m_secondSecondIconPath = ICON_PATH + tmpList.at(9).cond_code + ".png";
    m_secondThirdIconPath = ICON_PATH + tmpList.at(10).cond_code + ".png";
    m_secondFourthIconPath = ICON_PATH + tmpList.at(11).cond_code + ".png";
    m_secondFifthIconPath = ICON_PATH + tmpList.at(12).cond_code + ".png";
    m_secondSixthIconPath = ICON_PATH + tmpList.at(13).cond_code + ".png";
    m_secondSeventhIconPath = ICON_PATH + tmpList.at(14).cond_code + ".png";
    m_secondEighthIconPath = ICON_PATH + tmpList.at(15).cond_code + ".png";

    m_thirdFirstIconPath = ICON_PATH + tmpList.at(16).cond_code + ".png";
    m_thirdSecondIconPath = ICON_PATH + tmpList.at(17).cond_code + ".png";
    m_thirdThirdIconPath = ICON_PATH + tmpList.at(18).cond_code + ".png";
    m_thirdFourthIconPath = ICON_PATH + tmpList.at(19).cond_code + ".png";
    m_thirdFifthIconPath = ICON_PATH + tmpList.at(20).cond_code + ".png";
    m_thirdSixthIconPath = ICON_PATH + tmpList.at(21).cond_code + ".png";
    m_thirdSeventhIconPath = ICON_PATH + tmpList.at(22).cond_code + ".png";
    m_thirdEighthIconPath = ICON_PATH + tmpList.at(23).cond_code + ".png";

    QFileInfo m_firstFirstIconPathFile(m_firstFirstIconPath);
    if (m_firstFirstIconPathFile.exists() == false) {
        m_firstFirstIconPath = ICON_FILE;
    }
    QFileInfo m_firstSecondIconPathFile(m_firstSecondIconPath);
    if (m_firstSecondIconPathFile.exists() == false) {
        m_firstSecondIconPath = ICON_FILE;
    }
    QFileInfo m_firstThirdIconPathFile(m_firstThirdIconPath);
    if (m_firstThirdIconPathFile.exists() == false) {
        m_firstThirdIconPath = ICON_FILE;
    }
    QFileInfo m_firstFourthIconPathFile(m_firstFourthIconPath);
    if (m_firstFourthIconPathFile.exists() == false) {
        m_firstFourthIconPath = ICON_FILE;
    }
    QFileInfo m_firstFifthIconPathFile(m_firstFifthIconPath);
    if (m_firstFifthIconPathFile.exists() == false) {
        m_firstFifthIconPath = ICON_FILE;
    }
    QFileInfo m_firstSixthIconPathFile(m_firstSixthIconPath);
    if (m_firstSixthIconPathFile.exists() == false) {
        m_firstSixthIconPath = ICON_FILE;
    }
    QFileInfo m_firstSeventhIconPathFile(m_firstSeventhIconPath);
    if (m_firstSeventhIconPathFile.exists() == false) {
        m_firstSeventhIconPath = ICON_FILE;
    }
    QFileInfo m_firstEighthIconPathFile(m_firstEighthIconPath);
    if (m_firstEighthIconPathFile.exists() == false) {
        m_firstEighthIconPath = ICON_FILE;
    }

    QFileInfo m_secondFirstIconPathFile(m_secondFirstIconPath);
    if (m_secondFirstIconPathFile.exists() == false) {
        m_secondFirstIconPath = ICON_FILE;
    }
    QFileInfo m_secondSecondIconPathFile(m_secondSecondIconPath);
    if (m_secondSecondIconPathFile.exists() == false) {
        m_secondSecondIconPath = ICON_FILE;
    }
    QFileInfo m_secondThirdIconPathFile(m_secondThirdIconPath);
    if (m_secondThirdIconPathFile.exists() == false) {
        m_secondThirdIconPath = ICON_FILE;
    }
    QFileInfo m_secondFourthIconPathFile(m_secondFourthIconPath);
    if (m_secondFourthIconPathFile.exists() == false) {
        m_secondFourthIconPath = ICON_FILE;
    }
    QFileInfo m_secondFifthIconPathFile(m_secondFifthIconPath);
    if (m_secondFifthIconPathFile.exists() == false) {
        m_secondFifthIconPath = ICON_FILE;
    }
    QFileInfo m_secondSixthIconPathFile(m_secondSixthIconPath);
    if (m_secondSixthIconPathFile.exists() == false) {
        m_secondSixthIconPath = ICON_FILE;
    }
    QFileInfo m_secondSeventhIconPathFile(m_secondSeventhIconPath);
    if (m_secondSeventhIconPathFile.exists() == false) {
        m_secondSeventhIconPath = ICON_FILE;
    }
    QFileInfo m_secondEighthIconPathFile(m_secondEighthIconPath);
    if (m_secondEighthIconPathFile.exists() == false) {
        m_secondEighthIconPath = ICON_FILE;
    }

    QFileInfo m_thirdFirstIconPathFile(m_thirdFirstIconPath);
    if (m_thirdFirstIconPathFile.exists() == false) {
        m_thirdFirstIconPath = ICON_FILE;
    }
    QFileInfo m_thirdSecondIconPathFile(m_thirdSecondIconPath);
    if (m_thirdSecondIconPathFile.exists() == false) {
        m_thirdSecondIconPath = ICON_FILE;
    }
    QFileInfo m_thirdThirdIconPathFile(m_thirdThirdIconPath);
    if (m_thirdThirdIconPathFile.exists() == false) {
        m_thirdThirdIconPath = ICON_FILE;
    }
    QFileInfo m_thirdFourthIconPathFile(m_thirdFourthIconPath);
    if (m_thirdFourthIconPathFile.exists() == false) {
        m_thirdFourthIconPath = ICON_FILE;
    }
    QFileInfo m_thirdFifthIconPathFile(m_thirdFifthIconPath);
    if (m_thirdFifthIconPathFile.exists() == false) {
        m_thirdFifthIconPath = ICON_FILE;
    }
    QFileInfo m_thirdSixthIconPathFile(m_thirdSixthIconPath);
    if (m_thirdSixthIconPathFile.exists() == false) {
        m_thirdSixthIconPath = ICON_FILE;
    }
    QFileInfo m_thirdSeventhIconPathFile(m_thirdSeventhIconPath);
    if (m_thirdSeventhIconPathFile.exists() == false) {
        m_thirdSeventhIconPath = ICON_FILE;
    }
    QFileInfo m_thirdEighthIconPathFile(m_thirdEighthIconPath);
    if (m_thirdEighthIconPathFile.exists() == false) {
        m_thirdEighthIconPath = ICON_FILE;
    }

    m_onehourWeatherFirstFirst->m_icon->resize(ICON_SIZE);
    QPixmap firstFirstPixmap(m_firstFirstIconPath);
    m_onehourWeatherFirstFirst->m_icon->setPixmap(firstFirstPixmap);

    m_onehourWeatherFirstSecond->m_icon->resize(ICON_SIZE);
    QPixmap firstSecondPixmap(m_firstSecondIconPath);
    m_onehourWeatherFirstSecond->m_icon->setPixmap(firstSecondPixmap);

    m_onehourWeatherFirstThird->m_icon->resize(72, 66);
    QPixmap firstThirdPixmap(m_firstThirdIconPath);
    m_onehourWeatherFirstThird->m_icon->setPixmap(firstThirdPixmap);

    m_onehourWeatherFirstFourth->m_icon->resize(72, 66);
    QPixmap firstFourthPixmap(m_firstFourthIconPath);
    m_onehourWeatherFirstFourth->m_icon->setPixmap(firstFourthPixmap);

    m_onehourWeatherFirstFifth->m_icon->resize(ICON_SIZE);
    QPixmap firstFifthPixmap(m_firstFifthIconPath);
    m_onehourWeatherFirstFifth->m_icon->setPixmap(firstFifthPixmap);

    m_onehourWeatherFirstSixth->m_icon->resize(ICON_SIZE);
    QPixmap firstSixthPixmap(m_firstSixthIconPath);
    m_onehourWeatherFirstSixth->m_icon->setPixmap(firstSixthPixmap);

    m_onehourWeatherFirstSeventh->m_icon->resize(ICON_SIZE);
    QPixmap firstSeventhPixmap(m_firstSeventhIconPath);
    m_onehourWeatherFirstSeventh->m_icon->setPixmap(firstSeventhPixmap);

    m_onehourWeatherFirstEighth->m_icon->resize(ICON_SIZE);

    QPixmap firstEighthPixmap(m_firstEighthIconPath);
    m_onehourWeatherFirstEighth->m_icon->setPixmap(firstEighthPixmap);
    // ----------- 第二页 -------------
    m_onehourWeatherSecondFirst->m_icon->resize(ICON_SIZE);
    QPixmap secondFirstPixmap(m_secondFirstIconPath);
    m_onehourWeatherSecondFirst->m_icon->setPixmap(secondFirstPixmap);

    m_onehourWeatherSecondSecond->m_icon->resize(ICON_SIZE);
    QPixmap secondSecondPixmap(m_secondSecondIconPath);
    m_onehourWeatherSecondSecond->m_icon->setPixmap(secondSecondPixmap);

    m_onehourWeatherSecondThird->m_icon->resize(ICON_SIZE);
    QPixmap secondThirdPixmap(m_secondThirdIconPath);
    m_onehourWeatherSecondThird->m_icon->setPixmap(secondThirdPixmap);

    m_onehourWeatherSecondFourth->m_icon->resize(ICON_SIZE);
    QPixmap secondFourthPixmap(m_secondFourthIconPath);
    m_onehourWeatherSecondFourth->m_icon->setPixmap(secondFourthPixmap);

    m_onehourWeatherSecondFifth->m_icon->resize(ICON_SIZE);
    QPixmap secondFifthPixmap(m_secondFifthIconPath);
    m_onehourWeatherSecondFifth->m_icon->setPixmap(secondFifthPixmap);

    m_onehourWeatherSecondSixth->m_icon->resize(ICON_SIZE);
    QPixmap secondSixthPixmap(m_secondSixthIconPath);
    m_onehourWeatherSecondSixth->m_icon->setPixmap(secondSixthPixmap);

    m_onehourWeatherSecondSeventh->m_icon->resize(ICON_SIZE);
    QPixmap secondSeventhPixmap(m_secondSeventhIconPath);
    m_onehourWeatherSecondSeventh->m_icon->setPixmap(secondSeventhPixmap);

    m_onehourWeatherSecondEighth->m_icon->resize(ICON_SIZE);
    QPixmap secondEighthPixmap(m_secondEighthIconPath);
    m_onehourWeatherSecondEighth->m_icon->setPixmap(secondEighthPixmap);
    // ------------------ 第三页 ----------------------------
    m_onehourWeatherThirdFirst->m_icon->resize(ICON_SIZE);
    QPixmap thirdFirstPixmap(m_thirdFirstIconPath);
    m_onehourWeatherThirdFirst->m_icon->setPixmap(thirdFirstPixmap);

    m_onehourWeatherThirdSecond->m_icon->resize(ICON_SIZE);
    QPixmap thirdSecondPixmap(m_thirdSecondIconPath);
    m_onehourWeatherThirdSecond->m_icon->setPixmap(thirdSecondPixmap);

    m_onehourWeatherThirdThird->m_icon->resize(ICON_SIZE);
    QPixmap thirdThirdPixmap(m_thirdThirdIconPath);
    m_onehourWeatherThirdThird->m_icon->setPixmap(thirdThirdPixmap);

    m_onehourWeatherThirdFourth->m_icon->resize(ICON_SIZE);
    QPixmap thirdFourthPixmap(m_thirdFourthIconPath);
    m_onehourWeatherThirdFourth->m_icon->setPixmap(thirdFourthPixmap);

    m_onehourWeatherThirdFifth->m_icon->resize(ICON_SIZE);
    QPixmap thirdFifthPixmap(m_thirdFifthIconPath);
    m_onehourWeatherThirdFifth->m_icon->setPixmap(thirdFifthPixmap);

    m_onehourWeatherThirdSixth->m_icon->resize(ICON_SIZE);
    QPixmap thirdSixthPixmap(m_thirdSixthIconPath);
    m_onehourWeatherThirdSixth->m_icon->setPixmap(thirdSixthPixmap);

    m_onehourWeatherThirdSeventh->m_icon->resize(ICON_SIZE);
    QPixmap thirdSeventhPixmap(m_thirdSeventhIconPath);
    m_onehourWeatherThirdSeventh->m_icon->setPixmap(thirdSeventhPixmap);

    m_onehourWeatherThirdEighth->m_icon->resize(ICON_SIZE);
    QPixmap thirdEighthPixmap(m_thirdEighthIconPath);
    m_onehourWeatherThirdEighth->m_icon->setPixmap(thirdEighthPixmap);
}

void perhour::initConnect()
{
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherFirstFirst, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherFirstSecond, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherFirstThird, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherFirstFourth, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherFirstFifth, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherFirstSixth, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherFirstSeventh, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherFirstEighth, &onehourweather::isFullScreen);

    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherSecondFirst, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherSecondSecond, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherSecondThird, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherSecondFourth, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherSecondFifth, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherSecondSixth, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherSecondSeventh, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherSecondEighth, &onehourweather::isFullScreen);

    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherThirdFirst, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherThirdSecond, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherThirdThird, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherThirdFourth, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherThirdFifth, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherThirdSixth, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherThirdSeventh, &onehourweather::isFullScreen);
    connect(this, &perhour::sendIsFullScreen, m_onehourWeatherThirdEighth, &onehourweather::isFullScreen);

    connect(ViewVar::getInstance(), &ViewVar::sigChangeModeToPc, this, &perhour::slotChangePCMode);
    connect(ViewVar::getInstance(), &ViewVar::sigChangeModeToPadH, this, &perhour::slotChangePadHMode);
    connect(ViewVar::getInstance(), &ViewVar::sigChangeModeToPadV, this, &perhour::slotChangePadVMode);
    
    // 连接导航按钮信号
    connect(m_leftBtn, &QPushButton::clicked, this, &perhour::onLeftButtonClicked);
    connect(m_rightBtn, &QPushButton::clicked, this, &perhour::onRightButtonClicked);
}

void perhour::paintEvent(QPaintEvent *event)
{

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;

    //设置背景颜色
    QColor color = this->palette().color(QPalette::Dark);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    painter.setBrush(brush);
    painter.setOpacity(0.08);
    painter.setPen(Qt::transparent);
    int radius = 3 * ViewVar::SAME_ITEMRADIUS;
    QStyleOption opt;
    opt.init(this);
    painter.drawRoundedRect(opt.rect, radius, radius);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);

}
// 平板模式（竖）
void perhour::slotChangePadHMode()
{
    this->setFixedSize(858, 214); // 设置固定宽度为858px
    int availableWidth = this->width() - 20; // 减去左右边距
    int space = (availableWidth - (9 * 80)) / 8; // 9个组件的间距计算
    m_perhourLayout->setSpacing(space);
    m_perhourLayout->setMargin(0);
    m_perhourLayout->setContentsMargins(10, 40, 10, 0);
    // 调整按钮位置
    m_leftBtn->move(5, this->height() / 2 - 15);
    m_rightBtn->move(this->width() - 35, this->height() / 2 - 15);
    return;
}
// 平板模式（竖）
void perhour::slotChangePadVMode()
{ 
    this->setFixedSize(858, 214); // 设置固定宽度为858px
    int availableWidth = this->width() - 20; // 减去左右边距
    int space = (availableWidth - (9 * 80)) / 8; // 9个组件的间距计算
    m_perhourLayout->setSpacing(space);
    m_perhourLayout->setMargin(0);
    m_perhourLayout->setContentsMargins(10, 40, 10, 0);
    // 调整按钮位置
    m_leftBtn->move(5, this->height() / 2 - 15);
    m_rightBtn->move(this->width() - 35, this->height() / 2 - 15);
    return;
}
// PC模式
void perhour::slotChangePCMode()
{
    this->setFixedSize(858, 200); // 设置固定宽度为858px
    int availableWidth = this->width() - 20; // 减去左右边距
    int space = (availableWidth - (9 * 80)) / 8; // 9个组件的间距计算
    m_perhourLayout->setSpacing(space);
    m_perhourLayout->setMargin(0);
    m_perhourLayout->setContentsMargins(25, 5, 25, 5);
    // 调整按钮位置
    m_leftBtn->move(10, this->height() / 2 - 50);
    m_rightBtn->move(this->width() - 35, this->height() / 2 - 50 );
    return;
}
