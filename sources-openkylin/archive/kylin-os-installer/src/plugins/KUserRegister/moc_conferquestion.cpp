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
** Meta object code from reading C++ file 'conferquestion.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.8)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "conferquestion.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'conferquestion.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_KInstaller__conferUi_t {
    QByteArrayData data[8];
    char stringdata0[74];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_KInstaller__conferUi_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_KInstaller__conferUi_t qt_meta_stringdata_KInstaller__conferUi = {
    {
QT_MOC_LITERAL(0, 0, 20), // "KInstaller::conferUi"
QT_MOC_LITERAL(1, 21, 13), // "ClickedSignal"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 9), // "conferUi*"
QT_MOC_LITERAL(4, 46, 1), // "p"
QT_MOC_LITERAL(5, 48, 10), // "qClickedMe"
QT_MOC_LITERAL(6, 59, 3), // "str"
QT_MOC_LITERAL(7, 63, 10) // "cClickedMe"

    },
    "KInstaller::conferUi\0ClickedSignal\0\0"
    "conferUi*\0p\0qClickedMe\0str\0cClickedMe"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_KInstaller__conferUi[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   32,    2, 0x0a /* Public */,
       7,    1,   35,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,

 // slots: parameters
    QMetaType::Void, QMetaType::QString,    6,
    QMetaType::Void, QMetaType::QString,    6,

       0        // eod
};

void KInstaller::conferUi::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<conferUi *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->ClickedSignal((*reinterpret_cast< conferUi*(*)>(_a[1]))); break;
        case 1: _t->qClickedMe((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->cClickedMe((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< conferUi* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (conferUi::*)(conferUi * );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&conferUi::ClickedSignal)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject KInstaller::conferUi::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_KInstaller__conferUi.data,
    qt_meta_data_KInstaller__conferUi,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *KInstaller::conferUi::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KInstaller::conferUi::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_KInstaller__conferUi.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int KInstaller::conferUi::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}

// SIGNAL 0
void KInstaller::conferUi::ClickedSignal(conferUi * _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
struct qt_meta_stringdata_KInstaller__conferquestion_t {
    QByteArrayData data[13];
    char stringdata0[142];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_KInstaller__conferquestion_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_KInstaller__conferquestion_t qt_meta_stringdata_KInstaller__conferquestion = {
    {
QT_MOC_LITERAL(0, 0, 26), // "KInstaller::conferquestion"
QT_MOC_LITERAL(1, 27, 16), // "enableNextButton"
QT_MOC_LITERAL(2, 44, 0), // ""
QT_MOC_LITERAL(3, 45, 5), // "value"
QT_MOC_LITERAL(4, 51, 13), // "savconferDate"
QT_MOC_LITERAL(5, 65, 11), // "conferCheck"
QT_MOC_LITERAL(6, 77, 9), // "conferUi*"
QT_MOC_LITERAL(7, 87, 6), // "confer"
QT_MOC_LITERAL(8, 94, 11), // "conferLater"
QT_MOC_LITERAL(9, 106, 12), // "translateStr"
QT_MOC_LITERAL(10, 119, 11), // "checkactive"
QT_MOC_LITERAL(11, 131, 8), // "QWidget*"
QT_MOC_LITERAL(12, 140, 1) // "p"

    },
    "KInstaller::conferquestion\0enableNextButton\0"
    "\0value\0savconferDate\0conferCheck\0"
    "conferUi*\0confer\0conferLater\0translateStr\0"
    "checkactive\0QWidget*\0p"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_KInstaller__conferquestion[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    0,   47,    2, 0x0a /* Public */,
       5,    1,   48,    2, 0x0a /* Public */,
       8,    0,   51,    2, 0x0a /* Public */,
       9,    0,   52,    2, 0x0a /* Public */,
      10,    1,   53,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 11,   12,

       0        // eod
};

void KInstaller::conferquestion::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<conferquestion *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->enableNextButton((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->savconferDate(); break;
        case 2: _t->conferCheck((*reinterpret_cast< conferUi*(*)>(_a[1]))); break;
        case 3: _t->conferLater(); break;
        case 4: _t->translateStr(); break;
        case 5: _t->checkactive((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 2:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< conferUi* >(); break;
            }
            break;
        case 5:
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
            using _t = void (conferquestion::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&conferquestion::enableNextButton)) {
                *result = 0;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject KInstaller::conferquestion::staticMetaObject = { {
    &QWidget::staticMetaObject,
    qt_meta_stringdata_KInstaller__conferquestion.data,
    qt_meta_data_KInstaller__conferquestion,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *KInstaller::conferquestion::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *KInstaller::conferquestion::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_KInstaller__conferquestion.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int KInstaller::conferquestion::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void KInstaller::conferquestion::enableNextButton(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
