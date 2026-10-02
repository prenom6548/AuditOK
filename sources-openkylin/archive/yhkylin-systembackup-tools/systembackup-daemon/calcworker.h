#ifndef CALCWORKER_H
#define CALCWORKER_H

#include <QObject>
#include "../common/mydefine.h"

class CalcWorker : public QObject
{
    Q_OBJECT
public:
    explicit CalcWorker(QObject *parent = nullptr);
    // 计算用户目录备份所需空间大小
    qint64 calcSizeForHome();

public slots:
    // 计算备份所需空间大小
    void doWork();
};

#endif // CALCWORKER_H
