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

#include "officepyparserwrapper.h"

#include "utils/pythonutil.h"
#include "utils/pythonthreadlocker.h"

OfficePyParserWrapper::OfficePyParserWrapper()
{
    if (!Py_IsInitialized())
        Py_Initialize();

    PythonThreadLocker locker;
    PyRun_SimpleString("import sys");
    std::string command = "sys.path.append('" + std::string(DATA_MANAGEMENT_PYTHON_PATH) + "')";
    PyRun_SimpleString(command.c_str());
}

FileInfo OfficePyParserWrapper::parse(const std::string &filePath, const std::string &module, const std::string &className)
{
    PythonThreadLocker locker;
    FileInfo fileinfo;

    PyObject* parserModule = PyImport_ImportModule(module.c_str());
    if (!parserModule) {
        std::cerr << "Import module " << module << " failed." << std::endl;
        PyErr_Print();
        return FileInfo();
    }

    PyObject* constructor = PyObject_GetAttrString(parserModule, className.c_str());
    PyObject* parserObject = PyObject_CallFunction(constructor, "s", filePath.c_str());
    if (!parserObject) {
        std::cerr << "Create parser object for module " << module << " failed." << std::endl;
        PyErr_Print();
        return FileInfo();
    }

    PyObject* getContent = PyObject_GetAttrString(parserObject, "contents");
    PyObject* contentList = PyObject_CallObject(getContent, nullptr);
    fileinfo.contents = pythonutil::convertPythonStringListToStd(contentList);

    PyObject* getTitle = PyObject_GetAttrString(parserObject, "title");
    PyObject* title = PyObject_CallObject(getTitle, nullptr);
    fileinfo.title = pythonutil::convertPythonStringToStd(title);

    PyObject* getAuthor = PyObject_GetAttrString(parserObject, "author");
    PyObject* author = PyObject_CallObject(getAuthor, nullptr);
    fileinfo.author = pythonutil::convertPythonStringToStd(author);

    PyObject* getPages = PyObject_GetAttrString(parserObject, "pages");
    PyObject* pages = PyObject_CallObject(getPages, nullptr);
    fileinfo.pages = pythonutil::convertPythonIntToStd(pages);

    fileinfo.modifyTime = Parser::modifyTime(filePath);

    Py_DECREF(parserModule);
    Py_DECREF(constructor);
    Py_DECREF(parserObject);
    Py_DECREF(getContent);
    Py_DECREF(contentList);
    Py_DECREF(getTitle);
    Py_DECREF(title);
    Py_DECREF(getAuthor);
    Py_DECREF(author);
    Py_DECREF(getPages);
    Py_DECREF(pages);

    return fileinfo;
}
