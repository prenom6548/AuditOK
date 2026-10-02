#include <string.h>
#include <sstream>
#include <iomanip>
#include <QDebug>
#include "defines.h"
#include "database.h"

namespace fault_diagnosis {

sqlite3 *Database::sqlite3_conn_ = NULL;

Database::Database() {
    if (sqlite3_open(SOLUTION_ENCRYPTION_DATABASE_PATH, &sqlite3_conn_) == SQLITE_OK) {
        sqlite3_key(sqlite3_conn_, "kydoctor123", 11);
    } else {
        qCritical() << "create encryption database connection fail: " << sqlite3_errmsg(sqlite3_conn_);
        sqlite3_conn_ = NULL;
    }
}

Database::~Database() {

}

Database &Database::Instance() {
    static Database self;
    return self;
}

static int RowCallback(void *data, int argc, char **argv, char **column) {
    Database::SolutionRecord *solution_record = (Database::SolutionRecord *)data;

    for (int i = 0; i < argc; i++) {
        if (!strcmp(column[i], "errocode")) {
            solution_record->error_code = argv[i];
        } else if (!strcmp(column[i], "solotioncode")) {
            solution_record->solution_type = argv[i];
        } else if (!strcmp(column[i], "crccode")) {
            solution_record->crc = argv[i];
        } else if (!strcmp(column[i], "cmdline")) {
            solution_record->cmdline = argv[i];
        } else if (!strcmp(column[i], "packagelist")) {
            solution_record->package_list = argv[i];
        } else if (!strcmp(column[i], "imagelist")) {
            solution_record->image_list = argv[i];
        } else if (!strcmp(column[i], "video")) {
            solution_record->video = argv[i];
        } else if (!strcmp(column[i], "text")) {
            solution_record->text = argv[i];
        } else if (!strcmp(column[i], "link")) {
            solution_record->link = argv[i];
        } else if (!strcmp(column[i], "other")) {
            solution_record->other = argv[i];
        } else {
            qWarning() << "Unknown column name.";
        }
    }

    return SQLITE_OK;
}

bool Database::SolutionRecordWithErrorCode(qint64 error_code, SolutionRecord &solution_record) {
    if (sqlite3_conn_ == NULL) {
        return false;
    }

    std::stringstream ss;
    ss << "0x" << std::setw(16) << std::setfill('0') << std::hex << error_code;

    std::stringstream sql;
    sql << "SELECT * FROM solution WHERE errocode=" << "'" << ss.str() << "'";

    char *error = NULL;
    if (sqlite3_exec(sqlite3_conn_, sql.str().c_str(), RowCallback, &solution_record, &error) != SQLITE_OK) {
        qCritical() << "Get solution record with error code fail: " << (error == NULL? "": error);
        return false;
    }

    return true;
}

}
