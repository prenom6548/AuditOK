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
struct DiagnosticEntry {
    QString diagnosis_mark;
    DiagnosticItem item;
};

typedef QMap<QString, QList<DiagnosticEntry>> CategoryDiagnosticEntry;

struct CheckEntry {
    QString category;
    QString diagnosis_mark;
    qint64 entry_mark;
};

struct CheckEntryResult {
    bool ok;
    CheckEntry check;
    DiagnosticResult result;
    QMap<qint64, Database::SolutionRecord> solution_records;
};

struct RepairEntry {
    QString category;
    QString diagnosis_mark;
    qint64 entry_mark;
    QMap<qint64, Database::SolutionRecord> errors;
};

struct RepairEntryResult {
    bool ok;
    QString reason;
    RepairEntry repair;
};

}

Q_DECLARE_METATYPE(fault_diagnosis::CategoryDiagnosticEntry)
Q_DECLARE_METATYPE(fault_diagnosis::CheckEntry)
Q_DECLARE_METATYPE(fault_diagnosis::CheckEntryResult)
Q_DECLARE_METATYPE(fault_diagnosis::RepairEntry)
Q_DECLARE_METATYPE(fault_diagnosis::RepairEntryResult)

#endif
