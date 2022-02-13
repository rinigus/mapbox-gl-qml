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

#ifndef QQUICKITEMMAPBOXGL_H
#define QQUICKITEMMAPBOXGL_H

#include <QQuickItem>
#include <QTimer>
#include <QPointF>
#include <QPoint>
#include <QMarginsF>
#include <QRectF>
#include <QVariantList>
#include <QHash>
#include <QMutex>

#include <QMapboxGL>
#include <QGeoCoordinate>

#include <string>

#include "qmapboxsync_p.h"

///////////////////////////////////////////////////////////////////////////////////
/// \brief The QQuickItemMapboxGL class
///
/// Interface with QMapboxGL for QML. API is documented in api.md available at
/// https://github.com/rinigus/mapbox-gl-qml/blob/master/api.md
///
/// Note that there is a difference in the order of coordinates when using GeoJSON and
/// Qt native representation. Here, the methods accepting coordinates as explicit arguments,
/// such as updateSourcePoint, use the same order as in Qt.

class QQuickItemMapboxGL : public QQuickItem
{
  Q_OBJECT

  Q_PROPERTY(qreal bearing READ bearing WRITE setBearing NOTIFY bearingChanged)
  Q_PROPERTY(QGeoCoordinate center READ center WRITE setCenter NOTIFY centerChanged)
  Q_PROPERTY(qreal pitch READ pitch WRITE setPitch NOTIFY pitchChanged)
  Q_PROPERTY(qreal minimumZoomLevel READ minimumZoomLevel WRITE setMinimumZoomLevel NOTIFY minimumZoomLevelChanged)
  Q_PROPERTY(qreal maximumZoomLevel READ maximumZoomLevel WRITE setMaximumZoomLevel NOTIFY maximumZoomLevelChanged)
  Q_PROPERTY(qreal zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
  Q_PROPERTY(QRectF margins READ margins WRITE setMargins NOTIFY marginsChanged) /// see comments below on interpretation of RectF

  Q_PROPERTY(qreal devicePixelRatio READ devicePixelRatio WRITE setDevicePixelRatio NOTIFY devicePixelRatioChanged)
  Q_PROPERTY(qreal pixelRatio READ pixelRatio WRITE setPixelRatio NOTIFY pixelRatioChanged)
  Q_PROPERTY(qreal mapToQtPixelRatio READ mapToQtPixelRatio NOTIFY mapToQtPixelRatioChanged)
  Q_PROPERTY(QString styleJson READ styleJson WRITE setStyleJson NOTIFY styleJsonChanged)
  Q_PROPERTY(QString styleUrl READ styleUrl WRITE setStyleUrl)
  Q_PROPERTY(QString urlSuffix READ urlSuffix WRITE setUrlSuffix NOTIFY urlSuffixChanged)
  Q_PROPERTY(bool urlDebug READ urlDebug WRITE setUrlDebug NOTIFY urlDebugChanged)
  Q_PROPERTY(bool useFBO READ useFBO WRITE setUseFBO NOTIFY useFBOChanged)

  /// tracks meters per pixel for the map center
  Q_PROPERTY(qreal metersPerPixel READ metersPerPixel NOTIFY metersPerPixelChanged)
  Q_PROPERTY(qreal metersPerMapPixel READ metersPerMapPixel NOTIFY metersPerPixelChanged)
  Q_PROPERTY(qreal metersPerPixelTolerance READ metersPerPixelTolerance WRITE setMetersPerPixelTolerance NOTIFY metersPerPixelToleranceChanged)

  // error
  Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)

  // for internal use. used by map area to notify that the gesture is in progress
  Q_PROPERTY(bool gestureInProgress READ gestureInProgress WRITE setGestureInProgress NOTIFY gestureInProgressChanged)

  // used only on construction of the map object
  Q_PROPERTY(QString accessToken READ accessToken WRITE setAccessToken NOTIFY accessTokenChanged)
  Q_PROPERTY(QString apiBaseUrl READ apiBaseUrl WRITE setApiBaseUrl NOTIFY apiBaseUrlChanged)
  Q_PROPERTY(QString assetPath READ assetPath WRITE setAssetPath NOTIFY assetPathChanged)

  Q_PROPERTY(QString cacheDatabasePath READ cacheDatabasePath WRITE setCacheDatabasePath NOTIFY cacheDatabasePathChanged)
  Q_PROPERTY(int cacheDatabaseMaximalSize READ cacheDatabaseMaximalSize WRITE setCacheDatabaseMaximalSize NOTIFY cacheDatabaseMaximalSizeChanged)
  Q_PROPERTY(bool cacheDatabaseDefaultPath READ cacheDatabaseDefaultPath WRITE setCacheDatabaseDefaultPath NOTIFY cacheDatabaseDefaultPathChanged)
  Q_PROPERTY(bool cacheDatabaseStoreSettings READ cacheDatabaseStoreSettings WRITE setCacheDatabaseStoreSettings NOTIFY cacheDatabaseStoreSettingsChanged)

public:
  QQuickItemMapboxGL(QQuickItem *parent = nullptr);
  ~QQuickItemMapboxGL();

