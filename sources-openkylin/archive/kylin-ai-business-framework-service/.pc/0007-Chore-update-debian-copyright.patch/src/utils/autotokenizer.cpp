#include "autotokenizer.h"

#include "utils/pythonutil.h"
#include "utils/pythonthreadlocker.h"

#include <iostream>

AutoTokenizer::AutoTokenizer(const std::string& modelFolderPath)
{
    if (!Py_IsInitialized())
        Py_Initialize();

    PythonThreadLocker pythonThreadLocker;

    PyRun_SimpleString("import sys");
    std::string command = "sys.path.append('" + std::string(DATA_MANAGEMENT_PYTHON_PATH) + "')";
    PyRun_SimpleString(command.c_str());
    tokenizer_ = PyImport_ImportModule("autotokenizer");
    if (!tokenizer_) {
        std::cerr << "Import tokenizer error" << std::endl;
        PyErr_Print();
        return;
    }
    PyObject* initFunc = PyObject_GetAttrString(tokenizer_, "init_tokenizer");
    if (!initFunc) {
        std::cerr << "Get init_tokenizer from tokenizer error" << std::endl;
        PyErr_Print();
        return;
    }
    PyObject* args = Py_BuildValue("(s)", modelFolderPath.c_str());
    PyObject_CallObject(initFunc, args);

    Py_XDECREF(initFunc);
    Py_XDECREF(args);
}

AutoTokenizer::~AutoTokenizer()
{
    PythonThreadLocker pythonThreadLocker;

    if (tokenizer_ != nullptr)
        Py_DECREF(tokenizer_);
}

Token AutoTokenizer::tokenize(const std::string &text)
{
    PythonThreadLocker pythonThreadLocker;
    std::map<std::string, std::vector<int64_t>> result;

    if (!Py_IsInitialized()) {
        std::cerr << "Don't init python enveriment, can't tokenize" << std::endl;
        return result;
    }

    if (tokenizer_ == nullptr) {
        std::cerr << "Tokenizer don't be init, can't tokenize" << std::endl;
        return result;
    }

    PyObject* tokenizeFunc = PyObject_GetAttrString(tokenizer_, "tokenize_text");
    if (!tokenizeFunc || !PyCallable_Check(tokenizeFunc)) {
        std::cout << "Don't found function tokenize_text" << std::endl;

        Py_DECREF(tokenizeFunc);
        return result;
    }

    PyObject* args = Py_BuildValue("(s)", text.c_str());
    PyObject* pyResult = PyObject_CallObject(tokenizeFunc, args);

    if (!pyResult) {
        Py_XDECREF(args);
        Py_XDECREF(tokenizeFunc);
        return result;
    }
    if (PyDict_Check(pyResult)) {
        result = pythonutil::convertPythonTokenDictToMap(pyResult);
    }

    Py_XDECREF(args);
    Py_XDECREF(pyResult);
    Py_XDECREF(tokenizeFunc);

    return result;
}
