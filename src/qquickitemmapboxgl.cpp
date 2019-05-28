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

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QMutexLocker>
#include <QSettings>
#include <QStandardPaths>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariantMap>

#include <math.h>
#include <iostream>

#include <QDebug>

#ifdef USE_CURL_SSL
////////////////////////////////////////////////////////////////////////////////////////////////////
/// Special handling of cURL and OpenSSL locks, see https://curl.haxx.se/libcurl/c/threaded-ssl.html
/// not needed if used without curl mapbox gl http backend
#include <curl/curl.h>
#include <openssl/crypto.h>
static pthread_mutex_t *lockarray;
static void lock_callback(int mode, int type, const char *file, int line)
{
  (void)file;
  (void)line;
  if(mode & CRYPTO_LOCK) {
    pthread_mutex_lock(&(lockarray[type]));
  }
  else {
    pthread_mutex_unlock(&(lockarray[type]));
  }
}

static unsigned long thread_id(void)
{
  unsigned long ret;

  ret = (unsigned long)pthread_self();
  return ret;
}

static void init_locks(void)
{
  int i;

  lockarray = (pthread_mutex_t *)OPENSSL_malloc(CRYPTO_num_locks() *
                                                sizeof(pthread_mutex_t));
  for(i = 0; i<CRYPTO_num_locks(); i++) {
    pthread_mutex_init(&(lockarray[i]), NULL);
  }

  CRYPTO_set_id_callback((unsigned long (*)())thread_id);
  CRYPTO_set_locking_callback(lock_callback);
}

static void kill_locks(void)
{
  int i;

  CRYPTO_set_locking_callback(NULL);
  for(i = 0; i<CRYPTO_num_locks(); i++)
    pthread_mutex_destroy(&(lockarray[i]));

  OPENSSL_free(lockarray);
}
/// cURL + OpenSSL handling functions defined
/////////////////////////////////////////////////////////
#endif

QQuickItemMapboxGL::QQuickItemMapboxGL(QQuickItem *parent):
  QQuickItem(parent),
  m_margins(0, 0, 0, 0)
{
  setFlag(ItemHasContents);

  m_styleUrl = QMapbox::defaultStyles()[0].first;
  m_styleJson = QString(); // empty

  m_settings.setViewportMode(QMapboxGLSettings::DefaultViewport);

  m_settings.setResourceTransform(std::bind(&QQuickItemMapboxGL::resourceTransform,
                                            this, std::placeholders::_1));

  m_pixelRatio = 1;

  m_timer.setInterval(250);
  connect(&m_timer, &QTimer::timeout, this, &QQuickItemMapboxGL::update);
  connect(this, SIGNAL(startRefreshTimer()), &m_timer, SLOT(start()));
  connect(this, &QQuickItemMapboxGL::stopRefreshTimer, &m_timer, &QTimer::stop);

  // connect query signals to update to enforce rendering thread wakeup
  connect(this, SIGNAL(querySourceExists(QString)), this, SLOT(update()));
  connect(this, SIGNAL(queryLayerExists(QString)), this, SLOT(update()));
  connect(this, SIGNAL(queryCoordinateForPixel(QPointF,QVariant)), this, SLOT(update()));

#ifdef USE_CURL_SSL
  // init curl and add ssl locks

  /* Must initialize libcurl before any threads are started */
  curl_global_init(CURL_GLOBAL_ALL);
  init_locks();
#endif
}

QQuickItemMapboxGL::~QQuickItemMapboxGL()
{
#ifdef USE_CURL_SSL
  // curl openssl locks
  kill_locks();
#endif
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
  // check if path exists and create required directories if needed
  QFileInfo fi(path);
  if (!fi.exists())
    {
      QDir dir = fi.dir();
      if (!dir.mkpath("."))
        {
          setError("[ERROR] Failed to create directory for the cache database " + path +
                   " [directory that was attempted to create: " +
                   dir.path() + "]");
          return; // skipping non-working settings
        }
    }

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

  if (m_cache_store_settings)
    {
      QSettings settings;
      settings.setValue(const_cache_settings_name + "/" + const_cache_settings_maxsize,
                        cacheDatabaseMaximalSize());
    }

  emit cacheDatabaseMaximalSizeChanged(cacheDatabaseMaximalSize());
}

bool QQuickItemMapboxGL::cacheDatabaseDefaultPath() const
{
  return m_cache_default_path;
}

void QQuickItemMapboxGL::setCacheDatabaseDefaultPath(bool s)
{
  bool old = m_cache_default_path;
  m_cache_default_path = s;
  if (m_cache_default_path)
    {
      QDir dir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
      setCacheDatabasePath(dir.absoluteFilePath(const_cache_default_database_name));
    }
  if (old != s) emit cacheDatabaseDefaultPathChanged(s);
}

bool QQuickItemMapboxGL::cacheDatabaseStoreSettings() const
{
  return m_cache_store_settings;
}

