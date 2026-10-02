/**
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 * 
 * @brief : 简易版的命令行解析
 * @note  : 参数仅支持：bool，int/uint32_t/long/uint64_t/float/double等数值型, std::string, std::vector<std::string>，如需更多请自行优化
 */

#ifndef __KMBUILDINOPTIONS_H__
#define __KMBUILDINOPTIONS_H__

#include <string>
#include <any>
#include <memory>
#include <variant>
#include <vector>
#include <optional>
#include <typeindex>
#include <charconv>

namespace KMOption
{

class ValueTypeBase
{
public:
    virtual std::type_index valueType() const = 0;

    virtual void applyValue(const std::string& valueStore) = 0;

    virtual void applyDefault() = 0;

    virtual bool needValue() = 0;

    virtual ~ValueTypeBase() {}
};

template <typename T, typename Enable = void>
class OptionValue : public ValueTypeBase
{
public:
    OptionValue(T * saveTo)
        : m_storeTo(saveTo)
    {}

    std::type_index valueType() const
    {
        return std::type_index(typeid(T));
    }

    OptionValue* defaultValue(const T& v)
    {
        m_defaultValue = std::any(v);

        return this;
    }

    virtual bool needValue()
    {
        return m_needValue;
    }

    virtual void applyDefault()
    {
        if (m_isSet)
        {
            return ;
        }

        if (m_defaultValue.has_value() && m_storeTo)
        {
            T v = std::any_cast<T>(m_defaultValue);
            *m_storeTo = v;
        }
    }

    void applyValue(const std::string& valueStore)
    {
        if (!m_storeTo)
        {
            return ;
        }

        m_isSet = true;

        *m_storeTo = valueStore;
    }

private:
    static constexpr bool m_needValue = !std::is_same_v<T, bool>;
    T* m_storeTo;

    std::any m_defaultValue;
    bool m_isSet = false;
};

template <>
class OptionValue<bool> : public ValueTypeBase
{
public:
    OptionValue(bool * saveTo)
        : m_storeTo(saveTo)
    {}

    std::type_index valueType() const
    {
        return std::type_index(typeid(bool));
    }

    OptionValue<bool>* defaultValue(const bool& v)
    {
        m_defaultValue = std::any(v);

        return this;
    }

    virtual bool needValue()
    {
        return m_needValue;
    }

    virtual void applyDefault()
    {
        if (m_isSet)
        {
            return ;
        }

        if (m_defaultValue.has_value() && m_storeTo)
        {
            bool v = std::any_cast<bool>(m_defaultValue);
            *m_storeTo = v;
        }
    }

    void applyValue(const std::string& valueStore)
    {
        if (!m_storeTo)
        {
            return ;
        }

        m_isSet = true;

        *m_storeTo = true;
    }

private:
    static constexpr bool m_needValue = false;
    bool* m_storeTo;

    std::any m_defaultValue;
    bool m_isSet = false;
};

template <typename T>
class OptionValue<T, typename std::enable_if<std::is_floating_point_v<T> || std::is_integral_v<T>>::type> : public ValueTypeBase
{
public:
    OptionValue(T * saveTo)
        : m_storeTo(saveTo)
    {}

    std::type_index valueType() const
    {
        return std::type_index(typeid(T));
    }

    OptionValue* defaultValue(const T& v)
    {
        m_defaultValue = std::any(v);

        return this;
    }

    virtual bool needValue()
    {
        return m_needValue;
    }

    virtual void applyDefault()
    {
        if (m_isSet)
        {
            return ;
        }

        if (m_defaultValue.has_value() && m_storeTo)
        {
            T v = std::any_cast<T>(m_defaultValue);
            *m_storeTo = v;
        }
    }

    void applyValue(const std::string& valueStore)
    {
        if (!m_storeTo)
        {
            return ;
        }

        m_isSet = true;

        std::from_chars(valueStore.c_str(), valueStore.c_str() + valueStore.length(), *m_storeTo);
    }

private:
    static constexpr bool m_needValue = true;
    T* m_storeTo;

    std::any m_defaultValue;
    bool m_isSet = false;
};

template <>
class OptionValue<std::vector<std::string>> : public ValueTypeBase
{
public:
    OptionValue(std::vector<std::string> * saveTo)
        : m_storeTo(saveTo)
    {}

    std::type_index valueType() const
    {
        return std::type_index(typeid(std::vector<std::string>));
    }

    OptionValue* defaultValue(const std::vector<std::string>& v)
    {
        m_defaultValue = std::any(v);

        return this;
    }

    virtual bool needValue()
    {
        return m_needValue;
    }

    virtual void applyDefault()
    {
        if (m_isSet)
        {
            return ;
        }

        if (m_defaultValue.has_value() && m_storeTo)
        {
            std::vector<std::string> v = std::any_cast<std::vector<std::string>>(m_defaultValue);
            *m_storeTo = v;
        }
    }

    void applyValue(const std::string& valueStore)
    {
        if (!m_storeTo)
        {
            return ;
        }

        m_isSet = true;

        (*m_storeTo).push_back(valueStore);
    }

private:
    static constexpr bool m_needValue = true;
    std::vector<std::string>* m_storeTo;

    std::any m_defaultValue;
    bool m_isSet = false;
};

template<class T>
OptionValue<T>* value(T* saveTo)
{
    return new OptionValue<T>(saveTo);       
}

/**
 * @brief : 选项描述
 */
class OptionDescription
{
public:
    OptionDescription(const std::string &longName, const std::string &shortName, ValueTypeBase *value, const std::string &description, bool hide = false);
    ~OptionDescription();

    bool match(const std::string &name) const;

    std::string formatName() const;

    void print() const;

private:
    std::string m_longName;
    std::string m_shortName;
    std::string m_description;
    std::shared_ptr<ValueTypeBase> m_value;
    bool m_hide;

    friend class Options;
};

/**
 * @brief : 位置参数描述
 */ 
class PositionOptionDescription
{
public:
    PositionOptionDescription(const std::string &name, ValueTypeBase *value, int count, const std::string &description);
    ~PositionOptionDescription();

    void print() const;

private:
    std::string m_name;
    std::string m_description;
    std::shared_ptr<ValueTypeBase> m_value;
    int m_count;      // 占用几个连续位置参数
    int m_countSaved = 0; // 已结解析的数量
    int m_argIndex = -1;  // 在命令行参数中的位置索引

    friend class Options;
};

class Options
{
public:
    Options();
    virtual ~Options();

    void setDescription(const std::string &description) { m_description = description; }

    virtual void parseCommandLine(int argc, char **argv);

    void addOption(const std::string &longName, const std::string &shortName, ValueTypeBase *value, const std::string &description, bool hide = false);
    void addPositionOption(const std::string &name, ValueTypeBase *value, int count, const std::string &description);

    int getPositionalOptionIndex(int numOfPositionalOption = 1);
    int getPositionalOptionIndex(const std::string& name);

protected:
    void parseOption(std::string option, std::vector<std::string>& args);

    std::shared_ptr<OptionDescription> find(const std::string& name);

    virtual void preParseHook() = 0;
    virtual void postParseHook() {}
    virtual void showUsage() const;

protected:
    std::string m_description;

    std::vector<std::shared_ptr<OptionDescription>> m_options;
    std::vector<std::shared_ptr<PositionOptionDescription>> m_optionsOfPosition;
    std::vector<std::string> m_originalArgs;
    int m_argc = 0;
    char ** m_argv = nullptr;
    bool m_isOptionsEnd = false;

    int m_indexOfPosition = 0;
};

} // ! namespace KMOption

#endif // !__KMBUILDINOPTIONS_H__
