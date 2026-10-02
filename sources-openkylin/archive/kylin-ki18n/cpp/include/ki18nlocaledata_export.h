// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * Minimal ki18nlocaledata_export.h stub for C++ skeleton build.
 * Mirrors the pattern of ki18n_export.h for the localedata library.
 */

#ifndef KI18NLOCALEDATA_EXPORT_H
#define KI18NLOCALEDATA_EXPORT_H

#if defined(_WIN32) || defined(_WIN64)
  #ifdef KI18NLOCALEDATA_BUILDING_LIBRARY
    #define KI18NLOCALEDATA_EXPORT __declspec(dllexport)
  #else
    #define KI18NLOCALEDATA_EXPORT __declspec(dllimport)
  #endif
#else
  #define KI18NLOCALEDATA_EXPORT __attribute__((visibility("default")))
#endif

#endif // KI18NLOCALEDATA_EXPORT_H

