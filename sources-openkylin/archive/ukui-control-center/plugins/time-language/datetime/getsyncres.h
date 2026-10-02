#ifndef GETSYNCRES_H
#define GETSYNCRES_H
#include <QThread>
#include <QPixmap>

class GetSyncRes : public QThread{
    Q_OBJECT
public:
    explicit GetSyncRes(QObject *parent = nullptr);
    ~GetSyncRes();
protected:
    void run() override;

Q_SIGNALS:
    void inSync(QPixmap pix);
    void syncStatus(bool status);
};
#endif // GETSYNCRES_H
