#ifndef VECTORDB_H
#define VECTORDB_H

#include <Python.h>
#include <string>
#include <vector>
#include <map>
#include <variant>

enum class VectorDBErrorCode
{
    Success,
    ArgumentError,
    PyObjectCallMethodError,
    ValidDataBase,
    EmptyDataVector
};

struct VectorData{
    std::string id;
    std::vector<double> embedding = {};
    std::map<std::string, std::variant<std::string, int64_t>> metadata;
};
class VectorDB
{
public:
    explicit VectorDB();
    // explicit VectorDB(const std::string& databasePath);
    ~VectorDB() = default;
    std::string engineName() const {return "ChromaDB";}

    VectorDBErrorCode createClient(const std::string& databasePath);
    VectorDBErrorCode destroyClient();
    VectorDBErrorCode createCollection(const std::string& collectionName, const std::string& searchAlgorithm);
    std::vector<std::string> collections();
    VectorDBErrorCode addData(const std::string& collectionName, const std::vector<VectorData>& data);
    VectorDBErrorCode updateData(const std::string& collectionName, const std::vector<VectorData>& data);

    std::vector<VectorData> getData(const std::string& collectionName,
                                    const std::vector<std::string>& ids={}, const std::string& source="");

    VectorDBErrorCode deleteData(const std::string& collectionName,
                    const std::vector<std::string>& sourceIds={}, const std::vector<std::string>& ids={});

    std::vector<std::pair<VectorData, float>> queryData(const std::string& collectionName,
                                                         std::vector<std::vector<double>> embeddings,
                                                         int64_t numResults = 10);
private:
    void initializeChromaDB();
    std::vector<VectorData> convertPyObjectToVectorVectorData(PyObject* pyObject);

    PyObject* dbInstance_ = nullptr;
};

#endif // VECTORDB_H
