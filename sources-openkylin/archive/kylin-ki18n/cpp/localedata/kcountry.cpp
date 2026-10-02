// SPDX-FileCopyrightText: 2024 KI18n Rust Backend
// SPDX-License-Identifier: LGPL-2.0-or-later
// R05: KF5I18nLocaleData bridge - wraps ki18n_rs_* C API

#include "kcountry.h"
#include "kcountrysubdivision.h"

#include <QString>
#include <QStringList>
#include <QLocale>
#include <cstring>
#include <vector>

#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
#include <private/qlocale_p.h>
#endif

extern "C" {
struct Ki18nRsCountryHandle;
int ki18n_rs_country_from_alpha2(const char*, void**);
int ki18n_rs_country_from_alpha3(const char*, void**);
int ki18n_rs_country_from_name(const char*, void**);
int ki18n_rs_country_from_location(float, float, void**);
int ki18n_rs_country_all(void***, size_t*);
int ki18n_rs_country_index(const void*);
int ki18n_rs_country_handle_from_index(int, void**);
void ki18n_rs_country_destroy(void*);
int ki18n_rs_country_alpha2(const void*, char**);
int ki18n_rs_country_alpha3(const void*, char**);
int ki18n_rs_country_name(const void*, char**);
int ki18n_rs_country_emoji_flag(const void*, char**);
int ki18n_rs_country_currency_code(const void*, char**);
int ki18n_rs_country_timezones(const void*, char***, size_t*);
int ki18n_rs_country_subdivisions(const void*, void***, size_t*);
void ki18n_rs_string_free(char*);
void ki18n_rs_string_array_free(char**, size_t);
void ki18n_rs_country_list_destroy(void**, size_t);
int ki18n_rs_subdivision_index(const void*);
void ki18n_rs_subdivision_destroy(void*);
void ki18n_rs_subdivision_list_destroy(void**, size_t);
}

static QString str_from_capi(char* utf8) {
    if (!utf8) return {};
    QString s = QString::fromUtf8(utf8);
    ki18n_rs_string_free(utf8);
    return s;
}

static void* handle_from_index(int idx) {
    void* h = nullptr;
    if (ki18n_rs_country_handle_from_index(idx, &h) == 0 && h) {
        return h;
    }
    return nullptr;
}

/// `KCountry::d` 为 1-based 国家表下标（0 表示无效），避免与上游一样用 0 表示「表中第 0 国」时与「无效」冲突。
static void* country_handle_from_storage_id(uint16_t storage_id) {
    if (storage_id == 0) {
        return nullptr;
    }
    return handle_from_index(static_cast<int>(storage_id) - 1);
}

KCountry::KCountry() : d(0) {}
KCountry::KCountry(const KCountry& other) : d(other.d) {}
KCountry::~KCountry() = default;
KCountry& KCountry::operator=(const KCountry& other) { d = other.d; return *this; }

bool KCountry::operator==(const KCountry& other) const { return d == other.d; }
bool KCountry::operator!=(const KCountry& other) const { return d != other.d; }
bool KCountry::isValid() const { return d != 0; }

QString KCountry::alpha2() const {
    if (d == 0) return {};
    void* h = country_handle_from_storage_id(d);
    if (!h) return {};
    char* out = nullptr;
    ki18n_rs_country_alpha2(h, &out);
    ki18n_rs_country_destroy(h);
    return str_from_capi(out);
}

QString KCountry::alpha3() const {
    if (d == 0) return {};
    void* h = country_handle_from_storage_id(d);
    if (!h) return {};
    char* out = nullptr;
    ki18n_rs_country_alpha3(h, &out);
    ki18n_rs_country_destroy(h);
    return str_from_capi(out);
}

QString KCountry::name() const {
    if (d == 0) return {};
    void* h = country_handle_from_storage_id(d);
    if (!h) return {};
    char* out = nullptr;
    ki18n_rs_country_name(h, &out);
    ki18n_rs_country_destroy(h);
    return str_from_capi(out);
}

QString KCountry::emojiFlag() const {
    if (d == 0) return {};
    void* h = country_handle_from_storage_id(d);
    if (!h) return {};
    char* out = nullptr;
    ki18n_rs_country_emoji_flag(h, &out);
    ki18n_rs_country_destroy(h);
    return str_from_capi(out);
}

QLocale::Country KCountry::country() const {
    if (d == 0) return QLocale::AnyCountry;
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    return QLocale::codeToTerritory(alpha2());
#else
    // Qt5: QLocale("en_XX") yields a locale for country XX; .country() gives the enum
    QString a2 = alpha2();
    if (a2.isEmpty() || a2.size() != 2) return QLocale::AnyCountry;
    QLocale loc(QStringLiteral("en_") + a2);
    return loc.country();
#endif
}

