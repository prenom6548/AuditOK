#ifndef KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_MAINWINDOW_H
#define KYLIN_OS_MANAGER_PLUGINS_JUNK_CLEAN_MAINWINDOW_H

#include <utility>
#include <tuple>
#include <QWidget>
#include <QPushButton>
#include <QTreeWidget>
#include <kprogressbar.h>
#include "text_label.h"
#include "defines.h"
#include "basefunc.h"

namespace junk_clean {

class CleanUpEntryWidget;
class CleanUpGroupWidget;

class MainWindow: public QWidget {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    void StartScan();
    void AlwaySelectLog(bool ok);
    bool *getCancelPtr();

Q_SIGNALS:
    void sig_CleanUpItems();
    void sig_Scan(QString mark);
    void sig_Clean(junk_clean::CleanTask);
    void sig_SwitchHomepage();
    void sig_CleanFinished(quint64 total_size, quint64 total_file_num, quint64 time);

public Q_SLOTS:
    void on_CleanUpItemsFinish(junk_clean::TypeCleanUpItems type_clean_up_items);
    void on_ScanForJunk(QString mark, junk_clean::JunkItem junk_item);
    void on_ScanFinish(QString mark, quint64 totoal_size);
    void on_CleanForJunk(QString cleaner_mark, quint64 junk_mark);
    void on_CleanFinish(QString cleaner_mark);

private Q_SLOTS:
    void on_Expand(bool is_expand) const;
    void on_JunkEntryCheckBoxStateChanged();
    void on_CleanUpEntryCheckBoxStateChanged();
    void on_StartClean();
    void on_Cancel();
    void on_SetIconForCurrentTheme();

private:
    enum class State {
        kUnknow,
        kInitial,
        kScanning,
        kCancelingScan,
        kScanFinished,
        kCleaning,
        kCancelingClean,
        kCleanFinished,
    };

    void SwitchToInitial();
    void SwitchToScanning();
    void SwitchToCancelingScan();
    void SwitchToScanFinished();
    void SwitchToCleaning();
    void SwitchToCancelingClean();
    void SwitchToCleanFinished();
    void NextScan();
    void NextClean();
    std::pair<QTreeWidgetItem *, CleanUpEntryWidget *> CleanUpEntryWithMark(const QString &mark) const;
    std::pair<QTreeWidgetItem *, CleanUpGroupWidget *> CleanUpGroupWithType(enum Type type) const;
    std::pair<QTreeWidgetItem *, CleanUpGroupWidget *> CleanUpGroupWithMark(const QString &mark) const;
    /*
     * @1: 该分类下的 总垃圾大小 / 总痕迹数
     * @2: 该分类下已选中的 垃圾大小 / 痕迹数
     */
    std::pair<quint64, quint64> JunkByteForCategory(enum Type type) const;
    /*
     * @1: 所有垃圾大小
     * @2: 已选中的垃圾大小
     * @3: 总痕迹数
     * @4: 已选中的痕迹数
     */
    std::tuple<quint64, quint64, quint64, quint64> JunkSelectState() const;
    void inserItem(QString mark);
    QList<junk_clean::JunkItem>       m_itemList;
    QPushButton *                     icon_btn_;
    K::TextLabel *                    state_label_;
    K::TextLabel *                    detail_label_;
    QPushButton *                     return_btn_;
    QPushButton *                     one_click_clean_btn_;
    QPushButton *                     cancel_btn_;
    kdk::KProgressBar *               progress_bar_;
    QTreeWidget *                     clean_up_items_widget_;
    MainWindow::State                 state_;
    bool *                            cancel_;
    quint64                           selected_size_;           // 记录扫描结束后选中的垃圾项的大小
    quint64                           selected_trace_;          // 记录扫描结束后选中的痕迹的数量
    QMap<enum Type, QList<QString>>   need_scan_;
    QMap<enum Type, QList<CleanTask>> need_clean_;

    // 用于清理完成界面
    quint64                           cleaned_size_;            // 记录已清理的总大小
    quint64                           cleaned_file_count_;      // 记录已清理的文件数量
    time_t                            cleaning_start_time_;     // 记录清理操作开始时间

    // 用于进度条
    int                               total_num_need_scan_;     // 记录需要扫描的总个数
    int                               total_num_scanned_;       // 记录已经扫描的个数
    int                               total_num_need_clean_;    // 记录需要清理的总个数
    int                               total_num_cleaned_;       // 记录已经清理的个数

    // 用于处理构建清理项界面与扫描操作的顺序问题
    bool                              init_finished_;           // 记录所有清理项界面是否已经构建完成
    bool                              scan_cache_;              // 记录请理项界面构建完成前是否进行了扫描操作

    // 高优先级的设置
    bool                              alway_select_log_;        // 默认选中日志文件

    // 其他
    QList<QString>                    cleaned_junk_;            // 记录一次清理过程中所清理掉的垃圾
};

}

#endif
