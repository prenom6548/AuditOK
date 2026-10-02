#include "pythonutil.h"

#include <iostream>

namespace pythonutil {

std::map<std::string, std::vector<int64_t>>
convertPythonTokenDictToMap(PyObject* dictObject) {
    std::map<std::string, std::vector<int64_t>> resultMap;

    PyObject *keys = PyDict_Keys(dictObject);
    Py_ssize_t size = PyList_Size(keys);

    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject *key = PyList_GetItem(keys, i);
        PyObject *value = PyDict_GetItem(dictObject, key);

        if (PyList_Check(value)) {
            std::vector<int64_t> intList;
            Py_ssize_t listSize = PyList_Size(value);
            for (Py_ssize_t j = 0; j < listSize; ++j) {
                PyObject *item = PyList_GetItem(value, j);
                int intValue = PyLong_AsLong(item);
                intList.push_back(intValue);
            }
            resultMap[PyUnicode_AsUTF8(key)] = intList;
        }
    }

    Py_DECREF(keys);

    return resultMap;
}

std::vector<std::string> convertPythonStringListToStd(PyObject* listObject) {
    std::vector<std::string> resultVector;

    Py_ssize_t size = PyList_Size(listObject);

    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject *item = PyList_GetItem(listObject, i);
        if (PyUnicode_Check(item)) {
            const char *str = PyUnicode_AsUTF8(item);
            resultVector.emplace_back(str);
        } else {
            std::cerr << "This Python list does not contain a string type, cancel convert" << std::endl;
            return std::vector<std::string>();
        }
    }

    return resultVector;
}

std::string convertPythonStringToStd(PyObject* strObject) {
    if (!PyUnicode_Check(strObject)) {
        std::cerr << "Type check error, object isn't string type" << std::endl;
        return std::string();
    }
    const char* utf8Str = PyUnicode_AsUTF8(strObject);
    return std::string(utf8Str);
}

int convertPythonIntToStd(PyObject* intObject) {
    if (!PyLong_Check(intObject)) {
        std::cerr << "Type check error, object isn't int type" << std::endl;
        return -1;
    }

    long longValue = PyLong_AsLong(intObject);
    return static_cast<int>(longValue);
}

PyObject* convertVectorStringToPyObject(const std::vector<std::string>& vec)
{
    if (vec.empty()) {
        return Py_None;
    }
    PyObject* list = PyList_New(vec.size());
    if(!list) {
        std::cout << "convertVectorStringToPyObject error!!!" << std::endl;
    }

    for (size_t i = 0; i < vec.size(); ++i) {
        PyObject* py_str = PyUnicode_DecodeFSDefault(vec[i].c_str());
        if (!py_str) {
            Py_DECREF(list);
            return nullptr;
        }

        PyList_SET_ITEM(list, i, py_str);
    }
    return list;
}

PyObject* convertVectorDoubleToPyObject(const std::vector<double>& vec) {
    if (vec.empty()) {
         std::cout << "vec is empty" << std::endl;
        Py_RETURN_NONE;
    }
    PyObject* list = PyList_New(vec.size());
    if(!list) {
        std::cout << "convertVectorDoubleToPyObject error!!!" << std::endl;
    }

    for (size_t i = 0; i < vec.size(); ++i) {
        PyList_SET_ITEM(list, i, PyFloat_FromDouble(vec[i]));
    }
    return list;
}

PyObject* convertVectorVectorDoubleToPyObject(const std::vector<std::vector<double>>& vec_vec)
{
    if (vec_vec.empty()) {
        Py_RETURN_NONE;
    }
    PyObject* list = PyList_New(vec_vec.size());
    if(!list) {
        std::cout << "convertVectorVectorDoubleToPyObject error!!!" << std::endl;
    }

    for (size_t i = 0; i < vec_vec.size(); ++i) {
        if (vec_vec[i].empty()) {
            Py_RETURN_NONE ;
        } else {
            PyList_SET_ITEM(list, i, convertVectorDoubleToPyObject(vec_vec[i]));
        }
    }

    return list;
}

// 将std::variant<std::string, int64_t>转换为Python对象
PyObject* convertVariantToPyObject(const std::variant<std::string, int64_t>& var)
{
    if (std::holds_alternative<std::string>(var)) {
        return PyUnicode_FromString(std::get<std::string>(var).c_str());
    } else if (std::holds_alternative<int64_t>(var)) {
        return PyLong_FromLongLong(std::get<int64_t>(var));
    }
    // 如果variant中不是预期的类型，返回None或抛出异常
    Py_RETURN_NONE;
}

// 将std::map<std::string, std::variant<std::string, int64_t>>转换为Python字典对象
PyObject* convertMapVariantToPyObject(const std::map<std::string, std::variant<std::string, int64_t>>& map_var) {
    PyObject* dict = PyDict_New();
    for (const auto& entry : map_var) {
        PyObject* key = PyUnicode_FromString(entry.first.c_str());
        PyObject* value = convertVariantToPyObject(entry.second);
        if (key && value) {
            PyDict_SetItem(dict, key, value);
            Py_DECREF(key);
            Py_DECREF(value);
        } else {
            // 如果有错误发生，释放已分配的对象并返回NULL
            Py_XDECREF(key);
            Py_XDECREF(value);
            Py_DECREF(dict);
            return nullptr;
        }
    }
    return dict;
}

// 将std::vector<std::map<std::string, std::variant<std::string, int64_t>>>转换为Python列表对象
PyObject* convertVectorMapToPyObject(const std::vector<std::map<std::string, std::variant<std::string, int64_t>>>& vec_map_var) {
    PyObject* list = PyList_New(vec_map_var.size());
    for (size_t i = 0; i < vec_map_var.size(); ++i) {
        PyList_SET_ITEM(list, i, convertMapVariantToPyObject(vec_map_var[i]));
    }
    return list;
}

