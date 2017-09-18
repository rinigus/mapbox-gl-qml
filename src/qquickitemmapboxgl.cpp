/****************************************************************************
**
** Some parts of the code are based on the implementation of Mapbox GL Native
** QtLocation plugin at
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

#include "qquickitemmapboxgl.h"
#include "qsgmapboxglnode.h"

#include <mbgl/util/constants.hpp>

#include <QVariantMap>

#include <math.h>

#include <QDebug>

QQuickItemMapboxGL::QQuickItemMapboxGL(QQuickItem *parent):
  QQuickItem(parent)
{
  setFlag(ItemHasContents);

  m_styleUrl = QMapbox::defaultStyles()[0].first;
  m_styleJson = QString(); // empty

  m_settings.setViewportMode(QMapboxGLSettings::DefaultViewport);

  m_pixelRatio = 1;

  m_timer.setInterval(250);
  connect(&m_timer, &QTimer::timeout, this, &QQuickItemMapboxGL::update);
  connect(this, SIGNAL(startRefreshTimer()), &m_timer, SLOT(start()));
  connect(this, &QQuickItemMapboxGL::stopRefreshTimer, &m_timer, &QTimer::stop);
}

QQuickItemMapboxGL::~QQuickItemMapboxGL()
{
}

QVariantList QQuickItemMapboxGL::defaultStyles() const
{
  QVariantList array;
  auto styles = QMapbox::defaultStyles();
  for (const auto &i: styles)
    {
      QVariantMap o;
      o.insert("url", QVariant(i.first));
      o.insert("name", QVariant(i.second));
      array.append(o);
    }

  return array;
}

/// Properties that have to be set during construction of the map
QString QQuickItemMapboxGL::accessToken() const
{
  return m_settings.accessToken();
}

void QQuickItemMapboxGL::setAccessToken(const QString &token)
{
  m_settings.setAccessToken(token);
  emit accessTokenChanged(accessToken());
}

QString QQuickItemMapboxGL::apiBaseUrl() const
{
  return m_settings.apiBaseUrl();
}

void QQuickItemMapboxGL::setApiBaseUrl(const QString &url)
{
  m_settings.setApiBaseUrl(url);
  emit apiBaseUrlChanged(apiBaseUrl());
}

QString QQuickItemMapboxGL::assetPath() const
{
  return m_settings.assetPath();
}

void QQuickItemMapboxGL::setAssetPath(const QString &path)
{
  m_settings.setAssetPath(path);
  emit assetPathChanged(assetPath());
}

QString QQuickItemMapboxGL::cacheDatabasePath() const
{
  return m_settings.cacheDatabasePath();
}

void QQuickItemMapboxGL::setCacheDatabasePath(const QString &path)
{
  m_settings.setCacheDatabasePath(path);
  emit cacheDatabasePathChanged(cacheDatabasePath());
}

int QQuickItemMapboxGL::cacheDatabaseMaximalSize() const
{
  return m_settings.cacheDatabaseMaximumSize();
}

void QQuickItemMapboxGL::setCacheDatabaseMaximalSize(int sz)
{
  m_settings.setCacheDatabaseMaximumSize(sz);
  emit cacheDatabaseMaximalSizeChanged(cacheDatabaseMaximalSize());
}

/// Error feedback
QString QQuickItemMapboxGL::errorString() const
{
  return m_errorString;
}

/// Zoom properties
void QQuickItemMapboxGL::setMinimumZoomLevel(qreal zoom)
{
  zoom = qMax((qreal)mbgl::util::MIN_ZOOM, zoom);
  zoom = qMin(m_maximumZoomLevel, zoom);

  if (m_minimumZoomLevel == zoom) return;

  m_minimumZoomLevel = zoom;
  setZoomLevel(m_zoomLevel); // Constrain.

  emit minimumZoomLevelChanged();
}

qreal QQuickItemMapboxGL::minimumZoomLevel() const
{
  return m_minimumZoomLevel;
}

void QQuickItemMapboxGL::setMaximumZoomLevel(qreal zoom)
{
  zoom = qMin((qreal)mbgl::util::MAX_ZOOM, zoom);
  zoom = qMax(m_minimumZoomLevel, zoom);

  if (m_maximumZoomLevel == zoom) return;

  m_maximumZoomLevel = zoom;
  setZoomLevel(m_zoomLevel); // Constrain.

  emit maximumZoomLevelChanged();
}

qreal QQuickItemMapboxGL::maximumZoomLevel() const
{
  return m_maximumZoomLevel;
}

qreal QQuickItemMapboxGL::zoomLevel() const
{
  return m_zoomLevel;
}

void QQuickItemMapboxGL::setZoomLevel(qreal zoom, const QPointF &center)
{
  zoom = qMin(m_maximumZoomLevel, zoom);
  zoom = qMax(m_minimumZoomLevel, zoom);

  if (m_zoomLevel == zoom) return;

  m_zoomLevel = zoom;
  m_zoomLevelPoint = center;

  m_syncState |= ZoomNeedsSync;
  update();

  emit zoomLevelChanged(m_zoomLevel);
}

/// Position
void QQuickItemMapboxGL::setCenter(const QGeoCoordinate &coordinate)
{
  if (m_center == coordinate) return;

  m_center = coordinate;

  m_syncState |= CenterNeedsSync;
  update();

  emit centerChanged(m_center);
}

QGeoCoordinate QQuickItemMapboxGL::center() const
{
  return m_center;
}

void QQuickItemMapboxGL::pan(int dx, int dy)
{
  m_pan += QPointF(dx, dy);

  m_syncState |= PanNeedsSync;
  update();
}

qreal QQuickItemMapboxGL::bearing() const
{
  return m_bearing;
}

void QQuickItemMapboxGL::setBearing(qreal b)
{
  m_bearing = b;
  m_syncState |= BearingNeedsSync;
  update();
  emit bearingChanged(m_bearing);
}

qreal QQuickItemMapboxGL::pitch() const
{
  return m_pitch;
}

void QQuickItemMapboxGL::setPitch(qreal p)
{
  m_pitch = p;
  m_syncState |= PitchNeedsSync;
  update();
  emit pitchChanged(p);
}

/// Rendering details
qreal QQuickItemMapboxGL::pixelRatio() const
{
  return m_pixelRatio;
}

void QQuickItemMapboxGL::setPixelRatio(qreal pixelRatio)
{
  m_pixelRatio = qMax((qreal)1.0, pixelRatio);
  m_syncState |= PixelRatioNeedsSync;
  update();
  emit pixelRatioChanged(m_pixelRatio);
}

QString QQuickItemMapboxGL::styleJson() const
{
  return m_styleJson;
}

void QQuickItemMapboxGL::setStyleJson(const QString &json)
{
  m_styleJson = json;
  m_styleUrl = QString();
  m_syncState |= StyleNeedsSync;
  update();
  emit styleJsonChanged(json);
  emit styleUrlChanged(QString());
}

QString QQuickItemMapboxGL::styleUrl() const
{
  return m_styleUrl;
}

void QQuickItemMapboxGL::setStyleUrl(const QString &url)
{
  m_styleJson = QString();
  m_styleUrl = url;
  m_syncState |= StyleNeedsSync;
  update();
  emit styleJsonChanged(QString());
  emit styleUrlChanged(url);
}

/// Update map
QSGNode* QQuickItemMapboxGL::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
  QSGMapboxGLTextureNode *n = static_cast<QSGMapboxGLTextureNode *>(node);
  QSize sz(width(), height());

  if (!n)
    {
      n = new QSGMapboxGLTextureNode(m_settings, sz, m_pixelRatio, this);
      m_syncState = CenterNeedsSync | ZoomNeedsSync | BearingNeedsSync | PitchNeedsSync | StyleNeedsSync;
    }

  if (sz != m_last_size || m_syncState & PixelRatioNeedsSync)
    {
      n->resize(sz, m_pixelRatio);
      m_last_size = sz;
    }

  // update all settings
  QMapboxGL *map = n->map();

  if (m_syncState & CenterNeedsSync)
    {
      const auto& c = center();
      map->setCoordinateZoom({ c.latitude(), c.longitude() }, zoomLevel());
    }

  if (m_syncState & ZoomNeedsSync)
    {
      if (m_zoomLevelPoint.isNull())
        map->setZoom(zoomLevel());
      else
        {
          qreal newscale = pow(2.0, zoomLevel());
          map->setScale(newscale, m_zoomLevelPoint / m_pixelRatio);
          m_zoomLevelPoint = QPointF();
        }
    }

  if (m_syncState & BearingNeedsSync)
    map->setBearing(m_bearing);

  if (m_syncState & PitchNeedsSync)
    map->setPitch(m_pitch);

  if (m_syncState & PanNeedsSync)
    {
      map->moveBy(m_pan / m_pixelRatio);
      m_pan = QPointF();
      m_center = QGeoCoordinate(map->latitude(), map->longitude());
      emit centerChanged(m_center);
    }

  if (m_syncState & StyleNeedsSync)
    {
      if (m_styleJson.isEmpty())
        map->setStyleUrl(m_styleUrl);
      else
        map->setStyleJson(m_styleJson);
    }

  // settings done
  m_syncState = NothingNeedsSync;

  // render the map and trigger the timer if the map is not loaded fully
  bool loaded = n->render(window());
  if (!loaded && !m_timer.isActive())
    emit startRefreshTimer();
  else if (loaded && m_timer.isActive())
    emit stopRefreshTimer();
  return n;
}

