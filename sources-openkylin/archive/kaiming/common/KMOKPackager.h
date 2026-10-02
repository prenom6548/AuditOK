/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMOKPACKAGE_H__
#define __KMOKPACKAGE_H__

#include <string>
#include <memory>
#include <vector>
#include <optional>
#include <set>

#include "KMInfoJson.h"
#include "KMOABMetadata.h"
#include "KMSubPackagesJson.h"

class KMOKPackager
{
public:
    struct Layer
    {
        std::string m_layerDir;
        std::shared_ptr<KMInfoJson> m_info;
    };

public:
    KMOKPackager(const std::string &buildDir);
    ~KMOKPackager();

    void setBundleFilesystem(const std::string &filesystem);

    void setCompression(const std::string &compression);

    void setModulePaths(const std::string &moduleName, const std::string& paths);

    void setSubpackageMode(std::shared_ptr<KMSubPackagesJson> subpackagesJson, const KMSubPackagesJson::SubPackage& subpackage);
    
    void appendLayer(Layer layer);

    void pack(const std::string &okFilename);

private:
    void addNewSection(const std::string &sectionName, const std::string &dataFile);

    void packBundle();

    void prepareBundle(const std::string &bundleDir);
    void prepareBundleBySubpackageMode(const std::string &bundleDir);

    void amendLayer(Layer& layer);
    void amendSubpackage(std::vector<std::string> depends, std::set<std::string>& noRepeat);
    void copyChangelogToBundle(const std::string& appDir);
    void copyDependsToBundle(const std::vector<std::string>& depends, const std::string& appDir, std::vector<std::string>& appDepends, std::set<std::string>& noRepeat);
    void recordDependsInfo(const std::string& appDir, const std::vector<std::string>& appDepends);
    void copyIncludesToBundle(const std::string& srcDir, const std::string& destDir);
    void exportEntries(const std::string& appDir);

    void generateErofsBundle(const std::string &bundleFile, const std::string &bundleDir);
    void generateSquashfsBundle(const std::string &bundleFile, const std::string &bundleDir);

    void packMetadata();

private:
    class Private;
    std::shared_ptr<Private> d;
};

#endif // !__KMOKPACKAGE_H__
