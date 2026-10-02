#ifndef KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_DIAGNOSES_DIAGNOSIS_H
#define KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_DIAGNOSES_DIAGNOSIS_H

#include <QtGlobal>
#include <QString>
#include <QList>
#include <QPair>
#include <QObject>
#include <QVariant>

namespace fault_diagnosis {

struct DiagnosticItem {
    QString mark;    // 诊断项的唯一标识
    QString name;
    QString describe;
};

// 以二级检测项为基础检测项
struct BaseDiagnosticItem {
    DiagnosticItem base;
    QList<DiagnosticItem> subitem;
};

struct DiagnosticCategory {
    QString category;
    QList<BaseDiagnosticItem> base_diagnostic_items;
};

struct SubitemError {
    QString mark;
    QString error_code;
    QString describe;
};

// 指定错误的诊断结果
struct SpecificErrorR {
    QString name;             // 组件名
    QString error_code;
    QString desc;             // 错误描述
    QString context;          // 携带信息
    QString can_repair;       // 修复方案类型
    QString network_state;    // 网络状态
};

struct DiagnosticResult {
    QString overview;
    QString category;
    QString base_entry_name;                 // 基础检测项的名字
    QString base_entry_mark;                 // 基础检测项的标记
    QString error_code;                      // 主检测项的错误码，如果不为空，表明该基础检测项只有主检测项
    QString describe;                        // 主检测项的错误描述，如果不为空，表明该基础检测项只有主检测项
    QList<SubitemError> details;             // 子检测项的错误
    QList<SpecificErrorR> specific_errors;    // 修复指定错误类型时的检测结果
};

struct RepairItem {
    QString error_code;
};

class Diagnosis: public QObject {
    Q_OBJECT
public:
    Diagnosis(QObject *parent = nullptr): QObject(parent) {}
    virtual ~Diagnosis() {}
    virtual QString Mark() = 0;
    virtual bool DiagnosticsList(QList<DiagnosticCategory> &diagnostic_categories, QVariant data = QVariant()) = 0;
    virtual bool Check(const QString &base_diagnostic_mark, QVariant data = QVariant()) = 0;
    virtual bool Repair(const QString &base_diagnostic_mark, const QList<RepairItem> &repair_items) = 0;

Q_SIGNALS:
    void sig_CheckProgress(QString mark, qint64 base_diagnostic_mark, qint32 progress);
    void sig_RepairProgress(QString mark, qint64 base_diagnostic_mark, qint32 progress);
    void sig_Checked(QString mark, fault_diagnosis::DiagnosticResult diagnostic_result);
    void sig_CheckFinish(QString mark, QString base_diagnostic_mark, bool ok);
    void sig_Fixed(QString mark, QString error_code, bool ok, QString reason, qint64 solution_type, QString solution_context);
    void sig_RepairFinish(QString mark, QString base_diagnostic_mark, bool ok);
};

}

#endif