void QQuickItemMapboxGL::setCacheDatabaseStoreSettings(bool s)
{
  bool old = m_cache_store_settings;
  m_cache_store_settings = s;
  if (m_cache_store_settings)
    {
      QSettings settings;
      int sz = settings.value(const_cache_settings_name + "/" + const_cache_settings_maxsize,
                              cacheDatabaseMaximalSize()).toInt();
      setCacheDatabaseMaximalSize(sz);
    }
  if (old != s) emit cacheDatabaseStoreSettingsChanged(s);
}

/// Error feedback
QString QQuickItemMapboxGL::errorString() const
{
  return m_errorString;
}

void QQuickItemMapboxGL::setError(QString error)
{
  m_errorString = error;
  std::cerr << error.toStdString() << std::endl;
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

void QQuickItemMapboxGL::fitView(const QVariantList &coordinates)
{
  size_t counter = 0;

  for (int i = 0; i < coordinates.size(); ++i)
    {
      QGeoCoordinate c = coordinates[i].value<QGeoCoordinate>();
      if (c.isValid())
        {
          counter++;

          if (counter==1 || c.latitude() < m_fit_sw.first)
            m_fit_sw.first = c.latitude();
          if (counter==1 || c.longitude() < m_fit_sw.second)
            m_fit_sw.second = c.longitude();

          if (counter==1 || c.latitude() > m_fit_ne.first)
            m_fit_ne.first = c.latitude();
          if (counter==1 || c.longitude() > m_fit_ne.second)
            m_fit_ne.second = c.longitude();
        }
    }

  if (counter == 0) return;

  if (counter > 1) m_syncState |= FitViewNeedsSync;
  else /* counter==1 */ setCenter(QGeoCoordinate(m_fit_ne.first, m_fit_ne.second));
  update();
}

qreal QQuickItemMapboxGL::metersPerPixel() const
{
  return m_metersPerPixel;
}

void QQuickItemMapboxGL::setMetersPerPixelTolerance(qreal tol)
{
  m_metersPerPixelTolerance = tol;
  emit metersPerPixelToleranceChanged(m_metersPerPixelTolerance);
}

qreal QQuickItemMapboxGL::metersPerPixelTolerance() const
{
  return m_metersPerPixelTolerance;
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
  rect.setY(margins.bottom());
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

bool QQuickItemMapboxGL::addImagePath(const QString &name, const QString &path)
{
  QString p;
  QString furl = "file://";
  if (path.startsWith(furl)) p = path.right(path.size() - furl.size());
  else p = path;

  QImage image;
  if (!image.load(p)) return false;
  addImage(name, image);
  return true;
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
  m_paint_properties.add(layer, property, value); DATA_UPDATE;
}

void QQuickItemMapboxGL::setPaintPropertyList(const QString &layer, const QString &property, const QVariantList &value)
{
  m_paint_properties.add(layer, property, value); DATA_UPDATE;
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
  update();
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

/// Cache clearing
void QQuickItemMapboxGL::clearCache()
{
  const QString const_connection_name = "QQuickItemMapboxGL::clearCache::connection";

  { // to remove db as soon as we are done with it
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", const_connection_name);
    db.setDatabaseName( cacheDatabasePath() );
    if (db.open())
      {
        db.exec("PRAGMA foreign_keys = ON");
        db.exec("DELETE FROM region_resources");
        db.exec("DELETE FROM region_tiles");
        db.exec("DELETE FROM regions");
        db.exec("DELETE FROM tiles");
        db.exec("DELETE FROM resources");
        db.exec("VACUUM");
        db.close();
      }
  }

  QSqlDatabase::removeDatabase(const_connection_name);
}

/// Update map
QSGNode* QQuickItemMapboxGL::updatePaintNode(QSGNode *node, UpdatePaintNodeData *)
{
  QSGMapboxGLTextureNode *n = static_cast<QSGMapboxGLTextureNode *>(node);
  QSize sz(width(), height());

  if (!n)
    {
      bool wasFitView = (m_syncState & FitViewNeedsSync);

      n = new QSGMapboxGLTextureNode(m_settings, sz, m_pixelRatio, this);

      m_syncState = CenterNeedsSync | ZoomNeedsSync | BearingNeedsSync | PitchNeedsSync |
          StyleNeedsSync | MarginsNeedSync;
      if (wasFitView) m_syncState |= FitViewNeedsSync;

      m_block_data_until_loaded = true;
      m_finalize_data_loading = false; // set to true only if data is loaded on full style load

      /////////////////////////////////////////////////////
      /// connect all queries

      connect(n, &QSGMapboxGLTextureNode::replySourceExists, this, &QQuickItemMapboxGL::replySourceExists, Qt::QueuedConnection);
      connect(this, &QQuickItemMapboxGL::querySourceExists, n, &QSGMapboxGLTextureNode::querySourceExists, Qt::QueuedConnection);

      connect(n, &QSGMapboxGLTextureNode::replyLayerExists, this, &QQuickItemMapboxGL::replyLayerExists, Qt::QueuedConnection);
      connect(this, &QQuickItemMapboxGL::queryLayerExists, n, &QSGMapboxGLTextureNode::queryLayerExists, Qt::QueuedConnection);

      connect(n, &QSGMapboxGLTextureNode::replyCoordinateForPixel, this, &QQuickItemMapboxGL::replyCoordinateForPixel, Qt::QueuedConnection);
      connect(this, &QQuickItemMapboxGL::queryCoordinateForPixel, n, &QSGMapboxGLTextureNode::queryCoordinateForPixel, Qt::QueuedConnection);

      /////////////////////////////////////////////////////
      /// connect map changed failure signals
      connect(n->map(), &QMapboxGL::mapChanged, this, &QQuickItemMapboxGL::onMapChanged, Qt::QueuedConnection);
      connect(n->map(), &QMapboxGL::mapLoadingFailed, this, &QQuickItemMapboxGL::onMapLoadingFailed, Qt::QueuedConnection);
    }

  if (sz != m_last_size || m_syncState & PixelRatioNeedsSync)
    {
      n->resize(sz, m_pixelRatio);
      m_syncState |= MarginsNeedSync;
      m_last_size = sz;
    }

  // update all settings
  QMapboxGL *map = n->map();

  if (m_syncState & MarginsNeedSync)
    {
      QMargins margins(m_margins.left()*width()/m_pixelRatio, m_margins.top()*height()/m_pixelRatio,
                       m_margins.right()*width()/m_pixelRatio, m_margins.bottom()*height()/m_pixelRatio);
      map->setMargins(margins);
      m_syncState |= CenterNeedsSync; // center has to be updated after update of the margins
    }

  if (m_syncState & FitViewNeedsSync)
    {
      QMapbox::CoordinateZoom cz = map->coordinateZoomForBounds(m_fit_sw, m_fit_ne);
      setCenter(QGeoCoordinate(cz.first.first, cz.first.second));
      setZoomLevel(cz.second);
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
  // its probably not needed here since the condition should be reached
  // earlier in onMapChanged slot. keeping the check here for safety
  if (loaded && (m_block_data_until_loaded || m_finalize_data_loading))
    {
      m_syncState |= DataNeedsSetupSync;
      m_syncState |= DataNeedsSync;
      m_block_data_until_loaded = false;
      m_finalize_data_loading = false;
      update();
    }

  // check the variables that are tracked on the map

  { // metersPerPixel
    const double tol = metersPerPixelTolerance(); // tolerance used when comparing floating point numbers
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

void QQuickItemMapboxGL::onMapChanged(QMapboxGL::MapChange change)
{
  // check if we can add user-added sources, layers ...
  if (QMapboxGL::MapChangeDidFinishLoadingStyle == change && m_block_data_until_loaded)
    {
      m_syncState |= DataNeedsSetupSync;
      m_syncState |= DataNeedsSync;
      m_block_data_until_loaded = false;

      /// Finalize data loading was introduced to ensure the
      /// data reload after full map is loaded. Without it, it was
      /// once observed that the data layer failed to load on the
      /// start of application. Hard to reproduce though
      m_finalize_data_loading = true;
      update();
    }
}

void QQuickItemMapboxGL::onMapLoadingFailed(QMapboxGL::MapLoadingFailure /*type*/, const QString &description)
{
  setError(description);
}

///////////////////////////////////////////////////////////
/// methods related to ResourceTransform of requested URLs
///
/// Since these methods can be called either from main thread
/// or Mapbox GL threads, all access to variables has to be
/// protected by mutex. Exception is getters, since they are accessed
/// from the main thread only as the setters are.

bool QQuickItemMapboxGL::urlDebug() const
{
  return m_urlDebug;
}

void QQuickItemMapboxGL::setUrlDebug(bool debug)
{
  {
    QMutexLocker lk(&m_resourceTransformMutex);
    m_urlDebug = debug;
  }
  emit urlDebugChanged(debug);
}

QString QQuickItemMapboxGL::urlSuffix() const
{
  return QString::fromStdString(m_urlSuffix);
}

void QQuickItemMapboxGL::setUrlSuffix(const QString &urlsfx)
{
  {
    QMutexLocker lk(&m_resourceTransformMutex);
    m_urlSuffix = urlsfx.toStdString();
  }
  emit urlSuffixChanged(urlsfx);
}

std::string QQuickItemMapboxGL::resourceTransform(const std::string &&url)
{
  QMutexLocker lk(&m_resourceTransformMutex);
  std::string newurl = url + m_urlSuffix;
  if (m_urlDebug)
    std::cout << "MapboxGL requested URL: " << newurl << std::endl;
  return newurl;
}
