#ifndef PYTHONUTIL_H
#define PYTHONUTIL_H

#include <Python.h>

#include <string>
#include <vector>
#include <map>

#define DATA_MANAGEMENT_PYTHON_PATH "/usr/share/kylin-ai-business-framework/utils/python/"

namespace pythonutil {

std::map<std::string, std::vector<int64_t>> convertPythonTokenDictToMap(PyObject* dictObject);
std::vector<std::string> convertPythonStringListToStd(PyObject* listObject);
std::string convertPythonStringToStd(PyObject* strObject);
int convertPythonIntToStd(PyObject* intObject);

}


#endif // PYTHONUTIL_H
