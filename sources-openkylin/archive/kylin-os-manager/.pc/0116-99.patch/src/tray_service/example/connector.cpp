#include "protocol.h"

extern "C"
bool kyconnect(kyplugin::Host &host) {
    host.add(new example::ProtocolProvider());

    return true;
}