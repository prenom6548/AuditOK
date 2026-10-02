#ifndef UKCCSEARCHSESERVER_H
#define UKCCSEARCHSESERVER_H

#include <QObject>
#include <QDBusAbstractAdaptor>
#include "mainwindow.h"

class UkccSearchServer : public QDBusAbstractAdaptor //QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.search.interface")

public:
    explicit UkccSearchServer(MainWindow* parent, bool isDbus = false);
    virtual ~UkccSearchServer();
    inline MainWindow *parent() const;

private:
    QTimer *m_autoExitTimer;

public Q_SLOTS:
    QVariantMap getSearchItems();
};

#endif // UKCCSEARCHSEVER_H
