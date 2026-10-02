/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "kerror.h"

#include <QString>

class KErrorData : public QSharedData
{
public:
    // 默认构造
    KErrorData()
    {
    }
    // 拷贝构造
    KErrorData(const KErrorData &other)
        : QSharedData(other)
        , m_code(other.m_code)
        , m_subCode(other.m_subCode)
        , m_errName(other.m_errName)
        , m_errMessage(other.m_errMessage)
        , m_operation(other.m_operation)
    {
    }
    // 赋值
    KErrorData &operator=(const KErrorData &other)
    {
        if (this != &other) {
            m_code = other.m_code;
            m_subCode = other.m_subCode;
            m_errName = other.m_errName;
            m_errMessage = other.m_errMessage;
            m_operation = other.m_operation;
        }
        return *this;
    }
    ~KErrorData() {}

    friend class KError;

private:
    // 错误码
    int m_code = 0;
    // 子错误码
    int m_subCode = 0;
    // 错误名称
    QString m_errName = "";
    // 错误详细描述
    QString m_errMessage = "";
    // 操作
    QString m_operation = "";
};

KError::KError()
    : m_data(new KErrorData())
{

}

KError::KError(int code, int subCode, const QString &errName, const QString &errMessage)
    : m_data(new KErrorData())
{
    m_data->m_code = code;
    m_data->m_subCode = subCode;
    m_data->m_errName = errName;
    m_data->m_errMessage = errMessage;
    m_data->m_operation = "";
}

KError::KError(const KError &other)
    : m_data(other.m_data)
{
}

KError &KError::operator=(const KError &other)
{
    if (this != &other) {
        m_data.operator = (other.m_data);
    }
    return *this;
}

KError::~KError() {}

bool KError::operator==(const KError &other) const
{
    return (m_data->m_code == other.getCode() && m_data->m_subCode == other.getSubCode() &&
            m_data->m_errName == other.getSubCode() && m_data->m_errMessage == getErrorMessage());
}

int KError::getCode() const
{
    return m_data->m_code;
}

void KError::setCode(int code)
{
    m_data->m_code = code;
}

int KError::getSubCode() const
{
    return m_data->m_subCode;
}

void KError::setSubCode(int subCode)
{
    m_data->m_subCode = subCode;
}

QString KError::getErrorName() const
{
    return m_data->m_errName;
}

void KError::setErrorName(const QString &errName)
{
    m_data->m_errName = errName;
}

QString KError::getErrorMessage() const
{
    return m_data->m_errMessage;
}

void KError::setErrorMessage(const QString &errMessage)
{
    m_data->m_errMessage = errMessage;
}

bool operator!(const KError &ke) noexcept
{
    return !ke.operator bool();
}

bool KError::isValid() const
{
    return operator bool();
}

KError::operator bool() const
{
    return (m_data->m_code != 0 || m_data->m_subCode != 0);
}

void KError::reset()
{
    m_data->m_code = 0;
    m_data->m_subCode = 0;
    m_data->m_errName = "";
    m_data->m_errMessage = "";
}

void KError::setOperation(const QString &operation)
{
    m_data->m_operation = operation;
}

QString KError::getOperation() const
{
    return m_data->m_operation;
}


