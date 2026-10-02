#include "protocol.h"

extern "C"
bool kyconnect(kyplugin::Host &host) {
    host.add(new application_example::ProtocolProvider());
    return true;
}
