/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "SoundThemeModule.h"

#include <iostream>
#include <filesystem>
#include <vector>
#include <pugixml.hpp>

#include "AudioContext.h"


namespace UkuiAudioFramwork
{
std::string SoundThemeInfo::toString() const
{
    return FORMAT("name: {}, description: {}", name, description);
}

std::string SoundEffectFileInfo::toString() const
{
    return FORMAT("name: {}, description: {}", name, description);
}

SoundThemeModule& SoundThemeModule::getInstance()
{
    static SoundThemeModule instance;
    return instance;
}

std::list<SoundThemeInfo> SoundThemeModule::getSoundThemeList()
{
    setupThemeSelector();
    return m_soundThemeList;
}

std::list<SoundEffectFileInfo> SoundThemeModule::getSoundEffectFileList(const std::string& themeName)
{
    std::list<SoundEffectFileInfo> list;

    // 定义主要搜索路径和备用路径
    std::vector<std::string> soundDirs = {
        "/usr/share/sounds",
        "/opt/system/resource/sounds"
    };

    // 获取所有音效主题的 XML 文件
    auto xmlFiles = getSoundThemeXmlFiles(soundDirs);
    if (xmlFiles.empty()) {
        AUDIO_ERROR("No sound theme XML files found in any search directories.");
        return {};
    }

    // 遍历每个 XML 文件并提取音效列表
    for (const auto& xmlFile : xmlFiles) {
        AUDIO_DEBUG("getSoundEffectFileList xmlFile: {} .", xmlFile);
        //start parse all sound
        // if ((xmlFile.find(themeName) == std::string::npos)
        //     && (themeName.compare("custom"))) {
        //     continue;
        // }
        //end

        auto soundList = getSoundListFromXml(xmlFile);
        if (soundList.empty()) {
            AUDIO_ERROR("No sounds found in {}.", xmlFile);
            continue;
        }

        // 输出音效列表
        for (const auto& sound : soundList) {
            list.push_back({sound.name, sound.description});
        }
    }

    return list;
}

void SoundThemeModule::setupThemeSelector()
{
    GHashTable* hash;
    const char* const* dataDirs;
    std::string dataDir, dir;
    uint i;

    m_soundThemeList.clear();
    /* Add the theme names and their display name to a hash table,
     * makes it easy to avoid duplicate themes */
    hash = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    {
        dataDirs = g_get_system_data_dirs();
        for (i = 0; dataDirs[i] != nullptr; ++i) {
            dir = g_build_filename(dataDirs[i], "sounds", nullptr);
            soundThemeInDir(hash, dir);
        }
    }

    dataDir = g_get_user_data_dir();
    dir = g_build_filename(dataDir.c_str(), "sounds", nullptr);
    soundThemeInDir(hash, dir);

    /* If there isn't at least one theme, make everything
     * insensitive, LAME! */
    if (g_hash_table_size(hash) == 0) {
        AUDIO_WARN("Bad setup, install the freedesktop sound theme");
        g_hash_table_destroy(hash);
        return;
    }
    g_hash_table_destroy(hash);
}

void SoundThemeModule::soundThemeInDir(GHashTable* hash, const std::string& dir)
{
    GDir* d;
    std::string name;

    d = g_dir_open(dir.c_str(), 0, nullptr);
    if (nullptr == d) {
        return;
    }

    while(auto dName = g_dir_read_name(d)) {
        name = dName;
        std::string dirName, index, indexName;
        /* Look for directories */
        dirName = g_build_filename(dir.c_str(), name.c_str(), nullptr);
        if (g_file_test(dirName.c_str(), G_FILE_TEST_IS_DIR) == false) {
            continue;
        }

        /* Look for index files */
        index = std::string(g_build_filename(dirName.c_str(), "index.theme", nullptr));

        /* Check the name of the theme in the index.theme file */
        indexName = getIndexThemeName(index);

        if ("" == indexName || "Default" == indexName) {
            continue;
        }

        SoundThemeInfo info;
        info.name = name;
        info.description = indexName;
        AUDIO_DEBUG("Get soudtheme theme: {}, desc: {}", info.name, info.description);
        gchar* key = g_strdup(name.c_str());
        if (!g_hash_table_lookup(hash, key)) {
            g_hash_table_insert(hash, key, g_strdup(indexName.c_str()));
            m_soundThemeList.push_back(info);
        } else {
            g_free(key);
        }
    }

    g_dir_close (d);
}

/*
 *   获取下标的主题名
 */
std::string SoundThemeModule::getIndexThemeName(const std::string& index)
{
    GKeyFile* file;
    std::string indexname = "";
    bool hidden;

    file = g_key_file_new();
    if (g_key_file_load_from_file(file, index.c_str(), G_KEY_FILE_KEEP_TRANSLATIONS, nullptr) == false) {
        g_key_file_free(file);
        return indexname;
    }

    /* Don't add hidden themes to the list */
    hidden = g_key_file_get_boolean(file, "Sound Theme", "Hidden", nullptr);

    if (!hidden) {
        indexname = g_key_file_get_locale_string(file, "Sound Theme", "Name", nullptr, nullptr);
    }
    g_key_file_free(file);

    return indexname;
}

std::vector<std::string> SoundThemeModule::getSoundThemeXmlFiles(const std::vector<std::string>& soundDirs)
{
    std::vector<std::string> xmlFiles;

    for (const auto& soundDir : soundDirs) {
        try {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(soundDir)) {
                if (entry.is_regular_file() && entry.path().extension() == ".xml") {
                    //start
                    std::string xmlFilePath = entry.path().string();
                    size_t last_slash = xmlFilePath.find_last_of('/');
                    std::string themeDirs = xmlFilePath.substr(0, last_slash+1);
                    if (isExistDirPath(themeDirs.append("stereo"))) {
                        AUDIO_DEBUG("getSoundThemeXmlFiles push_back: {} .", entry.path().string());
                        xmlFiles.push_back(entry.path().string());
                    } else {
                        AUDIO_DEBUG("getSoundThemeXmlFiles stereoDir no found .");
                    }
                    //end
                }
            }
        } catch (const std::exception& e) {
            AUDIO_ERROR("Error accessing directory {} {}.", soundDir, e.what());
        }
    }

