#include "burnjob.h"

#include <QDebug>
#include <QThread>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace ky_plugin_burn;
using namespace DFMBURN;

static constexpr int kPipeBufferSize {4096};
static constexpr char kMapKeyStatus[] {"status"};
static constexpr char kMapKeyProgress[] {"progress"};
static constexpr char kMapKeySpeed[] {"speed"};
static constexpr char kMapKeyMsg[] {"msg"};
static constexpr char kMapKeyPhase[] {"phase"};

AbstractBurnJob::AbstractBurnJob(const QString &dev)
    : curDev(dev)
{

}

QVariantMap AbstractBurnJob::currentDeviceInfo() const
{
    return curDeviceInfo;
}

QVariant AbstractBurnJob::property(AbstractBurnJob::PropertyType type) const
{
    return curProperty[type];
}

void AbstractBurnJob::setProperty(AbstractBurnJob::PropertyType type, const QVariant &val)
{
    curProperty[type] = val;
}


bool AbstractBurnJob::fileSystemLimitsValid()
{
    return true;
}

void AbstractBurnJob::readFunc(int progressFd, int checkFd)
{
    while (true) {

        char buf[kPipeBufferSize] { 0 };
        if (read(progressFd, buf, kPipeBufferSize) <= 0) {
            qDebug() << "progressFd break";
            break;
        } else {
            QByteArray bufByes(buf);
            qDebug() << "burn files, read bytes json:" << bufByes;
            QJsonParseError jsonError;
            QJsonObject obj { QJsonDocument::fromJson(bufByes, &jsonError).object() };

            if (jsonError.error == QJsonParseError::NoError) {

                int stat { obj[kMapKeyStatus].toInt() };
                int progress { obj[kMapKeyProgress].toInt() };
                QString &&speed = obj[kMapKeySpeed].toString();
                QJsonArray &&jsonArray = obj[kMapKeyMsg].toArray();
                int phase { obj[kMapKeyPhase].toInt() };

                if (phase != curPhase) {
                    curPhase = phase;
                    lastProgress = 0;
                    if (curPhase == JobPhase::kCheckData)
                        curJobType = JobType::kOpticalCheck;
                }
                
                QStringList msgList;
                for (int i = 0; i < jsonArray.size(); i++)
                    msgList.append(jsonArray[i].toString());

                onJobUpdated(static_cast<JobStatus>(stat), progress, speed, msgList);
            }
        }
    }

    if (lastStatus != JobStatus::kIdle)
        comfort();

    // check
    auto opts { qvariant_cast<DFMBURN::BurnOptions>(curProperty[PropertyType::kBurnOpts]) };
    auto check { opts.testFlag(BurnOption::kVerifyDatas) };
    double bad {};
    if (check && lastStatus != JobStatus::kFailed)
        read(checkFd, &bad, sizeof(bad));
    bool checkRet { !(check && (bad > (2 + 1e-6))) };

    // show result dialog
    finishFunc(check, checkRet);
}

void AbstractBurnJob::writeFunc(int progressFd, int checkFd)
{
    Q_UNUSED(progressFd)
    Q_UNUSED(checkFd)
}

void AbstractBurnJob::finishFunc(bool verify, bool verifyRet)
{
    if (lastStatus == JobStatus::kFailed) {
        jobSuccess = false;
    } else {
        jobSuccess = true;
    }
    emit burnFinished(firstJobType, jobSuccess);
}

void AbstractBurnJob::run()
{
    work();
}

bool AbstractBurnJob::readyToWork()
{
    return true;
}

void AbstractBurnJob::workingInSubProcess()
{
    int progressPipefd[2] {};
    if (pipe(progressPipefd) < 0) {
        qWarning() << "pipe failed";
        return;
    }

    int badPipefd[2] {};
    if (pipe(badPipefd) < 0) {
        qWarning() << "pipe failed";
        return;
    }

    pid_t pid = fork();
    if (pid == 0) {   // child process: working
        close(progressPipefd[0]);
        close(badPipefd[0]);

        writeFunc(progressPipefd[1], badPipefd[1]);

        close(progressPipefd[1]);
        close(badPipefd[1]);
        _exit(0);
    } else if (pid > 0) {   // parent process: wait and notify
        close(progressPipefd[1]);
        close(badPipefd[1]);

        int status;
        waitpid(-1, &status, WNOHANG);
        qDebug() << "start read child process data";
        QThread::msleep(1000);

        readFunc(progressPipefd[0], badPipefd[0]);

        close(progressPipefd[0]);
        close(badPipefd[0]);
    } else {
        qWarning() << "fork failed";
    }
}

DOpticalDiscManager *AbstractBurnJob::createManager(int fd)
{
    DOpticalDiscManager *manager = new DOpticalDiscManager(curDev);

    connect(
            manager, &DOpticalDiscManager::jobStatusChanged, this,
            [=](DFMBURN::JobStatus status, int progress, const QString &speed, const QStringList &message) {
                QByteArray bytes(updatedInSubProcess(status, progress, speed, message));
                if (bytes.size() < kPipeBufferSize) {
                    char progressBuf[kPipeBufferSize] = { 0 };
                    strncpy(progressBuf, bytes.data(), kPipeBufferSize);
                    write(fd, progressBuf, strlen(progressBuf) + 1);
                }
            },
            Qt::DirectConnection);

    return manager;
}

