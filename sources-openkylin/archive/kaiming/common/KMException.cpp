/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMException.h"

KMException::KMException(const std::string &errMsg)
    : m_errMsg(errMsg)
{
}

KMException::~KMException()
{
}
