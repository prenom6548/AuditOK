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

#include "vectordb.h"
#include "utils/pythonthreadlocker.h"
#include "utils/pythonutil.h"

#include <Python.h>
#include <iostream>
#include <variant>

VectorDB::VectorDB(const std::string& databasePath)
{
    std::cout << "Initializing VectorDB..." << std::endl;
    initializeChromaDB(databasePath);
}

VectorDB::~VectorDB() {
    PythonThreadLocker locker;
    if (dbInstance_ != nullptr) {
        Py_XDECREF(dbInstance_);
    }
}

void VectorDB::initializeChromaDB(const std::string& databasePath) {
    // 初始化Python解释器
    if (!Py_IsInitialized())
        Py_Initialize();

    PythonThreadLocker locker;
    // 加载所需Python文件所在的路径
    PyRun_SimpleString("import sys");
    std::string command = "sys.path.append('" + std::string(DATA_MANAGEMENT_PYTHON_PATH) + "')";
    PyRun_SimpleString(command.c_str());

    // 导入模块
    std::cout << "Creating ChromaDB instance..." << std::endl;
    PyObject* pModule = PyImport_ImportModule("chroma_db");
    if (nullptr == pModule) {
        std::cerr << "Failed to load module: chroma_db" << std::endl;
        PyErr_Print();
        return;
    }

    // 获取ChromaDB类
    PyObject* pChromaDBClass = PyObject_GetAttrString(pModule, "ChromaDB");
    if (nullptr == pChromaDBClass) {
        std::cerr << "Failed to get ChromaDB class" << std::endl;
        PyErr_Print();

        Py_XDECREF(pModule);
        return;
    }

    // 创建ChromaDB实例
    PyObject* args = Py_BuildValue("(s)", databasePath.c_str());
    if(nullptr == args) {
        std::cerr << "Error: Could not create argument tuple." << std::endl;
        PyErr_Print();

        Py_XDECREF(pModule);
        Py_XDECREF(pChromaDBClass);
        return;
    }
    dbInstance_ = PyObject_CallObject(pChromaDBClass, args);

    Py_XDECREF(pModule);
    Py_XDECREF(args);
    Py_XDECREF(pChromaDBClass);
}

bool VectorDB::createCollection(const std::string& collectionName,  const std::string& searchAlgorithm)
{
    PythonThreadLocker locker;
    if (nullptr == dbInstance_) {
        std::cerr << "ChromaDB instance is not initialized." << std::endl;
        return false;
    }

    PyObject* args = Py_BuildValue("(ss)", collectionName.c_str(), searchAlgorithm.c_str());
    if (nullptr == args) {
        std::cerr << "Could not create argument tuple." << std::endl;
        PyErr_Print();
        return false;
    }

    PyObject* result = PyObject_CallMethod(dbInstance_, "get_or_create_collection", "O", args);
    if (nullptr == result) {
        std::cerr << "Failed to call method: get_or_create_collection" << std::endl;
        PyErr_Print();
        Py_XDECREF(args);
        return false;
    }

    Py_XDECREF(args);
    Py_XDECREF(result);
    return true;
}

std::vector<std::string> VectorDB::collections()
{
    PythonThreadLocker locker;
    std::vector<std::string> collectionLists={};
    if (nullptr == dbInstance_) {
        std::cerr << "ChromaDB instance is not initialized." << std::endl;
        return collectionLists;
    }
    PyObject* func = PyObject_GetAttrString(dbInstance_, "list_collections");
    if (nullptr == func) {
        std::cerr << "Could not find function 'list_collections'" << std::endl;
        return collectionLists;
    }

    PyObject* result = PyObject_CallObject(func, NULL);\
    if (nullptr == result) {
        std::cerr << "Failed to call method: list_collections" << std::endl;
        PyErr_Print();
        Py_XDECREF(func);
        return collectionLists;
    }
    collectionLists = pythonutil::convertPythonListToStringVector(result);

    Py_XDECREF(func);
    Py_XDECREF(result);
    return collectionLists;
}