QString KCountry::currencyCode() const {
    if (d == 0) return {};

    QString currency;
    const auto ls = QLocale::matchingLocales(QLocale::AnyLanguage, QLocale::AnyScript, country());
    for (const auto &l : ls) {
        if (currency.isEmpty()) {
            currency = l.currencySymbol(QLocale::CurrencyIsoCode);
        } else if (currency != l.currencySymbol(QLocale::CurrencyIsoCode)) {
            return {};
        }
    }
    return currency;
}

QList<const char*> KCountry::timeZoneIds() const {
    QList<const char*> list;
    if (d == 0) return list;
    void* h = country_handle_from_storage_id(d);
    if (!h) return list;
    char** ids = nullptr;
    size_t count = 0;
    if (ki18n_rs_country_timezones(h, &ids, &count) != 0 || !ids) {
        ki18n_rs_country_destroy(h);
        return list;
    }
    thread_local static std::vector<std::string> tz_storage;
    tz_storage.clear();
    for (size_t i = 0; i < count; ++i) {
        if (ids[i]) tz_storage.emplace_back(ids[i]);
    }
    ki18n_rs_string_array_free(ids, count);
    ki18n_rs_country_destroy(h);
    for (const auto& s : tz_storage) list.append(s.c_str());
    return list;
}

QStringList KCountry::timeZoneIdsStringList() const {
    QStringList list;
    for (const char* tz : timeZoneIds()) {
        list.append(QString::fromUtf8(tz));
    }
    return list;
}

QList<KCountrySubdivision> KCountry::subdivisions() const {
    QList<KCountrySubdivision> list;
    if (d == 0) return list;
    void* h = country_handle_from_storage_id(d);
    if (!h) return list;
    void** arr = nullptr;
    size_t count = 0;
    if (ki18n_rs_country_subdivisions(h, &arr, &count) != 0 || !arr) {
        ki18n_rs_country_destroy(h);
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
    ki18n_rs_country_destroy(h);
    return list;
}

KCountry KCountry::fromAlpha2(const char* code) {
    if (!code) return {};
    if (QByteArray ba(code); ba.size() < 2) return {};
    void* h = nullptr;
    if (ki18n_rs_country_from_alpha2(code, &h) != 0 || !h) return {};
    int idx = ki18n_rs_country_index(h);
    ki18n_rs_country_destroy(h);
    KCountry c;
    c.d = static_cast<uint16_t>(idx >= 0 ? idx + 1 : 0);
    return c;
}

KCountry KCountry::fromAlpha2(QStringView code) {
    return fromAlpha2(code.toString().toUtf8().constData());
}

KCountry KCountry::fromAlpha3(const char* code) {
    if (!code) return {};
    if (QByteArray ba(code); ba.size() < 3) return {};
    void* h = nullptr;
    if (ki18n_rs_country_from_alpha3(code, &h) != 0 || !h) return {};
    int idx = ki18n_rs_country_index(h);
    ki18n_rs_country_destroy(h);
    KCountry c;
    c.d = static_cast<uint16_t>(idx >= 0 ? idx + 1 : 0);
    return c;
}

KCountry KCountry::fromAlpha3(QStringView code) {
    return fromAlpha3(code.toString().toUtf8().constData());
}

KCountry KCountry::fromLocation(float lat, float lon) {
    void* h = nullptr;
    if (ki18n_rs_country_from_location(lat, lon, &h) != 0 || !h) return {};
    int idx = ki18n_rs_country_index(h);
    ki18n_rs_country_destroy(h);
    KCountry c;
    c.d = static_cast<uint16_t>(idx >= 0 ? idx + 1 : 0);
    return c;
}

KCountry KCountry::fromQLocale(QLocale::Country qc) {
    // 与上游 ki18n 一致：QLocale::Country 是 Qt 枚举，不是 ISO 3166-1 numeric。
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
    return fromAlpha2(QLocale::territoryToCode(qc).data());
#else
    return fromAlpha2(QLocalePrivate::countryToCode(qc).data());
#endif
}

KCountry KCountry::fromName(QStringView name) {
    if (name.isEmpty()) return {};
    void* h = nullptr;
    QByteArray ba = name.toString().toUtf8();
    if (ki18n_rs_country_from_name(ba.constData(), &h) != 0 || !h) return {};
    int idx = ki18n_rs_country_index(h);
    ki18n_rs_country_destroy(h);
    KCountry c;
    c.d = static_cast<uint16_t>(idx >= 0 ? idx + 1 : 0);
    return c;
}

QList<KCountry> KCountry::allCountries() {
    QList<KCountry> list;
    void** arr = nullptr;
    size_t count = 0;
    if (ki18n_rs_country_all(&arr, &count) != 0 || !arr) return list;
    for (size_t i = 0; i < count; ++i) {
        void* h = arr[i];
        if (h) {
            int idx = ki18n_rs_country_index(h);
            KCountry c;
            c.d = static_cast<uint16_t>(idx >= 0 ? idx + 1 : 0);
            list.append(c);
        }
    }
    ki18n_rs_country_list_destroy(arr, count);
    return list;
}
