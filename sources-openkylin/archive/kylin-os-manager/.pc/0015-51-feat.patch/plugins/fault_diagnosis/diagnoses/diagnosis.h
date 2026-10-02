#ifndef KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_DIAGNOSES_DIAGNOSIS_H
#define KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_DIAGNOSES_DIAGNOSIS_H

#include <QtGlobal>
#include <QString>
#include <QList>
#include <QPair>
#include <QObject>

// XXX: 目前程序内部使用 qint64 类型，D-Bus 接口传递的时候使用 string 类型

namespace fault_diagnosis {

struct DiagnosticItem {
    qint64 mark;    // 诊断项的唯一标识
    QString name;
    QString describe;
};

struct DiagnosticCategory {
    QString category;
    QList<DiagnosticItem> diagnostic_items;
};

struct DiagnosticError {
    qint64 error_code;
    QString name;
    QString describe;
};

struct DiagnosticResult {
    QString overview;
    QList<DiagnosticError> details;
};

struct RepairItem {
    QString error_code;
    QString solution_type;
    QString crc;
    QString cmdline;
    QString package_list;
    QString other;
};

class Diagnosis: public QObject {
    Q_OBJECT
public:
    Diagnosis(QObject *parent = nullptr): QObject(parent) {}
    virtual ~Diagnosis() {}
    virtual QString Mark() = 0;
    virtual bool DiagnosticsList(QList<DiagnosticCategory> &diagnostic_categories) = 0;
    virtual bool Check(qint64 mark, DiagnosticResult &diagnostic_result) = 0;
    virtual bool Repair(const QList<RepairItem> &repair_items) = 0;

Q_SIGNALS:
    void sig_CheckProgress(QString diagnosis_mark, qint64 item_mark, qint32 progress);
    void sig_RepairProgress(QString diagnosis_mark, qint64 item_mark, qint32 progress);
    void sig_Fixed(QString diagnosis_mark, qint64 error_code, bool ok, QString reason);
};

}

#endif
