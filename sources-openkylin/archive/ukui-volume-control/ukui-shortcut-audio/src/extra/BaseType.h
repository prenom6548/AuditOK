#ifndef BASETYPE_H
#define BASETYPE_H

#include <list>
#include <memory>
#include <unordered_map>
#include <string>

// 快捷面板本地副本：与 backend/BaseType.h 对齐，但不依赖 simple_logger。

namespace UkuiAudioFramwork
{
enum class Module
{
    Service = 0,
    Tray,
    ControlCenter
};

//
enum class BackendType
{
    PULSEAUDIO = 0,
    PIPEWIRE,
    ALSA,
    JACK,
    UNKNOWN
};

enum class VolumeType
{
    BASE_VOLUME = 0,
    BALANCE_VOLUME
};

enum class NodeType
{
    INPUT = 0,
    OUTPUT,
    SINK_INPUT,
    SOURCE_OUTPUT
};

enum class PaDataType
{
    CARD = 0,
    SINK,
    SOURCE,
    SINK_INPUT,
    SOURCE_OUTPUT,
    MODULE
};

typedef struct NodeInfo
{
    uint32_t index;
    uint8_t channels;
} NodeInfo;
}

#endif // BASETYPE_H
