/****************************************************************************
**
** Based on the implementation of Mapbox GL Native QtLocation plugin at
** https://github.com/qt/qtlocation/tree/5.9/src/plugins/geoservices/mapboxgl
** and later versions. Developed further for integration with the plugin
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

#include "qsgmapboxglnode.h"
#include "qsgtextureplain.h"

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>

#include <math.h>

#include <QDebug>

static const QSize minTextureSize = QSize(16, 16);

////////////////////////////////////////////
/// QSGMapboxGLAbstractNode

QSGMapboxGLAbstractNode::QSGMapboxGLAbstractNode(const QMapboxGLSettings &settings, const QSize &size,
                                                 qreal devicePixelRatio, qreal pixelRatio, QQuickItem *item):
  QObject(), m_map_size(size), m_item_size(size),
  m_device_pixel_ratio(devicePixelRatio), m_pixel_ratio(pixelRatio)
{
  m_map.reset(new QMapboxGL(nullptr, settings, size.expandedTo(minTextureSize), pixelRatio));

  QObject::connect(m_map.data(), &QMapboxGL::needsRendering, item, &QQuickItem::update);
  QObject::connect(m_map.data(), &QMapboxGL::copyrightsChanged, item, &QQuickItem::update);
}

void QSGMapboxGLAbstractNode::resize(const QSize &size, qreal pixelRatio)
{
  m_item_size = size;
  m_pixel_ratio = pixelRatio;
}

float QSGMapboxGLAbstractNode::mapToQtPixelRatio() const
{
  return 0.5 * (width()/m_item_size.width() +
                height()/m_item_size.height());
}

///////////////////////////////////
/// queries

void QSGMapboxGLAbstractNode::querySourceExists(const QString &sourceID)
{
  emit replySourceExists(sourceID, m_map->sourceExists(sourceID));
}

void QSGMapboxGLAbstractNode::queryLayerExists(const QString &sourceID)
{
  emit replyLayerExists(sourceID, m_map->layerExists(sourceID));
}

void QSGMapboxGLAbstractNode::queryCoordinateForPixel(QPointF p, const QVariant &tag)
{
  float rx = ((float)m_map_size.width()) / ((float)m_item_size.width());
  float ry = ((float)m_map_size.height()) / ((float)m_item_size.height());

  p.setX(p.x() * rx);
  p.setY(p.y() * ry);
  QMapbox::Coordinate mbc = m_map->coordinateForPixel(p);
  QGeoCoordinate coor(mbc.first, mbc.second);

  // get sensitivity of coordinates to the changes in pixel coordinates
  double bearing = m_map->bearing() / 180. * M_PI;
  double sinB = sin(bearing);
  double cosB = cos(bearing);
  p += QPointF(cosB + sinB, -sinB + cosB);
  QMapbox::Coordinate mbc_shift = m_map->coordinateForPixel(p);

  qreal degLatPerPixel = fabs(mbc_shift.first - mbc.first) * rx;
  qreal degLonPerPixel = fabs(mbc_shift.second - mbc.second) * ry;

  emit replyCoordinateForPixel(p, coor, degLatPerPixel, degLonPerPixel, tag);
}


////////////////////////////////////////////
/// QSGMapboxGLTextureNode

QSGMapboxGLTextureNode::QSGMapboxGLTextureNode(const QMapboxGLSettings &settings, const QSize &size,
                                               qreal devicePixelRatio,
                                               qreal pixelRatio, QQuickItem *item)
  : QSGMapboxGLAbstractNode(settings, size, devicePixelRatio, pixelRatio, item),
    QSGSimpleTextureNode()
{
  qInfo() << "Using QSGMapboxGLTextureNode for map rendering."
          << "devicePixelRatio:" << devicePixelRatio;

  setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
  setFiltering(QSGTexture::Linear);

  resize(size, pixelRatio); // to fill and attach fbo
}

QSGMapboxGLTextureNode::~QSGMapboxGLTextureNode()
{
}

void QSGMapboxGLTextureNode::resize(const QSize &size, qreal pixelRatio)
{
  const QSize minSize = size.expandedTo(minTextureSize);
  QSGMapboxGLAbstractNode::resize(minSize, pixelRatio);

  const QSize fbSize = minSize * m_device_pixel_ratio; // physical pixels
  m_map_size = minSize * m_device_pixel_ratio / m_pixel_ratio; // ensure zoom

  m_map->resize(m_map_size);

  m_fbo.reset(new QOpenGLFramebufferObject(fbSize, QOpenGLFramebufferObject::CombinedDepthStencil));
  m_map->setFramebufferObject(m_fbo->handle(), fbSize); //minSize);

  QSGTexturePlain *fboTexture = static_cast<QSGTexturePlain *>(texture());
  if (!fboTexture)
    fboTexture = new QSGTexturePlain;

  fboTexture->setTextureId(m_fbo->texture());
  fboTexture->setTextureSize(fbSize);

  if (!texture()) {
      setTexture(fboTexture);
      setOwnsTexture(true);
    }

  setRect(QRectF(QPointF(), minSize));
}

void QSGMapboxGLTextureNode::render(QQuickWindow *window)
{
  QOpenGLFunctions *f = window->openglContext()->functions();
  f->glViewport(0, 0, m_fbo->width(), m_fbo->height());

  GLint alignment;
  f->glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);

  m_fbo->bind();
  m_map->render();
  //    m_logo.render();
  m_fbo->release();

  // QTBUG-62861
  f->glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
  f->glDepthRangef(0, 1);

  window->resetOpenGLState();
}

#if HAS_SGRENDERNODE
////////////////////////////////////////////
/// QSGMapboxGLRenderNode

QSGMapboxGLRenderNode::QSGMapboxGLRenderNode(const QMapboxGLSettings &settings, const QSize &size,
                                             qreal devicePixelRatio, qreal pixelRatio, QQuickItem *item)
  : QSGMapboxGLAbstractNode(settings, size, devicePixelRatio, pixelRatio, item)
{
  qInfo() << "Using QSGMapboxGLRenderNode for map rendering. "
          << "devicePixelRatio:" << devicePixelRatio;
}

void QSGMapboxGLRenderNode::resize(const QSize &size, qreal pixelRatio)
{
  const QSize minSize = size.expandedTo(minTextureSize);
  QSGMapboxGLAbstractNode::resize(size, pixelRatio);
  m_map_size = minSize / pixelRatio;
  m_map->resize(m_map_size);
}

void QSGMapboxGLRenderNode::render(const RenderState *state)
{
  // QMapboxGL assumes we've prepared the viewport prior to render().
  QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
  f->glViewport(state->scissorRect().x(), state->scissorRect().y(), state->scissorRect().width(), state->scissorRect().height());
  f->glScissor(state->scissorRect().x(), state->scissorRect().y(), state->scissorRect().width(), state->scissorRect().height());
  f->glEnable(GL_SCISSOR_TEST);

  GLint alignment;
  f->glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);

  m_map->render();

  // QTBUG-62861
  f->glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
  f->glDepthRangef(0, 1);
}

QSGRenderNode::StateFlags QSGMapboxGLRenderNode::changedStates() const
{
  return QSGRenderNode::DepthState
      | QSGRenderNode::StencilState
      | QSGRenderNode::ScissorState
      | QSGRenderNode::ColorState
      | QSGRenderNode::BlendState
      | QSGRenderNode::ViewportState
      | QSGRenderNode::RenderTargetState;
}

#endif
