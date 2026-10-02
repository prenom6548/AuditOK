/* -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*-
 * Copyright (c) 2016, gperftools Contributors
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _NALLOCX_H_
#define _NALLOCX_H_
#include <stddef.h>

#ifndef PERFTOOLS_DLL_DECL
#ifdef _WIN32
#define PERFTOOLS_DLL_DECL __declspec(dllimport)
#else
#define PERFTOOLS_DLL_DECL
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define MALLOCX_LG_ALIGN(la) ((int)(la))

/*
 * The nallocx function allocates no memory, but it performs the same size
 * computation as the malloc function, and returns the real size of the
 * allocation that would result from the equivalent malloc function call.
 * nallocx is a malloc extension originally implemented by jemalloc:
 * http://www.unix.com/man-page/freebsd/3/nallocx/
 *
 * Note, we only support MALLOCX_LG_ALIGN flag and nothing else.
 */
PERFTOOLS_DLL_DECL size_t nallocx(size_t size, int flags);

/* same as above but never weak */
PERFTOOLS_DLL_DECL size_t tc_nallocx(size_t size, int flags);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _NALLOCX_H_ */