QByteArray AbstractBurnJob::updatedInSubProcess(JobStatus status, int progress, const QString &speed, const QStringList &message)
{
    QJsonObject obj;
    obj[kMapKeyStatus] = int(status);
    obj[kMapKeyProgress] = progress;
    obj[kMapKeySpeed] = speed;
    obj[kMapKeyMsg] = QJsonArray::fromStringList(message);
    obj[kMapKeyPhase] = curPhase;
    return QJsonDocument(obj).toJson();
}

void AbstractBurnJob::comfort()
{
    // must show %100, this psychological comfort
    auto tmp = lastStatus;
    if (lastStatus != JobStatus::kFailed) {
        for (int i = 0; i != 10; i++) {
            onJobUpdated(JobStatus::kRunning, 100, "", {});
            QThread::msleep(100);
        }
    }
    lastStatus = tmp;
}

void AbstractBurnJob::onJobUpdated(JobStatus status, int progress, const QString &speed, const QStringList &message)
{
    lastStatus = status;
   
    // update progress
    if (progress > 0 && progress <= 100 && progress >= lastProgress) {
        lastProgress = progress;
        emit burnProgressChanged(lastProgress);
    }

    // show failed dialog
    if (status == JobStatus::kFailed) {
        lastSrcMessages = message;
        emit requestLogRecorded(0, lastSrcMessages);
        return;
    }

    if(!message.isEmpty()){
        emit requestLogRecorded(1, message);
    }

}

BurnISOFilesJob::BurnISOFilesJob(const QString &dev)
    : AbstractBurnJob(dev)
{
}

bool BurnISOFilesJob::fileSystemLimitsValid()
{
    return true;
}

void BurnISOFilesJob::writeFunc(int progressFd, int checkFd)
{
    auto url { curProperty[PropertyType::KStagingUrl].toUrl() };
    auto speeds { curProperty[PropertyType::kSpeeds].toInt() };
    auto volName { curProperty[PropertyType::kVolumeName].toString() };
    auto opts { qvariant_cast<DFMBURN::BurnOptions>(curProperty[PropertyType::kBurnOpts]) };

    QString localPath { url.toLocalFile() };
    auto manager = createManager(progressFd);
    manager->setStageFile(localPath);
    curPhase = kWriteData;

    bool isSuccess { manager->commit(opts, speeds, volName) };
    qInfo() << "Burn ret: " << isSuccess << manager->lastError() << localPath;

    auto check { opts.testFlag(BurnOption::kVerifyDatas) };
    if (check && isSuccess) {
        double gud, slo, bad;
        curPhase = kCheckData;
        manager->checkmedia(&gud, &slo, &bad);
        write(checkFd, &bad, sizeof(bad));
    }
    delete manager;
}

void BurnISOFilesJob::work()
{
    qInfo() << "Start burn ISO files: " << curDev;
    firstJobType = curJobType = JobType::kOpticalBurn;
    
    //disable file system check
    if (!fileSystemLimitsValid())
        return;

    if (!readyToWork())
        return;

    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    workingInSubProcess();

    qInfo() << "End burn ISO files: " << curDev;
}


EraseJob::EraseJob(const QString &dev)
    : AbstractBurnJob(dev)
{
}

void EraseJob::work()
{
    firstJobType = curJobType = JobType::kOpticalBlank;
    if (!readyToWork())
        return;

    QScopedPointer<DOpticalDiscManager> manager { new DOpticalDiscManager(curDev) };
    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    connect(manager.data(), &DOpticalDiscManager::jobStatusChanged, this, &AbstractBurnJob::onJobUpdated, Qt::DirectConnection);

    bool ret = manager->erase();

    if (!ret)
        qWarning() << "Erase Failed: " << manager->lastError();
    qInfo() << "End erase device: " << curDev;

    comfort();

    // Due to disc don't ejected after erase, we must readlod optical info again
    //emit requestReloadDisc(curDevId);
    emit burnFinished(1,ret);
}


BurnISOImageJob::BurnISOImageJob(const QString &dev)
    : AbstractBurnJob(dev)
{
}

void BurnISOImageJob::writeFunc(int progressFd, int checkFd)
{
    auto url { curProperty[PropertyType::kImageUrl].toUrl() };
    auto speeds { curProperty[PropertyType::kSpeeds].toInt() };
    auto opts { qvariant_cast<DFMBURN::BurnOptions>(curProperty[PropertyType::kBurnOpts]) };

    QString imgPath { url.toLocalFile() };
    auto manager = createManager(progressFd);
    curPhase = kWriteData;
    bool isSuccess { manager->writeISO(imgPath, speeds) };
    qInfo() << "Burn ISO ret: " << isSuccess << manager->lastError() << imgPath;

    auto check { opts.testFlag(BurnOption::kVerifyDatas) };
    if (check && isSuccess) {
        double gud, slo, bad;
        curPhase = kCheckData;
        manager->checkmedia(&gud, &slo, &bad);
        write(checkFd, &bad, sizeof(bad));
    }
    delete manager;
}

