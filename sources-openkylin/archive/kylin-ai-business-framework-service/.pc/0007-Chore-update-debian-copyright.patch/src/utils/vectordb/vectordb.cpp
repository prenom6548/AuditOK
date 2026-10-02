#include "vectordb.h"
#include "utils/pythonthreadlocker.h"
#include "utils/pythonutil.h"

#include <Python.h>
#include <iostream>
#include <variant>

VectorDB::VectorDB()
{
    std::cout << "Initializing VectorDB..." << std::endl;
    initializeChromaDB();
}

void VectorDB::initializeChromaDB() {
    std::cout << "Initializing Python..." << std::endl;
    // 初始化Python解释器
    if (!Py_IsInitialized())
        Py_Initialize();

    PythonThreadLocker locker;
    PyRun_SimpleString("import sys");
    std::string command = "sys.path.append('" + std::string(DATA_MANAGEMENT_PYTHON_PATH) + "')";
    PyRun_SimpleString(command.c_str());
}

VectorDBErrorCode VectorDB::createClient(const std::string& databasePath)
{
    PythonThreadLocker locker;
    std::cout << "Creating ChromaDB instance..." << std::endl;
    // 导入模块
    PyObject* pModule = PyImport_ImportModule("chroma_db");
    if (pModule == nullptr) {
        std::cerr << "Failed to load module: chroma_db" << std::endl;
        PyErr_Print();
        return VectorDBErrorCode::ArgumentError;
    }

    // 获取ChromaDB类
    PyObject* pChromaDBClass = PyObject_GetAttrString(pModule, "ChromaDB");
    if (pChromaDBClass == nullptr) {
        std::cerr << "Failed to get ChromaDB class" << std::endl;
        Py_DECREF(pModule);
        return VectorDBErrorCode::ArgumentError;
    }

    // 创建ChromaDB实例
    PyObject* args = Py_BuildValue("(s)", databasePath.c_str());
    if(args == nullptr) {
        std::cerr << "Error: Could not create argument tuple." << std::endl;
    }
    dbInstance_ = PyObject_CallObject(pChromaDBClass, args);

    Py_DECREF(pModule);
    Py_DECREF(args);
    Py_DECREF(pChromaDBClass);

    return VectorDBErrorCode::Success;
}

VectorDBErrorCode VectorDB::destroyClient()
{
    PythonThreadLocker locker;
    if (dbInstance_ != nullptr) {
        Py_DECREF(dbInstance_);
        dbInstance_ = nullptr; // 设置为nullptr以避免悬空指针
    }

    return VectorDBErrorCode::Success;
}

VectorDBErrorCode VectorDB::createCollection(const std::string& collectionName,  const std::string& searchAlgorithm)
{
    PythonThreadLocker locker;
    if (dbInstance_ == nullptr) {
        return VectorDBErrorCode::ValidDataBase;
    }

    PyObject* args = Py_BuildValue("(ss)", collectionName.c_str(), searchAlgorithm.c_str());
    if (!args) {
        std::cerr << "Error: Could not create argument tuple." << std::endl;
        Py_XDECREF(args);
        return VectorDBErrorCode::ArgumentError;
    }

    PyObject* result = PyObject_CallMethod(dbInstance_, "get_or_create_collection", "O", args);
    if (result == nullptr) {
        // 处理错误
        PyErr_Print();
        Py_XDECREF(args);
        Py_XDECREF(result);
        return VectorDBErrorCode::PyObjectCallMethodError;
    }

    Py_XDECREF(args);
    Py_XDECREF(result);
    std::cout << "createCollection success" << std::endl;
    return VectorDBErrorCode::Success;
}

std::vector<std::string> VectorDB::collections()
{
    PythonThreadLocker locker;
    std::vector<std::string> collectionLists={};
    if (dbInstance_ == nullptr) {
        return collectionLists;
    }
    PyObject* func = PyObject_GetAttrString(dbInstance_, "list_collections");
    if (func == NULL) {
        std::cerr << "Could not find function 'list_collections'" << std::endl;
        return collectionLists;
    }

    PyObject* result = PyObject_CallObject(func, NULL); // 假设list_collections不接受任何参数
    Py_DECREF(func); // 减少对函数的引用计数
    collectionLists = pythonutil::convertPythonListToStringVector(result);
    return collectionLists;
}