  /// Interaction with QML through properties
  ///
  qreal bearing() const;
  void setBearing(qreal b);

  qreal pitch() const;
  void setPitch(qreal p);

  void setMinimumZoomLevel(qreal minimumZoomLevel);
  qreal minimumZoomLevel() const;

  void setMaximumZoomLevel(qreal maximumZoomLevel);
  qreal maximumZoomLevel() const;

  Q_INVOKABLE void setZoomLevel(qreal zoomLevel, const QPointF &center = QPointF());
  qreal zoomLevel() const;

  QGeoCoordinate center() const;

  qreal metersPerPixel() const;
  qreal metersPerMapPixel() const;
  void  setMetersPerPixelTolerance(qreal tol);
  qreal metersPerPixelTolerance() const;

  QString errorString() const;

  QString styleJson() const;
  void setStyleJson(const QString &json);

  QString styleUrl() const;
  void setStyleUrl(const QString &url);

  void setDevicePixelRatio(qreal devicePixelRatio);
  qreal devicePixelRatio() const;

  void setPixelRatio(qreal pixelRatio);
  qreal pixelRatio() const;

  qreal mapToQtPixelRatio() const;

  /// Settings related
  QString accessToken() const;
  void setAccessToken(const QString &token);

  QString apiBaseUrl() const;
  void setApiBaseUrl(const QString &url);

  QString assetPath() const;
  void setAssetPath(const QString &path);

  QString cacheDatabasePath() const;
  void setCacheDatabasePath(const QString &path);

  int cacheDatabaseMaximalSize() const;
  void setCacheDatabaseMaximalSize(int sz);

  bool cacheDatabaseDefaultPath() const;
  void setCacheDatabaseDefaultPath(bool s);

  bool cacheDatabaseStoreSettings() const;
  void setCacheDatabaseStoreSettings(bool s);

  QString urlSuffix() const;
  void setUrlSuffix(const QString &urlsfx);

  bool urlDebug() const;
  void setUrlDebug(bool debug);

  bool useFBO() const;
  void setUseFBO(bool fbo);

  bool gestureInProgress() const;
  void setGestureInProgress(bool progress);

  /// Callable methods from QML
  ///
  Q_INVOKABLE void pan(int dx, int dy);

  /// \brief Set relative margins that determine position of the center
  ///
  /// Margins are given relative to the widget width (left and right margin) and
  /// height (top and bottom). By default, the margins are 0 for all. In this case,
  /// the given center will be on the center of the widget. To shift the center to the
  /// bottom by 25%, set the top margin to 0.5. Then the center will be found between
  /// the middle line and the bottom. Margins allow to switch between navigation and
  /// browsing mode, for example.
  Q_INVOKABLE void setMargins(qreal left, qreal top, qreal right, qreal bottom);

  /// Margins represented by RectF follow the similar pattern as in the setMargins above.
  /// You specify x and y that correspond to the bottom left offset and the width of the
  /// map inside the margins. See static functions in the implementation for the
  /// details.
  Q_INVOKABLE QRectF margins() const;
  Q_INVOKABLE void setMargins(const QRectF &margins_box);

  /// \brief Fits view to fit all given coordinates
  ///
  /// finds zoom and the center that would allow to fit the given list
  /// of coordinates in the map view. When set to preserve, will keep fitting
  /// unless invoked with preserve=false (even with the empty coordinates list),
  /// or user pans, changes the center, zoom, pitch, bearing.
  Q_INVOKABLE void fitView(const QVariantList &coordinates, bool preserve=false);

  /// \brief Stops active fit to view
  ///
  /// Use to stop fitting to the view which was set by calling fitView with the
  /// argument preserve=true.
  Q_INVOKABLE void stopFitView();

  /// \brief Clear cache
  ///
  /// Clear cache database
  Q_INVOKABLE void clearCache();

  /////////////////////////////////////////////////////////////////////////////
  /// Map interaction methods

  Q_INVOKABLE void addSource(const QString &sourceID, const QVariantMap& params);

  /// \brief Add source consisting of a single point coordinate
  Q_INVOKABLE void addSourcePoint(const QString &sourceID, const QGeoCoordinate &coordinate, const QString &name = QString());
  Q_INVOKABLE void addSourcePoint(const QString &sourceID, qreal latitude, qreal longitude, const QString &name = QString());

