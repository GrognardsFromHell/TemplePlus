/****************************************************************************
** Meta object code from reading C++ file 'qsgd3d11layer_p.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../qsgd3d11layer_p.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qsgd3d11layer_p.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QSGD3D11Layer_t {
    QByteArrayData data[4];
    char stringdata0[44];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QSGD3D11Layer_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QSGD3D11Layer_t qt_meta_stringdata_QSGD3D11Layer = {
    {
QT_MOC_LITERAL(0, 0, 13), // "QSGD3D11Layer"
QT_MOC_LITERAL(1, 14, 16), // "markDirtyTexture"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 11) // "invalidated"

    },
    "QSGD3D11Layer\0markDirtyTexture\0\0"
    "invalidated"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QSGD3D11Layer[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x0a /* Public */,
       3,    0,   25,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void QSGD3D11Layer::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        QSGD3D11Layer *_t = static_cast<QSGD3D11Layer *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->markDirtyTexture(); break;
        case 1: _t->invalidated(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject QSGD3D11Layer::staticMetaObject = {
    { &QSGLayer::staticMetaObject, qt_meta_stringdata_QSGD3D11Layer.data,
      qt_meta_data_QSGD3D11Layer,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *QSGD3D11Layer::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QSGD3D11Layer::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_QSGD3D11Layer.stringdata0))
        return static_cast<void*>(const_cast< QSGD3D11Layer*>(this));
    return QSGLayer::qt_metacast(_clname);
}

int QSGD3D11Layer::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QSGLayer::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
