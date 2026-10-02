#include "cleaner_manager.h"
#include <QDebug>
#include "installation_package_cleaner.h"
#include "file_trace_cleaner.h"
#include "command_line_trace_cleaner.h"
#include "qax_browser_cleaner.h"
#include "log_cleaner.h"
#include "qax_browser_cookies_cleaner.h"
#include "residual_cleaner.h"
#include "system_memory_cleaner.h"
#include "trash_cleaner.h"
#include "wechat_cleaner.h"
#include "enterprise_wechat_cleaner.h"
#include "thumbnail_cleaner.h"
#include "qq_cleaner.h"
#include "boot_partition_cleaner.h"

namespace junk_clean {

CleanerManager::CleanerManager(bool *cancelPtr, QObject *parent)
    : QObject(parent),
      m_cancelPtr(cancelPtr)
{
    Register(new InstallationPackageCleaner(this));
    Register(new FileTraceCleaner(this));
    Register(new CommandLineTraceCleaner(this));
    Register(new QaxBrowserCleaner(this));
    Register(new LogCleaner(this));
    Register(new QaxBrowserCookiesCleaner(this));
    Register(new ResidualCleaner(this));
    Register(new SystemMemoryCleaner(this));
    Register(new TrashCleaner(this));
    Register(new ThumbnailCleaner(this));
    Register(new BootPartitionCleaner(this));
    Register(new QQCleaner(this));
    Register(new WechatCleaner(this));
    Register(new EnterpriseWechatCleaner(this));
}

void CleanerManager::Register(Cleaner *cleaner) {
    if (!cleaner) {
        return;
    }
    cleaner->SetCancelPtr(m_cancelPtr);
    auto mark = cleaner->Mark();
    if (cleaners_.contains(mark)) {
        throw std::runtime_error("Cleaner mark repeat!");
    }
    cleaners_.insert(mark, cleaner);
    connect(cleaner, &Cleaner::sig_ScanForJunk, this, &CleanerManager::sig_ScanForJunk);
    connect(cleaner, &Cleaner::sig_ScanFinish, this, &CleanerManager::sig_ScanFinish);
    connect(cleaner, &Cleaner::sig_CleanForJunk, this, &CleanerManager::sig_CleanForJunk);
    connect(cleaner, &Cleaner::sig_CleanFinish, this, &CleanerManager::sig_CleanFinish);
}

const QMap<QString, Cleaner *> &CleanerManager::Cleaners() const {
    return cleaners_;
}

Cleaner *CleanerManager::CleanerWithMark(const QString &mark) const {
    auto iter = cleaners_.find(mark);
    if (iter == cleaners_.end()) {
        qCritical() << "No mark " << mark << " found in cleaner manager";
        return nullptr;
    }
    return iter.value();
}

}
