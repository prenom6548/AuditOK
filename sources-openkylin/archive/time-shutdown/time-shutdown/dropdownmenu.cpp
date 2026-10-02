/*
 * Ukui-shutdown-timer
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 *
 * Authors:  zhangtianze <zhangtianze@kylinos.cn>
 *
 */

#include "dropdownmenu.h"
#include <QWidgetAction>
#include <QStyleOption>
#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <QTimer>

MenuItem::MenuItem(QString week, int id, QWidget *parent) : QToolButton(parent), m_id(id)
{
    m_pweekLabel = new QLabel();
    m_pweekLabel->setText(week);

    setAutoRaise(true);

    m_pselectedLabelIcon = new QLabel();
    m_pselectedLabelIcon->setFixedSize(16, 16);

    m_pHWeekLayout = new QHBoxLayout();
    m_pHWeekLayout->setContentsMargins(17, 0, 0, 0);
    this->setLayout(m_pHWeekLayout);
    m_pHWeekLayout->addWidget(m_pselectedLabelIcon);
    m_pHWeekLayout->addWidget(m_pweekLabel);

    QSizePolicy policy = m_pselectedLabelIcon->sizePolicy();
    policy.setRetainSizeWhenHidden(true);
    m_pselectedLabelIcon->setSizePolicy(policy);
    m_pselectedLabelIcon->setVisible(false);

    this->setFixedSize(306, 35);
}

void MenuItem::setIconVisible(bool status)
{
    m_pselectedLabelIcon->setVisible(status);
}

void MenuItem::setSelectIcon(const QPixmap &pixmap)
{
    m_pselectedLabelIcon->setPixmap(pixmap);
}

//显示或取消勾选标志
void MenuItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (!this->m_pselectedLabelIcon->isVisible()) {
            m_pselectedLabelIcon->setVisible(true);
            emit showLabelIcon(m_id);
        } else {
            m_pselectedLabelIcon->setVisible(false);
            emit hideLabelIcon(m_id);
        }
    }
    QToolButton::mouseReleaseEvent(event);
}

DropDownMenu::DropDownMenu()
{
    initAllDateWidget();
    initItemSelectIcon();
}

void DropDownMenu::initAllDateWidget()
{
    this->setFixedWidth(MENU_WIDTH);
    m_selectWeek.clear();
    for (int i = 0; i < m_pWeekDateList.count(); i++) {
        MenuItem *item = new MenuItem(m_pWeekDateList[i], i, this);
        connect(item, SIGNAL(showLabelIcon(int)), this, SLOT(handleShowLabelSlot(int)));
        connect(item, SIGNAL(hideLabelIcon(int)), this, SLOT(handleHideLabelSlot(int)));
        QWidgetAction *action = new QWidgetAction(this);
        action->setDefaultWidget(item);
        this->addAction(action);
    }
}

void DropDownMenu::initLabelStatus(QList<int> selectWeek)
{
    m_selectWeek.clear();
    for(int i = 0; i < selectWeek.count(); i++) {
        QWidgetAction *wAction = dynamic_cast<QWidgetAction *>(this->actions().at(selectWeek[i]));
        MenuItem *item = dynamic_cast<MenuItem *>(wAction->defaultWidget());
        item->setIconVisible(true);
        m_selectWeek.append(selectWeek[i]);
    }
}

//返回列表选中的组合
QString DropDownMenu::getChosenList(QList<int> selectList)
{
    QString chooseList = m_pWeekDateList[selectList[0]];
    for (int i = 1; i < selectList.count(); i++) {
        chooseList += " " + m_pWeekDateList[selectList[i]];
    }
    return chooseList;
}

//从gsetting中获取选取星期值
QList<int> DropDownMenu::getWeekDateList(QString indexString)
{
    QStringList indexList = indexString.split(" ");
    QList<int> selectWeek;
    selectWeek.clear();
    for (int i = 0; i < indexList.count(); i++) {
        selectWeek.append(indexList.at(i).toInt());
    }
    return selectWeek;
}

