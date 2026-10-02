#ifndef BURNJOBMANAGER_H
#define BURNJOBMANAGER_H

#include "dopticaldiscmanager.h"

#include <QObject>

namespace ky_plugin_burn {

class AbstractBurnJob;
class BurnJobManager : public QObject
{
    Q_OBJECT
public:
    struct Config
    {
        QString volName;   // It's empty for the burned ISO image
        int speeds;
        DFMBURN::BurnOptions opts;
    };

public:
    static BurnJobManager *instance();

    void startEraseDisc(const QString &dev);
    void startBurnISOFiles(const QString &dev, const QUrl &stagingUrl, const Config &conf);
    void startBurnISOImage(const QString &dev, const QUrl &imageUrl, const Config &conf);
    void startDumpISOImage(const QString &dev, const QUrl &imageUrl);
    void startBurnUDFFile(const QString &dev, const QUrl &imageUrl, const BurnJobManager::Config &conf);
    void startCreateISOImage(const QString &dev, const QUrl &stagingUrl, const Config &conf);

private:
    void initBurnJobConnect(AbstractBurnJob *job);
    void initDumpJobConnect(AbstractBurnJob *job);

Q_SIGNALS:
    void currentJobFinished(int jobType, bool result);
    void currentJobProgressChanged(int currentValue);
    void logMsgGenerated(int level, QStringList msgList);

private:
    explicit BurnJobManager(QObject *parent = nullptr);
};

}

#endif   // BURNJOBMANAGER_H
