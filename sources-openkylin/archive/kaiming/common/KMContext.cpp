/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMContext.h"

#include <glib.h>
#include <vector>
#include <fcntl.h>

#include "KMLogger.h"
#include "KMStorageDir.h"
#include "common/KMUtil.h"
#include "common/KMStringUtils.h"
#include "common/KMFileUtils.h"
#include "kmtranslation.h"

KMContext::KMContext()
{
}

KMContext::~KMContext() { }

void KMContext::addEnvVar(const std::string &env, const std::string &value)
{
    if (env.empty())
    {
        return;
    }

    m_envVars.insert({ env, value });
}

void KMContext::merge(const KMContext &other)
{
    for (auto &pair : other.m_envVars)
    {
        if (!pair.first.empty())
        {
            m_envVars.insert(pair);
        }
    }
}
