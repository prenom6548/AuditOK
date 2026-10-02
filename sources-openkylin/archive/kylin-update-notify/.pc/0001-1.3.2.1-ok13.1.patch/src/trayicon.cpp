#include "trayicon.h"

trayicon::trayicon(QWidget *parent) : QSystemTrayIcon(parent)
{
    initAction();
}

void trayicon::initAction()
{
    this->setToolTip(tr("The system is updating silently"));
    this->setIcon(QIcon(":/update.png"));

    m_trayMenu = new QMenu();
    m_topWidget = new QWidget();
    m_topWidget->setFixedSize(30,30);
    m_topWidget->installEventFilter(this);
    m_topWidgetAction = new QWidgetAction(this);

}

/**
 * @brief trayicon::eventFilter
 * 事件过滤器
 */
bool trayicon::eventFilter(QObject *obj, QEvent *event)
{
//    if (obj == m_topWidget && event->type() == QEvent::MouseButtonPress)
//    {
//        QPainter painter(m_topWidget);
//        painter.setPen(Qt::NoPen);
//        painter.setBrush(QColor(42,120,192));
//        painter.drawRect(m_topWidget->rect());
//    }
    return QSystemTrayIcon::eventFilter(obj,event);
}
