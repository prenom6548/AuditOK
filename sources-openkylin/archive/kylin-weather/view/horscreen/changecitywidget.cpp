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


#include "changecitywidget.h"
#include "controller/Interaction.h"
#include <QGraphicsOpacityEffect>
#include <QMessageBox>
extern double detio;
ChangeCityWidget::ChangeCityWidget(QWidget *parent) : QWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->initWidget();
    this->initConnect();    
    this->setFixedWidth(200);
}

void ChangeCityWidget::initWidget()
{
    m_hLayout = new QHBoxLayout(this);

    m_curCity = new QLabel(this);
    m_curCity->setAccessibleName("kylin-weather_QLabel_ChangeCityWidget_curCity");
    m_collectCityBtn = new kdk::KToolButton(this);
    m_collectCityBtn->setAccessibleName("kylin-weather_QPushButton_ChangeCityWidget_collectCityBtn");
    m_changeCityBtn = new kdk::KToolButton(this);
    m_changeCityBtn->setAccessibleName("kylin-weather_QPushButton_ChangeCityWidget_changeCityBtn");
    m_changeCityBtn->setToolTip(tr("Change City"));
    m_changeCityBtn->setEnabled(false);

    m_hLayout->addSpacing(34);
    m_hLayout->addWidget(m_collectCityBtn);
    m_hLayout->addWidget(m_curCity);
    m_hLayout->addWidget(m_changeCityBtn);
    m_hLayout->addStretch();

    this->setLayout(m_hLayout);

    m_curCity->setText("N/A");
    m_curCity->setFixedHeight(27);
    m_curCity->setAlignment(Qt::AlignCenter);
    m_curCity->setStyleSheet("color: white");
    QFont font;
    font.setPixelSize(18);
    m_curCity->setFont(font);

    // 切换城市
    m_changeCityBtn->setFixedSize(21, 21);
    m_changeCityBtn->setIcon(QIcon(":/res/image/ukui-icon-city management/hover-symbolic.png"));
    m_changeCityBtn->setIconSize(QSize(21, 21));
    m_changeCityBtn->setIconColor(true, QColor(Qt::white));
    m_changeCityBtn->setProperty("isWindowButton", 0x1);
    m_changeCityBtn->setProperty("useIconHighlightEffect", 0x2);
    m_changeCityBtn->setType(kdk::KToolButtonType::Flat);
    m_changeCityBtn->setFocusPolicy(Qt::NoFocus);
    m_changeCityBtn->setEnabled(false);
//    m_changeCityBtn->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
    // 收藏城市setToolTip
    m_collectCityBtn->setFixedSize(21, 21);
    m_collectCityBtn->setIcon(QIcon::fromTheme("non-starred-symbolic"));//non-starred-symbolic
    m_collectCityBtn->setIconSize(QSize(19, 19));
    m_collectCityBtn->setIconColor(true, QColor(Qt::white));
    m_collectCityBtn->setProperty("isWindowButton", 0x1);
    m_collectCityBtn->setProperty("useIconHighlightEffect", 0x2);
    m_collectCityBtn->setType(kdk::KToolButtonType::Flat);
    m_collectCityBtn->setFocusPolicy(Qt::NoFocus);
    m_collectCityBtn->setEnabled(false);
//    m_collectCityBtn->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
    return;
}

void ChangeCityWidget::initConnect() 
{
    connect(m_collectCityBtn, &QPushButton::clicked, this, &ChangeCityWidget::slotClickCollectBtn);
    connect(Interaction::getInstance(), &Interaction::sigDeleCollectCity, this, &ChangeCityWidget::changeCollectBtnIcon); //接收当前城市信息
    connect(Interaction::getInstance(), &Interaction::sigAddCollectCity, this, &ChangeCityWidget::changeCollectBtnIcon); //接收当前城市信息
    return;
}

void ChangeCityWidget::resizeEvent(QResizeEvent *event) {}

