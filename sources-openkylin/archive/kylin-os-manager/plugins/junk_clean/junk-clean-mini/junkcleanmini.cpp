#include "junkcleanmini.h"
#include <QDebug>
#include "basefunc.h"
#include "boot_partition_cleaner.h"

JunkCleanMini::JunkCleanMini(QString cleaner)
{
//    qRegisterMetaType<junk_clean::JunkItem>();

    if (cleaner == "boot") {
        // 初始化内核信息
        junk_clean::BaseFunc::GetKernelEnv();
         m_cleaner = new junk_clean::BootPartitionCleaner(this);
    } else {
        qDebug()<<"未知的清理项："<< cleaner;
        exit(-2);
    }
    connect(m_cleaner,&junk_clean::Cleaner::sig_ScanForJunk,this,&JunkCleanMini::on_ScanForJunk);
    connect(m_cleaner,&junk_clean::Cleaner::sig_ScanFinish,this,&JunkCleanMini::on_ScanFinish);
    connect(m_cleaner,&junk_clean::Cleaner::sig_CleanFinish,this,&JunkCleanMini::on_CleanFinish);
}

void JunkCleanMini::start()
{
    qDebug()<<"开始扫描垃圾";
    m_cleaner->Scan();
}

void JunkCleanMini::on_ScanForJunk(QString mark, junk_clean::JunkItem item)
{
    m_itemList.append(item);
}

void JunkCleanMini::on_ScanFinish(QString mark, quint64 size)
{
    qDebug()<<"扫描完成";
    QList<quint64> junk_marks;
    for (auto &junk_item :m_itemList) {
        junk_clean::KernelState kernel_state = junk_clean::BaseFunc::getKernelState(junk_item.path);
        if (kernel_state == junk_clean::KernelState::kGeneric) {
            junk_marks.append(junk_item.mark);
            qDebug()<<"选中待清理垃圾："<< junk_item.path;
        }
    }
    m_cleaner->Clean(junk_marks);
}

void JunkCleanMini::on_CleanFinish(QString mark)
{
    qDebug()<<"清理完成";
    exit(0);
}
