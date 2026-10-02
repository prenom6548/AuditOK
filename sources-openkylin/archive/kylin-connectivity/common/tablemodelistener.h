//
// Created by sqp on 2022/11/8.
//

#ifndef KYLIN_CONNECTIVITY_TABLEMODELISTENER_H
#define KYLIN_CONNECTIVITY_TABLEMODELISTENER_H

#include <QObject>
#include <QDBusInterface>
#include <QDebug>

const QString NORMAL = "normal";
class TableModeListener : public QObject
{
    Q_OBJECT

public:
    TableModeListener(QObject *parent = nullptr);
    ~TableModeListener();
    bool getMode();
    QString getRotations();

Q_SIGNALS:
    void sigModeChange(bool);
    void sigRotationsChange(QString);

private:
    QDBusInterface *m_modeInterface = nullptr; //平板模式切换
};


#endif // KYLIN_CONNECTIVITY_TABLEMODELISTENER_H
