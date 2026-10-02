/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuilderQuery.h"
#include "common/kmtranslation.h"

#define REPOPATH "/opt/kaiming/repo/config"
#define LAYERSPATHOPT "/opt/kaiming/layers"

class KMBuilderQuery::Options: public KMOption::Options
{
public:
    std::string m_kind;
    std::string m_idPart;
    std::string m_version;
    std::string m_module;
    std::string m_base;
    std::string m_runtime;

    bool m_isHelp;
public:
    Options() = default;
    virtual ~Options() = default;

protected:
    void preParseHook() override;
    void postParseHook() override;

private:
    void addOptions();
};

void KMBuilderQuery::Options::preParseHook()
{
    addOptions();
}

void KMBuilderQuery::Options::postParseHook()
{
    if(m_isHelp)
    {
        showUsage();
        exit(EXIT_SUCCESS);
    }
}


void KMBuilderQuery::Options::addOptions()
{
    setDescription(_("\nUsage\n \tkaiming-builder query [OPTION…]\n"));
    
    addOption("help","h",KMOption::value<bool>(&m_isHelp),_("Show help options"));

    addOption("kind","", KMOption::value<std::string>(&m_kind), _("The kind of kaiming package, such as app/base/runtime/depend and so on"));
    addOption("id", "", KMOption::value<std::string>(&m_idPart), _("The id of kaiming package. It supports entering only a part of the exact ID and supports wildcards"));
    addOption("version","", KMOption::value<std::string>(&m_version), _("The matching version"));
    addOption("module", "", KMOption::value<std::string>(&m_module), _("Module, including binary and devel. Binary for running, devel for development"));
    addOption("base","", KMOption::value<std::string>(&m_base), _("The basic environmental information used by the application. Format: {channel}:{id}/{version}, example: stable:top.openkylin.base/1.0.0"));
    addOption("runtime", "", KMOption::value<std::string>(&m_runtime), _("The runtime environmental information used by the application. Format: {channel}:{id}/{version}, example: stable:top.openkylin.ukui/1.0.0"));
}

class KMBuilderQuery::Private
{
public:
    int m_argc;
    char **m_argv;
    std::unique_ptr<Options>m_kmOptions;
};
REGISTER_SUBCOMMAND_DYNCREATE(query, KMBuilderQuery)

KMBuilderQuery::KMBuilderQuery()
    : d(std::make_unique<Private>())
{
    d->m_kmOptions = std::make_unique<Options>();
}

KMBuilderQuery::~KMBuilderQuery()
{

}

int KMBuilderQuery::dispose(int argc, char **argv)
{
    init(argc, argv);
    return run();
}

void KMBuilderQuery::init(int argc, char **argv)
{
    d->m_kmOptions->parseCommandLine(argc,argv);
}

int KMBuilderQuery::run()
{
    std::string arch = KMBuilderUtils::getArch();
    std::vector<KMRef> refs = getRefsFromLocal(arch, 
                                               d->m_kmOptions->m_kind, 
                                               d->m_kmOptions->m_module,
                                               d->m_kmOptions->m_idPart,
                                               d->m_kmOptions->m_version, 
                                               d->m_kmOptions->m_base, 
                                               d->m_kmOptions->m_runtime);

    std::vector<KMRef> refs_summary = getRefsFromSummary(arch, 
                                                         d->m_kmOptions->m_kind, 
                                                         d->m_kmOptions->m_module,
                                                         d->m_kmOptions->m_idPart,
                                                         d->m_kmOptions->m_version, 
                                                         d->m_kmOptions->m_base, 
                                                         d->m_kmOptions->m_runtime);
    KMBuilderUtils::concatVectors(refs, refs_summary);
    std::sort(refs.begin(), refs.end());
    printRef(refs);
    return EXIT_SUCCESS;
}

void KMBuilderQuery::findTargetFile(const std::string &path,std::vector<std::string>&info_paths, const std::string &target_filename)
{
    for(auto &file: fs::directory_iterator(path))
    {
        std::string filename = file.path().filename();
        if (filename == "files" || filename == "entries" || fs::is_symlink(fs::path(file)))
        {
            continue;
        }
        if (fs::is_directory(fs::path(file)))
        {
            findTargetFile(file.path().string(), info_paths, target_filename);
        }
        if (file.is_regular_file() && filename == target_filename)
        {
            info_paths.push_back(file.path());
        }
    }
}

