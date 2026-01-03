#ifndef MACROS_H
#define MACROS_H

#include <QSize>
#include <QtGlobal>

#define IS_QT5 (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0) && QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#define IS_QT6 (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
#define HAS_SGRENDERNODE (QT_VERSION >= QT_VERSION_CHECK(5, 8, 0))

#define MIN_TEXTURE_SIZE QSize(16, 16)

#endif // MACROS_H