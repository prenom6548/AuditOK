/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#ifndef SEARCHTASK_H
#define SEARCHTASK_H

#include <QObject>
#include <QReadWriteLock>
#include <QSet>
#include<QTimer>
#include <QDBusInterface>
#include <QDBusReply>

#include <UkuiSearchTask>
#include "condition/condition.h"
#include "file/fileinfo.h"
#define SEARCH_LOAD_BATCH 10
class QReadWriteLock;
namespace UKUI {
namespace IDM {
class FileInfo;
}
}
class SearchTask : public QObject
{
    friend class SearchResultModel;
    friend class ModelController;
    Q_OBJECT
public:
    explicit SearchTask(QObject *parent = nullptr);
    void setSpaceId(int spaceId);
    ~SearchTask();

    const QVariant &getConditionsDescription() const;
    void setConditionsDescription(const QVariant &newConditionsDescription);
    QStringList getLabels();
    void updateSearchDirs();
    void setSearchPath(const QString path);
    void setNeedSearchFile(bool flag);
Q_SIGNALS:
    void conditionsDescriptionChanged();
    void searchFinished(bool successed);
    void searchContentFinished(bool successed);
    void searchWithLabelsFinished(bool successed);
    void finished();
    void fileReload(QString uri, QString modified, QString filetype,QString size);
    void searchLoadBatch(bool finish,bool forceUpdate = false);
public Q_SLOTS:
    void start();
    void cancel();
    void handleCancelWithSpaceId(int spaceId);
    void forceEmitResults();
protected:
    void handleSearchResult();

private:
    UkuiSearch::UkuiSearchTask *ukuiSearchTask;
    UkuiSearch::DataQueue<UkuiSearch::ResultItem> *queue;
    UkuiSearch::UkuiSearchTask *ukuiSearchContentTask;
    UkuiSearch::DataQueue<UkuiSearch::ResultItem> *contentQueue;
    QList<QSharedPointer<UKUI::IDM::FileInfo>> resultInfos;
    QList<QSharedPointer<UKUI::IDM::FileInfo>> contentResultInfos;
    QDBusInterface* m_interface;
    bool needSearchFile = false;
    bool needSearchFileContent = false;
    bool hasContentSearchAndCondition = false;

    bool searchDone = false;
    bool searchContentDone = false;
    bool cancelled = false;

    int spaceId = -1;

    QReadWriteLock rwlock;
    QVariant conditionsDescription;
    QList<std::shared_ptr<Condition>> conditions;
    QList<std::shared_ptr<Condition>> andConditions;
    QList<std::shared_ptr<Condition>> orConditions;
    QStringList labels;
    QStringList indexDirs;
    QSet<QString> filesMatchedWithLabels;
    QString currentSearchpath;
    QString m_searchKeyword;
    QTimer* m_finalTimer;           // 10秒保障定时器
    bool m_hasEmittedResults = false; // 标记是否已发送过结果
    Q_PROPERTY(QVariant conditionsDescription READ getConditionsDescription WRITE setConditionsDescription NOTIFY conditionsDescriptionChanged)
};

#endif // SEARCHTASK_H