bool KMBuilderQuery::checkSummary(const std::string &path)
{
    if (!fs::exists(path))
    {
        return false;
    }
    std::ifstream file(path);
    if (!file.is_open())
    {
        return false;
    }
    nlohmann::json json;
    try
    {
        file >> json;
    }
    catch (const json::parse_error& e)
    {
        return false;
    }
    // if (KMBuilderUtils::isFileExpired(path, 3600*24))
    // {
    //     return false;
    // }
    return true;
}

std::vector<KMRef> KMBuilderQuery::getRefsFromSummary(const std::string &arch, const std::string &kind, const std::string &model, const std::string &id, const std::string &version, const std::string &base, const std::string &runtime)
{
    std::vector<std::string> args;
    args.push_back("search");
    args.push_back("--kind=base");
    std::string ret;
    KMProcessUtils::spawn("kaiming", args, ret);

    std::vector<std::string> summarys;
    {
        std::string result;
        args.clear();
        args.push_back("-c");
        args.push_back("ls -1 /opt/kaiming/summary*.json");
        if (KMProcessUtils::spawn("bash", args, result))
        {
            summarys = KMBuilderUtils::splitString(result, "\n");
        }
    }

    std::vector<KMRef> refs;
    for (auto& summary : summarys)
    {
        KMParseSummaryJson summaryJson(summary);
        std::vector<KMRef> results = summaryJson.search(arch, kind, model, id, version, base, runtime);

        for (auto& ref : results)
        {
            bool exist = false;
            for (auto& tmp_ref : refs)
            {
                if (tmp_ref == ref)
                {
                    exist = true;
                    break;
                }
            }

            if (exist)
            {
                continue;
            }

            refs.push_back(ref);
        }
    }

    return refs;
}

std::vector<KMRef> KMBuilderQuery::getRefsFromLocal(const std::string &arch, const std::string &kind, const std::string &model, const std::string &id, const std::string &version, const std::string &base, const std::string &runtime, bool binary)
{
    std::vector<KMRef> refs;
    std::vector<std::string> infoJsons;
    if (binary)
    {
        findTargetFile(LAYERSPATHOPT, infoJsons, "info.json");
    }
    
    std::string layers_local = KMBuilderUtils::buildFilename(KMBuilderUtils::getHomeDir(), ".local", "share");
    std::string layers_home_path = KMBuilderUtils::buildFilename(layers_local, "kaiming", "layers");
    findTargetFile(layers_home_path, infoJsons, "info.json");

    for (auto &infojson: infoJsons)
    {
        KMRef ref;
        ref.loadFromInfoJson(infojson);

        if (!kind.empty() && ref.m_kind != kind)
        {
            continue;
        }
        if (!model.empty() && ref.m_module != model)
        {
            continue;
        }
        if (!id.empty() && !KMBuilderUtils::matchID(ref.m_id, id))
        {
            continue;
        }
        if (!version.empty() && !KMBuilderUtils::matchVersion(ref.m_version, version))
        {
            continue;
        }
        if (!KMBuilderUtils::checkEnvironment(ref.m_base, base) || !KMBuilderUtils::checkEnvironment(ref.m_runtime, runtime))
        {
            continue;
        }
        ref.m_repo = "local";
        refs.push_back(ref);
    }
    return refs;

}

void KMBuilderQuery::printRef(const std::vector<KMRef>&refs)
{
    std::vector<KMEntry> entries;
    for (auto &ref: refs)
    {
        KMEntry entry;
        entry.id = ref.m_id;
        entry.name = ref.m_name;
        entry.version = ref.m_version;
        entry.channel = ref.m_channel;
        entry.arch = ref.m_arch;
        entry.module = ref.m_module;
        entry.description = ref.m_description;
        entry.repo = ref.m_repo;
        entry.kind = ref.m_kind;
        entries.push_back(entry);
    }
    KMFormatter format;
    format.setShowSize(false);
    format.printEntries(entries);
}
