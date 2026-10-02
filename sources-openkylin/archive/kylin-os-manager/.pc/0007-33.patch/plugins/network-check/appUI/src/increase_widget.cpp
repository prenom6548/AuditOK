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

#include "increase_widget.h"
#include "ui_increase_widget.h"
#include <QDebug>
const static int ITEM_HEIGHT = 60;
const static int ITEM_SPACE = 0;

IncreaseWidget::IncreaseWidget(QWidget *parent) : QWidget(parent), ui(new Ui::IncreaseWidget)
{
    ui->setupUi(this);
    initUI();
}

void IncreaseWidget::initUI()
{
    m_WidgetVec.clear();
}

IncreaseWidget::~IncreaseWidget()
{
    delete ui;
}

void IncreaseWidget::setItemNums(int itemNums, QList<QString> listStr, IPWebWidgetType type)
{
    m_curType = type;

    if (type == IPWebWidgetType::IP_WIDGET) {
        qDebug() << "IncreaseWidget::setItemNums 设置IP条目：" << itemNums;
        qDebug() << "IncreaseWidget::setItemNums 设置IP地址：" << listStr;
    } else {
        qDebug() << "IncreaseWidget::setItemNums 设置WEB条目：" << itemNums;
        qDebug() << "IncreaseWidget::setItemNums 设置WEB地址：" << listStr;
    }
    for (auto widget : m_WidgetVec) {
        if (widget) {
            ui->listLayout->removeWidget(widget);
            delete widget;
            widget = nullptr;
        }
    }
    m_WidgetVec.clear();

    int listSize = 0;
    qDebug() << "IncreaseWidget::setItemNums listStr" << listStr.isEmpty();
    for (auto str : listStr) {
        if (!str.isEmpty()) {
            ++listSize;
        }
    }
    //这里有一点问题，实际上代码并不会走到 case 0 ,只会从case 1开始，但是在case 1中要处理第一条目 空和非空
    switch (itemNums) {
    case 0: {
        //        qDebug() << "0个检测项，但开启内网检测，需显示一空条目！";
        //        this->setFixedHeight(36);
        //        IPWebWidget *firstItem = new IPWebWidget(true,type);
        //        connect(firstItem,SIGNAL(addPressed()),this,SLOT(addNewWidget()),Qt::UniqueConnection);
        //        connect(firstItem,SIGNAL(userSettingsChanged()),this,SIGNAL(changedEvent()),Qt::UniqueConnection);
        //        firstItem->setLineText("");
        //        m_WidgetVec.append(firstItem);
        if (listSize != itemNums) {
            m_listStr = listStr;
        }
        qDebug() << "0个检测项，不显示！";
        this->hide();
        break;
    }
    case 1: {
        qDebug() << "1个检测项！";
        this->setFixedHeight(ITEM_HEIGHT);
        IPWebWidget *firstItem = new IPWebWidget(true, type, this);
        connect(firstItem, SIGNAL(addPressed()), this, SLOT(addNewWidget()), Qt::UniqueConnection);
        connect(firstItem, SIGNAL(userSettingsChanged()), this, SIGNAL(changedEvent()), Qt::UniqueConnection);
        if (listSize == 0) {
            firstItem->setLineText("");
        } else {
            firstItem->setLineText(listStr.at(0));
        }
        m_WidgetVec.append(firstItem);
        break;
    }
    case 2:
    case 3:
    case 4:
    case 5: {
        qDebug() << "多个检测项！";
        this->setFixedHeight(ITEM_HEIGHT * listSize + ITEM_SPACE * (listSize - 1));
        IPWebWidget *firstItem = new IPWebWidget(true, type, this);
        firstItem->setLineText(listStr.at(0));
        connect(firstItem, SIGNAL(addPressed()), this, SLOT(addNewWidget()), Qt::UniqueConnection);
        connect(firstItem, SIGNAL(userSettingsChanged()), this, SIGNAL(changedEvent()), Qt::UniqueConnection);

        m_WidgetVec.append(firstItem);
        for (int i = 1; i < listSize; ++i) {
            IPWebWidget *item = new IPWebWidget(false, type, this);
            item->setLineText(listStr.at(i));
            m_WidgetVec.append(item);
            connect(item, SIGNAL(delPressed()), this, SLOT(delOneWidget()), Qt::UniqueConnection);
            connect(item, SIGNAL(userSettingsChanged()), this, SIGNAL(changedEvent()), Qt::UniqueConnection);
        }
        break;
    }
    default:
        break;
    }
}

