#include "burnjobmanager.h"
#include "burnjob.h"

#include <QDebug>

using namespace ky_plugin_burn;

BurnJobManager *BurnJobManager::instance()
{
    static BurnJobManager manager;
    return &manager;
}

void BurnJobManager::startEraseDisc(const QString &dev)
{
    AbstractBurnJob *job = new EraseJob(dev);
    initBurnJobConnect(job);
    job->start();
}

void BurnJobManager::startBurnISOFiles(const QString &dev, const QUrl &stagingUrl, const BurnJobManager::Config &conf)
{
    AbstractBurnJob *job = new BurnISOFilesJob(dev);
    initBurnJobConnect(job);
    job->setProperty(AbstractBurnJob::PropertyType::KStagingUrl, stagingUrl);
    job->setProperty(AbstractBurnJob::PropertyType::kSpeeds, conf.speeds);
    job->setProperty(AbstractBurnJob::PropertyType::kVolumeName, conf.volName);
    job->setProperty(AbstractBurnJob::PropertyType::kBurnOpts, QVariant::fromValue(conf.opts));
    job->start();
}

void BurnJobManager::startBurnISOImage(const QString &dev, const QUrl &imageUrl, const BurnJobManager::Config &conf)
{
    AbstractBurnJob *job = new BurnISOImageJob(dev);
    initBurnJobConnect(job);
    job->setProperty(AbstractBurnJob::PropertyType::kImageUrl, imageUrl);
    job->setProperty(AbstractBurnJob::PropertyType::kSpeeds, conf.speeds);
    job->setProperty(AbstractBurnJob::PropertyType::kBurnOpts, QVariant::fromValue(conf.opts));
    job->start();
}

void BurnJobManager::startBurnUDFFile(const QString &dev, const QUrl &imageUrl, const BurnJobManager::Config &conf)
{
    AbstractBurnJob *job = new BurnUDFFilesJob(dev);
    initBurnJobConnect(job);
    job->setProperty(AbstractBurnJob::PropertyType::kImageUrl, imageUrl);
    job->setProperty(AbstractBurnJob::PropertyType::kSpeeds, conf.speeds);
    job->setProperty(AbstractBurnJob::PropertyType::kBurnOpts, QVariant::fromValue(conf.opts));
    job->start();
}

void BurnJobManager::startCreateISOImage(const QString &dev, const QUrl &stagingUrl, const BurnJobManager::Config &conf)
{
    AbstractBurnJob *job = new CreateISOJob(dev);
    initBurnJobConnect(job);
    job->setProperty(AbstractBurnJob::PropertyType::KStagingUrl, stagingUrl);
    job->setProperty(AbstractBurnJob::PropertyType::kSpeeds, conf.speeds);
    job->setProperty(AbstractBurnJob::PropertyType::kVolumeName, conf.volName);
    job->setProperty(AbstractBurnJob::PropertyType::kBurnOpts, QVariant::fromValue(conf.opts));
    job->start();
}

void BurnJobManager::startDumpISOImage(const QString &dev, const QUrl &imageUrl)
{
    DumpISOImageJob *job = new DumpISOImageJob(dev);
    initBurnJobConnect(job);
    job->setProperty(AbstractBurnJob::PropertyType::kImageUrl, imageUrl);
    job->start();
}

void BurnJobManager::initBurnJobConnect(AbstractBurnJob *job)
{
    Q_ASSERT(job);

    connect(job, &AbstractBurnJob::finished, job, &QObject::deleteLater);
    connect(job, &AbstractBurnJob::burnFinished, this, [this, job](int type, bool result) {
        //do something
        Q_UNUSED(type);

        emit currentJobFinished(type,result);
    });
    connect(job,&AbstractBurnJob::burnProgressChanged,this,&BurnJobManager::currentJobProgressChanged);
    connect(job,&AbstractBurnJob::requestLogRecorded, this,&BurnJobManager::logMsgGenerated);
}

BurnJobManager::BurnJobManager(QObject *parent)
    : QObject(parent)
{
}

void BurnJobManager::initDumpJobConnect(AbstractBurnJob *job)
{
    Q_ASSERT(job);
    connect(job, &AbstractBurnJob::finished, job, &QObject::deleteLater);
}
