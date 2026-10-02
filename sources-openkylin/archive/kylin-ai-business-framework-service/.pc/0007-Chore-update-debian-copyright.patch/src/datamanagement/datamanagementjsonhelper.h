#ifndef DATAMANAGEMENTJSONHELPER_H
#define DATAMANAGEMENTJSONHELPER_H

#include <string>
#include <vector>

class DataManagementJsonHelper
{
public:
    static std::pair<std::string, double> parserSearchConditions(const std::string& searchConditionsJson);
    static std::vector<std::pair<std::string, std::string>> parserFilePathAndFormat(const std::string& fileinfos);
    static std::vector<std::string> parserDeletedFileInfos(const std::string& fileinfos);
    static std::string convertSearchResultToJson(const std::vector<std::pair<std::string, double>>& searchResult);
    static std::string convertFileInfosResultToJson(const std::vector<std::pair<std::string, int>>& fileinfos);
};

#endif // DATAMANAGEMENTJSONHELPER_H
