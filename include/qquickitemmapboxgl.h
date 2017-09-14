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

#include <QMapboxGL>
#include <QGeoCoordinate>

class QQuickItemMapboxGL : public QQuickItem
{
  Q_OBJECT

  Q_PROPERTY(qreal minimumZoomLevel READ minimumZoomLevel WRITE setMinimumZoomLevel NOTIFY minimumZoomLevelChanged)
  Q_PROPERTY(qreal maximumZoomLevel READ maximumZoomLevel WRITE setMaximumZoomLevel NOTIFY maximumZoomLevelChanged)
  Q_PROPERTY(qreal zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
  Q_PROPERTY(QGeoCoordinate center READ center WRITE setCenter NOTIFY centerChanged)
  Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)
  Q_PROPERTY(qreal pixelRatio READ pixelRatio WRITE setPixelRatio NOTIFY pixelRatioChanged)

public:
  QQuickItemMapboxGL(QQuickItem *parent = nullptr);
  ~QQuickItemMapboxGL();

  /// Interaction with QML through properties
  ///
  void setMinimumZoomLevel(qreal minimumZoomLevel);
  qreal minimumZoomLevel() const;

  void setMaximumZoomLevel(qreal maximumZoomLevel);
  qreal maximumZoomLevel() const;

  void setZoomLevel(qreal zoomLevel);
  qreal zoomLevel() const;

  QGeoCoordinate center() const;

  QString errorString() const;

  void setPixelRatio(qreal pixelRatio);
  qreal pixelRatio() const;

  /// Callable methods from QML
  ///
  Q_INVOKABLE void pan(int dx, int dy);

signals:
  void startRefreshTimer();
  void stopRefreshTimer();

  // Map QML Type signals.
  void minimumZoomLevelChanged();
  void maximumZoomLevelChanged();
  void zoomLevelChanged(qreal zoomLevel);
  void centerChanged(const QGeoCoordinate &coordinate);
  void errorChanged();
  void pixelRatioChanged(qreal pixelRatio);

public slots:
  void setCenter(const QGeoCoordinate &center);

protected:
  QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *) override;


private:
  QMapboxGLSettings m_settings;

  QSize m_last_size; ///< Size of the item
  QTimer m_timer;    ///< Timer used to refresh the map

  qreal m_minimumZoomLevel = 0;
  qreal m_maximumZoomLevel = 20;
  qreal m_zoomLevel = 20;

  QPointF m_pan;

  QGeoCoordinate m_center;
  QString m_styleUrl;

  QString m_errorString;

  qreal m_pixelRatio;

  qreal m_bearing = 0;
  qreal m_pitch = 0;

  enum SyncState {
    NothingNeedsSync = 0,
    ZoomNeedsSync    = 1 << 0,
    CenterNeedsSync  = 1 << 1,
    StyleNeedsSync   = 1 << 2,
    PanNeedsSync     = 1 << 3,
    BearingNeedsSync = 1 << 4,
    PitchNeedsSync   = 1 << 5,
    PixelRatioNeedsSync = 1 << 6
  };
  int m_syncState = NothingNeedsSync;
};

#endif // QQUICKITEMMAPBOXGL_H
