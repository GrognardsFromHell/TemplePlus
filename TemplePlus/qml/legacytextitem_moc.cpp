/****************************************************************************
** Meta object code from reading C++ file 'legacytextitem.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.8.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "legacytextitem.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'legacytextitem.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.8.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_LegacyTextItem_t {
    QByteArrayData data[14];
    char stringdata0[180];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_LegacyTextItem_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_LegacyTextItem_t qt_meta_stringdata_LegacyTextItem = {
    {
QT_MOC_LITERAL(0, 0, 14), // "LegacyTextItem"
QT_MOC_LITERAL(1, 15, 11), // "textChanged"
QT_MOC_LITERAL(2, 27, 0), // ""
QT_MOC_LITERAL(3, 28, 12), // "colorChanged"
QT_MOC_LITERAL(4, 41, 15), // "gradientChanged"
QT_MOC_LITERAL(5, 57, 20), // "gradientColorChanged"
QT_MOC_LITERAL(6, 78, 17), // "dropShadowChanged"
QT_MOC_LITERAL(7, 96, 22), // "dropShadowColorChanged"
QT_MOC_LITERAL(8, 119, 4), // "text"
QT_MOC_LITERAL(9, 124, 5), // "color"
QT_MOC_LITERAL(10, 130, 8), // "gradient"
QT_MOC_LITERAL(11, 139, 13), // "gradientColor"
QT_MOC_LITERAL(12, 153, 10), // "dropShadow"
QT_MOC_LITERAL(13, 164, 15) // "dropShadowColor"

    },
    "LegacyTextItem\0textChanged\0\0colorChanged\0"
    "gradientChanged\0gradientColorChanged\0"
    "dropShadowChanged\0dropShadowColorChanged\0"
    "text\0color\0gradient\0gradientColor\0"
    "dropShadow\0dropShadowColor"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_LegacyTextItem[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       6,   50, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       6,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   44,    2, 0x06 /* Public */,
       3,    0,   45,    2, 0x06 /* Public */,
       4,    0,   46,    2, 0x06 /* Public */,
       5,    0,   47,    2, 0x06 /* Public */,
       6,    0,   48,    2, 0x06 /* Public */,
       7,    0,   49,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
       8, QMetaType::QString, 0x00495103,
       9, QMetaType::QColor, 0x00495103,
      10, QMetaType::Bool, 0x00495103,
      11, QMetaType::QColor, 0x00495103,
      12, QMetaType::Bool, 0x00495103,
      13, QMetaType::QColor, 0x00495103,

 // properties: notify_signal_id
       0,
       1,
       2,
       3,
       4,
       5,

       0        // eod
};

void LegacyTextItem::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        LegacyTextItem *_t = static_cast<LegacyTextItem *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->textChanged(); break;
        case 1: _t->colorChanged(); break;
        case 2: _t->gradientChanged(); break;
        case 3: _t->gradientColorChanged(); break;
        case 4: _t->dropShadowChanged(); break;
        case 5: _t->dropShadowColorChanged(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (LegacyTextItem::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&LegacyTextItem::textChanged)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (LegacyTextItem::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&LegacyTextItem::colorChanged)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (LegacyTextItem::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&LegacyTextItem::gradientChanged)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (LegacyTextItem::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&LegacyTextItem::gradientColorChanged)) {
                *result = 3;
                return;
            }
        }
        {
            typedef void (LegacyTextItem::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&LegacyTextItem::dropShadowChanged)) {
                *result = 4;
                return;
            }
        }
        {
            typedef void (LegacyTextItem::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&LegacyTextItem::dropShadowColorChanged)) {
                *result = 5;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        LegacyTextItem *_t = static_cast<LegacyTextItem *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QString*>(_v) = _t->getText(); break;
        case 1: *reinterpret_cast< QColor*>(_v) = _t->getColor(); break;
        case 2: *reinterpret_cast< bool*>(_v) = _t->gradient(); break;
        case 3: *reinterpret_cast< QColor*>(_v) = _t->getGradientColor(); break;
        case 4: *reinterpret_cast< bool*>(_v) = _t->dropShadow(); break;
        case 5: *reinterpret_cast< QColor*>(_v) = _t->getDropShadowColor(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        LegacyTextItem *_t = static_cast<LegacyTextItem *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setText(*reinterpret_cast< QString*>(_v)); break;
        case 1: _t->setColor(*reinterpret_cast< QColor*>(_v)); break;
        case 2: _t->setGradient(*reinterpret_cast< bool*>(_v)); break;
        case 3: _t->setGradientColor(*reinterpret_cast< QColor*>(_v)); break;
        case 4: _t->setDropShadow(*reinterpret_cast< bool*>(_v)); break;
        case 5: _t->setDropShadowColor(*reinterpret_cast< QColor*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
    Q_UNUSED(_a);
}

const QMetaObject LegacyTextItem::staticMetaObject = {
    { &QQuickPaintedItem::staticMetaObject, qt_meta_stringdata_LegacyTextItem.data,
      qt_meta_data_LegacyTextItem,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *LegacyTextItem::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *LegacyTextItem::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_LegacyTextItem.stringdata0))
        return static_cast<void*>(const_cast< LegacyTextItem*>(this));
    return QQuickPaintedItem::qt_metacast(_clname);
}

int LegacyTextItem::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QQuickPaintedItem::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
#ifndef QT_NO_PROPERTIES
   else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 6;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 6;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void LegacyTextItem::textChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, Q_NULLPTR);
}

// SIGNAL 1
void LegacyTextItem::colorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void LegacyTextItem::gradientChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}

// SIGNAL 3
void LegacyTextItem::gradientColorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, Q_NULLPTR);
}

// SIGNAL 4
void LegacyTextItem::dropShadowChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, Q_NULLPTR);
}

// SIGNAL 5
void LegacyTextItem::dropShadowColorChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, Q_NULLPTR);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
