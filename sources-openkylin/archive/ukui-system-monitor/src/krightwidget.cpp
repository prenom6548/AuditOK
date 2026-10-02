/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "krightwidget.h"
#include "macro.h"

#include <QProcess>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>
#include <QKeyEvent>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusMessage>
#include "util.h"

#include "dialog/settingdialog.h"
#include "dialog/onclosedialog.h"

QString KRightWidget::searchText = "";

KRightWidget::KRightWidget(QWidget *parent)
    : QWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->installEventFilter(this);
    initUI();

    const QByteArray ifid(MENU_SCHEMA);
    if(QGSettings::isSchemaInstalled(ifid)) {
        m_pSystemMonitorGSetting = new QGSettings(ifid);
    }
}

KRightWidget::~KRightWidget()
{
    if (m_styleSettings) {
        delete m_styleSettings;
        m_styleSettings = nullptr;
    }
}

void KRightWidget::initUI()
{
    // 初始化布局
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(0,0,0,0);
    m_mainLayout->setSpacing(16);
    // 初始化控件
    m_stackedWidget = new QStackedWidget();
    m_mainLayout->addWidget(m_stackedWidget);

    this->setLayout(m_mainLayout);
}

void KRightWidget::addPanel(QWidget* pWidget, QString strName, QString strIcon, int nPanelIdx)
{
    Q_UNUSED(strName);
    Q_UNUSED(strIcon);
    if (!pWidget)
        return;
    int nIndex = m_stackedWidget->indexOf(pWidget);
    if (nIndex == -1) { // statckedwidget not contains pWidget
        if (nPanelIdx >= 0) {
            m_stackedWidget->insertWidget(nPanelIdx, pWidget);
        } else {
            m_stackedWidget->addWidget(pWidget);
        }
    }
}

void KRightWidget::onSwichPanel(int nIndex)
{
    if (nIndex >= m_stackedWidget->count())
        return;

    m_stackedWidget->setCurrentIndex(nIndex);
}

int KRightWidget::currentIndex()
{
    return m_stackedWidget->currentIndex();
}
