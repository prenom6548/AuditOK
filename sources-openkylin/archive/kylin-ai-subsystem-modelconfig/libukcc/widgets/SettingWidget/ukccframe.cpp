#include "ukccframe.h"
#include <QPainter>
#include <QDBusReply>
#include <QDebug>
#include <QApplication>

UkccFrame::UkccFrame(QWidget *parent, BorderRadiusStyle style, bool heightAdaptive) :
    QFrame(parent),
    mRadiusType(style),
    mHeightAdaptive(heightAdaptive),
    mIscontainer(false)
{
    this->setFrameShape(QFrame::Box);
    mStatusSessionDbus = new QDBusInterface("com.kylin.statusmanager.interface",
                                             "/",
                                              "com.kylin.statusmanager.interface",
                                              QDBusConnection::sessionBus(), this);
    if (mStatusSessionDbus->isValid()) {
        QDBusReply<bool> is_tabletmode = mStatusSessionDbus->call("get_current_tabletmode");
        mode_change_signal_slots(is_tabletmode.isValid() ? is_tabletmode.value() : false);
        connect(mStatusSessionDbus, SIGNAL(mode_change_signal(bool)), this, SLOT(mode_change_signal_slots(bool)));
    } else {
        mode_change_signal_slots(false);
//        qWarning() << "Create com.kylin.statusmanager.interface Interface Failed When : " << QDBusConnection::systemBus().lastError();
    }

}

UkccFrame::~UkccFrame()
{

}

void UkccFrame::mode_change_signal_slots(bool b)
{
    if (mHeightAdaptive) {
        if (b) {
            this->setMinimumSize(550, TABLET_HEIGHT);
            this->setMaximumSize(16777215, TABLET_HEIGHT);
        } else {
            this->setMinimumSize(550, PC_HEIGHT);
            this->setMaximumSize(16777215, PC_HEIGHT);
        }
    }
    mIsTabletMode = b;
    Q_EMIT tabletModeChanged(b);
}


void UkccFrame::setRadiusType(BorderRadiusStyle style)
{
    mRadiusType = style;
    update();
}

void UkccFrame::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    if (this->frameShape() != QFrame::NoFrame) {
        painter.setBrush(qApp->palette().window());
    }
    painter.setPen(Qt::transparent);
    QRect rect = this->rect();

    if (mRadiusType == UkccFrame::None) {
        painter.drawRoundedRect(rect, 0, 0);
    } else {
        QPainterPath painterPath;
        painterPath.addRoundedRect(rect, RADIUS_VALUE, RADIUS_VALUE);

        painterPath.setFillRule(Qt::WindingFill);
        if (mRadiusType == UkccFrame::Bottom) {
            painterPath.addRect(rect.width() - RADIUS_VALUE, 0, RADIUS_VALUE, RADIUS_VALUE);
            painterPath.addRect(0, 0, RADIUS_VALUE, RADIUS_VALUE);
        } else if (mRadiusType == UkccFrame::Top) {
            painterPath.addRect(0, rect.height() - RADIUS_VALUE, RADIUS_VALUE, RADIUS_VALUE);
            painterPath.addRect(rect.width() - RADIUS_VALUE, rect.height() - RADIUS_VALUE, RADIUS_VALUE, RADIUS_VALUE);
        }
        painter.drawPath(painterPath);
    }
}
