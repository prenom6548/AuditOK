/*
 * Pluma - Plug-in Management Framework
 * Copyright (C) 2010-2012 Gil Costa (gsaurus@gmail.com)
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it freely,
 * subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented;
 *    you must not claim that you wrote the original software.
 *    If you use this software in a product, an acknowledgment
 *    in the product documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such,
 *    and must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <dlfcn.h>

#include <cstdio>
#include <string>

#include "dlibrary.h"

namespace kyplugin
{

DLibrary::DLibrary(void *handle) : m_handle(handle)
{
    /* nothing to do */
}

DLibrary::~DLibrary()
{
    if (m_handle) {
        dlclose(m_handle);
    }
}

DLibrary *DLibrary::load(const std::string &path)
{
    if (path.empty()) {
        fprintf(stderr, "Failed to load library: Empty path\n");
        return NULL;
    }

    void *handle = dlopen(path.c_str(), RTLD_NOW | RTLD_DEEPBIND | RTLD_GLOBAL);
    if (!handle) {
        fprintf(stderr, "Failed to load library \"%s\".", path.c_str());

        const char *errorString = dlerror();
        if (errorString) {
            fprintf(stderr, " OS returned error: \"%s\".", errorString);
        }

        fprintf(stderr, "\n");

        return NULL;
    }

    return new DLibrary(handle);
}

void *DLibrary::getSymbol(const std::string &symbol)
{
    if (!m_handle) {
        fprintf(stderr, "Cannot inspect library symbols, library isn't loaded.\n");
        return NULL;
    }

    void *res = (void *)(dlsym(m_handle, symbol.c_str()));
    if (!res) {
        fprintf(stderr, "Library symbol \"%s\" not found.\n", symbol.c_str());
        return NULL;
    }
    return res;
}

} // namespace kyplugin
