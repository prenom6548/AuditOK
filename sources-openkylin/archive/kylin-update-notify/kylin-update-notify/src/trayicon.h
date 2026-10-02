#ifndef TRAYICON_H
#define TRAYICON_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QAction>
#include <QMenu>
#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QWidgetAction>

class trayicon : public QSystemTrayIcon
{
    Q_OBJECT
public:
    explicit trayicon(QWidget *parent = nullptr);
    void initAction();

    QAction                    *action_show;
    QMenu                      *m_trayMenu;
    QWidget                    *m_topWidget;
    QWidgetAction              *m_topWidgetAction;
    QMenu*                     trayIconMenu;
    QAction*                   OpenSetUp;
protected:
    bool eventFilter(QObject *obj, QEvent *event);

signals:

};

#endif // TRAYICON_H
