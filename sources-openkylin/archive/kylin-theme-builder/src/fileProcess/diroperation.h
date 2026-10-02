#ifndef DIROPERATION_H
#define DIROPERATION_H

#include <QObject>
#include <QDebug>
#include <QDir>

class DirOperation : public QObject
{
    Q_OBJECT
public:
    explicit DirOperation(QObject *parent = nullptr);
    void DeleteHistoryDir(const QString& date);

signals:

};

#endif // DIROPERATION_H
