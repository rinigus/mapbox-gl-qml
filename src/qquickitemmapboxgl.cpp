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
#include <QJsonDocument>

#include <math.h>

#include <QDebug>

QQuickItemMapboxGL::QQuickItemMapboxGL(QQuickItem *parent):
  QQuickItem(parent),
  m_margins(0, 0, 0, 0)
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

void QQuickItemMapboxGL::setError(QString error)
{
  m_errorString = error;
  emit errorChanged(error);
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

qreal QQuickItemMapboxGL::metersPerPixel() const
{
  return m_metersPerPixel;
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

void QQuickItemMapboxGL::setMargins(qreal left, qreal top, qreal right, qreal bottom)
{
  m_margins.setLeft(left);
  m_margins.setTop(top);
  m_margins.setRight(right);
  m_margins.setBottom(bottom);
  m_syncState |= MarginsNeedSync;
  update();
  emit marginsChanged(m_margins);
}

static QMarginsF qrect2qmargins(const QRectF &box)
{
  QMarginsF margins;
  margins.setLeft(box.x());
  margins.setBottom(box.y());
  margins.setRight(1.0 - box.width() - box.x());
  margins.setTop(1.0 - box.height() - box.y());
  return margins;
}

static QRectF qmargins2qrect(const QMarginsF &margins)
{
  QRectF rect;
  rect.setX(margins.left());
  rect.setY(margins.right());
  rect.setWidth(1.0 - margins.right() - margins.left());
  rect.setHeight(1.0 - margins.bottom() - margins.top());
  return rect;
}

QRectF QQuickItemMapboxGL::margins() const
{
  return qmargins2qrect(m_margins);
}

void QQuickItemMapboxGL::setMargins(const QRectF &margins_box)
{
  QMarginsF margins = qrect2qmargins(margins_box);
  m_margins = margins;
  m_syncState |= MarginsNeedSync;
  update();
  emit marginsChanged(m_margins);
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
  m_syncState |= DataNeedsSetupSync;
  m_syncState |= DataNeedsSync;
  m_block_data_until_loaded = true;
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
  m_syncState |= DataNeedsSetupSync;
  m_syncState |= DataNeedsSync;
  m_block_data_until_loaded = true;
  update();
  emit styleJsonChanged(QString());
  emit styleUrlChanged(url);
}

/// Interaction with the map
#define DATA_UPDATE { m_syncState |= DataNeedsSync; update(); }

/// Sources

void QQuickItemMapboxGL::addSource(const QString &sourceID, const QVariantMap &params)
{
  m_sources.add(sourceID,params); DATA_UPDATE;
}

void QQuickItemMapboxGL::addSourcePoint(const QString &sourceID, const QGeoCoordinate &coordinate, const QString &name)
{
  updateSourcePoint(sourceID, coordinate, name); // same as add for sources
}

void QQuickItemMapboxGL::addSourcePoint(const QString &sourceID, qreal latitude, qreal longitude, const QString &name)
{
  updateSourcePoint(sourceID, latitude, longitude, name);
}

void QQuickItemMapboxGL::addSourcePoints(const QString &sourceID, const QVariantList &coordinates, const QVariantList &names)
{
  updateSourcePoints(sourceID, coordinates, names);
}

void QQuickItemMapboxGL::addSourceLine(const QString &sourceID, const QVariantList &coordinates, const QString &name)
{
  updateSourceLine(sourceID, coordinates, name);
}

void QQuickItemMapboxGL::updateSource(const QString &sourceID, const QVariantMap &params)
{
  m_sources.update(sourceID, params); DATA_UPDATE;
}

void QQuickItemMapboxGL::updateSourcePoint(const QString &sourceID, const QGeoCoordinate &coordinate, const QString &name)
{
  updateSourcePoint(sourceID, coordinate.latitude(), coordinate.longitude(), name);
}

static QVariantMap pointJson(qreal latitude, qreal longitude, const QString &name)
{
  QVariantList coordinates({longitude, latitude});
  QVariantMap geometry({{"type", "Point"}, {"coordinates", coordinates}});
  QVariantMap data({
                     {"type", "Feature"},
                     {"geometry", geometry}
                   });
  QVariantMap properties;
  if (!name.isEmpty())
    properties.insert("name", name);
  data.insert("properties", properties);
  return data;
}

void QQuickItemMapboxGL::updateSourcePoint(const QString &sourceID, qreal latitude, qreal longitude, const QString &name)
{
  QVariantMap params({{"type", "geojson"}, {"data", pointJson(latitude, longitude, name)}});
  updateSource(sourceID, params);
}

void QQuickItemMapboxGL::updateSourcePoints(const QString &sourceID, const QVariantList &coordinates, const QVariantList &names)
{
  QVariantMap feature_collection({{"type", "FeatureCollection"}});
  QVariantList points;

  for (int i = 0; i < coordinates.size(); ++i)
    {
      QGeoCoordinate c = coordinates[i].value<QGeoCoordinate>();
      if (c.isValid())
        {
          QString name;
          if (i < names.size() && names[i].type() == QVariant::String)
            name = names[i].toString();

          points.append(pointJson(c.latitude(), c.longitude(), name));
        }
      else
        {
          QString err = QString("Illegal point coordinates when read as QGeoCoordinate, point %1").arg(i);
          setError(err);
          qWarning() << err;
          return;
        }
    }

  feature_collection.insert("features", points);
  QVariantMap params({{"type", "geojson"}, {"data", feature_collection}});
  updateSource(sourceID, params);
}

void QQuickItemMapboxGL::updateSourceLine(const QString &sourceID, const QVariantList &coordinates, const QString &name)
{
  QVariantList coor;

  for (int i = 0; i < coordinates.size(); ++i)
    {
      QGeoCoordinate c = coordinates[i].value<QGeoCoordinate>();
      if (c.isValid())
        coor.push_back( QVariantList({c.longitude(), c.latitude()}) );
      else
        {
          QString err = QString("Illegal point coordinates when read as QGeoCoordinate, line point %1").arg(i);
          setError(err);
          qWarning() << err;
          return;
        }
    }

  QVariantMap geometry({{"type", "LineString"}, {"coordinates", coor}});
  QVariantMap properties;
  if (!name.isEmpty())
    properties.insert("name", name);
  QVariantMap data({
                     {"type", "Feature"},
                     {"properties", properties},
                     {"geometry", geometry}
                   });

  QVariantMap params({{"type", "geojson"}, {"data", data}});
  updateSource(sourceID, params);
}

void QQuickItemMapboxGL::removeSource(const QString &sourceID)
{
  m_sources.remove(sourceID); DATA_UPDATE;
}

/// Layers

void QQuickItemMapboxGL::addLayer(const QString &id, const QVariantMap &params, const QString &before)
{
  m_layers.add(id, params, before); DATA_UPDATE;
}

void QQuickItemMapboxGL::removeLayer(const QString &id)
{
  m_layers.remove(id); DATA_UPDATE;
}

/// Images

void QQuickItemMapboxGL::addImage(const QString &name, const QImage &sprite)
{
  m_images.add(name, sprite); DATA_UPDATE;
}

void QQuickItemMapboxGL::removeImage(const QString &name)
{
  m_images.remove(name); DATA_UPDATE;
}

/// Properties

void QQuickItemMapboxGL::setLayoutProperty(const QString &layer, const QString &property, const QVariant &value)
{
  m_layout_properties.add(layer, property, value); DATA_UPDATE;
}

void QQuickItemMapboxGL::setLayoutPropertyList(const QString &layer, const QString &property, const QVariantList &value)
{
  m_layout_properties.add(layer, property, value); DATA_UPDATE;
}

void QQuickItemMapboxGL::setPaintProperty(const QString &layer, const QString &property, const QVariant &value)
{
  m_paint_properties.add(layer, property, value);
}

void QQuickItemMapboxGL::setPaintPropertyList(const QString &layer, const QString &property, const QVariantList &value)
{
  m_paint_properties.add(layer, property, value);
}


/// Location tracking
QQuickItemMapboxGL::LocationTracker::LocationTracker(const QGeoCoordinate &location):
  m_location(location), m_last_visible(false)
{
}

bool QQuickItemMapboxGL::LocationTracker::set_position(const QPoint &p, const QSize &sz)
{
  bool visible = (p.x() >= 0 && p.y() >= 0 && p.x() <= sz.width() && p.y() <= sz.height());

  if (!visible && !m_last_visible)
    {
      // non-visible location - no need to update pixel position
      return false;
    }

  bool ret = false;
  if (p != m_last_position)
    {
      m_last_position = p;
      ret = true;
    }

  if (visible != m_last_visible)
    {
      m_last_visible = visible;
      ret = true;
    }

  return ret;
}

void QQuickItemMapboxGL::trackLocation(const QString &id, const QGeoCoordinate &location)
{
  m_location_tracker[id] = LocationTracker(location);
}

void QQuickItemMapboxGL::removeLocationTracking(const QString &id)
{
  if (m_location_tracker.remove(id) > 0)
    emit locationTrackingRemoved(id);
}

void QQuickItemMapboxGL::removeAllLocationTracking()
{
  m_location_tracker.clear();
}


/// Update map
QSGNode* QQuickItemMapboxGL::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
  QSGMapboxGLTextureNode *n = static_cast<QSGMapboxGLTextureNode *>(node);
  QSize sz(width(), height());

  if (!n)
    {
      n = new QSGMapboxGLTextureNode(m_settings, sz, m_pixelRatio, this);
      m_syncState = CenterNeedsSync | ZoomNeedsSync | BearingNeedsSync | PitchNeedsSync |
          StyleNeedsSync | MarginsNeedSync;
      m_block_data_until_loaded = true;

      /////////////////////////////////////////////////////
      /// connect all queries

      connect(n, &QSGMapboxGLTextureNode::replySourceExists, this, &QQuickItemMapboxGL::replySourceExists, Qt::QueuedConnection);
      connect(this, &QQuickItemMapboxGL::querySourceExists, n, &QSGMapboxGLTextureNode::querySourceExists, Qt::QueuedConnection);

      connect(n, &QSGMapboxGLTextureNode::replyLayerExists, this, &QQuickItemMapboxGL::replyLayerExists, Qt::QueuedConnection);
      connect(this, &QQuickItemMapboxGL::queryLayerExists, n, &QSGMapboxGLTextureNode::queryLayerExists, Qt::QueuedConnection);

      connect(n, &QSGMapboxGLTextureNode::replyCoordinateForPixel, this, &QQuickItemMapboxGL::replyCoordinateForPixel, Qt::QueuedConnection);
      connect(this, &QQuickItemMapboxGL::queryCoordinateForPixel, n, &QSGMapboxGLTextureNode::queryCoordinateForPixel, Qt::QueuedConnection);
    }

  if (sz != m_last_size || m_syncState & PixelRatioNeedsSync)
    {
      n->resize(sz, m_pixelRatio);
      m_last_size = sz;
    }

  // update all settings
  QMapboxGL *map = n->map();

  if (m_syncState & MarginsNeedSync)
    {
      QMargins margins(m_margins.left()*width(), m_margins.top()*height(), m_margins.right()*width(), m_margins.bottom()*height());
      map->setMargins(margins);
      m_syncState |= CenterNeedsSync; // center has to be updated after update of the margins
    }

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

  if (!m_block_data_until_loaded && m_syncState & DataNeedsSetupSync)
    {
      // setup new map
      m_sources.setup(map);
      m_layers.setup(map);
      m_images.setup(map);
      m_layout_properties.setup(map);
      m_paint_properties.setup(map);
    }

  if (!m_block_data_until_loaded && m_syncState & DataNeedsSync)
    {
      m_sources.apply(map);
      m_layers.apply(map);
      m_images.apply(map);
      m_layout_properties.apply(map);
      m_paint_properties.apply(map);
    }

  // settings done
  m_syncState = NothingNeedsSync;

  // render the map and trigger the timer if the map is not loaded fully
  bool loaded = n->render(window());

  // check if we can add user-added sources, layers ...
  if (loaded && m_block_data_until_loaded)
    {
      m_syncState |= DataNeedsSetupSync;
      m_syncState |= DataNeedsSync;
      m_block_data_until_loaded = false;
      update();
    }

  // check the variables that are tracked on the map

  const double tol = 1e-6; // tolerance used when comparing floating point numbers
  { // metersPerPixel
    qreal meters = map->metersPerPixelAtLatitude( map->coordinate().first, map->zoom() ) / m_pixelRatio;
    if ( fabs(meters - metersPerPixel()) > tol )
      {
        m_metersPerPixel = meters;
        emit metersPerPixelChanged(meters);
      }
  }

  for ( QHash<QString, LocationTracker>::iterator i = m_location_tracker.begin();
        i != m_location_tracker.end(); ++i)
    {
      LocationTracker& tracker = i.value();
      QPointF pf = m_pixelRatio * map->pixelForCoordinate({tracker.coordinate().latitude(), tracker.coordinate().longitude()});
      QPoint p(pf.x(), pf.y());
      if (tracker.set_position(p, sz))
        emit locationChanged(i.key(), tracker.visible(), tracker.position());
    }

  // check if timer is needed
  if (!loaded && !m_timer.isActive())
    emit startRefreshTimer();
  else if (loaded && m_timer.isActive())
    emit stopRefreshTimer();
  return n;
}

