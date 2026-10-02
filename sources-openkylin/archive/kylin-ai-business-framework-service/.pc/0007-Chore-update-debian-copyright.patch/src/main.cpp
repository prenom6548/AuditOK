#include "server.h"
#include "embeddingtaskmanager/embeddingtaskmanager.h"

#include <Python.h>
#include <iostream>

int main()
{
    // Init python multi-threaded support
    Py_Initialize();
    Py_BEGIN_ALLOW_THREADS

    auto &server = Server::getInstance();
    server.exec();

    Py_END_ALLOW_THREADS
    return 0;
}