VectorDBErrorCode VectorDB::addData(const std::string& collectionName, const std::vector<VectorData>& data)
{
    PythonThreadLocker locker;
    std::cout << "Adding data to ChromaDB..." << std::endl;
    if (dbInstance_ == nullptr) {
        return VectorDBErrorCode::ValidDataBase;
    }

    if (data.empty()) {
        std::cerr << "Warning: Attempted to addData with an empty data vector." << std::endl;
        return VectorDBErrorCode::EmptyDataVector;
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
            documents.emplace_back();
        }
    }

    PyObject* pyIds = pythonutil::convertVectorStringToPyObject(ids);
    PyObject* pyEmbeddings = pythonutil::convertVectorVectorDoubleToPyObject(embeddings);
    PyObject* pyMetadatas = pythonutil::convertVectorMapToPyObject(metadatas);
    PyObject* pyDocuments = pythonutil::convertVectorStringToPyObject(documents);

    // 创建包含所有参数的元组
    PyObject* args_tuple = PyTuple_New(5);
    PyTuple_SET_ITEM(args_tuple, 0, PyUnicode_FromString(collectionName.c_str()));
    PyTuple_SET_ITEM(args_tuple, 1, pyIds);
    PyTuple_SET_ITEM(args_tuple, 2, pyMetadatas);
    PyTuple_SET_ITEM(args_tuple, 3, pyEmbeddings);
    PyTuple_SET_ITEM(args_tuple, 4, pyDocuments);

    PyObject* result = PyObject_CallObject(PyObject_GetAttrString(dbInstance_, "add_data"), args_tuple);
    if (result == nullptr) {
        PyErr_Print();

        Py_XDECREF(pyIds);
        Py_XDECREF(pyEmbeddings);
        Py_XDECREF(pyMetadatas);
        Py_XDECREF(pyDocuments);
        Py_XDECREF(args_tuple);
        Py_XDECREF(result);

        return VectorDBErrorCode::PyObjectCallMethodError;
    }

    Py_XDECREF(pyIds);
    Py_XDECREF(pyEmbeddings);
    Py_XDECREF(pyMetadatas);
    Py_XDECREF(pyDocuments);
    Py_XDECREF(args_tuple);
    Py_XDECREF(result);

    return VectorDBErrorCode::Success;
}

VectorDBErrorCode VectorDB::updateData(const std::string& collectionName, const std::vector<VectorData>& data)
{
    PythonThreadLocker locker;
    if (dbInstance_ == nullptr) {
        return VectorDBErrorCode::ValidDataBase;
    }

    if (data.empty()) {
        std::cerr << "Warning: Attempted to updateData with an empty data vector." << std::endl;
        return VectorDBErrorCode::EmptyDataVector;
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
            documents.emplace_back();
        }
    }

    PyObject* pyIds = pythonutil::convertVectorStringToPyObject(ids);
    PyObject* pyEmbeddings = pythonutil::convertVectorVectorDoubleToPyObject(embeddings);
    PyObject* pyMetadatas = pythonutil::convertVectorMapToPyObject(metadatas);
    PyObject* pyDocuments = pythonutil::convertVectorStringToPyObject(documents);

    // 创建包含所有参数的元组
    PyObject* args_tuple = PyTuple_New(5);
    PyTuple_SET_ITEM(args_tuple, 0, PyUnicode_FromString(collectionName.c_str()));
    PyTuple_SET_ITEM(args_tuple, 1, pyIds);
    PyTuple_SET_ITEM(args_tuple, 2, pyMetadatas);
    PyTuple_SET_ITEM(args_tuple, 3, pyEmbeddings);
    PyTuple_SET_ITEM(args_tuple, 4, pyDocuments);

    PyObject* result = PyObject_CallObject(PyObject_GetAttrString(dbInstance_, "update_data"), args_tuple);
    if (result == nullptr) {
        PyErr_Print();

        Py_XDECREF(pyIds);
        Py_XDECREF(pyEmbeddings);
        Py_XDECREF(pyMetadatas);
        Py_XDECREF(pyDocuments);
        Py_XDECREF(args_tuple);
        Py_XDECREF(result);

        return VectorDBErrorCode::PyObjectCallMethodError;
    }

    Py_XDECREF(pyIds);
    Py_XDECREF(pyEmbeddings);
    Py_XDECREF(pyMetadatas);
    Py_XDECREF(pyDocuments);
    Py_XDECREF(args_tuple);
    Py_XDECREF(result);

    return VectorDBErrorCode::Success;
}

std::vector<VectorData> VectorDB::getData(const std::string& collectionName, const std::vector<std::string>& ids,
                            const std::string& source)
{
    PythonThreadLocker locker;
    std::vector<VectorData> vectorDates={};
    if (dbInstance_ == nullptr) {
        return vectorDates;
    }
    PyObject* pyIds = pythonutil::convertVectorStringToPyObject(ids);

    PyObject* args_tuple = PyTuple_New(3);
    PyTuple_SET_ITEM(args_tuple, 0, PyUnicode_FromString(collectionName.c_str()));
    PyTuple_SET_ITEM(args_tuple, 1, pyIds);
    PyTuple_SET_ITEM(args_tuple, 2, PyUnicode_FromString(source.c_str()));

    PyObject* result = PyObject_CallObject(PyObject_GetAttrString(dbInstance_, "get_data"), args_tuple);
    if (result == nullptr) {
        // 处理错误
        PyErr_Print();
        Py_XDECREF(pyIds);
        Py_XDECREF(args_tuple);
        return vectorDates;
    }

    Py_XDECREF(pyIds);
    Py_XDECREF(args_tuple);
    vectorDates = convertPyObjectToVectorVectorData(result);
    return vectorDates;
}

