#include "convenient-utils.h"
#include "file-enumerator.h"
#include "file-item-model.h"
#include "file-item-proxy-filter-sort-model.h"
#include "file-info-manager.h"
#include "file-info-job.h"
#include "file-infos-job.h"
#include "file-info.h"
#include "file-utils.h"

#include <QObject>
#include <QUrl>
#include <QElapsedTimer>
#include <QDebug>

using namespace Peony;

ConvenientUtils *ConvenientUtils::global_instance = nullptr;

ConvenientUtils *ConvenientUtils::getInstance()
{
    if (!global_instance) {
        global_instance = new ConvenientUtils;
    }
    return global_instance;
}

ConvenientUtils::ConvenientUtils(QObject *parent) : QObject(parent)
{

}

QStringList ConvenientUtils::getFileUrisInSequence(const QString &uri) const
{
    QStringList orderedFileUris;
    QElapsedTimer totalTimer;
    totalTimer.start();
    QElapsedTimer stageTimer;
    stageTimer.start();

    /* uri遍历方式获取子项 */
    FileEnumerator e;
    e.setEnumerateDirectory(uri);
    e.enumerateSync();
    auto children = e.getChildren();
    qDebug() << "[getFileUrisInSequence] enumerateSync + getChildren:" << stageTimer.elapsed() << "ms, count:" << children.size();

    stageTimer.restart();
    /* 仅查询排序/过滤所需的最小属性集：
     * 去掉 metadata::* 避免 D-Bus IPC，去掉 icon/symlink/target-uri 等
     * refreshSortInfoOnly 不使用的字段，减少 g_file_query_info 单次开销 */
    static const char* SORT_ONLY_ATTRS =
        "standard::display-name,"
        "standard::type,"
        "standard::size,"
        "standard::is-hidden,"
        "standard::content-type,"
        "standard::fast-content-type,"
        "time::modified,"
        "time::created,"
        "time::access";
    std::vector<std::shared_ptr<FileInfo>> fileInfoVec(children.begin(), children.end());
    FileInfosJob infosJob(fileInfoVec);
    fileInfoVec = infosJob.batchQuerySync(SORT_ONLY_ATTRS);
    qDebug() << "[getFileUrisInSequence] batchQuerySync:" << stageTimer.elapsed() << "ms";

    stageTimer.restart();
    FileItemModel *model = new FileItemModel();
    FileItemProxyFilterSortModel *proxy_model = new FileItemProxyFilterSortModel();
    proxy_model->setSourceModel(model);

    auto fileInfo = FileInfo::fromUri(uri);
    FileInfoJob infoJob(fileInfo);
    infoJob.querySync();
    auto item = new FileItem(fileInfo, nullptr, model, nullptr);
    qDebug() << "[getFileUrisInSequence] model build + sort:" << stageTimer.elapsed() << "ms";

    model->insetFileInfoData(fileInfoVec, item);
    proxy_model->checkSettingsAndSort();
    orderedFileUris = proxy_model->getAllFileUris();

    if(model){
        delete model;
        model = nullptr;
    }
    if(proxy_model){
        delete proxy_model;
        proxy_model = nullptr;
    }
    
    qDebug() << "[getFileUrisInSequence] total:" << totalTimer.elapsed() << "ms";
    return orderedFileUris;
}

QStringList ConvenientUtils::getFilePathsInSequence(const QString &filePath) const
{
    QString encodeUri = FileUtils::urlEncode("file://" + filePath);
    QStringList orderedFileUris = getFileUrisInSequence(encodeUri);

    QStringList orderedFilePaths;
    for (QString fileUri : orderedFileUris) {
        QString filePath = QUrl(fileUri).path();
        orderedFilePaths.append(filePath);
    }
    return orderedFilePaths;
}


