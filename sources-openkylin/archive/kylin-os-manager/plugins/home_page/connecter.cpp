#include "home_page.h"

extern "C"
bool kyconnect(kyplugin::Host &host) {
    host.add(new home_page::HomePageProvider);
    return true;
}
