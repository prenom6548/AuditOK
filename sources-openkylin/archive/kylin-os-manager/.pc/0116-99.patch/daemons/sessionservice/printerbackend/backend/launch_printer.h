#ifndef LAUNCH_PRINTER_H
#define LAUNCH_PRINTER_H

#include <QObject>
#include <QThread>
#include <QString>


class LaunchPrinter : public QObject
{
    Q_OBJECT
private:
    LaunchPrinter(/* args */);
private slots:
    void actionInvokeSlot(quint32 id, QString actionkey);
public:
    static LaunchPrinter *getInstance();
    ~LaunchPrinter();
    void add (const QString &);
    void remove (const QString &);
    void add_uri (const QString &);
    void findPrePrinter();
};



#endif // LAUNCH_PRINTER_H