void BurnISOImageJob::work()
{
    qInfo() << "Start burn ISO image: " << curDev;
    firstJobType = curJobType = JobType::kOpticalImageBurn;
    if (!readyToWork())
        return;
    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    workingInSubProcess();
    qInfo() << "End burn ISO image: " << curDev;
}

BurnUDFFilesJob::BurnUDFFilesJob(const QString &dev)
    : AbstractBurnJob(dev)
{
}

void BurnUDFFilesJob::writeFunc(int progressFd, int checkFd)
{
    auto url { curProperty[PropertyType::kImageUrl].toUrl() };
    auto speeds { curProperty[PropertyType::kSpeeds].toInt() };
    auto opts { qvariant_cast<DFMBURN::BurnOptions>(curProperty[PropertyType::kBurnOpts]) };

    QString imgPath { url.toLocalFile() };
    auto manager = createManager(progressFd);
    curPhase = kWriteData;
    bool isSuccess { manager->writeMultiISO(imgPath, speeds) };
    qInfo() << "Burn ISO ret: " << isSuccess << manager->lastError() << imgPath;

    auto check { opts.testFlag(BurnOption::kVerifyDatas) };
    if (check && isSuccess) {
        double gud, slo, bad;
        curPhase = kCheckData;
        manager->checkmedia(&gud, &slo, &bad);
        write(checkFd, &bad, sizeof(bad));
    }
    delete manager;
}

void BurnUDFFilesJob::work()
{
    qInfo() << "Start burn ISO image: " << curDev;
    firstJobType = curJobType = JobType::kOpticalImageBurn;
    if (!readyToWork())
        return;
    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    workingInSubProcess();
    qInfo() << "End burn ISO image: " << curDev;
}


DumpISOImageJob::DumpISOImageJob(const QString &dev)
    : AbstractBurnJob(dev)

{
}

void DumpISOImageJob::writeFunc(int progressFd, int checkFd)
{
    Q_UNUSED(checkFd)
    const auto &url { curProperty[PropertyType::kImageUrl].toUrl() };
    const QString &imagePath { url.toLocalFile() };
    auto manager = createManager(progressFd);
    curPhase = kWriteData;
    bool isSuccess { manager->dumpISO(imagePath) };
    qInfo() << "Dump ISO ret: " << isSuccess << manager->lastError() << imagePath;
    delete manager;
}

void DumpISOImageJob::finishFunc(bool verify, bool verifyRet)
{
    Q_UNUSED(verify)
    Q_UNUSED(verifyRet)
    if (lastStatus == JobStatus::kFailed || lastStatus == JobStatus::kIdle) {
        jobSuccess = false;
    } else {
        jobSuccess = true;
    }
    emit burnFinished(firstJobType, jobSuccess);
}

void DumpISOImageJob::work()
{
    qInfo() << "Start dump ISO image: " << curDev;
    firstJobType = curJobType = JobType::kOpticalImageDump;
    if (!readyToWork())
        return;
    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    workingInSubProcess();
    qInfo() << "End dump ISO image: " << curDev;
}

CreateISOJob::CreateISOJob(const QString &dev):AbstractBurnJob(dev)
{

}

bool CreateISOJob::fileSystemLimitsValid()
{
    return true;
}

void CreateISOJob::writeFunc(int progressFd, int checkFd)
{
    auto url { curProperty[PropertyType::KStagingUrl].toUrl() };
    auto speeds { curProperty[PropertyType::kSpeeds].toInt() };
    auto volName { curProperty[PropertyType::kVolumeName].toString() };
    auto opts { qvariant_cast<DFMBURN::BurnOptions>(curProperty[PropertyType::kBurnOpts]) };

    QString localPath { url.toLocalFile(), };
    auto manager = createManager(progressFd);
    manager->setStageFile(localPath,curDev);
    curPhase = kWriteData;

    bool isSuccess { manager->createISO(opts, speeds, volName) };
    qInfo() << "Burn ret: " << isSuccess << manager->lastError() << localPath;

    auto check { opts.testFlag(BurnOption::kVerifyDatas) };
    if (check && isSuccess) {
        double gud, slo, bad;
        curPhase = kCheckData;
        manager->checkmedia(&gud, &slo, &bad);
        write(checkFd, &bad, sizeof(bad));
    }
    delete manager;
}

void CreateISOJob::work()
{
    qInfo() << "Start create ISO files: " << curDev;
    firstJobType = curJobType = JobType::kOpticalBurn;

    //disable file system check
    if (!fileSystemLimitsValid())
        return;

    if (!readyToWork())
        return;

    onJobUpdated(JobStatus::kIdle, 0, {}, {});
    workingInSubProcess();

    qInfo() << "End create ISO files: " << curDev;
}
