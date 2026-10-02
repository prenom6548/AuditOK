/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __BUILD_CONTEXT_H__
#define __BUILD_CONTEXT_H__

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "KMBuilderManifest.h"
#include "common/KMSingleton.h"

namespace fs = std::filesystem;

#define STATE_DIR ".kaiming-builder"
#define builderContext KMBuilderContext::instance()

class KMBuilderContext : public KMSingleton<KMBuilderContext>
{
public:
    KMBuilderContext(token);
    ~KMBuilderContext();

    void parseCommandLine(int argc, char **argv);
    void loadConfig();

    std::string getArch() const;

    bool isBuildOnly() const;

    bool isExportOnly() const;

    bool isFinishOnly() const;

    bool isForceClean() const;

    bool isExport() const;

    bool isExportAll() const;

    bool isExportAppImage() const;

    bool isCcache() const;

    bool isVerbose() const;

    std::string getManifestPath() const;

    std::string getBaseDir() const;

    std::string getAppDir() const;

    std::string getRunDir() const;

    std::string getStateDir() const;

    std::string getCcacheDir() const;

    std::string getCacheDir() const;

    std::string getBuildDir() const;

    std::string getDownloadsDir() const;

    std::string getBundleFilesystem() const;

    std::string getCompression() const;

    bool getIsKeepBuildDirs();

    int getJobs() const;

    std::string getStopat() const;

    std::shared_ptr<KMBuilderManifest> manifestConfig();

private:

    void showCmdline() const;

    void checkOptions();

private:

    class Options;
    class Private;
    std::unique_ptr<Private> d;

};

#endif // !__BUILD_CONTEXT_H__
