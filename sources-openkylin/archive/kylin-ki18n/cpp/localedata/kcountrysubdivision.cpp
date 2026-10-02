// SPDX-FileCopyrightText: 2024 KI18n Rust Backend
// SPDX-License-Identifier: LGPL-2.0-or-later
// R05: KCountrySubdivision bridge - wraps ki18n_rs_* C API

#include "kcountrysubdivision.h"
#include "kcountry.h"

#include <QString>
#include <QStringList>
#include <QStringView>
#include <cstring>
#include <vector>

extern "C" {
struct Ki18nRsSubdivisionHandle;
int ki18n_rs_subdivision_from_code(const char*, void**);
int ki18n_rs_subdivision_from_location(float, float, void**);
int ki18n_rs_subdivision_index(const void*);
int ki18n_rs_subdivision_handle_from_index(int, void**);
void ki18n_rs_subdivision_destroy(void*);
int ki18n_rs_subdivision_code(const void*, char**);
int ki18n_rs_subdivision_name(const void*, char**);
int ki18n_rs_subdivision_country(const void*, void**);
int ki18n_rs_subdivision_parent(const void*, void**);
int ki18n_rs_subdivision_subdivisions(const void*, void***, size_t*);
int ki18n_rs_subdivision_timezones(const void*, char***, size_t*);
void ki18n_rs_string_free(char*);
void ki18n_rs_string_array_free(char**, size_t);
void ki18n_rs_subdivision_list_destroy(void**, size_t);
void ki18n_rs_country_destroy(void*);
int ki18n_rs_country_index(const void*);
}

static QString str_from_capi(char* utf8) {
    if (!utf8) return {};
    QString s = QString::fromUtf8(utf8);
    ki18n_rs_string_free(utf8);
    return s;
}

static void* subdivision_handle_from_index(uint32_t index) {
    if (index == 0) return nullptr;
    void* h = nullptr;
    if (ki18n_rs_subdivision_handle_from_index(static_cast<int>(index - 1), &h) == 0 && h) {
        return h;
    }
    return nullptr;
}

KCountrySubdivision::KCountrySubdivision() : d(0) {}
KCountrySubdivision::KCountrySubdivision(const KCountrySubdivision& other) : d(other.d) {}
KCountrySubdivision::~KCountrySubdivision() = default;
KCountrySubdivision& KCountrySubdivision::operator=(const KCountrySubdivision& other) {
    d = other.d;
    return *this;
}

bool KCountrySubdivision::operator==(const KCountrySubdivision& other) const { return d == other.d; }
bool KCountrySubdivision::operator!=(const KCountrySubdivision& other) const { return d != other.d; }
bool KCountrySubdivision::isValid() const { return d != 0; }

QString KCountrySubdivision::code() const {
    if (d == 0) return {};
    void* h = subdivision_handle_from_index(d);
    if (!h) return {};
    char* out = nullptr;
    ki18n_rs_subdivision_code(h, &out);
    ki18n_rs_subdivision_destroy(h);
    return str_from_capi(out);
}

QString KCountrySubdivision::name() const {
    if (d == 0) return {};
    void* h = subdivision_handle_from_index(d);
    if (!h) return {};
    char* out = nullptr;
    ki18n_rs_subdivision_name(h, &out);
    ki18n_rs_subdivision_destroy(h);
    return str_from_capi(out);
}

KCountry KCountrySubdivision::country() const {
    if (d == 0) return {};
    void* h = subdivision_handle_from_index(d);
    if (!h) return {};
    void* ch = nullptr;
    ki18n_rs_subdivision_country(h, &ch);
    ki18n_rs_subdivision_destroy(h);
    if (!ch) return {};
    int idx = ki18n_rs_country_index(ch);
    ki18n_rs_country_destroy(ch);
    KCountry c;
    c.d = static_cast<uint16_t>(idx >= 0 ? idx + 1 : 0);
    return c;
}

KCountrySubdivision KCountrySubdivision::parent() const {
    if (d == 0) return {};
    void* h = subdivision_handle_from_index(d);
    if (!h) return {};
    void* ph = nullptr;
    int ret = ki18n_rs_subdivision_parent(h, &ph);
    ki18n_rs_subdivision_destroy(h);
    if (ret != 0 || !ph) return {};
    int pidx = ki18n_rs_subdivision_index(ph);
    ki18n_rs_subdivision_destroy(ph);
    KCountrySubdivision s;
    s.d = static_cast<uint32_t>(pidx >= 0 ? pidx + 1 : 0);
    return s;
}

QList<const char*> KCountrySubdivision::timeZoneIds() const {
    QList<const char*> list;
    if (d == 0) return list;
    void* h = subdivision_handle_from_index(d);
    if (!h) return list;
    char** ids = nullptr;
    size_t count = 0;
    if (ki18n_rs_subdivision_timezones(h, &ids, &count) != 0 || !ids) {
        ki18n_rs_subdivision_destroy(h);
        return list;
    }
    thread_local static std::vector<std::string> tz_storage;
    tz_storage.clear();
    for (size_t i = 0; i < count; ++i) {
        if (ids[i]) tz_storage.emplace_back(ids[i]);
    }
    ki18n_rs_string_array_free(ids, count);
    ki18n_rs_subdivision_destroy(h);
    for (const auto& s : tz_storage) list.append(s.c_str());
    return list;
}

QStringList KCountrySubdivision::timeZoneIdsStringList() const {
    QStringList list;
    for (const char* tz : timeZoneIds()) {
        list.append(QString::fromUtf8(tz));
    }
    return list;
}

QList<KCountrySubdivision> KCountrySubdivision::subdivisions() const {
    QList<KCountrySubdivision> list;
    if (d == 0) return list;
    void* h = subdivision_handle_from_index(d);
    if (!h) return list;
    void** arr = nullptr;
    size_t count = 0;
    if (ki18n_rs_subdivision_subdivisions(h, &arr, &count) != 0 || !arr) {
        ki18n_rs_subdivision_destroy(h);
        return list;
    }
    for (size_t i = 0; i < count; ++i) {
        void* sh = arr[i];
        if (sh) {
            int idx = ki18n_rs_subdivision_index(sh);
            KCountrySubdivision s;
            s.d = static_cast<uint32_t>(idx >= 0 ? idx + 1 : 0);
            list.append(s);
        }
    }
    ki18n_rs_subdivision_list_destroy(arr, count);
    ki18n_rs_subdivision_destroy(h);
    return list;
}

KCountrySubdivision KCountrySubdivision::fromCode(const char* code) {
    if (!code || !*code) return {};
    void* h = nullptr;
    if (ki18n_rs_subdivision_from_code(code, &h) != 0 || !h) return {};
    int idx = ki18n_rs_subdivision_index(h);
    ki18n_rs_subdivision_destroy(h);
    KCountrySubdivision s;
    s.d = static_cast<uint32_t>(idx >= 0 ? idx + 1 : 0);
    return s;
}

KCountrySubdivision KCountrySubdivision::fromCode(QStringView code) {
    return fromCode(code.toString().toUtf8().constData());
}

KCountrySubdivision KCountrySubdivision::fromLocation(float lat, float lon) {
    void* h = nullptr;
    if (ki18n_rs_subdivision_from_location(lat, lon, &h) != 0 || !h) return {};
    int idx = ki18n_rs_subdivision_index(h);
    ki18n_rs_subdivision_destroy(h);
    KCountrySubdivision s;
    s.d = static_cast<uint32_t>(idx >= 0 ? idx + 1 : 0);
    return s;
}
