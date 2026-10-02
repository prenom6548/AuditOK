/**
 * brief 1.单例模板和其奇特递归模板模式的派生类相互辅助相互制约形成单例使用方式
 *       2.Singleton模板的默认构造函数是受保护的protected。故只能由派生类来构造
 *       3.其派生类的构造函数必须含有token类型的参数
 * author 赵民勇
 * note  使用方式如下：
    class MysqlPool :public Singleton<MysqlPool> {
    public:
        MysqlPool(token) {};
        ~MysqlPool();
        ...
    };
 */ 

#ifndef SINGLETON_H_
#define SINGLETON_H_

template <class T>
class Singleton
{
public:
    Singleton(const Singleton&) = delete;
    Singleton& operator= (const Singleton) = delete;
public:
    static T& instance() {
        static T _{ token{} };
        return _;
    }
protected:
    struct token {};
    Singleton() = default;
};

#endif

