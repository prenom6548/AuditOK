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

#include "aisearchtask.h"
#include "file/fileinfo.h"
#include "file/fileinfojob.h"
#include "ai/aiconditionshelper.h"
#include "ai/ailabelhelper.h"
#include "condition/condition.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileInfo>

#include <unistd.h>
#include "model/modelcontroller.h"
#include <QDebug>

/*
 * Data Management Search Interfaces:
 *
 * SDK provides these three search interfaces:
 *
 * 1. data_management_parse_query_conditions
 *    - Parses natural language description to structured conditions
 *    - Input: {"text": "昨天的word合同"}
 *    - Output: {"conditions": [...], "logic_relation": "AND"}
 *
 * 2. data_management_search_by_query_conditions
 *    - Searches files using structured conditions (file name, type, date, etc.)
 *    - Input: {"conditions": [...], "logic_relation": "AND", "limit": 100}
 *    - Output: JSON array of file paths
 *
 * 3. data_management_natural_language_search
 *    - Direct natural language semantic search
 *    - Input: {"text": "昨天的word合同", "limit": 100}
 *    - Output: JSON array of file paths
 *
 * These functions are provided by the SDK and should be called directly.
 */

AISearchTask::AISearchTask(const QString &description, QObject *parent)
    : QObject{parent}, m_description{description}
{
    cancelled = false;
}

void AISearchTask::cancel()
{
    cancelled = true;
}

