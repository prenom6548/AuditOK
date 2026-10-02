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

#ifndef VECTORDB_H
#define VECTORDB_H

#include <Python.h>
#include <string>
#include <vector>
#include <map>
#include <variant>

struct VectorData{
    std::string id;
    std::vector<double> embedding = {};
    std::map<std::string, std::variant<std::string, int64_t>> metadata;
};
class VectorDB
{
public:
    explicit VectorDB(const std::string& databasePath);
    ~VectorDB();
    std::string engineName() const {return "ChromaDB";}

    bool createCollection(const std::string& collectionName, const std::string& searchAlgorithm);
    std::vector<std::string> collections();
    bool addData(const std::string& collectionName, const std::vector<VectorData>& data);
    bool updateData(const std::string& collectionName, const std::vector<VectorData>& data);

    std::vector<VectorData> getData(const std::string& collectionName,
                                    const std::vector<std::string>& ids={}, const std::string& source="");

    bool deleteData(const std::string& collectionName,
                    const std::vector<std::string>& sourceIds={}, const std::vector<std::string>& ids={});

    std::vector<std::pair<VectorData, float>> queryData(const std::string& collectionName,
                                                         std::vector<std::vector<double>> embeddings,
                                                         int64_t numResults = 10);
private:
    void initializeChromaDB(const std::string& databasePath);
    std::vector<VectorData> convertPyObjectToVectorVectorData(PyObject* pyObject);

    PyObject* dbInstance_ = nullptr;
};

#endif // VECTORDB_H
