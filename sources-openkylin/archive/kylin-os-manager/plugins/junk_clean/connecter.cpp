#include "junk_clean.h"

extern "C"
bool kyconnect(kyplugin::Host &host) {
    host.add(new junk_clean::JunkCleanProvider);
    return true;
}
