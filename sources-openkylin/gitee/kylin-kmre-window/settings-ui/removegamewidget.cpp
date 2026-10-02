/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn/kobe24_lixiang@126.com
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

#include "removegamewidget.h"


#include <QDBusReply>
#include <QtDBus>
#include <QDBusInterface>

#define GAME_JSON_FILE "/usr/share/kmre/games.json"

//static const QString gamesFile = "/usr/share/kmre/games.json";

RemoveGameWidget::RemoveGameWidget(QWidget *parent)
    : QWidget(parent)
    , m_vLayout(new QVBoxLayout)
    , m_refreshBtn(new QPushButton(tr("Refresh")))
    , m_scrollArea(new QScrollArea(this))
{
    QFrame *centralWidget = new QFrame;
    this->setBackgroundRole(QPalette::Base);
    this->setAutoFillBackground(true);
//    this->setStyleSheet("QWidget{background: palette(base);}");

    m_refreshBtn->setFocusPolicy(Qt::NoFocus);
    m_refreshBtn->setFixedSize(64, 32);

    QWidget *titlebar = new QWidget(this);
    titlebar->setFixedSize(400,36);
    QHBoxLayout *layout = new QHBoxLayout(titlebar);
    QLabel *appName = new QLabel;
    QLabel *pkgName = new QLabel;
    QPushButton *line = new QPushButton;
    appName->setText(tr("appName"));
    pkgName->setText(tr("pkgName"));
    line->setFixedSize(1,11);
    line->setEnabled(false);
    layout->addWidget(appName);
    layout->addSpacing(80);
    layout->addWidget(line);
    layout->addWidget(pkgName);
    layout->addStretch();
//    layout->setMargin(0);
    titlebar->setLayout(layout);

    m_view = new GameListView;
    m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_view->refreshList();
    m_view->setFrameShape(QFrame::NoFrame);
//    m_listLayout = new QVBoxLayout(m_listWidget);

    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setWidget(m_view);
    m_scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_vLayout->setSpacing(0);
    m_vLayout->setMargin(0);
    m_vLayout->addWidget(titlebar);
    m_vLayout->addWidget(m_scrollArea);
    centralWidget->setLayout(m_vLayout);
    centralWidget->setObjectName("central");
    centralWidget->setStyleSheet("QFrame#central{border: 1px solid #E9E9E9;}");
    QHBoxLayout *mainlayout = new QHBoxLayout;
    mainlayout->addWidget(centralWidget);

    this->setLayout(mainlayout);
//    this->setLayout(m_vLayout);
}

RemoveGameWidget::~RemoveGameWidget()
{
    QLayoutItem *child;
    while ((child = m_vLayout->takeAt(0)) != 0) {
        if (child->widget())
            child->widget()->deleteLater();
        delete child;
    }

}



//void RemoveGameWidget::addStretchOnTail()
//{
//    m_listLayout->addStretch();//添加弹簧
//}



//void RemoveGameWidget::reset()
//{
//    m_map.clear();
//    //去掉所有弹簧
//    for (int i = 0; i < m_listLayout->count(); ++i) {
//        QLayoutItem *layoutItem = m_listLayout->itemAt(i);
//        if (layoutItem->spacerItem()) {
//            m_listLayout->removeItem(layoutItem);
//            --i;
//        }
//    }
//    while (m_listLayout->count() > 0) {
//        QWidget *widget = m_listLayout->itemAt(0)->widget();
//        Q_ASSERT(widget);
//        m_listLayout->removeWidget(widget);
//        delete widget;
//        widget = nullptr;
//    }
//}
