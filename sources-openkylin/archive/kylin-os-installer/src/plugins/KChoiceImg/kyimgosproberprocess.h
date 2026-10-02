#ifndef KYIMGOSPROBERPROCESS_H
#define KYIMGOSPROBERPROCESS_H

#include <QObject>
#include <QThread>

namespace KInstaller {

class KyimgOsproberProcess : public QThread
{
    Q_OBJECT
public:
    explicit KyimgOsproberProcess(QObject *parent = nullptr);

protected:
    void run();

signals:
    void signalKyimgOsprober();

public slots:
   // void exec();

public:


};
}
#endif // KYIMGOSPROBERPROCESS_H