bool VectorDB::addData(const std::string& collectionName, const std::vector<VectorData>& data)
{
    PythonThreadLocker locker;
    std::cout << "Adding data to ChromaDB..." << std::endl;
    if (nullptr == dbInstance_) {
        std::cerr << "ChromaDB instance is not initialized." << std::endl;
        return false;
    }

    if (data.empty()) {
        std::cerr << "Attempted to addData with an empty data vector." << std::endl;
        return false;
    }

    std::vector<std::string> ids;
    std::vector<std::vector<double>> embeddings;
    std::vector<std::map<std::string, std::variant<std::string, int64_t>>> metadatas;
    std::vector<std::string> documents;

    for (int i = 0; i < data.size(); i++) {
        ids.push_back(data[i].id);
        embeddings.push_back(data[i].embedding);
        metadatas.push_back(data[i].metadata);
        auto it = data[i].metadata.find("source");
        if (it != data[i].metadata.end()) {
            documents.push_back(std::get<std::string>(it->second));
        } else {
            documents.emplace_back(std::string()); // documents 的长度需要和其他参数相等
        }
    }

    PyObject* pyIds = pythonutil::convertVectorStringToPyObject(ids);
    PyObject* pyEmbeddings = pythonutil::convertVectorVectorDoubleToPyObject(embeddings);
    PyObject* pyMetadatas = pythonutil::convertVectorMapToPyObject(metadatas);
    PyObject* pyDocuments = pythonutil::convertVectorStringToPyObject(documents);

    // 创建包含所有参数的元组
    PyObject* tuple = PyTuple_New(5);
    PyTuple_SET_ITEM(tuple, 0, PyUnicode_FromString(collectionName.c_str()));
    PyTuple_SET_ITEM(tuple, 1, pyIds);
    PyTuple_SET_ITEM(tuple, 2, pyMetadatas);
    PyTuple_SET_ITEM(tuple, 3, pyEmbeddings);
    PyTuple_SET_ITEM(tuple, 4, pyDocuments);

    PyObject* func = PyObject_GetAttrString(dbInstance_, "add_data");
    if (nullptr == func) {
        std::cerr << "Could not find function 'add_data'" << std::endl;
        PyErr_Print();
        Py_XDECREF(tuple);
        return false;
    }

    PyObject* result = PyObject_CallObject(func, tuple);
    if (nullptr == result) {
        std::cerr << "Failed to call method: add_data" << std::endl;
        PyErr_Print();
        Py_XDECREF(tuple);
        Py_XDECREF(func);
        return false;
    }

    Py_XDECREF(tuple);
    Py_XDECREF(result);
    Py_XDECREF(func);

    return true;
}

bool VectorDB::updateData(const std::string& collectionName, const std::vector<VectorData>& data)
{
    PythonThreadLocker locker;
    if (nullptr == dbInstance_) {
        std::cerr << "ChromaDB instance is not initialized." << std::endl;
        return false;
    }

    if (data.empty()) {
        std::cerr << "Warning: Attempted to updateData with an empty data vector." << std::endl;
        return false;
    }
    std::vector<std::string> ids;
    std::vector<std::vector<double>> embeddings;
    std::vector<std::map<std::string, std::variant<std::string, int64_t>>> metadatas;
    std::vector<std::string> documents;

    for (int i = 0; i < data.size(); i++) {
        ids.push_back(data[i].id);
        embeddings.push_back(data[i].embedding);
        metadatas.push_back(data[i].metadata);
        auto it = data[i].metadata.find("source");
        if (it != data[i].metadata.end()) {
            documents.push_back(std::get<std::string>(it->second));
        } else {
            documents.emplace_back(std::string()); // documents 的长度需要和其他参数相等
        }
    }

    PyObject* pyIds = pythonutil::convertVectorStringToPyObject(ids);
    PyObject* pyEmbeddings = pythonutil::convertVectorVectorDoubleToPyObject(embeddings);
    PyObject* pyMetadatas = pythonutil::convertVectorMapToPyObject(metadatas);
    PyObject* pyDocuments = pythonutil::convertVectorStringToPyObject(documents);

    // 创建包含所有参数的元组
    PyObject* tuple = PyTuple_New(5);
    PyTuple_SET_ITEM(tuple, 0, PyUnicode_FromString(collectionName.c_str()));
    PyTuple_SET_ITEM(tuple, 1, pyIds);
    PyTuple_SET_ITEM(tuple, 2, pyMetadatas);
    PyTuple_SET_ITEM(tuple, 3, pyEmbeddings);
    PyTuple_SET_ITEM(tuple, 4, pyDocuments);

    PyObject* func = PyObject_GetAttrString(dbInstance_, "update_data");
    if (nullptr == func) {
        std::cerr << "Could not find function 'update_data'" << std::endl;
        PyErr_Print();
        Py_XDECREF(tuple);
        return false;
    }

    PyObject* result = PyObject_CallObject(func, tuple);
    if (nullptr == result) {
        std::cerr << "Failed to call method: update_data" << std::endl;
        PyErr_Print();
        Py_XDECREF(tuple);
        Py_XDECREF(func);

        return false;
    }

    Py_XDECREF(tuple);
    Py_XDECREF(result);
    Py_XDECREF(func);

    return true;
}

