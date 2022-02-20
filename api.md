# Unofficial Mapbox GL Native bindings for Qt QML: API

This is a description of an API that was written for platforms lacking
QtLocation 5.9 support. I would like to acknowledge the use of QtLocation 5.9
Mapbox GL plugin documentation while this API documentation.

API consists of two items:
[MapboxMap](#qquickitemmapboxgl-c--mapboxmap-qml) for displaying the
map and [MapboxMapGestureArea](#mapboxmapgesturearea) that can be used
within MapboxMap for interacting with it.

## Table of Contents

   * [QQuickItemMapboxGL (C++) / MapboxMap (QML)](#qquickitemmapboxgl-c--mapboxmap-qml)
      * [Include statements](#include-statements)
      * [General comments](#general-comments)
      * [Properties](#properties)
	 * [Map settings on initialization](#map-settings-on-initialization)
	 * [Map rendering](#map-rendering)
	 * [Mangling of URLs](#mangling-of-urls)
	 * [Other properties](#other-properties)
      * [Queries and Signals](#queries-and-signals)
      * [Methods](#methods)
	 * [General methods](#general-methods)
	 * [Map sources](#map-sources)
	 * [Map layers](#map-layers)
	 * [Map layout and paint properties](#map-layout-and-paint-properties)
	 * [Tracking locations on the map](#tracking-locations-on-the-map)
   * [MapboxMapGestureArea](#mapboxmapgesturearea)
      * [Signals](#signals)
      * [Properties](#properties-1)


# QQuickItemMapboxGL (C++) / MapboxMap (QML)

QML Quick Item for displaying maps using Mapbox GL. See
`qquickitemmapboxgl.h` for specific syntax, if needed.

In QML, `MapboxMap` type is defined by this plugin. In C++, use
`QQuickItemMapboxGL` class.

## Include statements

In QML

```
import MapboxMap 1.0
```

In C++

```
#include <qquickitemmapboxgl.h>
```

## General comments

This Mapbox GL interface is written to keep map properties, sources,
layers, and properties persistent to the style changes. In particular,
all sources, layers, paint and layer properties that were introduced
via `add*` or `update*` methods will be re-added to the map when
changing styles until they are removed using `remove*` methods.

When using GeoJSON sources, note that there is a difference in the
order of coordinates when using GeoJSON and Qt native
representation. Here, the methods accepting coordinates as explicit
arguments, such as `updateSourcePoint`, use the same order as in Qt.

As an example of the use of this API, see QML file
[app/qml/main.qml](https://github.com/rinigus/mapbox-gl-qml/blob/master/app/qml/main.qml)
in this repository.

## Properties

Map properties are classified and listed in the following
sub-sections. Each property is given with the corresponding type in
front of it.

### Map settings on initialization

The properties in this subsection should be set on initialization of
the map. If modified later, the properties may not be applied until
the map's frame buffer object is recreated. Some of them (such as
cache-related properties) would be used as they were specified for the
first created map.

* `string `**`accessToken`** When hosted vector tiles and styles require an API
    [access token](https://www.mapbox.com/help/define-access-token/), you can specify
    them using `accessToken`. For example , for Mapbox you can obtain it from the
    [Mapbox account page](https://www.mapbox.com/studio/account/tokens/). Access
    tokens associate requests to Mapbox's vector tile and style APIs
    with your Mapbox account. They also deter other developers from
    using your styles without your permission.

* `string `**`apiBaseUrl`** The API base URL is the URL that the `mapbox://`
    protocol will be resolved to. It defaults to
    "https://api.mapbox.com" but can be changed, for instance, to a
    tile cache server address.

* `string `**`assetPath`** Returns the asset path, which is the root directory from where
    the `asset://` scheme gets resolved in a style. `asset://` can be used
    for loading a resource from the disk in a style rather than fetching
    it from the network.

    By default, it is set to the value returned by
    `QCoreApplication::applicationDirPath()`.

* `bool `**`cacheDatabaseDefaultPath`** When set to `true`, the path
    of the cache database is named `mapboxgl-qml-cache.db` and is
    stored at
    [QStandardPaths::CacheLocation](http://doc.qt.io/qt-5/qstandardpaths.html#StandardLocation-enum). Set
    to `false` by default.

    See also general comment regarding cache settings given in the
    description of `cacheDatabasePath`.

* `int `**`cacheDatabaseMaximalSize`** Returns the cache database maximum
    hard size in bytes. The database will grow until the limit is
    reached. Setting a maximum size smaller than the current size of
    an existing database results in undefined behavior

    By default, it is set to 50 MB.

    See also general comment regarding cache settings given in the
    description of `cacheDatabasePath`.

* `string `**`cacheDatabasePath`** Returns the cache database path. The cache is used for storing
    recently used resources like tiles and also an offline tile database
    pre-populated by the [Offline Tool](https://github.com/mapbox/mapbox-gl-native/blob/master/bin/offline.sh).

    By default, it is set to `:memory:` meaning it will create an in-memory
    cache instead of a file on disk.

    Cache-related settings are shared between all QMapboxGL instances because different
    maps will share the same cache database file. The first map to configure cache properties
    such as size and path will force the configuration to all newly instantiated QMapboxGL
    objects.

* `bool `**`cacheDatabaseStoreSettings`** When set to `true`, cache
    database settings will be preserved between runs and, on
    initialization, loaded from configuration file using
    [QSettings](http://doc.qt.io/qt-5/qsettings.html). At present,
    this concerns only cache maximal size.

* `qreal`**`devicePixelRatio`** Device pixel ratio that is used during
    construction of the map widget. This should correspond to the true
    physical to logical pixel ratio as given in Qt via
    devicePixelRatio method of QScreen. This property allows sharp
    rendering of the widget in HiDPI cases if the rendering through
    framebuffer is used (see `useFBO`). For direct rendering case
    (`useFBO=false`), `devicePixelRatio` is not used.

    If `devicePixelRatio` is not specified, it is determined on the
    basis of the parent QQuickItem, or if absent, on the basis of the
    primary screen.

    See also `pixelRatio` that can be changed after the widget is
    constructed.

* `bool `**`useFBO`** When set to `true` (default), Mapbox GL will use
    a framebuffer object to render a map. When set to `false`, the map
    is rendered issuing OpenGL commands directly, through a
    QSGRenderNode. This is supported only for platforms starting with
    Qt version 5.8.


### Map rendering

* `real `**`bearing`** The map bearing angle in degrees. Negative values and
    values over 360 are valid and will wrap. The direction of the
    rotation is counterclockwise.

    When `center` is defined, the map will rotate around the center
    pixel coordinate respecting the margins if defined.

* `QGeoCoordinate `**`center`** Coordinates of the map center. Centers the
    map at a geographic coordinate respecting the margins, if set.

    In QML, use `QtPositioning.coordinate(latitude, longitude)`
    construct (see http://doc.qt.io/qt-5/qml-coordinate.html and
    http://doc.qt.io/qt-5/qgeocoordinate.html for details).

* `real `**`mapToQtPixelRatio`** Ratio between logical map and Qt
    pixel sizes. When wishing to add elements on a map which should
    have a fixed size in relation to other Qt UI elements, use this
    property to when specifying sizes in map styles. For example, when
    adding icons to the map.

* `rect `**`margins`** Relative margins that determine position of the
    center on the map. Margins specify the relative active area of the
    widget and the position of the active area. When given by a
    rectangle (_x_, _y_, _width_, _height_), the margins are specified
    by the active area with the left bottom given by _x_ and _y_ as
    well as the size of the active area as specified by _width_ and
    _height_.

    For example, `margins` given with the rectangle (0.1,
    0.2, 0.8, 0.3) would correspond to the active area that is shifted
    `0.1*width` from the left, `0.2*height` up from the bottom, is
    `0.8*width` wide leaving `0.1*width` pixels to the right margin,
    and is `0.3*height` high leaving the top half of the widget as a
    margin. Through use of the margins one can shift the center within
    the widget and ensure that it is not covered by other GUI elements
    drawn on the top of the map, for example.

* `real `**`maximumZoomLevel`** Maximum zoom level allowed for the map.

* `real `**`minimumZoomLevel`** Minimum zoom level allowed for the map.

* `real `**`pitch`** Pitch toward the horizon measured in degrees, with 0
    resulting in a two-dimensional map. It is constrained at 60
    degrees.

* `real `**`pixelRatio`** Relative pixel density of the screen when
    compared to 96dpi. All the map elements will be scaled by this
    ratio when drawn. This allows to use the same style on the screens
    with the different pixel densities and adjust the scale on
    fly. Note that the minimal `pixelRatio` is set to
    `devicePixelRatio` used at the map widget construction.

    It is recommended to use this property for adjusting map overall
    zoom for HiDPI cases. However, do not assume relationships between
    Qt and map logical pixel sizes as it will depend on selected
    implementation of the rendering and may change in future. Instead,
    for adding custom elements through map styles, use the properties
    made for such conversion: `mapToQtPixelRatio`, `metersPerPixel`,
    `metersPerMapPixel`.

* `string `**`styleJson`** The map style in JSON given as a
    string. Sets a new style from a JSON that must conform to the
    [Mapbox style
    specification](https://www.mapbox.com/mapbox-gl-style-spec/).  If
    the style is loaded via URL, one can track changes in `styleJson`
    and adjust it via setting to a new value for runtime styling in
    QML. For example, replacing all `{name_en}` to some other
    [supported language
    indicator](https://docs.mapbox.com/help/troubleshooting/change-language/)
    would allow to change the language of the map.

* `string `**`styleUrl`** The map style URL. Sets a URL for fetching a JSON
    that will be later fed to `setStyleJson`. URLs using the Mapbox
    scheme (`mapbox://`) are also accepted and translated
    automatically to an actual HTTPS request.

    The Mapbox scheme is not enforced and a style can be fetched
    from anything that QNetworkAccessManager can handle.

    Note that all given URLs can be altered before fetching from
    internet, as described below under `Mangling of URLs`.

* `real `**`zoomLevel`** The map zoom factor.  This property is used to zoom
    the map.


### Mangling of URLs

Before fetching resources from internet, URLs can be either printed
for debugging purposes or changed by adding them given suffix.

* `bool `**`urlDebug`** When set to `true`, all URLs are printed out in
   `stdout` before fetching them.

* `string `**`urlSuffix`** When set, this string will be appended to all
  URLs before fetching them online. For example, you can append your
  server's specific api key by setting `urlSuffix` to `?apikey=ABCD`
  if it has to be specified in such format at the end of each
  requested URL.


### Other properties

* `string `**`errorString`** Current error string. Please note that this
  property is not covering all possible errors in the API. When set,
  it is never cleared. Thus, please connect to the signal
  `errorChanged` to trigger error processing and don't rely on
  clearance of errorString property.

* `real `**`metersPerPixel`** Meters per Qt logical pixel at the
  center of the map.

* `real `**`metersPerMapPixel`** Meters per map logical pixel at the
  center of the map. Use this property when drawing objects on the map
  that are specified in pixels, such as map style, and can be then
  converted to meters. For example, when drawing location precision
  with a circle by giving its radius in map pixels.

* `real `**`metersPerPixelTolerance`** Tolerance with which
  `metersPerPixel` is updated.


## Queries and Signals

Several properties of the map can be queried via signals. This
relatively complicated way of querying the properties is induced to
ensure thread-safety of the queries.

In general, the queries consist of a method that is called to initiate
it and a signal that is emitted by the map object with the response to
the query. Query can carry an _id_ or a _tag_ that can be used to
filter only the responses that are of interest.

* `void `**`querySourceExists`**`(const QString id)`

  `signal `**`replySourceExists`**`(const QString id, bool exists)`

  Query whether the source with given _id_ exists.

* `void `**`queryLayerExists`**`(const QString id)`

  `signal `**`replyLayerExists`**`(const QString id, bool exists)`

  Query whether the layer with given _id_ exists.

* `void `**`queryCoordinateForPixel`**`(const QPointF p, const QVariant &tag = QVariant())`

  `signal `**`replyCoordinateForPixel`**`(const QPointF pixel, QGeoCoordinate geocoordinate, qreal degLatPerPixel, qreal degLonPerPixel, const QVariant &tag)`

  Query geographical location _geocoordinate_ for a position in the
  widget given as a pixel coordinates _p_. For bookkeeping purposes,
  _tag_ of any type supported by `QVariant` can be added. In addition
  to geocoordinates, an approximated sensitivity of geocoordinate
  changes to pixel coordinates are given by _degLatPerPixel_ and
  _degLonPerPixel_. _degLatPerPixel_ and _degLonPerPixel_ can be used
  to process mouse clicks by the user to decide whether the user
  pressed a certain location within few pixels from it or not.

  For example, the following code will compare the geographical
  location returned by the signal to the current position and run a
  specific method if it was deemed to be nearby:

  ```javascript
	// 15 pixels at 96dpi would correspond to 4 mm
	var nearby_lat = 15 * degLatPerPixel;
	var nearby_lon = 15 * degLonPerPixel;

	// check if its current position
	if ( Math.abs(coordinate.longitude - map.position.coordinate.longitude) < nearby_lon &&
		Math.abs(coordinate.latitude - map.position.coordinate.latitude) < nearby_lat ) {
	    positionMarker.mouseClick();
	    return;
	}
   ```


## Methods

Map methods are classified and listed in the following sub-sections.

### General methods

* `void `**`clearCache`**`()`

  Deletes all data from the current cache database. In addition to
  performing `DELETE` statements, `VACUUM` is performed to recover the
  storage.

* `QVariantList `**`defaultStyles`**`() const`

  List of default Mapbox styles returned as a JSON array

* `void `**`fitView`**`(const QVariantList &coordinates, bool preserve = false)`

  Finds zoom and the center that would allow to fit the given list of
  coordinates in the map view taking into account current `margins`,
  `pitch`, and `bearing`. Note that the expected list elements are
  coordinates as given by `QGeoCoordinate` (`QtPositioning.coordinate`
  in QML). For example, to fit Helsinki and Tallinn on a map, use
  ```javascript
     map.fitView([QtPositioning.coordinate(60.170448, 24.942046),
		  QtPositioning.coordinate(59.436962, 24.753574)])
  ```

  Optionally, it can fit the given list if `preserve` is set to `true`. This
  will compensate changes in margins or map size to ensure that the given
  coordinates are visible. Such automatic fit can be stopped either by user
  interaction (pan, change of center, zoom, bearing, or pitch) or by calling
  `stopFitView`.

  When only one coordinate is given in `coordinates`, the given
  coordinate will be centered if it falls out of the current
  margins. As with the list, `preserve` can be used to make it
  automatic until disabled.


* `void `**`pan`**`(int dx, int dy)`

  Pan the map by _dx_, _dy_ pixels.

* `void `**`setMargins`**`(qreal left, qreal top, qreal right, qreal bottom)`

  Margins are given relative to the widget width (left and right
  margin) and height (top and bottom). By default, the margins are 0
  for all. In this case, the given center will be on the center of the
  widget. To shift the center to the bottom by 25%, set the top margin
  to 0.5. Then the center will be found between the middle line and
  the bottom. Margins allow to switch between navigation and browsing
  mode, for example.

  See also property `margins` for description on how to set margins
  using active area described by rectangle.

* `void `**`setZoomLevel`**`(qreal zoomLevel, const QPointF &center = QPointF())`

  Method to set `zoomLevel` that can be used with the defined _center_
  (pixel coordinates on the widget) to keep the given `center`
  position unchanged while zooming the map. This function could be
  used for implementing a pinch gesture or zooming by using the mouse
  scroll wheel.

* `void `**`stopFitView`**`()`

  Stops automatic fit to view. See `fitView` and its argument
  `preserve` for description.


### Map sources

Map sources can be added, updated, and removed. Note that, since in
Mapbox GL update would automatically add source when such source is
absent, addition and update in these bindings use internally update
methods of QMapboxGL and can be considered synonyms withing this API.

* `void `**`addSource`**`(const QString &sourceID, const QVariantMap&
  params)`

  Adds a source _id_ to the map as specified by the
    [Mapbox style specification](https://www.mapbox.com/mapbox-gl-js/style-spec/#sources)
    with _params_. For example, GeoJSON type can be used to specify
    road, position on the map, or many other features that can added
    and later modified by application.

    The following QML snippet adds a point on the map
    ```javascript
		map.addSource("location",
			  {"type": "geojson",
			      "data": {
				  "type": "Feature",
				  "properties": { "name": "location" },
				  "geometry": {
				      "type": "Point",
				      "coordinates": [
					  (24.94),
					  (60.16)
				      ]
				  }
			      }
			  })
    ```

    and the following QML snippet adds a small line source

    ```
	    var routeSource = {
		"type": "geojson",
		"data": '{
		    "type": "Feature",
		    "properties": {},
		    "geometry": {
			"type": "LineString",
			"coordinates": [
			    [24.942046, 60.170448],
			    [
				24.934420000000003,
				60.163500000000006
			    ],
			    [
				24.923490008921945,
				60.16159500239787
			    ],
			    [
				24.916150000000002,
				60.171530000000004
			    ],
			    [
				24.931620000000002,
				60.18218
			    ],
			    [
				24.961660000000002,
				60.17557000000001
			    ],
			    [
				24.954860000000004,
				60.158930000000005
			    ],
			    [
				24.943690000000004,
				60.155280000000005
			    ]
			]
		    }
		}'
	    }

	    map.addSource("route", routeSource)

    ```

    Notice that the `data` component for GeoJSON can be given either
    as a string or a JSON object.

* `void `**`addSourceLine`**`(const QString &sourceID, const QVariantList &coordinates, const QString &name = QString())`

  Constructs and adds line source to the map. Source description is
  constructed in GeoJSON format with the given _sourceID_ as a
  `LineString`, _coordinates_ given as a list of `QGeoCoordinate`
  objects, and, if specified, property _name_.

* `void `**`addSourcePoint`**`(const QString &sourceID, const QGeoCoordinate &coordinate, const QString &name = QString())`

  `void `**`addSourcePoint`**`(const QString &sourceID, qreal latitude, qreal longitude, const QString &name = QString())`

  Constructs and adds source to the map consisting of a single
  point. Source description is constructed in GeoJSON format with the
  given _sourceID_, coordinates, and, if specified, property _name_.

* `void `**`addSourcePoints`**`(const QString &sourceID, const QVariantList &coordinates, const QVariantList &names = QVariantList())`

  Constructs and adds source to the map consisting of a list of
  points. Source description is constructed in GeoJSON format with the
  given _sourceID_ as a `FeatureCollection`, _coordinates_ given as a
  list of `QGeoCoordinate` objects, and, if specified, _names_ for
  each of the points as a list of strings encoded in GeoJSON as a
  properties for each of the points. For example, this method together
  with the corresponding layer could be used to add POIs to the map.

* `void `**`updateSource`**`(const QString &sourceID, const QVariantMap& params)`

  Update source given by _sourceID_. If absent, the corresponding
  source will be added. See `addSource` for the description of the
  arguments.

* `void `**`updateSourceLine`**`(const QString &sourceID, const QVariantList &coordinates, const QString &name = QString())`

  Update source given by _sourceID_. If absent, the corresponding
  source will be added. See `addSourceLine` for the description of
  the arguments.

* `void `**`updateSourcePoint`**`(const QString &sourceID, const QGeoCoordinate &coordinate, const QString &name = QString())

  `void `**`updateSourcePoint`**(const QString &sourceID, qreal latitude, qreal longitude, const QString &name = QString())`

  Update source given by _sourceID_. If absent, the corresponding
  source will be added. See `addSourcePoint` for the description of
  the arguments.

* `void `**`updateSourcePoints`**`(const QString &sourceID, const QVariantList &coordinates, const QVariantList &names = QVariantList())`

  Update source given by _sourceID_. If absent, the corresponding
  source will be added. See `addSourcePoints` for the description of
  the arguments.

* `void `**`removeSource`**`(const QString &sourceID)`

  Remove the source with _sourceID_ from the map. This method has no
  effect if the source does not exist.


### Map layers

Map layers can be added or removed.

* `void `**`addLayer`**`(const QString &id, const QVariantMap &params, const QString& before = QString())`

  Adds a style layer with _id_ to the map as specified by the
  [Mapbox style specification](https://www.mapbox.com/mapbox-gl-js/style-spec/#layers)
  with _params_. The layer will be added under the layer specified
  by _before_, if specified.  Otherwise it will be added as the
  topmost layer.

  If the layer with such _id_ already exists, the old layer will be
  removed and it will be replaced with the layer specified here.

* `void `**`removeLayer`**`(const QString &id)`

  Removes the layer with given _id_.

* `void `**`addImage`**`(const QString &name, const QImage &sprite)`

  Add image with _name_ given by QImage _sprite_. After addition of the image,
  it can be referred to by its name in the layer or paint properties.

* `bool `**`addImagePath`**`(const QString &name, const QString &path, int svgX=-1, int svgY=-1)`

  Add image using image file path or url starting with `file://`. For
  example, this can be used to specify the image in QML as in
  ```javascript
  map.addImagePath("position-icon", Qt.resolvedUrl("icons/position"))
  ```

  The method can load and render SVGs (determined by file extension
  "svg") using the size provided via `svgX` and `svgY`. If only `svgX`
  is given, it is assumed that `svgY=svgX`. If none of the sizes are
  given and file is assumed to be SVG, it is rendered to 32x32 pixels.

  The method would return `true` if image was found and loaded by Qt.

* `void `**`removeImage`**`(const QString &name)`

  Remove image specified by _name_.


### Map layout and paint properties

* `void `**`setLayoutProperty`**`(const QString &layer, const QString &property, const QVariant &value)`

  `void `**`setLayoutPropertyList`**`(const QString &layer, const QString &property, const QVariantList &value)`

  Sets a layout _property_ _value_ to an existing _layer_. The
    _property_ string can be any as defined by the
    [Mapbox style specification](https://www.mapbox.com/mapbox-gl-style-spec/)
    for layout properties. Use `setLayoutPropertyList` when the
    property value is given by the list, such as `text-offset`, for
    example.

* `void `**`setPaintProperty`**`(const QString &layer, const QString &property, const QVariant &value)`

  `void `**`setPaintPropertyList`**`(const QString &layer, const QString &property, const QVariantList &value)`

  Sets a paint _property_ _value_ to an existing _layer_. The
    _property_ string can be any as defined by the
    [Mapbox style specification](https://www.mapbox.com/mapbox-gl-style-spec/)
    for layout properties. Use `setPaintPropertyList` when the
    property value is given by the list, such as `text-translate`, for
    example.


### Tracking locations on the map

To allow tracking position of certain locations on the widget, a set
of methods and signals are available. This allows to draw interactive
QML items on the top of the certain geographical location shown on the
map and change the location of QML items on the screen and their
visibility in response to the map movement, rotation, and pitch
changes.

* `void `**`trackLocation`**`(const QString &id, const QGeoCoordinate &coordinates)`

  Register new location given by its _coordinates_ using _id_. If
  there was a location registered for tracking with the same _id_, its
  coordinates will be replaced by the _coordinates_ given here.

  On the first update and the change of the position on screen, signal
  `locationChanged` is emitted.

* `void `**`removeLocationTracking`**`(const QString &id)`

  Remove location _id_ from tracking. On removal, signal
  `locationTrackingRemoved` is emitted.

* `void `**`removeAllLocationTracking`**`()`

  Remove tracking for all locations. No signals are emitted in this
  case.

* `signal `**`locationChanged`**`(QString id, bool visible, const QPoint pixel)`

  On the change of location position in the widget or its visibility,
  `locationChanged` signal is emitted specifying _id_ of the tracked
  location, its visibility given by _visible_, and location in the
  widget given by _pixel_.

* `signal `**`locationTrackingRemoved`**`(QString id)`

  When location tracking is removed through `removeLocationTracking`,
  this signal is emitted specifying the removed location _id_.


# MapboxMapGestureArea

MapboxMapGestureArea is intended to be used within MapboxMap for mouse
and touch interaction. On construction, MapboxMapGestureArea property
_map_ should be given MapboxMap _id_ for interaction with the map.

Example of MapboxMapGestureArea use is shown below

```javascript
    MapboxMap {
	id: map
	anchors.fill: parent

	MapboxMapGestureArea {
	    id: mouseArea
	    map: map
	    activeClickedGeo: true
	    activeDoubleClickedGeo: true
	    activePressAndHoldGeo: true

	    onClicked: console.log("Click: " + mouse)
	    onDoubleClicked: console.log("Double click: " + mouse)
	    onPressAndHold: console.log("Press and hold: " + mouse)

	    onClickedGeo: console.log("Click geo: " + geocoordinate + " sensitivity: " + degLatPerPixel + " " + degLonPerPixel)
	    onDoubleClickedGeo: console.log("Double click geo: " + geocoordinate + " sensitivity: " + degLatPerPixel + " " + degLonPerPixel)
	    onPressAndHoldGeo: console.log("Press and hold geo: " + geocoordinate + " sensitivity: " + degLatPerPixel + " " + degLonPerPixel)
	}
    }
```

## Signals

Emitted signals follow the signals emitted by MouseArea. In addition
to regular MouseArea signals forwarded by MapboxMapGestureArea, there
are similar signals that forward geographical coordinates at which
user performed a click, double click, or pressed and hold the
mouse. To enable signals with geographical coordinates, a
corresponding property has to be set to true (see description of
properties below). Note that finding geographical coordinates
corresponding to the event requires some additional processing by the
map object and, if not used, is recommended to be left disabled.

For regular MouseArea signals, see Qt documentation.

* **`clicked`**`(MouseEvent mouse)`

  **`doubleClicked`**`(MouseEvent mouse)`

  **`pressAndHold`**`(MouseEvent mouse)`

In addition to regular MouseArea signals, geographical coordinate at
which user has interacted with the map and its sensitivity is
forwarded via the following signals:

* **`clickedGeo`**`(var geocoordinate, real degLatPerPixel, real degLonPerPixel)`

  **`doubleClickedGeo`**`(var geocoordinate, real degLatPerPixel, real degLonPerPixel)`

  **`pressAndHoldGeo`**`(var geocoordinate, real degLatPerPixel, real degLonPerPixel)`

In these signals, _geocoordinate_ is QtPositioning.coordinate
object. See description of `replyCoordinateForPixel` signal of
MapboxMap for the description of _degLatPerPixel_ and _degLonPerPixel_
values.

## Properties

* `var `**`map`** Map object to be interacted with through the gesture
  area. This property must be set on construction.

* `bool `**`activeClickedGeo`** If true, clickedGeo signal will be
  emitted after a click event. False by default.

* `bool `**`activeDoubleClickedGeo`** If true, doubleClickedGeo signal will be
  emitted after a double click event. False by default.

* `bool `**`activePressAndHoldGeo`** If true, pressAndHoldGeo signal will be
  emitted after a double click event. False by default.

* `bool`**`integerZoomLevels`** If true, the result of pinch-zoom will snap to
  integer zoom levels, which should look better with raster sources in terms of
  sharpness and level of detail. False by default.