  /// in addSourcePoints and addSourceLine, coordinates are given as a list of QGeoCoordinate
  Q_INVOKABLE void addSourcePoints(const QString &sourceID, const QVariantList &coordinates, const QVariantList &names = QVariantList());
  Q_INVOKABLE void addSourceLine(const QString &sourceID, const QVariantList &coordinates, const QString &name = QString());

  Q_INVOKABLE void updateSource(const QString &sourceID, const QVariantMap& params);

  /// \brief Update source consisting of a single point coordinate
  Q_INVOKABLE void updateSourcePoint(const QString &sourceID, const QGeoCoordinate &coordinate, const QString &name = QString());
  Q_INVOKABLE void updateSourcePoint(const QString &sourceID, qreal latitude, qreal longitude, const QString &name = QString());

  /// in updateSourcePoints and updateSourceLine, coordinates are given as a list of QGeoCoordinate
  Q_INVOKABLE void updateSourcePoints(const QString &sourceID, const QVariantList &coordinates, const QVariantList &names = QVariantList());
  Q_INVOKABLE void updateSourceLine(const QString &sourceID, const QVariantList &coordinates, const QString &name = QString());

  Q_INVOKABLE void removeSource(const QString &sourceID);

  Q_INVOKABLE void addLayer(const QString &id, const QVariantMap &params, const QString& before = QString());
  Q_INVOKABLE void removeLayer(const QString &id);

  Q_INVOKABLE void addImage(const QString &name, const QImage &sprite);
  Q_INVOKABLE bool addImagePath(const QString &name, const QString &path, const int svgX=0, const int svgY=0); ///< Add image using image file path or url starting with file://
  Q_INVOKABLE void removeImage(const QString &name);

  Q_INVOKABLE void setLayoutProperty(const QString &layer, const QString &property, const QVariant &value);
  Q_INVOKABLE void setLayoutPropertyList(const QString &layer, const QString &property, const QVariantList &value);
  Q_INVOKABLE void setPaintProperty(const QString &layer, const QString &property, const QVariant &value);
  Q_INVOKABLE void setPaintPropertyList(const QString &layer, const QString &property, const QVariantList &value);

  /// Track locations

  Q_INVOKABLE void trackLocation(const QString &id, const QGeoCoordinate &location);
  Q_INVOKABLE void removeLocationTracking(const QString &id);
  Q_INVOKABLE void removeAllLocationTracking();

  /// \brief List of default Mapbox styles returned as JSON array
  ///
  Q_INVOKABLE QVariantList defaultStyles() const;

signals:
  void startRefreshTimer();
  void stopRefreshTimer();

  // Map QML Type signals.
  void bearingChanged(qreal bearing);
  void pitchChanged(qreal pitch);
  void minimumZoomLevelChanged();
  void maximumZoomLevelChanged();
  void zoomLevelChanged(qreal zoomLevel);
  void centerChanged(const QGeoCoordinate &coordinate);
  void marginsChanged(const QMarginsF &margins);

  void devicePixelRatioChanged(qreal devicePixelRatio);
  void pixelRatioChanged(qreal pixelRatio);
  void styleJsonChanged(QString json);
  void styleUrlChanged(QString url);
  void urlSuffixChanged(QString urlSuffix);
  void urlDebugChanged(bool urlDebug);
  void useFBOChanged(bool useFBO);

  void errorChanged(QString error);

  void gestureInProgressChanged(bool gestureInProgress);

  void accessTokenChanged(QString token);
  void apiBaseUrlChanged(QString url);
  void assetPathChanged(QString path);

  void cacheDatabasePathChanged(QString path);
  void cacheDatabaseMaximalSizeChanged(int size);
  void cacheDatabaseAppNameChanged(QString name);
  void cacheDatabaseDefaultPathChanged(bool defaultpath);
  void cacheDatabaseStoreSettingsChanged(bool storesettings);

  void locationChanged(QString id, bool visible, const QPoint pixel);
  void locationTrackingRemoved(QString id);

  ////////////////////////////////////////////////////////
  /// Queries
  ///
  /// Queries are supported AFTER construction of the map.
  /// As a practical limitation, queries filed during QML
  /// Component.onCompleted will be not responded to. This
  /// is due to the implementation limitations.
  ///
  /// To use queries, ask it via queryXXXX methods (implemented
  /// as signals) and get the response via replyXXXX signals.
  ///
  void querySourceExists(const QString id);
  void replySourceExists(const QString id, bool exists);

  void queryLayerExists(const QString id);
  void replyLayerExists(const QString id, bool exists);

