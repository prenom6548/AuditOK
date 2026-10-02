#include "KMBuilderDebian.h"
#include "KMBuilderUtils.h"

KMBuilderDebian::KMBuilderDebian()
{

}

KMBuilderDebian::~KMBuilderDebian()
{

}

void KMBuilderDebian::parseControl(const std::string &path)
{
    m_control = KMBuilderUtils::parseTxt(path);
    m_source = getStrFromKey(m_control, "Source:");
    m_maintainer = getStrFromKey(m_control, "Maintainer:");
    m_homepage = getStrFromKey(m_control, "Homepage:");
    m_description = getStrFromKey(m_control, "Description:");

    getDepends();
}

void KMBuilderDebian::parseCopyRight(const std::string &path)
{
    std::vector<std::string> copyright = KMBuilderUtils::parseTxt(path);
    m_license = getStrFromKey(copyright, "License:");
}

std::string KMBuilderDebian::getStrFromKey(const std::vector<std::string> &data, const std::string &key)
{
    bool flag = false;
    for (auto &line: data)
    {
        if (flag)
        {
            return KMBuilderUtils::trimLeftStr(line);
        }
        if (KMBuilderUtils::startsWith(line,key))
        {
            if (KMBuilderUtils::splitString(line, ":").size() >= 2)
            {
                size_t index = line.find_first_of(':');
                return KMBuilderUtils::trimLeftStr(line.substr(index+1));
            }
            flag = true;
        }
    }
    return "";
}

std::map<std::string, std::string> KMBuilderDebian::parseDepend(const std::string &line, bool isOr)
{
    std::string depend = KMBuilderUtils::trimStr(line);
    std::string core = KMBuilderUtils::getSysCore();
    std::map<std::string, std::string> result;

    // depend中出现或|,例如a|b
    std::vector<std::string> depends = KMBuilderUtils::splitString(depend, "|");
    if (depends.size() > 1)
    {
        std::map<std::string, std::string> or_depends;
        for (auto &item: depends)
        {
            std::map<std::string, std::string> result = parseDepend(item, true);
            if (!result.empty())
            {
                for (auto &depend : result)
                {
                    or_depends[depend.first] = depend.second;
                }
            }
        }
        m_or_depends.push_back(or_depends);
        return result;
    }

    size_t left_pos = depend.find("(");
    if (left_pos != std::string::npos)
    {
        size_t right_pos = depend.find(")");
        if (right_pos != std::string::npos)
        {
            if (!isOr)
            {
                m_depends[KMBuilderUtils::trimStr(depend.substr(0,left_pos))] = KMBuilderUtils::trimStr(depend.substr(left_pos+1, right_pos - left_pos - 1));
            }
            result[KMBuilderUtils::trimStr(depend.substr(0,left_pos))] = KMBuilderUtils::trimStr(depend.substr(left_pos+1, right_pos - left_pos - 1));
        }
        else
        {
            if (!isOr)
            {
                m_depends[KMBuilderUtils::trimStr(depend.substr(0,left_pos))] = KMBuilderUtils::trimStr(depend.substr(left_pos+1));
            }
            result[KMBuilderUtils::trimStr(depend.substr(0,left_pos))] = KMBuilderUtils::trimStr(depend.substr(left_pos+1));
        }
        
    }
    else
    {
        left_pos = depend.find("<");
        // <!nocheck>
        if (left_pos != std::string::npos)
        {
            size_t right_pos = depend.find(">");
            if (right_pos != std::string::npos)
            {
                if (!isOr)
                {
                    m_depends[KMBuilderUtils::trimStr(depend.substr(0,left_pos))] = KMBuilderUtils::trimStr(depend.substr(left_pos, right_pos - left_pos + 1));
                }
                result[KMBuilderUtils::trimStr(depend.substr(0,left_pos))] = KMBuilderUtils::trimStr(depend.substr(left_pos, right_pos - left_pos + 1));
            }
            else
            {
                if (!isOr)
                {
                    m_depends[KMBuilderUtils::trimStr(depend.substr(0,left_pos))] = KMBuilderUtils::trimStr(depend.substr(left_pos));
                }
                result[KMBuilderUtils::trimStr(depend.substr(0,left_pos))] = KMBuilderUtils::trimStr(depend.substr(left_pos));
            }
        }
        else
        {
            left_pos = depend.find("[");
            if (left_pos != std::string::npos)
            {
                if (KMBuilderUtils::contains(KMBuilderUtils::toLowerUtf8(depend.substr(left_pos)), KMBuilderUtils::toLowerUtf8(core)))
                {
                    if (!isOr)
                    {
                        m_depends[KMBuilderUtils::trimStr(depend.substr(0,left_pos))] = "";
                    }
                    result[KMBuilderUtils::trimStr(depend.substr(0,left_pos))] = "";
                }
            }
            else
            {
                if (!isOr)
                {
                    m_depends[KMBuilderUtils::trimStr(depend)] = "";
                } 
                result[KMBuilderUtils::trimStr(depend)] = "";
            }
        }
    }
    return result;
}

void KMBuilderDebian::getDepends(const std::string &dependKind)
{
    bool flag = false;
    std::string binaryVersion_str = "${binary:Version}";
    std::string sourceVersion_str = "${source:Version}";
    std::string sourceVersion;
    for (auto &line: m_control)
    {
        if (KMBuilderUtils::startsWith(line, "#"))
        {
            continue;
        }
        if (KMBuilderUtils::startsWith(line, dependKind))
        {
            flag = true;
            if (KMBuilderUtils::splitString(line, ":").size() < 2)
            {
                continue;
            }
            size_t pos_comma = line.find_first_of(':');
            std::string line_end = line.substr(pos_comma+1);
            std::vector<std::string> depends = KMBuilderUtils::splitString(line_end, ",");
            for (auto &depend : depends)
            {
                if (!depend.empty())
                {
                    parseDepend(depend);
                }
            }
            if (!KMBuilderUtils::endsWith(line, ","))
            {
                flag = false;
            }
            continue;
        }

        else if (flag)
        {
            //control中depends字段部分结束
            size_t pos_dollar = line.find_first_of("$");
            size_t pos_semi = line.find_first_of(':');
            if (pos_dollar > pos_semi)
            {
                flag = false;
                continue;
            }
            std::vector<std::string> depends = KMBuilderUtils::splitString(line, ",");
            if (depends.size() > 1)
            {
                for (auto &depend : depends)
                {
                    if (!depend.empty())
                    {
                        parseDepend(depend);
                    }
                }
            }
            else
            {
                parseDepend(line);
            }

            if (!KMBuilderUtils::endsWith(line, ","))
            {
                flag = false;
            }
        }
        else
        {
            continue;
        }
    }
}


void KMBuilderDebian::getDepends()
{
    //build-depends
    getDepends("Build-Depends:");

    //depends
    getDepends("Depends:");
}