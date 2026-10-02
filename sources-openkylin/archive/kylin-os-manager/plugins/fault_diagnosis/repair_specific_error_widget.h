#ifndef KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_REPAIR_SPECIFIC_ERROR_WIDGET_H
#define KYLIN_OS_MANAGER_PLUGINS_FAULT_DIAGNOSIS_REPAIR_SPECIFIC_ERROR_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QString>
#include <QPushButton>
#include "defines.h"
#include "text_label.h"

// 该控件要求布局格式统一，因为要统一清空界面重新构建，格式为：
// 主布局为垂直布局，每一行为一个具体的控件

namespace fault_diagnosis {

class RepairSpecificErrorWidget: public QWidget {
    Q_OBJECT
public:
    enum State {
        kNone,
        kDiagnosising,    // 诊断中状态
        kCancel,          // 已取消
        kRepairing,       // 修复中状态
        kResult,          // 显示结果状态
        kRepairFinish,    // 修复完成状态
    };
    enum NetworkState {
        kUnknow,
        kNormal,
        kException
    };
    struct ShowContent {
        ShowContent(): ok(false), report_issues(false), upgrade_run(false), software_run(false), junk_clean(false), network_state(NetworkState::kUnknow) {}

        bool ok;                            // 结果是否正确
        QString desc;                       // 描述信息
        bool report_issues;                 // 是否显示上报问题界面
        bool upgrade_run;                   // 是否显示升级修复脚本运行界面
        bool software_run;                  // 是否显示软件商店修复脚本运行界面
        bool junk_clean;                    // 是否显示垃圾清理界面
        enum NetworkState network_state;    // 是否显示网络状态界面
        QString error_code;                 // 错误码
        QString report_path;                // 上报日志路径
        QString data;                       // 外带数据
    };

    RepairSpecificErrorWidget(QWidget *parent = nullptr);
    ~RepairSpecificErrorWidget();

    void SetRawData(SpecificErrorModule module, QString raw_error_code);
    enum RepairSpecificErrorWidget::State State() const;
    void SetState(enum RepairSpecificErrorWidget::State state);
    void SetTitle(const QString &title) const;
    void SwitchRunning();
    void SwitchResult(const ShowContent &show_content);
    void SetRepairResult(enum SpecificErrorModule module, bool ok);

Q_SIGNALS:
    void sig_Cancel();
    void sig_Repair(QString error_code);
    void sig_Return();

private Q_SLOTS:
    void on_ProblemReport();
    void on_JunkClean();

private:
    void Clear();
    QWidget *DescWidget(bool ok, const QString &desc);
    QWidget *SuggestWidget();
    /**
     * @param has_prerepair 是否有前置修复，有前置修复和没有前置修复显示的文案不一样
     */
    QWidget *ReportIssuesWidget(bool has_prerepair);
    QWidget *UpgradeRunWidget();
    QWidget *NetworkStateWidget(enum NetworkState network_state);
    QWidget *ServiceSupportWidget();
    QWidget *SoftwareRunWidget();
    QWidget *JunkCleanWidget();
    QWidget *ReturnWidget();

    K::TextLabel *title_label_;
    QVBoxLayout *body_vbox_layout_;
    ShowContent show_content_;
    enum RepairSpecificErrorWidget::State state_;

    // 记录诊断中界面的动图加载到了哪张图片
    int current_normal_image_num_;

    // 当界面上存在运行按钮时，点击后需要显示运行结果，所以此处将所有运行界面的按钮及状态控件提取到类中，以便于更改状态
    // 系统更新运行部分
    QPushButton *upgrade_run_btn_;
    QLabel *upgrade_state_icon_label_;
    K::TextLabel *upgrade_state_text_label_;

    // 软件商店运行部分
    QPushButton *software_run_btn_;
    QLabel *software_state_icon_label_;
    K::TextLabel *software_state_text_label_;

    // 原生数据
    enum SpecificErrorModule module_;
    QString raw_error_code_;
};

}

#endif
