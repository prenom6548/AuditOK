/* SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd. */
/* SPDX-License-Identifier: LGPL-2.0-or-later */
#pragma once

#ifndef KTRANSCRIPT_EXPORT
#  if defined(_WIN32)
#    define KTRANSCRIPT_EXPORT __declspec(dllexport)
#  else
#    define KTRANSCRIPT_EXPORT __attribute__((visibility("default")))
#  endif
#endif
