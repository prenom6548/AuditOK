#ifndef OFFICEPYPARSERWRAPPER_H
#define OFFICEPYPARSERWRAPPER_H

#include "parser.h"

class OfficePyParserWrapper {
public:
    static OfficePyParserWrapper* getInstance() {
        static OfficePyParserWrapper warpper;
        return &warpper;
    }


    FileInfo parse(const std::string& filePath, const std::string& module, const std::string& className);

private:
    OfficePyParserWrapper();
};

#endif // OFFICEPYPARSERWRAPPER_H
