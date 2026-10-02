#include "parser.h"

#include "docxparser.h"
#include "pptxparser.h"
#include "pdfparser.h"
#include "txtparser.h"

#include <sys/stat.h>
#include <filesystem>
#include <iostream>

time_t Parser::modifyTime(const std::string &filePath)
{
    if (!std::filesystem::exists(filePath)) {
        std::cerr << "File " << filePath << " don't exist, "
                  << "can't get modify time" << std::endl;
        return -1;
    }

    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) == 0) {
        // 获取修改时间
        return fileStat.st_mtime;
    }

    return -1;
}
