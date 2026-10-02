// SPDX-FileCopyrightText: 2024 KI18n Rust Backend
// SPDX-License-Identifier: LGPL-2.0-or-later
// R05: KTimeZone bridge

#include "ktimezone.h"
#include "kcountry.h"

#include <string>
#include <cstring>

extern "C" {
int ki18n_rs_timezone_from_location(float, float, char**);
int ki18n_rs_timezone_country(const char*, void**);
void ki18n_rs_country_destroy(void*);
int ki18n_rs_country_index(const void*);
void ki18n_rs_string_free(char*);
}

namespace KTimeZone {

const char* fromLocation(float latitude, float longitude) {
    thread_local static std::string last;
    last.clear();
    char* out = nullptr;
    if (ki18n_rs_timezone_from_location(latitude, longitude, &out) != 0 || !out) {
        return nullptr;
    }
    last = out;
    ki18n_rs_string_free(out);
    return last.c_str();
}

KCountry country(const char* ianaId) {
    if (!ianaId || !*ianaId) return {};
    void* h = nullptr;
    if (ki18n_rs_timezone_country(ianaId, &h) != 0 || !h) return {};
    int idx = ki18n_rs_country_index(h);
    ki18n_rs_country_destroy(h);
    KCountry c;
    c.d = static_cast<uint16_t>(idx >= 0 ? idx + 1 : 0);
    return c;
}

}
