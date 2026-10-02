#ifndef BUILD_H
#define BUILD_H

#include "../fileProcess/fileprocess.h"
#include "../module/infocreatewidget.h"

#include <QProgressBar>
#include <QObject>
#include <QCoreApplication>
#include <QThread>
#include <QProcess>
#include <QDebug>
#include <QDir>

class build: public QThread
{
    Q_OBJECT
public:
    explicit build(QObject *parent = nullptr) : QThread(parent){}

protected:
    void run() override;

    bool m_iserror = false;
    QString m_errorinfo;
signals:
    void progressChanged(int value);
    void progressError(QString error);
};

#endif // BUILD_H
