//
// Created by sqp on 2022/2/28.
//

#ifndef SEARCHSERVER_H
#define SEARCHSERVER_H

#include <QObject>
#include <QMutex>
#include "fileinfo.h"

class SearchServer : public QObject
{
    Q_OBJECT
public:
    SearchServer(const QString &path, QObject *parent = nullptr);
    ~SearchServer();

Q_SIGNALS:
    void sigInitSearch();
    void sigSearchFile(const QString &);
    void sigLoadFail();
    void sigSearchResult(const QMap<QString, FileInfo> &);

public Q_SLOTS:
    void slotInitSearch();
    void slotSearchFile(const QString &keyword);


private:
    QString m_searchDbPath = "";         // 搜索数据库路径
    QMutex m_searchMutex;                // 搜索数据库操作锁
    QMap<QString, FileInfo> m_resultMap; // 搜索结果
};

#endif // SEARCHSERVER_H