std::vector<VectorData> VectorDB::getData(const std::string& collectionName, const std::vector<std::string>& ids,
                            const std::string& source)
{
    PythonThreadLocker locker;
    std::vector<VectorData> vectorDates={};
    if (nullptr == dbInstance_) {
        return vectorDates;
    }

    PyObject* pyIds = pythonutil::convertVectorStringToPyObject(ids);
    PyObject* tuple = PyTuple_New(3);
    PyTuple_SET_ITEM(tuple, 0, PyUnicode_FromString(collectionName.c_str()));
    PyTuple_SET_ITEM(tuple, 1, pyIds);
    PyTuple_SET_ITEM(tuple, 2, PyUnicode_FromString(source.c_str()));

    PyObject* func = PyObject_GetAttrString(dbInstance_, "get_data");
    if (nullptr == func) {
        std::cerr << "Could not find function 'get_data'" << std::endl;
        PyErr_Print();
        Py_XDECREF(tuple);
        return vectorDates;
    }
    PyObject* result = PyObject_CallObject(func, tuple);
    if (nullptr == result) {
        std::cerr << "Failed to call method: get_data" << std::endl;
        PyErr_Print();
        Py_XDECREF(tuple);
        Py_XDECREF(func);
        return vectorDates;
    }

    vectorDates = convertPyObjectToVectorVectorData(result);

    Py_XDECREF(tuple);
    Py_XDECREF(func);
    Py_XDECREF(result);
    return vectorDates;
}

bool VectorDB::deleteData(const std::string& collectionName, const std::vector<std::string>& sourceIds, const std::vector<std::string>& ids)
{
    PythonThreadLocker locker;
    if (nullptr == dbInstance_) {
        return false;
    }
    PyObject* pySourceIds = pythonutil::convertVectorStringToPyObject(sourceIds);
    PyObject* pyIds = pythonutil::convertVectorStringToPyObject(ids);
    PyObject* tuple = PyTuple_New(3);
    PyTuple_SET_ITEM(tuple, 0, PyUnicode_FromString(collectionName.c_str()));
    PyTuple_SET_ITEM(tuple, 1, pySourceIds);
    PyTuple_SET_ITEM(tuple, 2, pyIds);

    PyObject* func = PyObject_GetAttrString(dbInstance_, "delete_data");
    if (nullptr == func) {
        std::cerr << "Could not find function 'delete_data'" << std::endl;
        PyErr_Print();

        Py_XDECREF(tuple);
        return false;
    }
    PyObject* result = PyObject_CallObject(func, tuple);
    if (nullptr == result) {
        std::cerr << "Call function delete_data failed" << std::endl;
        PyErr_Print();

        Py_XDECREF(tuple);
        Py_XDECREF(func);
        return false;
    }

    Py_XDECREF(tuple);
    Py_XDECREF(func);
    Py_XDECREF(result);
    return true;
}

