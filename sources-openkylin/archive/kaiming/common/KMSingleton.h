/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

/**
 * brief 1.单例模板和其奇特递归模板模式的派生类相互辅助相互制约形成单例使用方式
 *       2.Singleton模板的默认构造函数是受保护的protected。故只能由派生类来构造
 *       3.其派生类的构造函数必须含有token类型的参数
 * note  使用方式如下：
    class MyClass :public KMSingleton<MyClass> {
    public:
        MyClass(token) {};
        ~MyClass();
        ...
    };
 */

#ifndef SINGLETON_H_
#define SINGLETON_H_

#include <utility>

/**
 * @brief 1.单例模板和其奇特递归模板模式的派生类相互辅助相互制约形成单例使用方式
 *        2.Singleton模板的默认构造函数是受保护的protected。故只能由派生类来构造
 *        3.其派生类的构造函数必须含有token类型的参数
 */
template<class T>
class KMSingleton
{
public:
    KMSingleton(const KMSingleton &) = delete;
    KMSingleton &operator=(const KMSingleton) = delete;

public:
    // static T& instance() {
    //     static T _{ token{} };
    //     return _;
    // }

    template<typename... Args>
    static T &instance(Args &&...args)
    {
        static T _{ token{}, std::forward<Args>(args)... };
        return _;
    }

protected:
    struct token
    {
    };

    KMSingleton() = default;
};

#endif
