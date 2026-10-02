/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */
#include "KMBuilderNewProject.h"

#include<filesystem>
#include<cstring>

#include "common/KMBuildinOptions.h"
#include "KMBuilderUtils.h"
#include "common/kmtranslation.h"
#include "common/KMLogger.h"

namespace fs = std::filesystem;

class KMBuilderNewProject::Options: public KMOption::Options
{
public:
    Options() = default;
    virtual ~Options() = default;

protected:
    void preParseHook() override;
    void postParseHook() override;

private:
    void addOptions();

public:
    bool m_isHelp = false;
    std::string  m_appid = "top.openkylin.demo";
    
};

void KMBuilderNewProject::Options::preParseHook()
{
    addOptions();
}

void KMBuilderNewProject::Options::postParseHook()
{
    if(m_isHelp)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }

    std::string errMsg;
    if (!KMBuilderUtils::isValidAppName(m_appid, errMsg))
    {
        KMError(errMsg);
        exit(EXIT_FAILURE);
    }
}

void KMBuilderNewProject::Options::addOptions()
{
    setDescription(_("\nUsage\n \tkaiming-builder create [OPTION…] [APPID] \n"));

    addOption("help","h",KMOption::value<bool>(&m_isHelp),_("Show help options"));

    addPositionOption("APPID",KMOption::value<std::string>(&m_appid),1,_("The unique identifier of the app. If the appid is left blank, it is top.openkylin.demo by default"));

}

class KMBuilderNewProject::Private
{
public:
    std::unique_ptr<Options>m_kmOptions;
};

REGISTER_SUBCOMMAND_DYNCREATE(create, KMBuilderNewProject)

KMBuilderNewProject::KMBuilderNewProject()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
}

KMBuilderNewProject::~KMBuilderNewProject()
{

}

fs::path KMBuilderNewProject::createDir()
{
    fs::path currentPath = fs::current_path();
    fs::path destYamlPath = currentPath / d->m_kmOptions->m_appid ;

    if(fs::exists(destYamlPath))
    {
        std::cout << "The project dir exists" << std::endl;
    }
    else
    {
        fs::create_directory(destYamlPath);
        std::cout << "The project dir id created successfully" << std::endl;
    }
    return destYamlPath;

}

//因为以yaml格式读取yaml文件无法读取注释，所以暂时用字符文件格式处理yaml文件
void KMBuilderNewProject::createYaml()
{

    fs::path destDir = createDir();
    std::cout << "project creating..." << std::endl;

    //读取yaml模板
    std::ifstream yamlOriFile(m_yamlOriPath);
    std::vector<std::string> yamlContens;
    if(yamlOriFile.is_open())
    {
        std::string line;
        while(std::getline(yamlOriFile,line))
        {
            KMBuilderUtils::replace(line, "@id@", d->m_kmOptions->m_appid);
            yamlContens.push_back(line); 
        } 
    }

    //修改package中的id
    // std::vector<std::string>tmpIdValue = KMBuilderUtils::splitString(yamlContens[5] , ":");
    // yamlContens[5] = tmpIdValue[0] + ": " + d->m_kmOptions->m_appid;   

    //写入目标yaml文件
    fs::path destPath = destDir / (d->m_kmOptions->m_appid+".km.yaml");
    std::ofstream yamlTarget(destPath);
    for(auto line : yamlContens)
    {
        yamlTarget << line + "\n"; 
    }

    std::cout << "project created: " << destPath << std::endl;

}

int KMBuilderNewProject::dispose(int argc, char **argv)
{
    init(argc , argv);
    return run();
}

void KMBuilderNewProject::init(int argc , char **argv)
{
    d->m_kmOptions->parseCommandLine(argc,argv);
    
}

int KMBuilderNewProject::run()
{
    createYaml();
    return EXIT_SUCCESS;
}

