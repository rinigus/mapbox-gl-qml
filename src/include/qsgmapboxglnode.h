/****************************************************************************
**
** Based on the implementation of Mapbox GL Native QtLocation plugin at
** https://github.com/qt/qtlocation/tree/5.9/src/plugins/geoservices/mapboxgl
**
** The original code license is below
**
** Copyright (C) 2017 The Qt Company Ltd.
** Copyright (C) 2017 Mapbox, Inc.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSGMAPBOXGLNODE_H
#define QSGMAPBOXGLNODE_H

#define HAS_SGRENDERNODE (QT_VERSION >= QT_VERSION_CHECK(5,8,0))

#include <QtQuick/QQuickWindow>
#include <QtQuick/QSGSimpleTextureNode>
#include <QtGui/QOpenGLFramebufferObject>
#include <QQuickItem>
#include <QGeoCoordinate>

#include <QMapboxGL>

class QSGMapboxGLAbstractNode : public QObject
{
  Q_OBJECT

public:
  QSGMapboxGLAbstractNode(const QMapboxGLSettings &, const QSize &, qreal devicePixelRatio, qreal pixelRatio, QQuickItem *item);

  QMapboxGL* map() const { return m_map.data(); }
  float height() const { return m_map_size.height(); }
  float width() const { return m_map_size.width(); }
  float mapToQtPixelRatio() const;

  virtual void resize(const QSize &size, qreal pixelRatio);
  virtual void render(QQuickWindow *) {}

public slots:
  void querySourceExists(const QString &id);
  void queryLayerExists(const QString &id);
  void queryCoordinateForPixel(QPointF p, const QVariant &tag);

signals:
  void replySourceExists(const QString id, bool exists);
  void replyLayerExists(const QString id, bool exists);
  void replyCoordinateForPixel(const QPointF p, QGeoCoordinate geo, qreal degLatPerPixel, qreal degLonPerPixel, const QVariant &tag);

protected:
  QScopedPointer<QMapboxGL> m_map;
  QSize m_map_size; ///<- size as set for map
  QSize m_item_size; ///<- size of Qt item in Qt logical pixels units
  qreal m_pixel_ratio;
  qreal m_device_pixel_ratio{1};
};

class QSGMapboxGLTextureNode : public QSGMapboxGLAbstractNode, public QSGSimpleTextureNode
{
  Q_OBJECT

public:
  QSGMapboxGLTextureNode(const QMapboxGLSettings &, const QSize &, qreal devicePixelRatio, qreal pixelRatio, QQuickItem *item);
  ~QSGMapboxGLTextureNode();

  QMapboxGL* map() const { return m_map.data(); }

  void resize(const QSize &size, qreal pixelRatio) override;
  void render(QQuickWindow *) override;

private:
  QScopedPointer<QOpenGLFramebufferObject> m_fbo;
};

#if HAS_SGRENDERNODE
#include <QSGRenderNode>

class QSGMapboxGLRenderNode : public QSGMapboxGLAbstractNode, public QSGRenderNode
{
  Q_OBJECT

public:
  QSGMapboxGLRenderNode(const QMapboxGLSettings &, const QSize &, qreal devicePixelRatio, qreal pixelRatio, QQuickItem *item);

  QMapboxGL* map() const;

  void resize(const QSize &size, qreal pixelRatio);

  // QSGRenderNode
  void render(const RenderState *state) override;
  StateFlags changedStates() const override;
};
#endif

#endif // QSGMAPBOXGLNODE_H
