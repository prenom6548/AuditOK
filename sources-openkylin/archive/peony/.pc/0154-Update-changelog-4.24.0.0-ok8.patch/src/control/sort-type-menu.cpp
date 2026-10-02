/*
 * Peony-Qt
 *
 * Copyright (C) 2020, Tianjin KYLIN Information Technology Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "sort-type-menu.h"
#include "global-settings.h"

#include <QDebug>

SortTypeMenu::SortTypeMenu(QWidget *parent) : QMenu(parent)
{
    auto sortTypeGroup = new QActionGroup(this);
    m_sort_types = sortTypeGroup;
    sortTypeGroup->setExclusive(true);

    auto fileName = addAction(tr("Name"));
    fileName->setCheckable(true);
    sortTypeGroup->addAction(fileName);

    auto modifiedDate = addAction(tr("Time Modified"));
    modifiedDate->setCheckable(true);
    sortTypeGroup->addAction(modifiedDate);

    auto fileType = addAction(tr("File Type"));
    fileType->setCheckable(true);
    sortTypeGroup->addAction(fileType);

    auto fileSize = addAction(tr("File Size"));
    fileSize->setCheckable(true);
    sortTypeGroup->addAction(fileSize);

    auto originalPath = addAction(tr("Original Path"));
    m_origin_path = originalPath;
    originalPath->setCheckable(true);
    sortTypeGroup->addAction(originalPath);

    m_file_path = addAction(tr("Path"));
    originalPath->setCheckable(true);
    sortTypeGroup->addAction(m_file_path);

    addSeparator();

    auto mostUsed = addAction(tr("Intelligent sort"));
    m_mostUsed = mostUsed;
    mostUsed->setCheckable(true);
    sortTypeGroup->addAction(mostUsed);

    auto addMode = addMenu(tr("Add Mode"));
    m_addMode = addMode;

    auto userAdded = addMode->addAction(tr("Manual Add Priority"));
    m_userAdded = userAdded;
    userAdded->setCheckable(true);
    sortTypeGroup->addAction(userAdded);

    auto autoAdded = addMode->addAction(tr("Conditional Screening Priority"));
    m_autoAdded = autoAdded;
    autoAdded->setCheckable(true);
    sortTypeGroup->addAction(autoAdded);

    auto onlyUserAdded = addMode->addAction(tr("Manually added only"));
    m_onlyUserAdded = onlyUserAdded;
    onlyUserAdded->setCheckable(true);
    sortTypeGroup->addAction(onlyUserAdded);

    auto onlyAutoAdded = addMode->addAction(tr("Conditional additions only"));
    m_onlyAutoAdded = onlyAutoAdded;
    onlyAutoAdded->setCheckable(true);
    sortTypeGroup->addAction(onlyAutoAdded);

    connect(sortTypeGroup, &QActionGroup::triggered, this, [=](QAction *action) {
        int index = sortTypeGroup->actions().indexOf(action);
        switchSortTypeRequest(index);
    });

    addSeparator();

    auto sortOrderGroup = new QActionGroup(this);
    m_sort_orders = sortOrderGroup;
    sortOrderGroup->setExclusive(true);

    auto descending = addAction(tr("Descending Order"));
    descending->setCheckable(true);
    sortOrderGroup->addAction(descending);

    //switch defautl Descending Ascending order, fix bug#99924
    auto ascending = addAction(tr("Ascending Order"));
    ascending->setCheckable(true);
    sortOrderGroup->addAction(ascending);

    connect(sortOrderGroup, &QActionGroup::triggered, this, [=](QAction *action) {
        int index = sortOrderGroup->actions().indexOf(action);
        switchSortOrderRequest(Qt::SortOrder(index));
    });

    addSeparator();

    auto useGlobalSortAction = new QAction(tr("Use current sorting for all folders"), this);
    useGlobalSortAction->setCheckable(true);
    useGlobalSortAction->setChecked(Peony::GlobalSettings::getInstance()->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool());
    connect(useGlobalSortAction, &QAction::triggered, this, [=](bool checked){
        Peony::GlobalSettings::getInstance()->setGSettingValue(USE_GLOBAL_DEFAULT_SORTING, checked);
        Q_EMIT globalSortingPolicyChanged();
    });

    connect(Peony::GlobalSettings::getInstance(), &Peony::GlobalSettings::valueChanged, 
            this, [=](const QString &key) {
        if (key == USE_GLOBAL_DEFAULT_SORTING) {
            bool newValue = Peony::GlobalSettings::getInstance()->getValue(USE_GLOBAL_DEFAULT_SORTING).toBool();
            if (useGlobalSortAction->isChecked() != newValue) {
                useGlobalSortAction->setChecked(newValue);
            }
        }
    });

    addAction(useGlobalSortAction);
}

void SortTypeMenu::setOriginPathVisible(bool visible)
{
    m_origin_path->setVisible(visible);
}

void SortTypeMenu::setFilePathVisible(bool visible)
{
    m_file_path->setVisible(false);//hide sort by path
}

QString SortTypeMenu::getSortTypeName(int type)
{
    QString sortTypeName = "";
    // 检查 type 是否在有效范围内
    if (type >= 0 && type < m_sort_types->actions().count()) {
        sortTypeName = QString(tr("By %1")).arg(m_sort_types->actions().at(type)->text());
    }
    return sortTypeName;
}

void SortTypeMenu::updateSortOrderName(int type)
{
    QStringList names;
    if (type == 1) {
        names.append(tr("Newest to oldest"));
        names.append(tr("Oldest to newest"));
    } else if (type == 3) {
        names.append(tr("Files from large to small"));
        names.append(tr("Files from small to large"));
    } else {
        names.append(tr("Descending Order"));
        names.append(tr("Ascending Order"));
    }
    if (m_sort_orders->actions().count() >= 2) {
        m_sort_orders->actions().at(0)->setText(names.at(0));
        m_sort_orders->actions().at(1)->setText(names.at(1));
    }
}

void SortTypeMenu::setUserAddedEnable(bool visiable)
{
    m_addMode->setEnabled(visiable);
}

bool SortTypeMenu::isAutoModeType(int type)
{
    if (type >= 0 && type < m_sort_types->actions().count()) {
        auto action = m_sort_types->actions().at(type);
        if (m_addMode->actions().contains(action)) {
            return true;
        }
    }
    return false;
}

void SortTypeMenu::setSortType(int type)
{
    // 检查 type 是否在有效范围内
    if (type >= 0 && type < m_sort_types->actions().count()) {
        m_sort_types->actions().at(type)->setChecked(true);
        qDebug() << "Setting sort type:" << type;
        if (m_sort_type != type) {
            m_sort_type = type;
        }
    } else {
        qWarning() << "Invalid sort type index:" << type << "Valid range is [0, " << m_sort_types->actions().count() - 1 << "]";
    }
}

void SortTypeMenu::setSortOrder(Qt::SortOrder order)
{
    qDebug()<<m_sort_order<<order;
    if (order < 0)
        return;
    m_sort_orders->actions().at(order)->setChecked(true);
    if (m_sort_order != order) {
        m_sort_order = order;
    }
}
