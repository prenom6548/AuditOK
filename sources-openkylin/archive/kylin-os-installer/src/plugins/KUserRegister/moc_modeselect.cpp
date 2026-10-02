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
** Meta object code from reading C++ file 'modeselect.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "modeselect.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'modeselect.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_KInstaller__chiceBtn_t {
    QByteArrayData data[5];
    char stringdata0[50];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_KInstaller__chiceBtn_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_KInstaller__chiceBtn_t qt_meta_stringdata_KInstaller__chiceBtn = {
    {
QT_MOC_LITERAL(0, 0, 20), // "KInstaller::chiceBtn"
QT_MOC_LITERAL(1, 21, 13), // "clickedSignal"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 9), // "chiceBtn*"
QT_MOC_LITERAL(4, 46, 3) // "bnt"

    },
    "KInstaller::chiceBtn\0clickedSignal\0\0"
    "chiceBtn*\0bnt"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_KInstaller__chiceBtn[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

       0        // eod
};

void KInstaller::chiceBtn::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<chiceBtn *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->clickedSignal((*reinterpret_cast< chiceBtn*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< chiceBtn* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (chiceBtn::*)(chiceBtn * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&chiceBtn::clickedSignal)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject KInstaller::chiceBtn::staticMetaObject = { {
    &QPushButton::staticMetaObject,
    qt_meta_stringdata_KInstaller__chiceBtn.data,
    qt_meta_data_KInstaller__chiceBtn,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *KInstaller::chiceBtn::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KInstaller::chiceBtn::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_KInstaller__chiceBtn.stringdata0))
        return static_cast<void*>(this);
    return QPushButton::qt_metacast(_clname);
}

int KInstaller::chiceBtn::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QPushButton::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}

// SIGNAL 0
void KInstaller::chiceBtn::clickedSignal(chiceBtn * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_KInstaller__modeselect_t {
    QByteArrayData data[12];
    char stringdata0[120];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_KInstaller__modeselect_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_KInstaller__modeselect_t qt_meta_stringdata_KInstaller__modeselect = {
    {
QT_MOC_LITERAL(0, 0, 22), // "KInstaller::modeselect"
QT_MOC_LITERAL(1, 23, 10), // "modeSelect"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 9), // "chiceBtn*"
QT_MOC_LITERAL(4, 45, 3), // "bnt"
QT_MOC_LITERAL(5, 49, 16), // "enableNextButton"
QT_MOC_LITERAL(6, 66, 5), // "value"
QT_MOC_LITERAL(7, 72, 12), // "translateStr"
QT_MOC_LITERAL(8, 85, 11), // "changemodel"
QT_MOC_LITERAL(9, 97, 11), // "checkactive"
QT_MOC_LITERAL(10, 109, 8), // "QWidget*"
QT_MOC_LITERAL(11, 118, 1) // "p"

    },
    "KInstaller::modeselect\0modeSelect\0\0"
    "chiceBtn*\0bnt\0enableNextButton\0value\0"
    "translateStr\0changemodel\0checkactive\0"
    "QWidget*\0p"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_KInstaller__modeselect[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       5,    1,   42,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    0,   45,    2, 0x0a /* Public */,
       8,    1,   46,    2, 0x0a /* Public */,
       9,    1,   49,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::Bool,    6,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, 0x80000000 | 10,   11,

       0        // eod
};

void KInstaller::modeselect::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<modeselect *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->modeSelect((*reinterpret_cast< chiceBtn*(*)>(_a[1]))); break;
        case 1: _t->enableNextButton((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 2: _t->translateStr(); break;
        case 3: _t->changemodel((*reinterpret_cast< chiceBtn*(*)>(_a[1]))); break;
        case 4: _t->checkactive((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< chiceBtn* >(); break;
            }
            break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< chiceBtn* >(); break;
            }
            break;
        case 4:
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
            using _t = void (modeselect::*)(chiceBtn * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&modeselect::modeSelect)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (modeselect::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&modeselect::enableNextButton)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject KInstaller::modeselect::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_KInstaller__modeselect.data,
    qt_meta_data_KInstaller__modeselect,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *KInstaller::modeselect::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KInstaller::modeselect::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_KInstaller__modeselect.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int KInstaller::modeselect::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void KInstaller::modeselect::modeSelect(chiceBtn * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void KInstaller::modeselect::enableNextButton(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
