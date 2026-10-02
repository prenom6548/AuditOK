#ifndef BURNJOB_H
#define BURNJOB_H

#include "dopticaldiscmanager.h"
#include <QThread>
#include <QVariantMap>

namespace ky_plugin_burn {

class AbstractBurnJob : public QThread
{
    Q_OBJECT

public:

    enum JobType {
        kOpticalBurn,
        kOpticalBlank,
        kOpticalImageBurn,
        kOpticalCheck,
        kOpticalImageDump
    };

    enum PropertyType {
        KStagingUrl,
        kImageUrl,
        kVolumeName,
        kSpeeds,
        kBurnOpts
    };

    enum JobPhase {
        kReady,
        kWriteData,
        kCheckData
    };

    using ProperyMap = QMap<PropertyType, QVariant>;

public:
    explicit AbstractBurnJob(const QString &dev);
    virtual ~AbstractBurnJob() override {}

    QVariantMap currentDeviceInfo() const;
    QVariant property(PropertyType type) const;
    void setProperty(PropertyType type, const QVariant &val);

protected:
    virtual bool fileSystemLimitsValid();
    virtual void readFunc(int progressFd, int checkFd);
    virtual void writeFunc(int progressFd, int checkFd);
    virtual void finishFunc(bool verify, bool verifyRet);
    virtual void work() = 0;

    void run() override;

    bool readyToWork();
    void workingInSubProcess();

    DFMBURN::DOpticalDiscManager *createManager(int fd);
    QByteArray updatedInSubProcess(DFMBURN::JobStatus status, int progress, const QString &speed, const QStringList &message);
    void comfort();

Q_SIGNALS:
    void requestErrorMessageDialog(const QString &title, const QString &message);
    void requestLogRecorded(int type, const QStringList &details);
    void requestCompletionDialog(const QString &msg, const QString &icon);
    void requestReloadDisc(const QString &devId);
    void burnFinished(int type, bool reuslt);
    void burnProgressChanged(int currentValue);

public slots:
    void onJobUpdated(DFMBURN::JobStatus status, int progress, const QString &speed, const QStringList &message);

protected:
    QString curDev;
    QString curDevId;
    QVariantMap curDeviceInfo;
    ProperyMap curProperty;
    JobType firstJobType;
    JobType curJobType;
    int lastProgress {};
    int curPhase {};
    QString lastError;
    QStringList lastSrcMessages;
    DFMBURN::JobStatus lastStatus;
    bool jobSuccess {};   // delete staging files if sucess
};

class BurnISOFilesJob : public AbstractBurnJob
{
    Q_OBJECT

public:
    explicit BurnISOFilesJob(const QString &dev);
    virtual ~BurnISOFilesJob() override {}

protected:
    virtual bool fileSystemLimitsValid() override;
    virtual void writeFunc(int progressFd, int checkFd) override;
    virtual void work() override;
};

class CreateISOJob : public AbstractBurnJob
{
    Q_OBJECT

public:
    explicit CreateISOJob(const QString &dev);
    virtual ~CreateISOJob() override {}

protected:
    virtual bool fileSystemLimitsValid() override;
    virtual void writeFunc(int progressFd, int checkFd) override;
    virtual void work() override;
};

class BurnISOImageJob : public AbstractBurnJob
{
    Q_OBJECT

public:
    explicit BurnISOImageJob(const QString &dev);
    virtual ~BurnISOImageJob() override {}

protected:
    virtual void writeFunc(int progressFd, int checkFd) override;
    virtual void work() override;
};


class BurnUDFFilesJob : public AbstractBurnJob
{
    Q_OBJECT

public:
    explicit BurnUDFFilesJob(const QString &dev);
    virtual ~BurnUDFFilesJob() override {}

protected:
    virtual void writeFunc(int progressFd, int checkFd) override;
    virtual void work() override;
};

class EraseJob : public AbstractBurnJob
{
    Q_OBJECT

public:
    explicit EraseJob(const QString &dev);
    virtual ~EraseJob() override {}

protected:
    virtual void work() override;
};

class DumpISOImageJob : public AbstractBurnJob
{
    Q_OBJECT

public:
    explicit DumpISOImageJob(const QString &dev);
    virtual ~DumpISOImageJob() override {}

Q_SIGNALS:
    void requestOpticalDumpISOSuccessDialog(const QUrl &imageUrl);
    void requestOpticalDumpISOFailedDialog();

protected:
    virtual void writeFunc(int progressFd, int checkFd) override;
    virtual void finishFunc(bool verify, bool verifyRet) override;
    virtual void work() override;
};
}

// namespace ky_plugin_burn
Q_DECLARE_METATYPE(DFMBURN::BurnOptions)

#endif   // BURNJOB_H
