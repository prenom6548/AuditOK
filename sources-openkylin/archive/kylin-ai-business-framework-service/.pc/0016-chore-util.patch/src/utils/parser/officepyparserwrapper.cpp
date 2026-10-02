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
    if (filePath.empty() || module.empty() || className.empty()) {
        std::cerr << "Input strings cannot be empty." << std::endl;
        return FileInfo();
    }

    PythonThreadLocker locker;
    FileInfo fileinfo;

    PyObject* parserModule = PyImport_ImportModule(module.c_str());
    if (!parserModule) {
        std::cerr << "Import module " << module << " failed." << std::endl;
        PyErr_Print();
        return FileInfo();
    }

    PyObject* constructor = PyObject_GetAttrString(parserModule, className.c_str());
    if (!constructor) {
        std::cerr << "Get constructor for module " << module << " failed." << std::endl;
        PyErr_Print();
        Py_XDECREF(parserModule);
        return FileInfo();
    }
    PyObject* parserObject = PyObject_CallFunction(constructor, "s", filePath.c_str());
    if (!parserObject) {
        std::cerr << "Create parser object for module " << module << " failed." << std::endl;
        PyErr_Print();
        Py_XDECREF(parserModule);
        Py_XDECREF(constructor);
        return FileInfo();
    }

    PyObject* getContent = PyObject_GetAttrString(parserObject, "contents");
    if (!getContent) {
        std::cerr << "Get contents for module " << module << " failed." << std::endl;
        PyErr_Print();
        Py_XDECREF(parserModule);
        Py_XDECREF(constructor);
        Py_XDECREF(parserObject);
        return FileInfo();
    }
    PyObject* contentList = PyObject_CallObject(getContent, nullptr);
    if (!contentList) {
        std::cerr << "Get contents for module " << module << " failed." << std::endl;
        PyErr_Print();
        Py_XDECREF(parserModule);
        Py_XDECREF(constructor);
        Py_XDECREF(parserObject);
        Py_XDECREF(getContent);
        return FileInfo();
    }
    fileinfo.contents = pythonutil::convertPythonStringListToStd(contentList);

    PyObject* getTitle = PyObject_GetAttrString(parserObject, "title");
    if (!getTitle) {
        std::cerr << "Get title for module " << module << " failed." << std::endl;
        PyErr_Print();
        Py_XDECREF(parserModule);
        Py_XDECREF(constructor);
        Py_XDECREF(parserObject);
        Py_XDECREF(getContent);
        Py_XDECREF(contentList);
        return FileInfo();
    }
    PyObject* title = PyObject_CallObject(getTitle, nullptr);
    if (!title) {
        std::cerr << "Get title for module " << module << " failed." << std::endl;
        PyErr_Print();
        Py_XDECREF(parserModule);
        Py_XDECREF(constructor);
        Py_XDECREF(parserObject);
        Py_XDECREF(getContent);
        Py_XDECREF(contentList);
        Py_XDECREF(getTitle);
        return FileInfo();
    }
    fileinfo.title = pythonutil::convertPythonStringToStd(title);

    PyObject* getAuthor = PyObject_GetAttrString(parserObject, "author");
    if (!getAuthor) {
        std::cerr << "Get author for module " << module << " failed." << std::endl;
        PyErr_Print();
        Py_XDECREF(parserModule);
        Py_XDECREF(constructor);
        Py_XDECREF(parserObject);
        Py_XDECREF(getContent);
        Py_XDECREF(contentList);
        Py_XDECREF(getTitle);
        Py_XDECREF(title);
        return FileInfo();
    }
    PyObject* author = PyObject_CallObject(getAuthor, nullptr);
    if (!author) {
        std::cerr << "Get author for module " << module << " failed." << std::endl;
        PyErr_Print();
        Py_XDECREF(parserModule);
        Py_XDECREF(constructor);
        Py_XDECREF(parserObject);
        Py_XDECREF(getContent);
        Py_XDECREF(contentList);
        Py_XDECREF(getTitle);
        Py_XDECREF(title);
        Py_XDECREF(getAuthor);
        return FileInfo();
    }
    fileinfo.author = pythonutil::convertPythonStringToStd(author);

    PyObject* getPages = PyObject_GetAttrString(parserObject, "pages");
    if (!getPages) {
        std::cerr << "Get pages for module " << module << " failed." << std::endl;
        PyErr_Print();
        Py_XDECREF(parserModule);
        Py_XDECREF(constructor);
        Py_XDECREF(parserObject);
        Py_XDECREF(getContent);
        Py_XDECREF(contentList);
        Py_XDECREF(getTitle);
        Py_XDECREF(title);
        Py_XDECREF(getAuthor);
        Py_XDECREF(author);
        return FileInfo();
    }
    PyObject* pages = PyObject_CallObject(getPages, nullptr);
    if (!pages) {
        std::cerr << "Get pages for module " << module << " failed." << std::endl;
        PyErr_Print();
        Py_XDECREF(parserModule);
        Py_XDECREF(constructor);
        Py_XDECREF(parserObject);
        Py_XDECREF(getContent);
        Py_XDECREF(contentList);
        Py_XDECREF(getTitle);
        Py_XDECREF(title);
        Py_XDECREF(getAuthor);
        Py_XDECREF(author);
        Py_XDECREF(getPages);
        return FileInfo();
    }
    fileinfo.pages = pythonutil::convertPythonIntToStd(pages);

    fileinfo.modifyTime = Parser::modifyTime(filePath);

    Py_XDECREF(parserModule);
    Py_XDECREF(constructor);
    Py_XDECREF(parserObject);
    Py_XDECREF(getContent);
    Py_XDECREF(contentList);
    Py_XDECREF(getTitle);
    Py_XDECREF(title);
    Py_XDECREF(getAuthor);
    Py_XDECREF(author);
    Py_XDECREF(getPages);
    Py_XDECREF(pages);

    return fileinfo;
}
