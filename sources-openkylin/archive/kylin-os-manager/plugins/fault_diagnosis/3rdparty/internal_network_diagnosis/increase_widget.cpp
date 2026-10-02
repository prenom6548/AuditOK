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

// XXX: 有待优化
void IncreaseWidget::setItemNums(int itemNums, const QList<IpWeb> &ip_web_items, IPWebWidgetType type)
{
    m_curType = type;

    for (auto &widget: m_WidgetVec) {
        if (widget) {
            ui->listLayout->removeWidget(widget);
            delete widget;
            widget = nullptr;
        }
    }
    m_WidgetVec.clear();

    // FIXME: 应该过滤掉 ip_web_items 中 IpWeb.txt 为空的元素
    int listSize = 0;
    for (const auto &item: ip_web_items) {
        if (!item.text.isEmpty())
            ++listSize;
    }

    switch (itemNums) {
    case 0: {
        ip_web_items_ = ip_web_items;
        this->hide();
        break;
    }
    case 1: {
        this->setFixedHeight(ITEM_HEIGHT);

        IPWebWidget *firstItem = new IPWebWidget(true, type, this);
        if (listSize == 0) {
            firstItem->setLineText("");
        } else {
            auto ip_web_item = ip_web_items.at(0);
            firstItem->setLineText(ip_web_item.text);
            firstItem->SetIpWeb(ip_web_item);
            if (ip_web_item.level == IpWeb::Level::kSystem)
                firstItem->Disable(true);
        }

        connect(firstItem, SIGNAL(addPressed()), this, SLOT(addNewWidget()), Qt::UniqueConnection);
        connect(firstItem, SIGNAL(userSettingsChanged()), this, SIGNAL(changedEvent()), Qt::UniqueConnection);

        m_WidgetVec.append(firstItem);

        break;
    }
    case 2:
    case 3:
    case 4:
    case 5: {
        this->setFixedHeight(ITEM_HEIGHT * listSize + ITEM_SPACE * (listSize - 1));

        IPWebWidget *firstItem = new IPWebWidget(true, type, this);
        auto ip_web_item = ip_web_items.at(0);
        firstItem->setLineText(ip_web_item.text);
        firstItem->SetIpWeb(ip_web_item);
        if (ip_web_item.level == IpWeb::Level::kSystem)
            firstItem->Disable(true);

        connect(firstItem, SIGNAL(addPressed()), this, SLOT(addNewWidget()), Qt::UniqueConnection);
        connect(firstItem, SIGNAL(userSettingsChanged()), this, SIGNAL(changedEvent()), Qt::UniqueConnection);

        m_WidgetVec.append(firstItem);

        for (int i = 1; i < listSize; ++i) {
            IPWebWidget *item = new IPWebWidget(false, type, this);
            auto ip_web_item = ip_web_items.at(i);
            item->setLineText(ip_web_item.text);
            item->SetIpWeb(ip_web_item);
            if (ip_web_item.level == IpWeb::Level::kSystem)
                item->Disable(false);

            connect(item, SIGNAL(delPressed()), this, SLOT(delOneWidget()), Qt::UniqueConnection);
            connect(item, SIGNAL(userSettingsChanged()), this, SIGNAL(changedEvent()), Qt::UniqueConnection);

            m_WidgetVec.append(item);
        }

        break;
    }
    default:
        break;
    }
}

void IncreaseWidget::showListWidget(bool isShow, IPWebWidgetType type)
{
    // XXX: 未理解其用意
    if (isShow) {
        if (m_WidgetVec.size() == 0) {
            int curListSize = 0;
            for (auto ip_web_item : ip_web_items_) {
                if (!ip_web_item.text.isEmpty()) {
                    ++curListSize;
                }
            }
            if (curListSize == 0) {
                setItemNums(1, QList<IpWeb>{IpWeb()}, type);
            } else {
                setItemNums(curListSize, ip_web_items_, type);
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

        if (w->GetLevel() == IpWeb::Level::kSystem)
            continue;

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