    return xmlFiles;
}

std::vector<SoundEffectFileInfo> SoundThemeModule::getSoundListFromXml(const std::string& xmlFilePath)
{
    std::vector<SoundEffectFileInfo> soundList;
    pugi::xml_document doc;
    if (!doc.load_file(xmlFilePath.c_str())) {
        std::cerr << "Failed to load XML file: " << xmlFilePath << std::endl;
        return {};
    }

    auto sounds = doc.child("sounds");
    if (!sounds) {
        std::cerr << "Invalid sound theme XML file: " << xmlFilePath << std::endl;
        return {};
    }

    std::string currentLanguage = std::getenv("LANGUAGE") ? : "";
    AUDIO_DEBUG("getSoundListFromXml currentLanguage: {}", currentLanguage);
    for (const auto& sound : sounds.children("sound")) {
        SoundEffectFileInfo soundInfo {};

        auto defaultNameNode = sound.child("filename");
        if (defaultNameNode) {
            soundInfo.name = defaultNameNode.child_value();
        } else {
            continue;
        }

        if (!currentLanguage.empty()) {
            auto descriptionNode = sound.find_child_by_attribute("name", "xml:lang", currentLanguage.substr(0, currentLanguage.find(':')).c_str());
            if (descriptionNode) {
                soundInfo.description = descriptionNode.child_value();
            } else {
                //start adapter en language
                auto nameNode = sound.child("name");
                if (nameNode) {
                    soundInfo.description = nameNode.child_value();
                }
                //end
            }
        }

        soundList.push_back(soundInfo);
    }
    return soundList;
}

bool SoundThemeModule::isExistDirPath(const std::string& path)
{
    bool ret = false;
    std::filesystem::path dir(path);
    if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
        AUDIO_DEBUG("Dir: {} found .", dir);
        ret = true;
    } else {
        AUDIO_DEBUG("Dir: {} no found .", dir);
    }

    return ret;
}
}
