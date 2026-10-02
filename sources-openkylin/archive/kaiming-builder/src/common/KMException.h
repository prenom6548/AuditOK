/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#ifndef __KMEXCEPTION_H__
#define __KMEXCEPTION_H__

#include <exception>
#include <string>

/**
 * @brief : 异常类，用于抛出业务错误信息
 */
class KMException : public std::exception
{
public:
    KMException(const std::string &errMsg);
    ~KMException();

    const char *what() const throw()
    {
        return m_errMsg.c_str();
    };

private:
    std::string m_errMsg;
};

#endif // !__KMEXCEPTION_H__