void DropDownMenu::handleShowLabelSlot(int id)
{
    if (id <= 2) {
        m_selectWeek.clear();
        for (int i = 0; i < this->actions().count(); ++i) {
            if (i == id) {
                continue;
            }
            QWidgetAction *wAction = dynamic_cast<QWidgetAction *>(this->actions().at(i));
            MenuItem *item = dynamic_cast<MenuItem *>(wAction->defaultWidget());
            item->setIconVisible(false);
        }
    } else {
        for (int i = 0; i <= 2; i++) {
            QWidgetAction *wAction = dynamic_cast<QWidgetAction *>(this->actions().at(i));
            MenuItem *item = dynamic_cast<MenuItem *>(wAction->defaultWidget());
            item->setIconVisible(false);
            m_selectWeek.removeAll(i);
        }
    }
    m_selectWeek.append(id);
    qSort(m_selectWeek.begin(), m_selectWeek.end());

    if (id <= 2) {
        this->hide();
    }
}

void DropDownMenu::handleHideLabelSlot(int id)
{
    m_selectWeek.removeAll(id);
    m_selectWeek.removeAll(0);
    if (m_selectWeek.isEmpty()) {
        m_selectWeek.append(0);
        QWidgetAction *wAction = dynamic_cast<QWidgetAction *>(this->actions().at(0));
        MenuItem *item = dynamic_cast<MenuItem *>(wAction->defaultWidget());
        item->setIconVisible(true);
        this->hide();
    }
}

QStringList DropDownMenu::getWeekStringList()
{
    QStringList weekStringList;
    for (int i = 0; i < m_selectWeek.size(); ++i) {
        weekStringList.append(QString().number(m_selectWeek.at(i)));
    }
    return weekStringList;
}

void DropDownMenu::initItemSelectIcon()
{
    QIcon originalIcon = QIcon::fromTheme("object-select-symbolic");
    QPixmap whiteStylePixmap = originalIcon.pixmap(16, 16);
    setItemSelectIcon(whiteStylePixmap);

    if (!QGSettings::isSchemaInstalled("org.ukui.style")) {
        qWarning() << "Style GSettings schema is not installed";
        return;
    }

    m_pStyleGSettings = new QGSettings("org.ukui.style", "/org/ukui/style/", this);
    if (m_pStyleGSettings == nullptr) {
        qWarning() << "m_pStyleGSettings is nullptr";
        return;
    }
    if (!m_pStyleGSettings->keys().contains("styleName")) {
        qWarning() << "GSettings key 'styleName' not found in available keys";
        return;
    }

    QPixmap blackStylePixmap = getDarkStylePixmap(whiteStylePixmap);

    if (m_pStyleGSettings->get("styleName").toString() == "ukui-dark") {
        setItemSelectIcon(blackStylePixmap);
    }

    connect(m_pStyleGSettings, &QGSettings::changed, this, [this, blackStylePixmap, whiteStylePixmap](const QString &key){
        if (key != "styleName") {
            return;
        }
        if (m_pStyleGSettings->get("styleName").toString() == "ukui-dark") {
            setItemSelectIcon(blackStylePixmap);
        } else {
            setItemSelectIcon(whiteStylePixmap);
        }
    });
}

void DropDownMenu::setItemSelectIcon(const QPixmap &pixmap)
{
    for (QAction *action: this->actions()) {
        QWidgetAction *widgetAction = qobject_cast<QWidgetAction*>(action);
        if (widgetAction == nullptr) {
            continue;
        }
        MenuItem *item = qobject_cast<MenuItem*>(widgetAction->defaultWidget());
        if (item == nullptr) {
            continue;
        }
        item->setSelectIcon(pixmap);
    }
}

QPixmap DropDownMenu::getDarkStylePixmap(const QPixmap &originalPixmap) const
{

    QImage image = originalPixmap.toImage();

    for (int x = 0; x < image.width(); x++) {
        for (int y = 0; y < image.height(); y++) {
            QRgb pixel = image.pixel(x, y);
            if (qAlpha(pixel) <= 0) {
                continue;
            }
            image.setPixel(x, y, qRgba(255 - qRed(pixel), 255 - qGreen(pixel), 255 - qBlue(pixel), qAlpha(pixel)));
        }
    }

    return QPixmap::fromImage(image);
}
