#ifndef KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_DEFINES_H
#define KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_DEFINES_H

#include <QMetaType>
#include <QMap>
#include "diagnosis.h"
#include "database.h"

#define SOLUTION_ENCRYPTION_DATABASE_PATH "/opt/kydoctor/common/db/solution_key.db"
#define PUSHBUTTON_ICON_STYLE "QPushButton{border:0px;border-radius:4px;background:transparent;}\
QPushButton:Hover{border:0px;border-radius:4px;background:transparent;}\
QPushButton:Pressed{border:0px;border-radius:4px;background:transparent;}"

namespace fault_diagnosis {

// 界面与 diagnosis_core 交互的数据结构
enum class Mode {
    kNormalMode,                   // 正常模式
    kRepairSpecificErrorMode,      // 修复指定错误模式
    kDiagnosisSpecificTypeMode,    // 诊断指定类型模式
};

enum class DiagnosisType {
    kNone,
    kNetwork,                     // 诊断网络
    kPerformanceAndComponents,    // 系统组件与性能
    kApplication,                 // 应用程序检测
};

enum class SpecificErrorModule {
    kNone,
    kUpgrade,     // 系统升级
    kSoftware,    // 软件商店
};

struct DiagnosticEntry {
    QString diagnosis_mark;
    BaseDiagnosticItem item;
};

using CategoryDiagnosticEntry = QHash<QString, QList<DiagnosticEntry>>;

struct CheckEntry {
    QString diagnosis_mark;
    QString entry_mark;    // 修复指定错误时，使用错误码作为基础检测项的标识
    QVariant data;         // 外带数据
};

struct RepairEntry {
    QString diagnosis_mark;
    QString entry_mark;
    QList<QString> errors;
};

struct RepairEntryResult {
    bool ok;
    QString reason;
    QString diagnosis_mark;
    QString entry_mark;
};

}

Q_DECLARE_METATYPE(fault_diagnosis::DiagnosisType)
Q_DECLARE_METATYPE(fault_diagnosis::Mode)
Q_DECLARE_METATYPE(fault_diagnosis::CategoryDiagnosticEntry)
Q_DECLARE_METATYPE(fault_diagnosis::CheckEntry)
Q_DECLARE_METATYPE(fault_diagnosis::DiagnosticResult)
Q_DECLARE_METATYPE(fault_diagnosis::RepairEntry)
Q_DECLARE_METATYPE(fault_diagnosis::RepairEntryResult)

#endif
