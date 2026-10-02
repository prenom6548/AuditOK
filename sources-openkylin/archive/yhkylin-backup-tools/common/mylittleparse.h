#pragma once

#include <QFile>
class QString;

class MyLittleParse {
public:
    enum error_enum {
        DEFAULT,
        OPEN_FILE_FAIL,
        KEY_EXITS, //key存在，value为空, 适用于add
        REPEAT, //key和value都存在，且重复，适用于mod
        KEY_NOEXITS, //不能找到key, 适用于find
        SUCCESS
    };
    MyLittleParse(const QString& path);

    ~MyLittleParse();

    /**
     * @brief add
     * @param key
     * @param value
     * @param e
     * @return
     *
     * 文件不存在，返回false， e = OPEN_FILE_FAIL
     * 如果key不存在, 增加条目 key=value, e = SUCCESS
     * 如果key存在，=后是空，转到 mod 函数。
     * 如果key存在，=后不为空，返回false, e = KEY_EXISTS
     *
     */
    bool add(const QString& key, const QString& value, error_enum& e);
    bool add(const QString& key, const QString& value);
    bool add(const QString&& key, const QString&& value);

    /**
     * @brief Add
     * @param key
     * @param value
     * @return
     *
     * 如果key不存在, 增加条目 key=value
     * 如果key存在，转到 mod 函数。
     *
     */
    bool Add(const QString& key, const QString& value);

    /**
     * @brief del
     * @param key
     * @param e
     * @return
     *
     * 如果find_only（key） = false，返回false
     * 文件不存在，返回false， e = OPEN_FILE_FAIL
     *
     */
    bool del(const QString& key, error_enum& e);
    bool del(const QString& key);
    bool del(const QString&& key);

    /**
     * @brief mod
     * @param key
     * @param value
     * @param e
     * @return
     *
     * 如果find_only（key） = false，返回false
     * 文件不存在，返回false， e = OPEN_FILE_FAIL
     * 如果key对应的值和value相等，返回false， e = REPEAT
     */
    bool mod(const QString& key, const QString& value, error_enum& e);
    bool mod(const QString& key, const QString& value);
    bool mod(const QString&& key, const QString&& value);

    /**
     * @brief Mod
     * @param key
     * @param value
     * @return
     *
     *
     * 如果key对应的值和value相等，返回true
     */
    bool Mod(const QString& key, const QString& value);

    /**
     * @brief find
     * @param key
     * @param value
     * @param e
     * @return
     *
     * 文件不存在，返回false， e = OPEN_FILE_FAIL
     * 找到key，并赋值给value
     */
    bool find(const QString& key, QString& value, error_enum& e);
    bool find(const QString& key, QString& value);
    bool find(const QString&& key, QString&& value);

    /**
     * @brief find_only
     * @param key
     * @param e
     * @return
     * 文件不存在，返回false， e = OPEN_FILE_FAIL
     */
    bool find_only(const QString& key, error_enum& e);
    bool find_only(const QString& key);
    bool find_only(const QString&& key);

    error_enum getErrEnum() const;

    QString getErrMsg();

private:
    QFile* file;
    error_enum m_e;
};