VectorDBErrorCode VectorDB::deleteData(const std::string& collectionName, const std::vector<std::string>& sourceIds, const std::vector<std::string>& ids)
{
    PythonThreadLocker locker;
    if (dbInstance_ == nullptr) {
        return VectorDBErrorCode::ValidDataBase;
    }
    PyObject* pySourceIds = pythonutil::convertVectorStringToPyObject(sourceIds);
    PyObject* pyIds = pythonutil::convertVectorStringToPyObject(ids);
    PyObject* args_tuple = PyTuple_New(3);
    PyTuple_SET_ITEM(args_tuple, 0, PyUnicode_FromString(collectionName.c_str()));
    PyTuple_SET_ITEM(args_tuple, 1, pySourceIds);
    PyTuple_SET_ITEM(args_tuple, 2, pyIds);

    if (PyObject_CallObject(PyObject_GetAttrString(dbInstance_, "delete_data"), args_tuple) == nullptr) {
        PyErr_Print();

        Py_XDECREF(pySourceIds);
        Py_XDECREF(pyIds);
        Py_XDECREF(args_tuple);
        return VectorDBErrorCode::PyObjectCallMethodError;
    }

    Py_XDECREF(pySourceIds);
    Py_XDECREF(pyIds);
    Py_XDECREF(args_tuple);

    return VectorDBErrorCode::Success;
}

std::vector<std::pair<VectorData, float>> VectorDB::queryData(const std::string& collectionName, std::vector<std::vector<double>> vectors,
                                                         int64_t numResults)
{
    PythonThreadLocker locker;
    std::vector<std::pair<VectorData, float>> queryDatas={};
    if (dbInstance_ == nullptr) {
        return queryDatas;
    }
    std::vector<std::string> ids;
    std::vector<std::vector<double>> embeddings;
    std::vector<std::map<std::string, std::variant<std::string, int64_t>>> metadatas;
    std::vector<float> distances;

    PyObject* pyEmbeddings = pythonutil::convertVectorVectorDoubleToPyObject(vectors);

    PyObject* args_tuple = PyTuple_New(3);
    PyTuple_SET_ITEM(args_tuple, 0, PyUnicode_FromString(collectionName.c_str()));
    PyTuple_SET_ITEM(args_tuple, 1, pyEmbeddings);
    PyTuple_SET_ITEM(args_tuple, 2, PyLong_FromLongLong(numResults));

    PyObject* result = PyObject_CallObject(PyObject_GetAttrString(dbInstance_, "query_data"), args_tuple);
    if (result == nullptr) {
        // 处理错误
        PyErr_Print();
        Py_XDECREF(pyEmbeddings);
        Py_XDECREF(args_tuple);
        return queryDatas;
    }

    PyObject* ids_obj = PyDict_GetItemString(result, "ids");
    PyObject* metadatas_obj = PyDict_GetItemString(result, "metadatas");
    PyObject* distances_obj = PyDict_GetItemString(result, "distances");

    ids = pythonutil::convertPythonListToStringVector(ids_obj);
    metadatas = pythonutil::convertPythonListToMapOfVariant(metadatas_obj);
    distances = pythonutil::convertPythonListToVectorFloat(distances_obj);

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

    Py_XDECREF(pyEmbeddings);
    Py_XDECREF(args_tuple);
    return queryDatas;
}

std::vector<VectorData> VectorDB::convertPyObjectToVectorVectorData(PyObject* pyObject)
{
    std::vector<VectorData> vectorDats;
    std::vector<std::string> ids;
    std::vector<std::vector<double>> embeddings;
    std::vector<std::map<std::string, std::variant<std::string, int64_t>>> metadatas;

    PyObject* ids_obj = PyDict_GetItemString(pyObject, "ids");
    PyObject* embeddings_obj = PyDict_GetItemString(pyObject, "embeddings");
    PyObject* metadatas_obj = PyDict_GetItemString(pyObject, "metadatas");

    ids = pythonutil::convertPythonListToStringVector(ids_obj);
    embeddings = pythonutil::convertPythonListToVectorOfVectors(embeddings_obj);
    metadatas = pythonutil::convertPythonListToMapOfVariant(metadatas_obj);

    if (ids.size() == embeddings.size() && ids.size() == metadatas.size()) {
        for (size_t i = 0; i < ids.size(); ++i) {
            VectorData data;
            data.id = ids[i];
            data.embedding = embeddings[i];
            data.metadata = metadatas[i];

            vectorDats.push_back(data);
        }
    } else {
        std::cout << "ids, embeddings, 和 metadatas的大小不匹配 " << std::endl;
    }

    return vectorDats;
}