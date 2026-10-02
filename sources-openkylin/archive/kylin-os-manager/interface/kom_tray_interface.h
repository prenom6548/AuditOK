#ifndef KYMANAGER_TRAY_INTERFACE_H
#define KYMANAGER_TRAY_INTERFACE_H

#include <string>
#include <functional>
#include <vector>

#include "kyplugin.h"

enum class KomActionType {
    NONE,

    // 可以包含一个左侧的图标和文本
    NORMAL,

    // 显示为一个分隔线
    // 分隔线类型下，ActionOption 中所有选项失效
    SEPARATORS,

    // 复选类型，选中左侧图标显示对号，不选中左侧不显示图标
    // 该类型下 ActionOption 中 iconPath 字段失效
    // ActionOption 中的 callBack 返回 ture 表示选中，false 表示不选中
    CHECK
};

typedef struct _KomActionEntry KomActionEntry;
struct _KomActionEntry {
    _KomActionEntry(std::string name,
                    std::string label,
                    std::function<bool()> callback,
                    std::string icon = "",
                    std::string tooltip = "",
                    KomActionType type = KomActionType::NORMAL,
                    bool isActive = true)
        : name(name)
        , label(label)
        , callback(callback)
        , icon(icon)
        , tooltip(tooltip)
        , type(type)
        , isActive(isActive) {}

    std::string name;
    std::string label;
    std::function<bool()> callback;
    std::string icon;
    std::string tooltip;
    KomActionType type;
    bool isActive;
};

class KomTrayInterface {
public:
    virtual ~KomTrayInterface() {}
    virtual std::string name() = 0;
    virtual std::string icon() = 0;
    virtual std::string tooltip() = 0;
    virtual void start() {}
    virtual void stop() {}
    virtual void trigger() {}
    virtual std::vector<KomActionEntry> actions() {return std::vector<KomActionEntry>();}
};

class KomTrayProvider : public kyplugin::Provider {
public:
    virtual KomTrayInterface *create() const = 0;
    unsigned int getVersion() const {return providerVersion;}

private:
    friend class kyplugin::KyPlugin;
    std::string kypluginGetType() const {return providerType;}

    static const unsigned int providerVersion;
    static const unsigned int providerLowestVersion;
    static const std::string providerType;
};

#endif
