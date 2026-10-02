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

}
