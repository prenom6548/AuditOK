#ifndef SINGLEAPPLICATION_H
#define SINGLEAPPLICATION_H

#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>

class SingleApplication
{
public:
    SingleApplication(const std::string& name);

    virtual ~SingleApplication();

    bool isAlreadyRuning();

private:
    std::string m_name;
    int m_lockFile = -1;
};

#endif // SINGLEAPPLICATION_H