std::vector<std::pair<VectorData, float>> VectorDB::queryData(const std::string& collectionName, std::vector<std::vector<double>> vectors,
                                                         int64_t numResults)
{
    PythonThreadLocker locker;
    std::vector<std::pair<VectorData, float>> queryDatas={};
    if (nullptr == dbInstance_) {
        return queryDatas;
    }
    std::vector<std::string> ids;
    std::vector<std::vector<double>> embeddings;
    std::vector<std::map<std::string, std::variant<std::string, int64_t>>> metadatas;
    std::vector<float> distances;

    PyObject* pyEmbeddings = pythonutil::convertVectorVectorDoubleToPyObject(vectors);

    PyObject* tuple = PyTuple_New(3);
    PyTuple_SET_ITEM(tuple, 0, PyUnicode_FromString(collectionName.c_str()));
    PyTuple_SET_ITEM(tuple, 1, pyEmbeddings);
    PyTuple_SET_ITEM(tuple, 2, PyLong_FromLongLong(numResults));

    PyObject* func = PyObject_GetAttrString(dbInstance_, "query_data");
    if (nullptr == func) {
        std::cerr << "Could not find function 'query_data'" << std::endl;
        PyErr_Print();
        Py_XDECREF(tuple);
        return queryDatas;
    }
    PyObject* result = PyObject_CallObject(func, tuple);
    if (nullptr == result) {
        std::cerr << "Call function query_data failed" << std::endl;
        PyErr_Print();
        Py_XDECREF(tuple);
        Py_XDECREF(func);
        return queryDatas;
    }

    PyObject* idsObj = PyDict_GetItemString(result, "ids");
    PyObject* metadatasObj = PyDict_GetItemString(result, "metadatas");
    PyObject* distancesObj = PyDict_GetItemString(result, "distances");
    ids = pythonutil::convertPythonListToStringVector(idsObj);
    metadatas = pythonutil::convertPythonListToMapOfVariant(metadatasObj);
    distances = pythonutil::convertPythonListToVectorFloat(distancesObj);

    for (size_t i = 0; i < ids.size(); ++i) {
        std::pair<VectorData, float> queryData;
        VectorData data;
        float distance;

        data.id = ids[i];
        data.metadata = metadatas[i];
        distance = distances[i];
        queryData.first = data;
        queryData.second = distance;
        queryDatas.push_back(queryData);
    }

    Py_XDECREF(func);
    Py_XDECREF(result);
    Py_XDECREF(tuple);
    return queryDatas;
}

std::vector<VectorData> VectorDB::convertPyObjectToVectorVectorData(PyObject* pyObject)
{
    std::vector<VectorData> vectorDats;
    std::vector<std::string> ids;
    std::vector<std::vector<double>> embeddings;
    std::vector<std::map<std::string, std::variant<std::string, int64_t>>> metadatas;

    PyObject* idsObj = PyDict_GetItemString(pyObject, "ids");
    PyObject* embeddingsObj = PyDict_GetItemString(pyObject, "embeddings");
    PyObject* metadatasObj = PyDict_GetItemString(pyObject, "metadatas");

    ids = pythonutil::convertPythonListToStringVector(idsObj);
    embeddings = pythonutil::convertPythonListToVectorOfVectors(embeddingsObj);
    metadatas = pythonutil::convertPythonListToMapOfVariant(metadatasObj);

    if (ids.size() != embeddings.size() || ids.size() != metadatas.size() || embeddings.size() != metadatas.size()) {
        std::cerr << "The size of ids, embeddings and metadatas are not equal" << std::endl;
        return vectorDats;
    }

    for (size_t i = 0; i < ids.size(); ++i) {
        VectorData data;
        data.id = ids[i];
        data.embedding = embeddings[i];
        data.metadata = metadatas[i];

        vectorDats.push_back(data);
    }

    return vectorDats;
}