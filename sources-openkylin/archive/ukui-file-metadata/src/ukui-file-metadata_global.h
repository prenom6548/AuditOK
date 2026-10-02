/*
    SPDX-FileCopyrightText: 2022 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#ifndef UKUIFILEMETADATA_GLOBAL_H
#define UKUIFILEMETADATA_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(UKUIFILEMETADATA_LIBRARY)
#  define UKUIFILEMETADATA_EXPORT Q_DECL_EXPORT
#else
#  define UKUIFILEMETADATA_EXPORT Q_DECL_IMPORT
#endif

#endif // UKUIFILEMETADATA_GLOBAL_H
