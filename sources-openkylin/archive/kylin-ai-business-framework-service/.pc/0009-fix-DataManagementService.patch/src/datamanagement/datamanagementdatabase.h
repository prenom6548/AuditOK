/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DATAMANAGEMENTDATABASE_H
#define DATAMANAGEMENTDATABASE_H

#include "utils/vectordb/vectordb.h"

// pair.first: filepath, pair.second: similarity
using SimilaritySearchResult = std::vector<std::pair<std::string, double>>;

// pair.first: filepath, pair.second: modify time
using GetAllFileInfosResult = std::vector<std::pair<std::string, int>>;

struct DBFileInfo {
    DBFileInfo(const std::string& filepath,
               time_t modifyTime,
               const std::vector<std::vector<float>>& vectors)
        : filepath(filepath)
        , modifyTime(modifyTime)
        , vectors(vectors) {}

    bool empty() const {
        return (vectors.empty());
    }

    std::string filepath;
    time_t modifyTime;
    std::vector<std::vector<float>> vectors;
};
class DataManagementDatabase
{
public:
    DataManagementDatabase() = default;
    ~DataManagementDatabase();
    bool createClient();
    void destroyClient();

    SimilaritySearchResult visionSearch(const std::vector<float> &vector);
    SimilaritySearchResult textSearch(const std::vector<float> &vector);
    void addImageDatas(const std::vector<DBFileInfo> &fileinfos);
    void addTextDatas(const std::vector<DBFileInfo>& fileinfos);
    GetAllFileInfosResult getAllFileInfos();
    void deleteFiles(const std::vector<std::string> &files);

private:
    SimilaritySearchResult similaritySearch(const std::string& collection, const std::vector<float> &vector);
    void addDatas(const std::vector<DBFileInfo>& fileinfos, const std::string& collection);

    bool hasCollections(const std::vector<std::string>& collections);
    void checkAndCreateCollections(const std::vector<std::string>& collections);
    VectorData getFileDataFromFileInfo(const DBFileInfo& fileinfo) const;
    std::vector<VectorData> getVectorDatasFromFileInfo(const DBFileInfo& fileinfo, const std::string &sourceId) const;

private:
    VectorDB database_;
    bool connected_ = false;
};

#endif // DATAMANAGEMENTDATABASE_H
