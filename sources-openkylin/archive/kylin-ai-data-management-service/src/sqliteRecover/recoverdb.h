#pragma once
#include <string>

class Sqlite3Recover {
public:
    static bool checkDatabaseIntegrity(const std::string& dbPath);
    static bool recoverDatabase(const std::string& dbFilePath);

private:
    static bool replaceDatabase(const std::string& targetFilePath, const std::string& tmpFilePath);
};