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

#ifndef PYTHONUTIL_H
#define PYTHONUTIL_H

#include <Python.h>

#include <string>
#include <vector>
#include <map>
#include <variant>

#define DATA_MANAGEMENT_PYTHON_PATH "/usr/share/kylin-ai-business-framework/utils/python/"

namespace pythonutil {

std::map<std::string, std::vector<int64_t>> convertPythonTokenDictToMap(PyObject* dictObject);
std::vector<std::string> convertPythonStringListToStd(PyObject* listObject);
std::string convertPythonStringToStd(PyObject* strObject);
int convertPythonIntToStd(PyObject* intObject);

PyObject* convertVectorStringToPyObject(const std::vector<std::string>& vec);

PyObject* convertVectorDoubleToPyObject(const std::vector<double>& vec);
PyObject* convertVectorVectorDoubleToPyObject(const std::vector<std::vector<double>>& vec_vec);

PyObject* convertVariantToPyObject(const std::variant<std::string, int64_t>& var);
PyObject* convertMapVariantToPyObject(const std::map<std::string, std::variant<std::string, int64_t>>& map_var);
PyObject* convertVectorMapToPyObject(const std::vector<std::map<std::string, std::variant<std::string, int64_t>>>& vec_map_var);

std::vector<std::string> convertPythonListToStringVector(PyObject* ids_obj);

std::vector<std::vector<double>> convertPythonListToVectorOfVectors(PyObject* embeddings_obj);
std::vector<std::vector<double>> extractInnerVectors(PyObject* outer_list);
std::vector<double> extractDoubles(PyObject* inner_list);

std::vector<std::map<std::string, std::variant<std::string, int64_t>>> convertPythonListToMapOfVariant(PyObject* metadatas_obj);
std::map<std::string, std::variant<std::string, int64_t>> extractMapFromDict(PyObject* dict_obj);
std::string extractStringFromUnicode(PyObject* unicode_obj);
std::variant<std::string, int64_t> extractVariantFromObj(PyObject* value_obj);

std::vector<float> convertPythonListToVectorFloat(PyObject* obj);

}


#endif // PYTHONUTIL_H
