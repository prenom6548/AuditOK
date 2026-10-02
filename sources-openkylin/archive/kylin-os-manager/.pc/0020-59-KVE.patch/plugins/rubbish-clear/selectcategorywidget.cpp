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

#include "selectcategorywidget.h"
#include "utils.h"
#include "systembutton.h"
#include "custom_label.h"

#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QStyle>
#include <QStyleOption>
#include <QHBoxLayout>
//#include "window_management.hpp"

namespace KylinRubbishClear
{

SelectCategoryWidget::SelectCategoryWidget(CleanerCategoryID id, const QString &title, bool needMin, QWidget *parent)
    : KDialog(parent), m_id(id), m_mousePressed(false)
{

    this->setFixedSize(420, 224);
    setWindowIcon(QIcon::fromTheme("kylin-os-manager"));
    setWindowTitle(title);
    menuButton()->setVisible(false);
    minimumButton()->setVisible(false);

    m_listWidget = new SelectListWidget(true, this);
    //    m_listWidget->setFixedSize(this->width()-30, this->height() - 100);
    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(24, 16, 24, 24);
    m_mainLayout->addWidget(m_listWidget);
    connect(closeButton(), SIGNAL(clicked()), this, SLOT(onClose()));
    connect(m_listWidget, SIGNAL(notifyMainCheckBox(int)), this, SIGNAL(notifyMainCheckBox(int)));

    mainWidget()->setLayout(m_mainLayout);

    //边框阴影效果
    //    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    //    shadow_effect->setBlurRadius(5);
    //    shadow_effect->setColor(QColor(0, 0, 0, 127));
    //    shadow_effect->setOffset(2, 4);
    //    this->setGraphicsEffect(shadow_effect);

    //    QWidget* father = this->parentWidget();
    //    QRect rect = father->frameGeometry();
    //    this->move(rect.x()+(rect.width() - this->width())/2, rect.y()+(rect.height() - this->height())/2);

    //    QDesktopWidget* desktop = QApplication::desktop();
    //    this->move((desktop->width() - this->width())/2, (desktop->height() - this->height())/3);
}

SelectCategoryWidget::~SelectCategoryWidget() {}

void SelectCategoryWidget::onClose()
{
    qDebug() << "SelectCategoryWidget::onClose";
    Q_EMIT refreshSelectedItems(m_id, m_listWidget->getSelectedItems());
    //    qDebug() <<  m_listWidget->getSelectedItems();
    this->close();
}

void SelectCategoryWidget::loadData(const QStringList &arglist, const QStringList &statuslist, const QStringList &baklist)
{
    m_listWidget->loadListItemsWithTips(arglist, statuslist, baklist, this->width() - 48);
}

void SelectCategoryWidget::moveCenter()
{
    /*QPoint pos = QCursor::pos();
    QRect primaryGeometry;
    for (QScreen *screen : qApp->screens()) {
        if (screen->geometry().contains(pos)) {
            primaryGeometry = screen->geometry();
        }
    }

    if (primaryGeometry.isEmpty()) {
        primaryGeometry = qApp->primaryScreen()->geometry();
    }

    this->move(primaryGeometry.x() + (primaryGeometry.width() - this->width())/2,
               primaryGeometry.y() + (primaryGeometry.height() - this->height())/2);
    this->show();
    this->raise();*/
}

void SelectCategoryWidget::paintEvent(QPaintEvent *event)
{
    //    Q_UNUSED(event)
    //    QPainterPath path;
    //    path.setFillRule(Qt::WindingFill);
    //    path.addRoundRect(10,10,this->width()-20,this->height()-20,5,5);
    //    QPainter painter(this);
    //    painter.setRenderHint(QPainter::Antialiasing,true);
    //    painter.fillPath(path,QBrush(Qt::white));
    //    QColor color(0,0,0,50);
    //    for(int i = 0 ; i < 10 ; ++i)
    //    {
    //        QPainterPath path;
    //        path.setFillRule(Qt::WindingFill);
    //        path.addRoundRect(10-i,10-i,this->width()-(10-i)*2,this->height()-(10-i)*2,5,5);
    //        color.setAlpha(150 - qSqrt(i)*50);
    //        painter.setPen(color);
    //        painter.drawPath(path);
    //    }

    QWidget::paintEvent(event);
}


void SelectCategoryWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        this->m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        this->m_mousePressed = true;
    }

    QDialog::mousePressEvent(event);
}

void SelectCategoryWidget::mouseReleaseEvent(QMouseEvent *event)
{
    this->m_mousePressed = false;
    setWindowOpacity(1);

    QDialog::mouseReleaseEvent(event);
}

void SelectCategoryWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (this->m_mousePressed) {
        move(event->globalPos() - this->m_dragPosition);
        setWindowOpacity(0.9);
    }

    QDialog::mouseMoveEvent(event);
}

} // namespace KylinRubbishClear
