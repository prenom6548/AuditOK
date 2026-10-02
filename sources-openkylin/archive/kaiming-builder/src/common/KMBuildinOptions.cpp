/*
 * Copyright (c) KylinSoft  Co., Ltd. 2024. All rights reserved.
 *
 * kaiming-builder is licensed under the GPL v2.0+.
 * 
 * See the LICENSE file for more details.
 */

#include "KMBuildinOptions.h"

#include <iostream>
#include <iomanip>
#include "KMException.h"
#include "kmtranslation.h"

namespace KMOption
{

OptionDescription::OptionDescription(const std::string &longName, const std::string &shortName, ValueTypeBase *value, const std::string &description, bool hide)
    : m_longName(longName),
      m_shortName(shortName),
      m_description(description),
      m_value(value),
      m_hide(hide)
{}

OptionDescription::~OptionDescription()
{}

bool OptionDescription::match(const std::string &name) const
{
    return name == m_longName || name == m_shortName;
}

std::string OptionDescription::formatName() const
{
    if (!m_shortName.empty())
    {
        return m_longName.empty()
            ? std::string("-") + m_shortName 
            : std::string("-") + m_shortName + " [ --" + m_longName + " ]";
    }

    return std::string("--") + m_longName;
}

void OptionDescription::print() const
{
    if (m_hide)
    {
        return ;
    }
    
    std::cout << "  " << std::setw(32) << std::left << formatName() + (m_value&&m_value->needValue() ? " arg" : "") << std::setw(80) << std::left << m_description << std::endl;
}

PositionOptionDescription::PositionOptionDescription(const std::string &name, ValueTypeBase *value, int count, const std::string &description)
    : m_name(name),
      m_description(description),
      m_value(value),
      m_count(count)
{}

PositionOptionDescription::~PositionOptionDescription()
{}

void PositionOptionDescription::print() const
{
    std::cout << "  " << std::setw(32) << std::left << m_name << std::setw(80) << std::left << m_description << std::endl;
}

Options::Options()
{}

Options::~Options()
{}

void Options::addOption(const std::string &longName, const std::string &shortName, ValueTypeBase *value, const std::string &description, bool hide)
{
    std::shared_ptr<OptionDescription> option = std::make_shared<OptionDescription>(longName, shortName, value, description, hide);
    m_options.push_back(option);
}

void Options::addPositionOption(const std::string &name, ValueTypeBase *value, int count, const std::string &description)
{
    std::shared_ptr<PositionOptionDescription> option = std::make_shared<PositionOptionDescription>(name, value, count, description);
    m_optionsOfPosition.push_back(option);
}

std::shared_ptr<OptionDescription> Options::find(const std::string& name)
{
    for (auto& option : m_options)
    {
        if (option->match(name))
        {
            return option;
        }
    }

    return std::shared_ptr<OptionDescription>();
}

void Options::parseOption(std::string option, std::vector<std::string>& args)
{
    std::string longName, shortName, value;
    if (!m_isOptionsEnd && option.size() >=3 && option[0] == '-' && option[1] == '-')
    {
        option = option.substr(2);
        std::size_t pos = option.find_first_of('=');
        if (pos == std::string::npos)
        {
            longName = option;

            std::shared_ptr<OptionDescription> optionPtr = find(longName);
            if (optionPtr)
            {
                if (optionPtr->m_value->needValue())
                {
                    args.erase(args.begin());
                    if (args.size() > 0)
                    {
                        std::string next = args.front();

                        if (next[0] != '-')
                        {
                            optionPtr->m_value->applyValue(next);
                        }
                        else
                        {
                            throw KMException("Option '--" + longName + "' hasn't value.");
                        }
                    }
                    else
                    {
                        throw KMException("Option '--" + longName + "' hasn't value.");
                    }
                }
                else
                {
                    optionPtr->m_value->applyValue(value);
                }
            }
            // else
            // {
            //     std::cout << std::string("Unsupport option : --" + longName) << std::endl;
            // }
        }
        else
        {
            longName = option.substr(0, pos);
            value = option.substr(pos + 1);

            std::shared_ptr<OptionDescription> optionPtr = find(longName);
            if (optionPtr)
            {
                optionPtr->m_value->applyValue(value);
            }
        }
    }
    else if (!m_isOptionsEnd && option.size() >=2 && option[0] == '-' && option[1] != '-')
    {
        option = option.substr(1);
        while (option.size() > 0)
        {
            shortName = option.substr(0, 1);
            option = option.substr(1);
            std::shared_ptr<OptionDescription> optionPtr = find(shortName);
            if (optionPtr)
            {
                if (optionPtr->m_value->needValue() && option.size() > 0)
                {
                    throw KMException("Option '-" + shortName + "' hasn't value.");
                }
                else if (optionPtr->m_value->needValue())
                {
                    args.erase(args.begin());
                    if (args.size() > 0)
                    {
                        std::string next = args.front();

                        if (next[0] != '-')
                        {
                            optionPtr->m_value->applyValue(next);
                        }
                        else
                        {
                            throw KMException("Option '-" + shortName + "' hasn't value.");
                        }
                    }
                    else
                    {
                        throw KMException("Option '-" + shortName + "' hasn't value.");
                    }
                }
                else
                {
                    optionPtr->m_value->applyValue(value);
                }
            }
            // else
            // {
            //     std::cout << std::string("Unsupport option : -" + shortName) << std::endl;
            // }
        }
    }
    else if (option == "--")
    {
        // 在命令行选项中，“--”没有选项名称的情况通常被称为“结束选项”（end of options）。这是一个约定俗成的做法，用于指示后续的参数不再是选项，而是普通的非选项参数(位置参数)。
        for (auto& position : m_optionsOfPosition)
        {
            if (position->m_name == option)
            {
                position->m_value->applyValue(option);
                position->m_countSaved++;
                position->m_argIndex = m_originalArgs.size() - args.size() - (m_originalArgs.size() - m_argc);
                ++m_indexOfPosition;
            }
        }

        m_isOptionsEnd = true;
    }
    else
    {
        // 位置参数
        if (m_indexOfPosition < m_optionsOfPosition.size())
        {
            std::shared_ptr<PositionOptionDescription> cur = m_optionsOfPosition[m_indexOfPosition];
            if (cur->m_name == "--")
            {
                // 走到此分支，说明命令行参数中没有传入--，故此需要跳过--位置参数
                cur = m_optionsOfPosition[++m_indexOfPosition];
            }

            cur->m_value->applyValue(option);
            cur->m_countSaved++;

            if (cur->m_countSaved == 1)
            {
                cur->m_argIndex = m_originalArgs.size() - args.size() - (m_originalArgs.size() - m_argc);
            }

            if (cur->m_countSaved == cur->m_count)
            {
                ++m_indexOfPosition;
            }
        }
    }

    args.erase(args.begin());
}

void Options::parseCommandLine(int argc, char **argv)
{
    if (argc <= 0 || argv == nullptr)
    {
        return ;
    }
    m_argc = argc;
    m_argv = argv;
    m_originalArgs.insert(m_originalArgs.end(), m_argv, m_argv+m_argc);

    preParseHook();

    std::vector<std::string> args(m_argv+1, m_argv+m_argc);

    try
    {
        while (args.size() > 0)
        {
            parseOption(args.at(0), args);
        }

        // 应用默认值
        for (auto& option : m_options)
        {
            option->m_value->applyDefault();
        }
        
        for (auto& option : m_optionsOfPosition)
        {
            option->m_value->applyDefault();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error occurred during parsing commandline, error massage : " << e.what() << std::endl;
        showUsage();
        exit(EXIT_FAILURE);
    }
    catch (...)
    {
        std::cerr << "Unknown error." << std::endl;
        showUsage();
        exit(EXIT_FAILURE);
    }

    postParseHook();
}

void Options::showUsage() const
{
    std::cout << m_description << std::endl;

    if (!m_options.empty())
    {
        std::cout << _("Options:") << std::endl;
        for (auto& option : m_options)
        {
            option->print();
        }
    }
    std::cout << std::endl;
    
    if (!m_optionsOfPosition.empty())
    {
        std::cout << _("Position Options:") << std::endl;
        for (auto& option : m_optionsOfPosition)
        {
            option->print();
        }
    }
    std::cout << std::endl;
}

int Options::getPositionalOptionIndex(int numOfPositionalOption)
{
    if (numOfPositionalOption <= m_optionsOfPosition.size() && 0 < numOfPositionalOption)
    {
        auto& option = m_optionsOfPosition[numOfPositionalOption - 1];

        return option->m_argIndex;
    }

    return -1;
}

int Options::getPositionalOptionIndex(const std::string& name)
{
    for (auto& option : m_optionsOfPosition)
    {
        if (option->m_name == name)
        {
            return option->m_argIndex;
        }
    }

    return -1;
}

} // !namespace KMOption
