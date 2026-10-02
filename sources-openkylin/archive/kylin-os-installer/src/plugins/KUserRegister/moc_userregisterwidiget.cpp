/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

/****************************************************************************
** Meta object code from reading C++ file 'userregisterwidiget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "userregisterwidiget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'userregisterwidiget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_KInstaller__userregisterwidiget_t {
    QByteArrayData data[21];
    char stringdata0[264];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_KInstaller__userregisterwidiget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_KInstaller__userregisterwidiget_t qt_meta_stringdata_KInstaller__userregisterwidiget = {
    {
QT_MOC_LITERAL(0, 0, 31), // "KInstaller::userregisterwidiget"
QT_MOC_LITERAL(1, 32, 16), // "enableNextButton"
QT_MOC_LITERAL(2, 49, 0), // ""
QT_MOC_LITERAL(3, 50, 5), // "value"
QT_MOC_LITERAL(4, 56, 15), // "setComputerText"
QT_MOC_LITERAL(5, 72, 13), // "checkUserName"
QT_MOC_LITERAL(6, 86, 10), // "m_username"
QT_MOC_LITERAL(7, 97, 13), // "checkHostName"
QT_MOC_LITERAL(8, 111, 10), // "m_hostname"
QT_MOC_LITERAL(9, 122, 11), // "checkPasswd"
QT_MOC_LITERAL(10, 134, 3), // "pwd"
QT_MOC_LITERAL(11, 138, 12), // "checkCPasswd"
QT_MOC_LITERAL(12, 151, 4), // "cpwd"
QT_MOC_LITERAL(13, 156, 19), // "markLineEditCorrect"
QT_MOC_LITERAL(14, 176, 21), // "findFingerPrintDevice"
QT_MOC_LITERAL(15, 198, 12), // "getAutoLogin"
QT_MOC_LITERAL(16, 211, 16), // "saveRegisterDate"
QT_MOC_LITERAL(17, 228, 12), // "translateStr"
QT_MOC_LITERAL(18, 241, 11), // "checkactive"
QT_MOC_LITERAL(19, 253, 8), // "QWidget*"
QT_MOC_LITERAL(20, 262, 1) // "p"

    },
    "KInstaller::userregisterwidiget\0"
    "enableNextButton\0\0value\0setComputerText\0"
    "checkUserName\0m_username\0checkHostName\0"
    "m_hostname\0checkPasswd\0pwd\0checkCPasswd\0"
    "cpwd\0markLineEditCorrect\0findFingerPrintDevice\0"
    "getAutoLogin\0saveRegisterDate\0"
    "translateStr\0checkactive\0QWidget*\0p"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_KInstaller__userregisterwidiget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      12,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   74,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   77,    2, 0x0a /* Public */,
       5,    1,   78,    2, 0x0a /* Public */,
       7,    1,   81,    2, 0x0a /* Public */,
       9,    1,   84,    2, 0x0a /* Public */,
      11,    1,   87,    2, 0x0a /* Public */,
      13,    0,   90,    2, 0x0a /* Public */,
      14,    0,   91,    2, 0x0a /* Public */,
      15,    0,   92,    2, 0x0a /* Public */,
      16,    0,   93,    2, 0x0a /* Public */,
      17,    0,   94,    2, 0x0a /* Public */,
      18,    1,   95,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::QString,    8,
    QMetaType::Void, QMetaType::QString,   10,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void,
    QMetaType::Bool,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 19,   20,

       0        // eod
};

void KInstaller::userregisterwidiget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<userregisterwidiget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->enableNextButton((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->setComputerText(); break;
        case 2: _t->checkUserName((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->checkHostName((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->checkPasswd((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->checkCPasswd((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 6: _t->markLineEditCorrect(); break;
        case 7: { bool _r = _t->findFingerPrintDevice();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 8: _t->getAutoLogin(); break;
        case 9: _t->saveRegisterDate(); break;
        case 10: _t->translateStr(); break;
        case 11: _t->checkactive((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 11:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QWidget* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (userregisterwidiget::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&userregisterwidiget::enableNextButton)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject KInstaller::userregisterwidiget::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_KInstaller__userregisterwidiget.data,
    qt_meta_data_KInstaller__userregisterwidiget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *KInstaller::userregisterwidiget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KInstaller::userregisterwidiget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_KInstaller__userregisterwidiget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int KInstaller::userregisterwidiget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 12)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 12;
    }
    return _id;
}

// SIGNAL 0
void KInstaller::userregisterwidiget::enableNextButton(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
