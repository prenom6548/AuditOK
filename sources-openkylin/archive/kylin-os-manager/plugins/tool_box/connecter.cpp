#include "tool_box.h"

extern "C" bool kyconnect(kyplugin::Host &host) {
    host.add(new tool_box::ToolBoxProvider);
    return true;
}