  void queryCoordinateForPixel(const QPointF p, const QVariant &tag = QVariant());
  void replyCoordinateForPixel(const QPointF pixel, QGeoCoordinate geocoordinate, qreal degLatPerPixel, qreal degLonPerPixel, const QVariant &tag);

  /////////////////////////////////////////////////////////
  /// Tracking the state of the map
  void metersPerPixelChanged(qreal metersPerPixel);
  void metersPerPixelToleranceChanged(qreal metersPerPixelTolerance);
  void mapToQtPixelRatioChanged(qreal mapToQtPixelRatio);

public slots:
  void setCenter(const QGeoCoordinate &center);

protected:
  QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *) override;

private:

  void onMapChanged(QMapboxGL::MapChange change); ///< Follow the state of the map
  void onMapLoadingFailed(QMapboxGL::MapLoadingFailure type, const QString &description);

  std::string resourceTransform(const std::string &url); ///< Use resource transform API to change requested URL

  void setError(QString error); ///< Set error string, used internally

private:

  /// \brief Private class to track locations
  class LocationTracker {
  public:
    LocationTracker() {}
    LocationTracker(const QGeoCoordinate &location);

    bool set_position(const QPoint &p, const QSize &sz);

    const QGeoCoordinate& coordinate() const { return m_location; }
    bool visible() const { return m_last_visible; }
    const QPoint& position() const { return m_last_position; }

  protected:
    QGeoCoordinate m_location;
    bool m_last_visible;
    QPoint m_last_position;
  };

private:
  QMapboxGLSettings m_settings;

  /// Signals that the first full init is done and setup-related
  /// properties cannot be changed
  bool m_first_init_done{false};

  bool m_cache_default_path{false};
  bool m_cache_store_settings{false};

  QSize m_last_size; ///< Size of the item
  QTimer m_timer;    ///< Timer used to refresh the map

  qreal m_minimumZoomLevel = 0;
  qreal m_maximumZoomLevel = 20;
  qreal m_zoomLevel = 20;
  QPointF m_zoomLevelPoint;

  QPointF m_pan;

  QGeoCoordinate m_center;
  double m_metersPerPixel = -1;
  double m_metersPerMapPixel = -1;
  double m_metersPerPixelTolerance = 1e-6;

  qreal m_bearing = 0;
  qreal m_pitch = 0;
  QMarginsF m_margins;  

  QMapbox::Coordinate m_fit_sw;
  QMapbox::Coordinate m_fit_ne;
  QGeoCoordinate m_fit_center;
  qreal m_fit_zoomLevel = -1;
  bool m_fit_preserve_box = false;
  bool m_fit_preserve_center = false;

  QString m_errorString;

  qreal m_devicePixelRatio = 1;
  qreal m_pixelRatio = 1;
  qreal m_mapToQtPixelRatio = 1;
  QString m_styleUrl;
  QString m_styleJson;
  bool    m_useUrlForStyle = true;
  bool    m_useFBO = true;

  QMutex m_resourceTransformMutex;
  std::string m_urlSuffix;
  bool m_urlDebug{false};

  QHash<QString, LocationTracker> m_location_tracker;

  bool m_gestureInProgress = false;

  bool m_block_data_until_loaded{true}; ///< Blocks loading of additional data until base map is loaded
  bool m_finalize_data_loading{true}; ///< Used to load additional data when the base map is fully loaded
  QMapboxSync::SourceList m_sources;
  QMapboxSync::LayerList m_layers;
  QMapboxSync::LayoutPropertyList m_layout_properties;
  QMapboxSync::PaintPropertyList m_paint_properties;
  QMapboxSync::ImageList m_images;

  enum SyncState {
    NothingNeedsSync = 0,
    ZoomNeedsSync    = 1 << 0,
    CenterNeedsSync  = 1 << 1,
    StyleNeedsSync   = 1 << 2,
    PanNeedsSync     = 1 << 3,
    BearingNeedsSync = 1 << 4,
    PitchNeedsSync   = 1 << 5,
    PixelRatioNeedsSync = 1 << 6,
    MarginsNeedSync = 1 << 7,
    DataNeedsSync = 1 << 8,
    DataNeedsSetupSync = 1 << 9,
    FitViewNeedsSync = 1 << 10,
    FitViewCenterNeedsSync = 1 << 11,
    GestureInProgressNeedsSync = 1 << 12
  };
  int m_syncState = NothingNeedsSync;

  const QString const_cache_settings_name{"MapboxGL-QML"};
  const QString const_cache_settings_maxsize{"maximal_size"};
  const QString const_cache_default_database_name{"mapboxgl-qml-cache.db"};
};

#endif // QQUICKITEMMAPBOXGL_H