std::vector<std::string> convertPythonListToStringVector(PyObject* ids_obj)
{
    std::vector<std::string> ids={};
    if (!PyList_Check(ids_obj)) {
        return ids;
    }

    Py_ssize_t size = PyList_Size(ids_obj);
    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject* item = PyList_GetItem(ids_obj, i);

        if (!PyUnicode_Check(item)) {
            continue;
        }

        PyObject* unicode_str = PyUnicode_AsUTF8String(item);
        const char* c_str = PyBytes_AsString(unicode_str);

        ids.push_back(std::string(c_str));
        Py_DECREF(unicode_str);
    }

    return ids;
}

std::vector<std::vector<double>> convertPythonListToVectorOfVectors(PyObject* embeddings_obj) {
    std::vector<std::vector<double>> embeddings;
    if (PyList_Check(embeddings_obj)) {
        embeddings = extractInnerVectors(embeddings_obj);
    } else {
        std::cerr << "The embeddings object is not a list." << std::endl;
    }
    return embeddings;
}

std::vector<std::vector<double>> extractInnerVectors(PyObject* outer_list) {
    std::vector<std::vector<double>> embeddings;
    Py_ssize_t outer_size = PyList_Size(outer_list);
    for (Py_ssize_t i = 0; i < outer_size; ++i) {
        PyObject* inner_list = PyList_GetItem(outer_list, i);
        if (PyList_Check(inner_list)) {
            std::vector<double> inner_vec = extractDoubles(inner_list);
            embeddings.push_back(inner_vec);
        } else {
            std::cerr << "Non-list value found in the embeddings list." << std::endl;
        }
    }
    return embeddings;
}

std::vector<double> extractDoubles(PyObject* inner_list) {
    std::vector<double> inner_vec;
    Py_ssize_t inner_size = PyList_Size(inner_list);
    for (Py_ssize_t j = 0; j < inner_size; ++j) {
        PyObject* item = PyList_GetItem(inner_list, j);
        if (PyFloat_Check(item)) {
            double value = PyFloat_AsDouble(item);
            inner_vec.push_back(value);
        } else {
            std::cerr << "Non-float value found in the embeddings list." << std::endl;
        }
    }
    return inner_vec;
}

std::vector<std::map<std::string, std::variant<std::string, int64_t>>> convertPythonListToMapOfVariant(PyObject* metadatas_obj) {
    std::vector<std::map<std::string, std::variant<std::string, int64_t>>> metadatas;
    if (!PyList_Check(metadatas_obj)) {
        return metadatas;
    }
    Py_ssize_t size = PyList_Size(metadatas_obj);
    for (Py_ssize_t i = 0; i < size; ++i) {
        PyObject* item = PyList_GetItem(metadatas_obj, i);
        if (PyDict_Check(item)) {
            std::map<std::string, std::variant<std::string, int64_t>> metadata_map = extractMapFromDict(item);
            metadatas.push_back(metadata_map);
        }
    }

    return metadatas;
}

std::map<std::string, std::variant<std::string, int64_t>> extractMapFromDict(PyObject* dict_obj) {
    std::map<std::string, std::variant<std::string, int64_t>> metadata_map;
    PyObject* keys = PyDict_Keys(dict_obj);
    Py_ssize_t key_count = PyList_Size(keys);
    for (Py_ssize_t j = 0; j < key_count; ++j) {
        PyObject* key_obj = PyList_GetItem(keys, j);
        if (PyUnicode_Check(key_obj)) {
            std::string key = extractStringFromUnicode(key_obj);
            PyObject* value_obj = PyDict_GetItem(dict_obj, key_obj);
            std::variant<std::string, int64_t> value = extractVariantFromObj(value_obj);
            metadata_map[key] = value;
        }
    }
    Py_DECREF(keys);
    return metadata_map;
}

std::string extractStringFromUnicode(PyObject* unicode_obj) {
    PyObject* unicode_as_utf8 = PyUnicode_AsUTF8String(unicode_obj);
    const char* c_str = PyBytes_AsString(unicode_as_utf8);
    std::string str(c_str);
    Py_DECREF(unicode_as_utf8);
    return str;
}

std::variant<std::string, int64_t> extractVariantFromObj(PyObject* value_obj) {
    std::variant<std::string, int64_t> value;
    if (PyUnicode_Check(value_obj)) {
        value = extractStringFromUnicode(value_obj);
    } else if (PyLong_Check(value_obj)) {
        int64_t int_value = PyLong_AsLongLong(value_obj);
        value = int_value;
    }
    return value;
}

std::vector<float> convertPythonListToVectorFloat(PyObject* obj)
{
    std::vector<float> result;
    if (!PySequence_Check(obj)) {
        PyErr_SetString(PyExc_TypeError, "Input object is not a sequence");
        return result;
    }
    Py_ssize_t length = PySequence_Size(obj);
    result.reserve(length);
    for (Py_ssize_t i = 0; i < length; ++i) {
        PyObject* item = PySequence_GetItem(obj, i);
        if (item == NULL) {
            PyErr_Print();
            Py_XDECREF(item);
            return result;
        }
        if (!PyFloat_Check(item)) {
            PyErr_Format(PyExc_TypeError, "Sequence item %zd is not a float", i);
            Py_XDECREF(item);
            return result;
        }
        double value = PyFloat_AsDouble(item);
        if (value == -1.0 && PyErr_Occurred()) {
            PyErr_Print();
            Py_XDECREF(item);
            return result;
        }
        result.push_back(static_cast<float>(value));

        Py_XDECREF(item);
    }
    return result;
}

}