void ChangeCityWidget::getCurrentInfor(CurrentWeather m_currentWeather)
{
    m_curCity->setText(m_currentWeather.city);
    qDebug() << " ===== INFO ===== Current Info ===== " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
}

void ChangeCityWidget::isFullScreen(int m_isFullScreen)
{
    m_judgeFullScreen = m_isFullScreen;
    initQss(m_judgeFullScreen);
}

void ChangeCityWidget::initQss(int m_judgeFullScreen)
{
    if (m_judgeFullScreen == 0) {

//        m_font.setPointSize(14);
//        m_curCity->setFont(m_font);
        //    设置显示白色字体
        QPalette curCityPalette = m_curCity->palette();
        curCityPalette.setColor(QPalette::WindowText, Qt::white);
        m_curCity->setPalette(curCityPalette);
    } else {
//        m_font.setPixelSize(24);
//        m_curCity->setFont(m_font);
        //    设置显示白色字体
        QPalette curCityPalette = m_curCity->palette();
        curCityPalette.setColor(QPalette::WindowText, Qt::white);
        m_curCity->setPalette(curCityPalette);
    }
}


kdk::KToolButton *ChangeCityWidget::getChangeCityButton()
{
    return m_changeCityBtn;
}

kdk::KToolButton *ChangeCityWidget::getCollectCityButton()
{
    return m_collectCityBtn;
}

void ChangeCityWidget::changeCollectBtnIcon()
{
    // 判断 当前显示的城市 是否为收藏城市
    QString id = CoreVar::CURRENT_CITY_ID;
    QStringList list = CoreVar::CURRENT_COLLECT_LIST;
    if (list.contains(id)) {
        m_collectCityBtn->setToolTip(tr("unCollect"));
        m_collectCityBtn->setIcon(QIcon::fromTheme("starred-symbolic"));
        m_collectCityBtn->setIconColor(true, QColor("#FFCE14"));
        m_isCollect = true;
    } else {
        m_collectCityBtn->setToolTip(tr("Collect"));
        m_collectCityBtn->setIcon(QIcon::fromTheme("non-starred-symbolic"));
        m_collectCityBtn->setIconColor(true, QColor(Qt::white));
        m_isCollect = false;
        if (CoreVar::LOCATION_CITY_ID == id) {
            CoreVar::CURRENT_CITY_TYPE = CityType::Location;
        } else {
            CoreVar::CURRENT_CITY_TYPE = CityType::Other;
        }
    }
    return;
}

void ChangeCityWidget::slotClickCollectBtn()
{
    qDebug() << " ===== INFO ===== 点击收藏标志 ===== " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
    if (m_isCollect) {
        m_isCollect = false;
        m_collectCityBtn->setToolTip(tr("Collect"));
        m_collectCityBtn->setIcon(QIcon::fromTheme("non-starred-symbolic"));
        m_collectCityBtn->setIconColor(true, QColor(Qt::white));
        Interaction::getInstance()->delCollectCityFromWidget(CoreVar::CURRENT_CITY_ID);
    } else {
        m_collectCityBtn->setToolTip(tr("unCollect"));
        if (CoreVar::CURRENT_COLLECT_LIST.size() == 17) {
            QMessageBox *msg = new QMessageBox((QWidget*)this->parent());
            msg->setAccessibleName("kylin-weather_QMessageBox_ChangeCityWidget_msg");
            msg->setWindowTitle(tr(""));
            msg->setText(tr("Add favorite cities to reach the upper limit !"));
            msg->setIcon(QMessageBox::Information);
            msg->exec();
            return;
        }
        m_isCollect = true;
        m_collectCityBtn->setIcon(QIcon::fromTheme("starred-symbolic"));
        m_collectCityBtn->setIconColor(true, QColor("#FFCE14"));
        Interaction::getInstance()->addCollectCityFromWidget(CoreVar::CURRENT_CITY_ID);
    }
    return;
}
