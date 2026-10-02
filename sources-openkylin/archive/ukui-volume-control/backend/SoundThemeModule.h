#ifndef SOUNDTHEME_H
#define SOUNDTHEME_H

#include <string>
#include <list>
#include <vector>

#include <gio/gio.h>
#include <libxml/tree.h>
#include <glib-object.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <gobject/gparamspecs.h>
#include <glib/gstdio.h>

namespace UkuiAudioFramwork
{

typedef struct SoundThemeInfo {
    std::string name;           // 音效主题的名称
    std::string description;    // 音效主题的描述

    std::string toString() const;
} SoundThemeInfo;

typedef struct SoundEffectFileInfo {
    std::string name;
    std::string description;

    std::string toString() const;
} SoundEffectFileInfo;

class SoundThemeModule
{
public:
    static SoundThemeModule& getInstance();

public:
    std::list<SoundThemeInfo> getSoundThemeList();

    std::list<SoundEffectFileInfo> getSoundEffectFileList(const std::string&);

private:
    std::string getIndexThemeName(const std::string&);

    void soundThemeInDir(GHashTable*, const std::string&);

    void setupThemeSelector();

    std::vector<std::string> getSoundThemeXmlFiles(const std::vector<std::string>& soundDirs);

    std::vector<SoundEffectFileInfo> getSoundListFromXml(const std::string& xmlFilePath);

    bool isExistDirPath(const std::string&);

private:
    SoundThemeModule() = default;
    SoundThemeModule(const SoundThemeModule&) = delete;
    SoundThemeModule(SoundThemeModule&&) = delete;
    SoundThemeModule operator=(const SoundThemeModule&) = delete;
    SoundThemeModule operator=(SoundThemeModule&&) = delete;
    virtual ~SoundThemeModule() = default;

private:
    std::list<SoundThemeInfo> m_soundThemeList {};
};
}
#endif // SOUNDTHEME_H
