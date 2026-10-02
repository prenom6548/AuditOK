#ifndef KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_DATABASE_H
#define KYLIN_OS_MANAGER_PLUGIN_FAULT_DIAGNOSIS_DATABASE_H

#include <sqlite3.h>

namespace fault_diagnosis {

class Database {
public:
    struct SolutionRecord {
        QString error_code;
        QString solution_type;
        QString crc;
        QString cmdline;
        QString package_list;
        QString image_list;
        QString video;
        QString text;
        QString link;
        QString other;
    };

    static Database &Instance();
    static bool SolutionRecordWithErrorCode(qint64 error_code, SolutionRecord &solution_record);

private:
    static sqlite3 *sqlite3_conn_;

    Database();
    ~Database();
};

}

#endif
