#include "clean_up_service.h"
#include "defines.h"

namespace junk_clean {

CleanUpService::CleanUpService(QObject *parent)
    : QObject(parent),
      cleaner_manager_(new CleanerManager(this))
{
    connect(cleaner_manager_, &CleanerManager::sig_ScanForJunk, this, &CleanUpService::sig_ScanForJunk);
    connect(cleaner_manager_, &CleanerManager::sig_ScanFinish, this, &CleanUpService::sig_ScanFinish);
    connect(cleaner_manager_, &CleanerManager::sig_CleanForJunk, this, &CleanUpService::sig_CleanForJunk);
    connect(cleaner_manager_, &CleanerManager::sig_CleanFinish, this, &CleanUpService::sig_CleanFinish);
}

void CleanUpService::on_CleanUpItems() {
    TypeCleanUpItems type_clean_up_items;

    const auto cleaners = cleaner_manager_->Cleaners();
    for (auto iter = cleaners.cbegin(); iter != cleaners.cend(); iter++) {
        CleanUpItem clean_up_item;
        clean_up_item.mark = iter.value()->Mark();
        clean_up_item.name = iter.value()->Name();
        clean_up_item.describe = iter.value()->Description();
        clean_up_item.level = iter.value()->Level();
        type_clean_up_items[iter.value()->Type()].push_back(clean_up_item);
    }
    Q_EMIT sig_CleanUpItemsFinish(type_clean_up_items);
}

void CleanUpService::on_Scan(QString mark) {
    auto *cleaner = cleaner_manager_->CleanerWithMark(mark);
    if (cleaner == nullptr) {
        throw std::runtime_error("Mark does not exist.");
    }

    cleaner->Scan();
}

void CleanUpService::on_Clean(junk_clean::CleanTask task) {
    auto *cleaner = cleaner_manager_->CleanerWithMark(task.cleaner_mark);
    if (cleaner == nullptr) {
        throw std::runtime_error("Mark does not exist");
    }
    cleaner->Clean(task.junk_marks);
}

}
