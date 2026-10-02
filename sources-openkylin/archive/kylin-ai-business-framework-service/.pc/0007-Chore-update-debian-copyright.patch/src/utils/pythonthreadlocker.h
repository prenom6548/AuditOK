#ifndef PYTHONTHREADLOCKER_H
#define PYTHONTHREADLOCKER_H

#include <Python.h>

class PythonThreadLocker
{
public:
    PythonThreadLocker() : state(PyGILState_Ensure())
    {}
    ~PythonThreadLocker() {
        PyGILState_Release(state);
    }

private:
    PyGILState_STATE state;
};

#endif // PYTHONTHREADLOCKER_H
