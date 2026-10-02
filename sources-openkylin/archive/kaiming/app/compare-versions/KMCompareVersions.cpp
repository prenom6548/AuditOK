/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

 #include "KMCompareVersions.h"

 #include <unordered_map>

 #include "common/KMBuildinOptions.h"
 #include "common/KMException.h"
 #include "common/KMLogger.h"
 #include "common/kmtranslation.h"
 #include "common/KMVersion.h"
 
 class KMCompareVersions::Options : public KMOption::Options
 {
 public:
     Options() = default;
     virtual ~Options() = default;
 
 protected:
     void preParseHook();
     void postParseHook();
 
 private:
     void addOptions();
 
 public:
     bool m_help = false;
     std::string m_version1;
     std::string m_operator;
     std::string m_version2;
 };
 
 void KMCompareVersions::Options::preParseHook()
 {
     addOptions();
 }
 
 void KMCompareVersions::Options::postParseHook()
 {
     if (m_help)
     {
         showUsage();
         exit(EXIT_SUCCESS);
     }
 
     if (m_version1.empty() || m_operator.empty() || m_version2.empty())
     {
         throw KMException("<version1> <operator> <version2> must be specified");
     }
 }
 
 void KMCompareVersions::Options::addOptions()
 {
     setDescription(_("\nUsage:\n \tkaiming compare-versions <version1> <operator> <version2>\n"));
 
     addOption("help", "h", KMOption::value(&m_help), _("Show help options"));
 
     addPositionOption("version1", KMOption::value(&m_version1), 1, _("The version of kaiming package."));
     addPositionOption("operator", KMOption::value(&m_operator), 1, _("Comparison operator. Such as : lt(or '<' or \"<\"),le(or <=),eq(or ==),ne(or !=),ge(>=),gt(or '>' or \">\")"));
     addPositionOption("version2", KMOption::value(&m_version2), 1, _("The version of kaiming package."));
 }
 
 class KMCompareVersions::Private
 {
 public:
     typedef bool (Private::*MemFuncPointer)(const std::string&, const std::string&);
 public:
     Private();
     ~Private() = default;

     bool establish();

     bool lt(const std::string& version1, const std::string& version2);
     bool le(const std::string& version1, const std::string& version2);
     bool eq(const std::string& version1, const std::string& version2);
     bool ne(const std::string& version1, const std::string& version2);
     bool ge(const std::string& version1, const std::string& version2);
     bool gt(const std::string& version1, const std::string& version2);

 public:
     std::unique_ptr<Options> m_kmOptions;
     std::unordered_map<std::string, MemFuncPointer> m_mapMemFunc;
 };

 KMCompareVersions::Private::Private()
 {
     m_mapMemFunc.insert({ "lt", &Private::lt });
     m_mapMemFunc.insert({ "<",  &Private::lt });
     m_mapMemFunc.insert({ "le", &Private::le });
     m_mapMemFunc.insert({ "<=", &Private::le });
     m_mapMemFunc.insert({ "eq", &Private::eq });
     m_mapMemFunc.insert({ "==", &Private::eq });
     m_mapMemFunc.insert({ "ne", &Private::ne });
     m_mapMemFunc.insert({ "!=", &Private::ne });
     m_mapMemFunc.insert({ "ge", &Private::ge });
     m_mapMemFunc.insert({ ">=", &Private::ge });
     m_mapMemFunc.insert({ "gt", &Private::gt });
     m_mapMemFunc.insert({ ">",  &Private::gt });
 }

 bool KMCompareVersions::Private::lt(const std::string& version1, const std::string& version2)
 {
     return KMVersion::compare(version1, version2) < 0;
 }

 bool KMCompareVersions::Private::le(const std::string& version1, const std::string& version2)
 {
     return KMVersion::compare(version1, version2) <= 0;
 }

 bool KMCompareVersions::Private::eq(const std::string& version1, const std::string& version2)
 {
     return KMVersion::compare(version1, version2) == 0;
 }

 bool KMCompareVersions::Private::ne(const std::string& version1, const std::string& version2)
 {
     return KMVersion::compare(version1, version2) != 0;
 }

 bool KMCompareVersions::Private::ge(const std::string& version1, const std::string& version2)
 {
     return KMVersion::compare(version1, version2) >= 0;
 }

 bool KMCompareVersions::Private::gt(const std::string& version1, const std::string& version2)
 {
     return KMVersion::compare(version1, version2) > 0;
 }

 bool KMCompareVersions::Private::establish()
 {
    MemFuncPointer pMemFunc = nullptr;
    auto it = m_mapMemFunc.find(m_kmOptions->m_operator);
    if (it != m_mapMemFunc.end())
    {
        pMemFunc = it->second; 
    }
    else
    {
        throw KMException("Unsupported operator : " + m_kmOptions->m_operator);
    }

    return (this->*pMemFunc)(m_kmOptions->m_version1, m_kmOptions->m_version2);
 }
 
 REGISTER_SUBCOMMAND_DYNCREATE(compare-versions, KMCompareVersions)
 
 KMCompareVersions::KMCompareVersions()
     : d(std::make_unique<Private>())
 {
     d->m_kmOptions = std::make_unique<Options>();
 }
 
 KMCompareVersions::~KMCompareVersions()
 {
 }
 
 int KMCompareVersions::dispose(int argc, char **argv)
 {
     init(argc, argv);
 
     return run();
 }
 
 void KMCompareVersions::init(int argc, char **argv)
 {
     d->m_kmOptions->parseCommandLine(argc, argv);
 }
 
 int KMCompareVersions::run()
 {
     if (d->establish())
     {
         KMInfo("'" + d->m_kmOptions->m_version1 + " " + d->m_kmOptions->m_operator + " " + d->m_kmOptions->m_version2 + "' is right.");
         return EXIT_SUCCESS;
     }

     KMInfo("'" + d->m_kmOptions->m_version1 + " " + d->m_kmOptions->m_operator + " " + d->m_kmOptions->m_version2 + "' is wrong.");
     return EXIT_FAILURE;
 }
 