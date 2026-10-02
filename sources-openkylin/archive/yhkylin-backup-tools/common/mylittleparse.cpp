/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "mylittleparse.h"
#include <QString>
#include <QTextStream>

MyLittleParse::MyLittleParse(const QString& path)
    : file(new QFile(path))
    , m_e(MyLittleParse::DEFAULT)
{
}

MyLittleParse::~MyLittleParse()
{
    file->close();
    delete file;
}

bool MyLittleParse::add(const QString& key, const QString& value, MyLittleParse::error_enum& e)
{
    QString find_value;
    if (find(key, find_value, e)) {
        //key=<不空值>
        if (!find_value.isEmpty()) {
            e = MyLittleParse::KEY_EXITS;
            return false;
        }
        //key=<空值>
        return mod(key, value, e);
    }

    if (!file->open(QIODevice::ReadWrite)) {
        e = MyLittleParse::OPEN_FILE_FAIL;
        return false;
    }

    QTextStream out(file);
    file->readAll();
    out << key << "=" << value << "\n";

    file->close();
    e = MyLittleParse::SUCCESS;
    return true;
}

bool MyLittleParse::add(const QString&& key, const QString&& value)
{
    QString _key = key;
    QString _value = value;
    return add(_key, _value);
}

bool MyLittleParse::Add(const QString &key, const QString &value)
{
    MyLittleParse::error_enum e;
    bool t = add(key, value, e);
    if (!t && e == MyLittleParse::KEY_EXITS) {
        t = mod(key, value);
    }
    return t;
}

bool MyLittleParse::add(const QString& key, const QString& value)
{
    return add(key, value, m_e);
}

bool MyLittleParse::del(const QString& key, MyLittleParse::error_enum& e)
{
    if (!find_only(key, e)) {
        return false;
    }

    if (!file->open(QIODevice::ReadOnly)) {
        e = MyLittleParse::OPEN_FILE_FAIL;
        return false;
    }

    QTextStream out(file);
    QString strAll = out.readAll();
    file->close();

    QStringList strList = strAll.split("\n");
    QStringList strDstList;
    QString tmp_key;
    for (QString& line : strList) {
        if (line.trimmed().isEmpty())
            continue;
        tmp_key = line.section('=', 0, 0).trimmed();
        if (tmp_key == key)
            continue;
        strDstList << line << "\n";
    }

    file->open(QIODevice::Truncate | QIODevice::WriteOnly);
    for (auto s : strDstList)
        out << s;
    out.flush();
    file->close();
    e = MyLittleParse::SUCCESS;
    return true;
}

bool MyLittleParse::del(const QString&& key)
{
    QString _key = key;
    return del(_key);
}

bool MyLittleParse::del(const QString& key)
{
    return del(key, m_e);
}

bool MyLittleParse::mod(const QString& key, const QString& value, MyLittleParse::error_enum& e)
{
    if (!find_only(key, e)) {
        return false;
    }

    if (!file->open(QIODevice::ReadOnly)) {
        e = MyLittleParse::OPEN_FILE_FAIL;
        return false;
    }

    QTextStream out(file);
    QString strAll = out.readAll();
    file->close();

    QStringList strList = strAll.split("\n");
    QStringList strDstList;
    QString tmp_key;
    QString tmp_value;
    for (QString& line : strList) {
        tmp_key = line.section('=', 0, 0).trimmed();
        if (tmp_key == key) {
            tmp_value = line.section('=', 1, 1).trimmed();
            if (tmp_value == value) {
                e = MyLittleParse::REPEAT;
                file->close();
                return false;
            }
            line.clear();
            line = key + "=" + value;
        }
        strDstList << line << "\n";
    }

    file->open(QIODevice::Truncate | QIODevice::WriteOnly);
    for (auto& s : strDstList)
        out << s;
    out.flush();
    file->close();
    e = MyLittleParse::SUCCESS;
    return true;
}

bool MyLittleParse::mod(const QString& key, const QString& value)
{
    return mod(key, value, m_e);
}

bool MyLittleParse::mod(const QString&& key, const QString&& value)
{
    QString _key = key;
    QString _value = value;
    return mod(_key, _value);
}

bool MyLittleParse::Mod(const QString &key, const QString &value)
{
    MyLittleParse::error_enum e;
    bool t = mod(key, value, e);
    if (!t && e == MyLittleParse::REPEAT) {
        return true;
    }
    return t;
}

bool MyLittleParse::find(const QString& key, QString& value, MyLittleParse::error_enum& e)
{
    if (!file->open(QIODevice::ReadOnly)) {
        e = MyLittleParse::OPEN_FILE_FAIL;
        return false;
    }

    QTextStream out(file);

    QString line;
    QString tmp_key;

    while (1) {
        if (out.atEnd())
            break;
        line = out.readLine();
        if (line.trimmed().isEmpty())
            continue;
        tmp_key = line.section('=', 0, 0).trimmed();
        if (key == tmp_key) {
            value = line.section('=', 1, 1).trimmed();
            file->close();
            e = MyLittleParse::SUCCESS;
            return true;
        }
    }

    file->close();
    e = MyLittleParse::KEY_NOEXITS;
    return false;
}

bool MyLittleParse::find(const QString& key, QString& value)
{
    return find(key, value, m_e);
}

bool MyLittleParse::find(const QString&& key, QString&& value)
{
    QString _key = key;
    QString _value = value;
    return find(_key, _value);
}

bool MyLittleParse::find_only(const QString& key, MyLittleParse::error_enum& e)
{
    QString value;
    if (find(key, value, e))
        return true;
    return false;
}

bool MyLittleParse::find_only(const QString& key)
{
    return find_only(key, m_e);
}

bool MyLittleParse::find_only(const QString&& key)
{
    QString _key = key;
    return find_only(_key);
}

MyLittleParse::error_enum MyLittleParse::getErrEnum() const
{
    return m_e;
}

QString MyLittleParse::getErrMsg()
{
    switch (m_e) {
    case MyLittleParse::DEFAULT:
        return "default";
    case MyLittleParse::KEY_EXITS:
        return "KEY_EXITS";
    case MyLittleParse::OPEN_FILE_FAIL:
        return "OPEN_FILE_FAIL";
    case MyLittleParse::KEY_NOEXITS:
        return "KEY_NOEXITS";
    case MyLittleParse::REPEAT:
        return "REPEAT";
    case MyLittleParse::SUCCESS:
        return "SUCCESS";
    default:
        break;
    }
    return "";
}
