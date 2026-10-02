/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#pragma once

#include "DisplayControl/display_control_api_functions.h"

/* This header and its declarations must be usable from C code. */

#ifdef __cplusplus
extern "C" {
#endif


#define DISPLAY_CONTROL_API_DECLARE(return_type, func_name, signature, callargs) \
    typedef return_type (*func_name ## Fn) signature; \
    return_type func_name signature;

LIST_DISPLAY_CONTROL_API_FUNCTIONS(DISPLAY_CONTROL_API_DECLARE)

#ifdef __cplusplus
}
#endif
