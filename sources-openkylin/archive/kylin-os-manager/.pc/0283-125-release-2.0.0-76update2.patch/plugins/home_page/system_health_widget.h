#ifndef KYLIN_OS_MANAGER_PLUGINS_HOME_PAGE_SYSTEM_HEALTH_WIDGET_H
#define KYLIN_OS_MANAGER_PLUGINS_HOME_PAGE_SYSTEM_HEALTH_WIDGET_H

#include <tuple>
#include <QWidget>
#include <QString>
#include <QThread>
#include <QMap>
#include <QMutex>
#include <QPushButton>
#include "text_label.h"

namespace home_page {

// 诊断项级别
enum class DiagnosisLevel {
    kLow,       // 低
    kMedium,    // 中
    kHigh,      // 高
};

// 诊断项分类
enum class DiagnosisCategory {
    kNetwork,             // 网络
    kSystemUpdate,        // 系统更新
    kSystemComponents,    // 系统组件
    kApplication,         // 应用程序
};

class ScoreWidget: public QWidget {
    Q_OBJECT
public:
    ScoreWidget(QWidget *parent = nullptr);
    ~ScoreWidget();

    void SetScore(int score);

protected:
    void paintEvent(QPaintEvent *evnet) override;

    int score_;
    const QString unit_text_;
};

class SystemHealthWorkThread: public QThread {
    Q_OBJECT
public:
    enum class Mode {
        kCheck,     // 检测模式
        kRepair,    // 修复模式
    };

    SystemHealthWorkThread(QObject *parent = nullptr);
    ~SystemHealthWorkThread();

    void SetMode(enum Mode mode);
    // 分数, 出错的网络分类的诊断项数量, 出错的系统故障分类的诊断项数量
    std::tuple<int, int, int> GetResult();

Q_SIGNALS:
    void sig_CheckFinish();
    void sig_RepairFinish();

protected:
    void run() override;

private:
    double low_score_;
    double medium_score_;
    double high_score_;
    QMap<QString, enum DiagnosisCategory> diagnosis_category_;    // 诊断项对应的分类 <诊断项 id, 分类>
    QMap<QString, enum DiagnosisLevel>    diagnosis_items_;       // 需要进行诊断的诊断项 <诊断项 id, 重要级别>

    enum Mode                             mode_;
    QMap<QString, QList<QString>>         diagnosis_result_;      // 存储诊断完成后的诊断结果 <诊断项 id, [错误码]>
    QMutex                                diagnosis_result_mutex_;

    void Check();
    void Repair();
    bool AutoRepair(QString error_desc);
};

class SystemHealthWidget: public QWidget {
    Q_OBJECT
public:
    SystemHealthWidget(QWidget *parent = nullptr);
    ~SystemHealthWidget();

private Q_SLOTS:
    void on_CheckFinish();
    void on_RepairFinish();
    void on_BtnClick();

private:
    enum BtnMode {
        kRepair = 0,        // 一键修复
        kDeepRepair,        // 深度修复
    };

    ScoreWidget * score_widget_;
    K::TextLabel *network_fault_num_label_;
    K::TextLabel *system_fault_num_label_;
    QPushButton * one_click_repair_btn_;

    SystemHealthWorkThread work_thread_;
};

}

#endif
