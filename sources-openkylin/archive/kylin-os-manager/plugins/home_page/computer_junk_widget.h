#include <tuple>
#include <utility>
#include <QWidget>
#include <QPushButton>
#include <QThread>
#include <QMap>
#include <QList>
#include <QMutex>
#include "defines.h"
#include "text_label.h"
#include "cleaner_manager.h"

namespace home_page {

class ComputerJunkWorkThread: public QThread {
    Q_OBJECT
public:
    enum class Mode {
        kScan,     // 扫描模式
        kClean,    // 清理模式
    };

    ComputerJunkWorkThread(QObject *parent = nullptr);
    ~ComputerJunkWorkThread();

    void SetMode(enum Mode mode);
    // 总大小, 系统垃圾大小, 上网垃圾大小, 使用痕迹数量
    std::tuple<quint64, quint64, quint64, quint64> GetResult();

Q_SIGNALS:
    void sig_ScanFinish();
    void sig_CleanFinish();

protected:
    void run() override;

private:
    junk_clean::CleanerManager                 cleaner_manager_;
    enum Mode                                  mode_;
    QMap<QString, QList<junk_clean::JunkItem>> scan_result_;
    QMutex                                     scan_result_mutex_;

    void Scan();
    void Clean();
};

class SizeWidget: public QWidget {
    Q_OBJECT
public:
    SizeWidget(QWidget *parent = nullptr);
    ~SizeWidget();

    void SetSize(QString size, QString unit);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString size_;    // 大小
    QString unit_;    // 单位
};

class ComputerJunkWidget: public QWidget {
    Q_OBJECT
public:
    ComputerJunkWidget(QWidget *parent = nullptr);
    ~ComputerJunkWidget();

private Q_SLOTS:
    void on_ScanFinish();
    void on_CleanFinish();
    void on_BtnClick();

private:
    enum BtnMode {
        kClean = 0,    // 一键清理
        kDeepClean,    // 深度清理
    };

    SizeWidget *           size_widget_;
    K::TextLabel *         system_junk_size_label_;
    K::TextLabel *         internet_junk_size_label_;
    K::TextLabel *         usage_traces_size_label_;
    QPushButton *          one_click_clean_btn_;
    ComputerJunkWorkThread work_thread_;

    void SetResult(quint64 total_size, quint64 system_junk_size, quint64 internet_junk_size, quint64 usage_traces);
    std::pair<QString, QString> TotalSizeForDisplay(quint64 total_size);

protected:
    void paintEvent(QPaintEvent *event) override;
};

}