void AISearchTask::doQuery()
{
//    connect(this, &AISearchTask::finished, this, [=](bool successed, const QList<QSharedPointer<UKUI::IDM::FileInfo>> &results, const QJsonDocument &conditions, const QString &errorMsg){
//        cancel();
//    });
    QList<QString> infos;
    DataManagementSession *session = data_management_create_session();

    if (!session) {
        Q_EMIT finished(false, infos, QJsonDocument(), tr("Failed to create ai search session."));
        return;
    }

    if (cancelled) {
        data_management_destroy_session(session);
       Q_EMIT finished(false, infos, QJsonDocument(), tr("Search task cancelled."));
        return;
    }

    QJsonDocument conditionsDoc;
    if (pharseConditions) {
        AIConditionsHelper helper;
        conditionsDoc = helper.pharseConditions(m_description);
        qDebug() << conditionsDoc;
        QJsonObject conditionsObj = conditionsDoc.object();
        auto conditions = Condition::generateContitionsFromDescription(conditionsObj);
        //将条件进行转换、AI文件内容返回的type为3对应文管文件内容类型type为7
        if(conditions.size()==1 && conditions[0]->type() == Condition::DateRange){
            auto fileContentObj = conditionsObj[QString::number(0)].toObject();
            fileContentObj["condition type"] = int(Condition::FileContent);
            fileContentObj["content"] = m_description;
            fileContentObj["orCondition"] = false;
            conditionsObj[QString::number(0)] = fileContentObj;
        }
        conditions = Condition::generateContitionsFromDescription(conditionsObj);
        auto labels = Condition::getLabelsFromDescription(conditionsObj);
        bool hasConditions = false;
        bool hasLabels = false;
        bool hasContentConditions = false;
        QString contentDescription;

        hasConditions = !conditions.isEmpty();

        // 获取文件内容描述条件
        for (auto condition : conditions) {
            if (condition->type() == Condition::FileContent) {
                hasContentConditions = true;
                contentDescription = condition->getFileContentContent();
                break;
            }
        }

        hasLabels = !labels.isEmpty();

        QSet<QString> labelsMatchedInfos;
        QSet<QString> contentMatchedInfos;
        QSet<QString> basicMatchedInfos;
        QSet<QString> resultInfos;

        if (cancelled) {
            goto out;
        }

        if (hasLabels) {
            // 获取标签匹配结果
            AILabelHelper labelHelper;
            QStringList files = labelHelper.getFilesByLabels(labels);
            for (auto path : files) {
                //auto info = UKUI::IDM::FileInfo::fromPath(path);
                labelsMatchedInfos << path;
            }
        }

        if (hasContentConditions) {
            // 获取文件内容匹配结果，注意这里使用的不是普通搜索按关键字的匹配方式
            QJsonObject obj;
            obj["text"] = contentDescription;
            QJsonDocument doc(obj);
            char* searchresults = nullptr;
            data_management_similarity_search(session, doc.toJson().constData(), &searchresults);
            QJsonDocument resultDoc = QJsonDocument::fromJson(searchresults);
            data_management_destroy_result(searchresults);
            QJsonArray resultArray = resultDoc.array();
            if (cancelled) {
                goto out;
            }

            // 使用语义解析获取的条件进行过滤
            QList<std::shared_ptr<Condition>> orConditions;
            QList<std::shared_ptr<Condition>> andConditions;
            for (auto condition : conditions) {
                if (condition->isAndCondition()) {
                    andConditions << condition;
                } else {
                    orConditions << condition;
                }
            }
            for (auto item : resultArray) {
                if (cancelled) {
                    goto out;
                }
                if (item.isObject()) {
                    auto result = item.toObject();
                    auto value = result.value("filepath");
                    QString path = value.toString();
                    auto info = UKUI::IDM::FileInfo::fromPath(path);
                    if (UKUI::IDM::FileInfoJob::infoMatchCondtions(QFileInfo(path), andConditions, orConditions))
                        {
                            contentMatchedInfos << path;
                        }
                }
            }
        } else if (!hasContentConditions && hasConditions) {
            // fixme: 不包含内容描述的语义搜索，获取符合基本条件的文件结果，这里应该退化成普通搜索的流程
        }

        if (!hasConditions) {
            // 如果只有标签条件则直接返回标签结果
            infos = labelsMatchedInfos.values();
        } else {
            // 如果同时有标签条件和基础条件，则需要取各自结果的交集
            if (hasContentConditions) {
                resultInfos = contentMatchedInfos;
                if (hasLabels)
                    resultInfos = resultInfos.intersect(labelsMatchedInfos);
                infos = resultInfos.values();
            } else if (hasConditions) {
                resultInfos = basicMatchedInfos;
                if (hasLabels)
                    resultInfos = resultInfos.intersect(labelsMatchedInfos);
                infos = resultInfos.values();
            }
        }
    } else {
        // 将整段description作为token进行搜索
        QJsonObject obj;
        obj["text"] = m_description;
        QJsonDocument doc(obj);
        char* searchresults = nullptr;
        data_management_similarity_search(session, doc.toJson().constData(), &searchresults);
        QJsonDocument resultDoc = QJsonDocument::fromJson(searchresults);
        data_management_destroy_result(searchresults);
        QJsonArray resultArray = resultDoc.array();
        if (cancelled) {
            goto out;
        }

        QSet<QString> labelsMatchedFiles;
        QSet<QString> conditionsMatchedFiles;
        bool hasLabels = false;
        auto labels = Condition::getLabelsFromDescription(m_conditionDescription);
        if (!labels.isEmpty()) {
            hasLabels = true;
            AILabelHelper helper;
            QStringList filesMatchedLabels = helper.getFilesByLabels(labels);
            for (auto filePath : filesMatchedLabels) {
                labelsMatchedFiles << filePath;//UKUI::IDM::FileInfo::fromPath(filePath);
            }
            if (cancelled) {
                goto out;
            }
        }

        // 使用手动设置的条件进行过滤
        auto conditions = Condition::generateContitionsFromDescription(m_conditionDescription);
        QList<std::shared_ptr<Condition>> orConditions;
        QList<std::shared_ptr<Condition>> andConditions;
        for (auto condition : conditions) {
            if (condition->isAndCondition()) {
                andConditions << condition;
            } else {
                orConditions << condition;
            }
        }
        for (auto item : resultArray) {
            if (cancelled) {
                goto out;
            }
            if (item.isObject()) {
                auto result = item.toObject();
                auto value = result.value("filepath");
                QString path = value.toString();
                //auto info = UKUI::IDM::FileInfo::fromPath(path);
                if (UKUI::IDM::FileInfoJob::infoMatchCondtions(QFileInfo(path), andConditions, orConditions)) {
                    conditionsMatchedFiles << path;
                }
            }
        }

        if (hasLabels) {
            auto set = conditionsMatchedFiles;
            set = set.intersect(labelsMatchedFiles);
            infos = set.values();
        } else {
            infos = conditionsMatchedFiles.values();
        }
    }

out:
    data_management_destroy_session(session);

    if (cancelled) {
        infos.clear();
        //Q_EMIT finished(false, infos, QJsonDocument(), tr("Search task cancelled."));
    } else {
        Q_EMIT finished(true,infos, conditionsDoc, nullptr);
        //resultInfos = infos;
    }
}

void AISearchTask::cancelWithSpaceId(int spaceId)
{
    if (this->spaceId == spaceId) {
        cancel();
    }
}

void AISearchTask::setPharseConditions(bool newPharseConditions)
{
    pharseConditions = newPharseConditions;
}

void AISearchTask::setFilterConditionDescription(const QVariant &conditionDescription)
{
    m_conditionDescription = conditionDescription;
}

void AISearchTask::setSpaceId(int spaceId)
{
    this->spaceId = spaceId;
}
