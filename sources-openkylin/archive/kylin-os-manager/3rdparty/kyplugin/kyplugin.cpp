/*
 * Pluma - Plug-in Management Framework
 * Copyright (C) 2010-2012 Gil Costa (gsaurus@gmail.com)
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it freely,
 * subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented;
 *    you must not claim that you wrote the original software.
 *    If you use this software in a product, an acknowledgment
 *    in the product documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such,
 *    and must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <ctime>

#include "kyplugin.h"


namespace kyplugin
{

std::map<std::string, std::string> KyPlugin::m_report;

KyPlugin::KyPlugin()
{
    /* nothing to do */
}

void KyPlugin::generateReport(std::string name, std::string data)
{
    std::map<std::string, std::string>::iterator it = m_report.find(name);
    if (it == m_report.end()) {
        std::string temp = generateReportHead(name);
        temp.append(data).append("\n");
        m_report.insert(std::pair<std::string, std::string>(name, temp));
        return;
    }

    std::string value = it->second;
    value.append(data).append("\n");
    m_report[name] = value;

    return;
}

std::string KyPlugin::generateReportHead(std::string name)
{
    std::string ret;
    ret.append("Name : ").append(name).append("\n");

    time_t now = time(NULL);
    struct tm time;
    localtime_r(&now, &time);

    char timeFormat[512] = {'\0'};
    snprintf(timeFormat, sizeof(timeFormat), "%d-%d-%d %d:%d:%d", time.tm_year + 1900, time.tm_mon + 1, time.tm_mday,
             time.tm_hour, time.tm_min, time.tm_sec);

    ret.append("Time : ").append(timeFormat).append("\n");

    return ret;
}

std::string KyPlugin::exportReport(void)
{
    std::string ret;
    for (std::map<std::string, std::string>::iterator it = m_report.begin(); it != m_report.end(); it++) {
        ret.append(it->second);
    }

    return ret;
}

}