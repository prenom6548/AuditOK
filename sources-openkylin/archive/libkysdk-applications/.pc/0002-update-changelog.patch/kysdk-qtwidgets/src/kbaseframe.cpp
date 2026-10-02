/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Sisheng He <hesisheng@kylinos.cn>
 *
 */
#include "kbaseframe.h"

#include "themeController.h"
#include <QApplication>
#include <QDBusReply>
#include <QPainter>
#include <QPainterPath>
#include <QtDBus/QDBusInterface>

namespace kdk
{
class KBaseFramePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KBaseFrame)
public:
    KBaseFramePrivate(KBaseFrame *parent);

protected:
    void paintEvent(QPaintEvent *e);

private:
    KBaseFrame *q_ptr;

    KBaseFrame::BorderRadiusStyle m_RadiusType;
    bool m_IsTabletMode = false;
    QDBusInterface *m_pStatusSessionDbus = nullptr;
    bool m_HeightAdaptive = false;
    bool m_IsContainer = false;

    void autoAdjustFollowTabletMode();

private Q_SLOTS:
    void mode_change_signal_slots(bool b);
};

KBaseFramePrivate::KBaseFramePrivate(KBaseFrame *parent)
    : q_ptr(parent)
{
    Q_Q(KBaseFrame);
    setParent(parent);
    q->setFrameShape(QFrame::Box);
}

void KBaseFramePrivate::autoAdjustFollowTabletMode()
{
    m_pStatusSessionDbus = new QDBusInterface("com.kylin.statusmanager.interface",
                                              "/",
                                              "com.kylin.statusmanager.interface",
                                              QDBusConnection::sessionBus(), this);
    if (m_pStatusSessionDbus->isValid()) {
        QDBusReply<bool> is_tabletmode = m_pStatusSessionDbus->call("get_current_tabletmode");
        mode_change_signal_slots(is_tabletmode.isValid() ? is_tabletmode.value() : false);
        connect(m_pStatusSessionDbus, SIGNAL(mode_change_signal(bool)), this, SLOT(mode_change_signal_slots(bool)));
    } else {
        mode_change_signal_slots(false);
        //        qWarning() << "Create com.kylin.statusmanager.interface Interface Failed When : " << QDBusConnection::systemBus().lastError();
    }
}

void KBaseFramePrivate::mode_change_signal_slots(bool b)
{
    Q_Q(KBaseFrame);
    if (m_HeightAdaptive) {
        if (b) {
            q->setMinimumSize(550, TABLET_HEIGHT);
            q->setMaximumSize(16777215, TABLET_HEIGHT);
        } else {
            q->setMinimumSize(550, PC_HEIGHT);
            q->setMaximumSize(16777215, PC_HEIGHT);
        }
    }
    m_IsTabletMode = b;
    Q_EMIT q->tabletModeChanged(b);
}

KBaseFrame::KBaseFrame(QWidget *parent, KBaseFrame::BorderRadiusStyle style, bool heightAdaptive)
    : QFrame(parent)
    , d_ptr(new KBaseFramePrivate(this))
{
    Q_D(KBaseFrame);
    setObjectName("KBaseFrame");

    d->m_RadiusType = style;
    d->m_HeightAdaptive = heightAdaptive;
    d->m_IsContainer = false;

    if (heightAdaptive) {
        d->autoAdjustFollowTabletMode();
    } else {
        d->mode_change_signal_slots(false);
    }
}

KBaseFrame::~KBaseFrame()
{
}

void KBaseFrame::setRadiusType(BorderRadiusStyle style)
{
    Q_D(KBaseFrame);
    d->m_RadiusType = style;
    update();
}

bool KBaseFrame::getTabletMode()
{
    Q_D(KBaseFrame);
    return d->m_IsTabletMode;
}

void KBaseFrame::setContainer(bool b)
{
    Q_D(KBaseFrame);
    d->m_IsContainer = b;
    if (b) {
        this->setFrameShape(QFrame::NoFrame);
    }
}

bool KBaseFrame::isContainer()
{
    Q_D(KBaseFrame);
    return d->m_IsContainer;
}

void KBaseFrame::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    Q_D(KBaseFrame);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    if (this->frameShape() != QFrame::NoFrame) {
        painter.setBrush(ThemeController::getPaletteColorFromDT("base"));
    }
    painter.setPen(Qt::transparent);
    QRect rect = this->rect();

    if (d->m_RadiusType == BorderRadiusStyle::None) {
        painter.drawRoundedRect(rect, 0, 0);
    } else {
        QPainterPath painterPath;
        painterPath.addRoundedRect(rect, RADIUS_VALUE, RADIUS_VALUE);

        painterPath.setFillRule(Qt::WindingFill);
        if (d->m_RadiusType == BorderRadiusStyle::Bottom) {
            painterPath.addRect(rect.width() - RADIUS_VALUE, 0, RADIUS_VALUE, RADIUS_VALUE);
            painterPath.addRect(0, 0, RADIUS_VALUE, RADIUS_VALUE);
        } else if (d->m_RadiusType == BorderRadiusStyle::Top) {
            painterPath.addRect(0, rect.height() - RADIUS_VALUE, RADIUS_VALUE, RADIUS_VALUE);
            painterPath.addRect(rect.width() - RADIUS_VALUE, rect.height() - RADIUS_VALUE, RADIUS_VALUE, RADIUS_VALUE);
        }
        painter.drawPath(painterPath);
    }
}
}

#include "kbaseframe.moc"
#include "moc_kbaseframe.cpp"
