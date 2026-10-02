#ifndef OBTAINSYSTEMINFORMATION_H
#define OBTAINSYSTEMINFORMATION_H

#include <QObject>

typedef QPair<QString, QString> SystemInfo;

class ObtainSystemInformation : public QObject
{
    Q_OBJECT
public:
    ObtainSystemInformation();

private slots:
    void start();
    void save(QList<SystemInfo> list);
};

#endif // OBTAINSYSTEMINFORMATION_H