void IncreaseWidget::showListWidget(bool isShow, IPWebWidgetType type)
{
    if (isShow) {
        if (m_WidgetVec.size() == 0) {
            int curListSize = 0;
            for (auto str : m_listStr) {
                if (!str.isEmpty()) {
                    ++curListSize;
                }
            }
            if (curListSize == 0) {
                setItemNums(1, {""}, type);
            } else {
                setItemNums(curListSize, m_listStr, type);
            }
        }
        for (auto widget : m_WidgetVec) {
            ui->listLayout->addWidget(widget);
            this->show();
        }
    } else {
        this->hide();
    }
}

void IncreaseWidget::delOneWidget()
{
    m_WidgetVec.at(0)->setAddBtnEnable(true);
    IPWebWidget *delWidget = dynamic_cast<IPWebWidget *>(sender());
    qDebug() << "IncreaseWidget::delOneWidget size:" << m_WidgetVec.size();
    for (int i = 0; i < m_WidgetVec.size(); ++i) {
        if (delWidget == m_WidgetVec.at(i)) {
            m_WidgetVec.remove(i);
            break;
        }
    }
    ui->listLayout->removeWidget(delWidget);
    if (delWidget) {
        delete delWidget;
        delWidget = nullptr;
    }
    qDebug() << "IncreaseWidget::delOneWidget remove before size:" << m_WidgetVec.size();
    this->setFixedHeight(ITEM_HEIGHT * (m_WidgetVec.size()) + ITEM_SPACE * (m_WidgetVec.size() - 1));
    emit minWinSize();
    emit changedEvent();
    //    this->show();
}
void IncreaseWidget::addNewWidget()
{
    int curItemNum = m_WidgetVec.size();

    if (curItemNum == 5) {
        m_WidgetVec.at(0)->setAddBtnEnable(false);
        return;
    }

    m_WidgetVec.at(0)->setAddBtnEnable(true);
    IPWebWidget *lastItem = new IPWebWidget(false, m_curType, this);
    connect(lastItem, SIGNAL(delPressed()), this, SLOT(delOneWidget()), Qt::UniqueConnection);
    connect(lastItem, SIGNAL(userSettingsChanged()), this, SIGNAL(changedEvent()), Qt::UniqueConnection);
    lastItem->setLineText("");
    m_WidgetVec.append(lastItem);
    ui->listLayout->addWidget(m_WidgetVec.at(curItemNum));
    this->setFixedHeight(ITEM_HEIGHT * m_WidgetVec.size() + ITEM_SPACE * (m_WidgetVec.size() - 1));
    emit addWinSize();
    emit changedEvent();
    if (curItemNum == 4) {
        m_WidgetVec.at(0)->setAddBtnEnable(false);
    }
    this->update();
}

int IncreaseWidget::getWidgetItemNums()
{
    return m_WidgetVec.size();
}

QString IncreaseWidget::getAllSettings()
{
    QString allSettings = "";
    bool isFirst = true;
    for (auto w : m_WidgetVec) {
        if ((w->getLineText()).contains(" ") || (w->getLineText()).contains(";")) {
            continue;
        }
        if (!(w->getLineText()).isEmpty()) {
            if (isFirst) {
                allSettings.append(w->getLineText());
                isFirst = false;
            } else {
                allSettings.append(";");
                allSettings.append(w->getLineText());
            }
        } else {
            continue;
        }
    }
    return allSettings;
}
bool IncreaseWidget::getAllFormatStatus()
{
    for (auto w : m_WidgetVec) {
        if (!w->getFormatStatus()) {
            return false;
        }
    }
    return true;
}
