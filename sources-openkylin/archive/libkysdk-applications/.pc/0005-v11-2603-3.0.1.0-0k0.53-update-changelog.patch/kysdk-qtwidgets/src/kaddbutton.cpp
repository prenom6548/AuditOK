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
#include "kaddbutton.h"
#include "themeController.h"
#include <QApplication>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDebug>
#include <QEvent>
#include <QGSettings/QGSettings>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>

#define TOP_LEFT_CORNER 1
#define TOP_RIGHT_CORNER 2
#define BOTTOM_RIGHT_CORNER 4
#define BOTTOM_LEFT_CORNER 8

namespace kdk
{
class KAddButtonPrivate : public QObject, public ThemeController
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(KAddButton)
public:
    KAddButtonPrivate(KAddButton *parent);

private:
    KAddButton *q_ptr;

    KBaseFrame::BorderRadiusStyle m_radiusType;
    bool m_isTabletMode = false;
    QDBusInterface *m_statusSessionDbus = nullptr;
    bool m_heightAdaptive = false;

private:
    QPainterPath getPaintrPath();

private Q_SLOTS:
    void mode_change_signal_slots(bool b);
};

KAddButtonPrivate::KAddButtonPrivate(KAddButton *parent)
    : q_ptr(parent)
{
    Q_Q(KAddButton);
    setParent(parent);

    QHBoxLayout *addLyt = new QHBoxLayout(q);

    QLabel *iconLabel = new QLabel(q);
    QLabel *textLabel = new QLabel(tr("Add"));

    QIcon mAddIcon = QIcon::fromTheme("list-add-symbolic");
    iconLabel->setPixmap(mAddIcon.pixmap(mAddIcon.actualSize(QSize(16, 16))));
    iconLabel->setProperty("iconHighlightEffectMode", 1);
    addLyt->addStretch();
    addLyt->addWidget(iconLabel);
    addLyt->addWidget(textLabel);
    addLyt->addStretch();
    q->setLayout(addLyt);

    if (ThemeController::themeMode() == DarkTheme)
        iconLabel->setProperty("useIconHighlightEffect", true);

    connect(m_gsetting, &QGSettings::changed, this, [=](const QString &key) {
        if (ThemeController::themeMode() == LightTheme)
            iconLabel->setProperty("useIconHighlightEffect", false);
        else
            iconLabel->setProperty("useIconHighlightEffect", true);
    });
}

void KAddButtonPrivate::mode_change_signal_slots(bool b)
{
    Q_Q(KAddButton);
    if (m_heightAdaptive) {
        if (b) {
            q->setMinimumSize(550, TABLET_HEIGHT);
            q->setMaximumSize(16777215, TABLET_HEIGHT);
        } else {
            q->setMinimumSize(550, PC_HEIGHT);
            q->setMaximumSize(16777215, PC_HEIGHT);
        }
    }
    m_isTabletMode = b;
    Q_EMIT q->tabletModeChanged(b);
}

KAddButton::KAddButton(QWidget *parent, KBaseFrame::BorderRadiusStyle style, bool heightAdaptive)
    : QPushButton(parent)
    , d_ptr(new KAddButtonPrivate(this))
{
    Q_D(KAddButton);
    setObjectName("AddButton");
    setProperty("useButtonPalette", true);
    setProperty("needTranslucent", true);
    setFlat(true);

    d->m_radiusType = style;
    d->m_heightAdaptive = heightAdaptive;
    d->m_statusSessionDbus = new QDBusInterface("com.kylin.statusmanager.interface",
                                                "/",
                                                "com.kylin.statusmanager.interface",
                                                QDBusConnection::sessionBus(), this);
    if (d->m_statusSessionDbus->isValid()) {
        QDBusReply<bool> is_tabletmode = d->m_statusSessionDbus->call("get_current_tabletmode");
        d->mode_change_signal_slots(is_tabletmode.isValid() ? is_tabletmode.value() : false);
        connect(d->m_statusSessionDbus, SIGNAL(mode_change_signal(bool)), this, SLOT(mode_change_signal_slots(bool)));
    } else {
        d->mode_change_signal_slots(false);
        qWarning() << "Create com.kylin.statusmanager.interface Interface Failed When : " << QDBusConnection::systemBus().lastError();
    }
}

KAddButton::~KAddButton()
{
}

void KAddButton::setRadiusType(KBaseFrame::BorderRadiusStyle style)
{
    Q_D(KAddButton);
    d->m_radiusType = style;
    update();
}

bool KAddButton::getTabletMode()
{
    Q_D(KAddButton);
    return d->m_isTabletMode;
}

void KAddButton::setHeightAdaptive(bool flag)
{
    Q_D(KAddButton);
    d->m_heightAdaptive = flag;
}

void KAddButton::paintEvent(QPaintEvent *event)
{
    Q_D(KAddButton);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);
    //    painter.setBrush(qApp->palette().base());
    painter.setBrush(ThemeController::getPaletteColorFromDT("base-active"));
    QPainterPath painterPath = d->getPaintrPath();
    painter.setClipPath(painterPath);
    painter.drawPath(painterPath);

    QPushButton::paintEvent(event);
}

QPainterPath KAddButtonPrivate::getPaintrPath()
{
    Q_Q(KAddButton);
    const QRect rect = q->rect();
    int radius = RADIUS_VALUE;
    int corner = 0;

    if (m_radiusType == KBaseFrame::BorderRadiusStyle::None) {
        corner = 0;
    } else if (m_radiusType == KBaseFrame::BorderRadiusStyle::Top) {
        corner = TOP_LEFT_CORNER + TOP_RIGHT_CORNER;
    } else if (m_radiusType == KBaseFrame::BorderRadiusStyle::Bottom) {
        corner = BOTTOM_LEFT_CORNER + BOTTOM_RIGHT_CORNER;
    } else {
        corner = TOP_LEFT_CORNER + TOP_RIGHT_CORNER + BOTTOM_LEFT_CORNER + BOTTOM_RIGHT_CORNER;
    }

    QPainterPath path;
    if (corner & TOP_LEFT_CORNER) {
        path.moveTo(rect.x() + radius, rect.y());
        path.arcTo(rect.x(), rect.y(), radius * 2, radius * 2, 90.f, 90.f); // 画左上圆角
    } else {
        path.moveTo(rect.x(), rect.y());
    }

    if (corner & BOTTOM_LEFT_CORNER) {
        path.lineTo(rect.x(), rect.y() + rect.height() - radius);
        path.arcTo(rect.x(), rect.y() + rect.height() - radius * 2, radius * 2, radius * 2, 180.0f, 90.f);
    } else {
        path.lineTo(rect.x(), rect.y() + rect.height());
    }

    if (corner & BOTTOM_RIGHT_CORNER) {
        path.lineTo(rect.x() + rect.width() - radius, rect.y() + rect.height());
        path.arcTo(rect.x() + rect.width() - 2 * radius, rect.y() + rect.height() - 2 * radius, radius * 2, radius * 2, 270.0f, 90.f);
    } else {
        path.lineTo(rect.x() + rect.width(), rect.y() + rect.height());
    }

    if (corner & TOP_RIGHT_CORNER) {
        path.lineTo(rect.x() + rect.width(), rect.y() + radius);
        path.arcTo(rect.x() + rect.width() - 2 * radius, rect.y(), radius * 2, radius * 2, 0.f, 90.f);
    } else {
        path.lineTo(rect.x() + rect.width(), rect.y());
    }
    return path;
}
}

#include "kaddbutton.moc"
#include "moc_kaddbutton.cpp"
