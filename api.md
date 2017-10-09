# Unofficial Mapbox GL Native bindings for Qt QML: API

This is a description of an API that was written for platforms lacking
QtLocation 5.9 support. I would like to acknowledge the use of QtLocation 5.9
Mapbox GL plugin documentation while this API documentation.

## QQuickItemMapboxGL (C++) / MapboxMap (QML)

QML Quick Item for displaying maps using Mapbox GL. See
`qquickitemmapboxgl.h` for specific syntax, if needed.

In QML, `MapboxMap` type is defined by this plugin. In C++, use
`QQuickItemMapboxGL` class.

### Include statements

In QML

```
import QQuickItemMapboxGL 1.0
```

In C++

```
#include <qquickitemmapboxgl.h>
```

### Properties

Map properties are classified and listed in the following
sub-sections. Each property is given with the corresponding type in
front of it.

#### Map settings on initialization

The properties in this subsection should be set on initialization of
the map. If modified later, the properties may not be applied until
the map's frame buffer object is recreated. Some of them (such as
cache-related properties) would be used as they were specified for the 
first created map.

* `string `**`accessToken`** Mapbox-hosted vector tiles and styles require an API
    [access token](https://www.mapbox.com/help/define-access-token/),
    which you can obtain from the
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
    
* `string `**`cacheDatabasePath`** Returns the cache database path. The cache is used for storing
    recently used resources like tiles and also an offline tile database
    pre-populated by the [Offline Tool](https://github.com/mapbox/mapbox-gl-native/blob/master/bin/offline.sh).

    By default, it is set to `:memory:` meaning it will create an in-memory
    cache instead of a file on disk.
    
    Cache-related settings are shared between all QMapboxGL instances because different
    maps will share the same cache database file. The first map to configure cache properties
    such as size and path will force the configuration to all newly instantiated QMapboxGL
    objects.

* `int `**`cacheDatabaseMaximalSize`** Returns the cache database maximum
    hard size in bytes. The database will grow until the limit is
    reached. Setting a maximum size smaller than the current size of
    an existing database results in undefined behavior

    By default, it is set to 50 MB. 
    
    See also general comment regarding cache settings given in the
    description of `cacheDatabasePath`.


#### Map rendering

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
    
* `rect `**`margins`** Relative margins that determine position of the
    center on the map. Margins specify the relative active area of the
    widget and the position of the active area. When given by a
    rectangle (_x_, _y_, _width_, _height_), the margins are specified
    by the active area with the left bottom given by _x_ and _y_ as
    well as the size of the active area as specified by _width_ and
    _height_. For example, `margins` given with the rectangle (0.1,
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
    
* `real `**`pixelRatio`** Relative pixel density of the screen when compared
    to 96dpi. All the map elements will be scaled by this ratio when
    drawn. This allows to use the same style on the screens with the
    different pixel densities. 
    
    Care should be taken when having `pixelRatio` different from one
    and using `metersPerPixel` to draw something on the map (such as
    uncertainty of position, for example). Since `metersPerPixel`
    gives the scale on the actual screen and all objects are scaled up
    by `pixelRatio`, you may have to reduce the drawn objects by
    `pixelRatio` when adding them on the map. For example, when
    drawing uncertainty of position as a circle around position, you
    would have to divide the radius of the circle by `pixelRatio` when
    modifying the circle radius by `setPaintProperty` method.
    
* `string `**`styleJson`** The map style in JSON given as a string. Sets a
    new style from a JSON that must conform to the
    [Mapbox style specification](https://www.mapbox.com/mapbox-gl-style-spec/).
    
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


#### Mangling of URLs

Before fetching resources from internet, URLs can be either printed
for debugging purposes or changed by adding them given suffix. 

* `bool `**`urlDebug`** When set to `true`, all URLs are printed out in
   `stdout` before fetching them.
   
* `string `**`urlSuffix`** When set, this string will be appended to all
  URLs before fetching them online. For example, you can append your
  server's specific api key by setting `urlSuffix` to `?apikey=ABCD`
  if it has to be specified in such format at the end of each
  requested URL.
  

#### Other properties

* `string `**`errorString`** Current error string. Please note that this
  property is not covering all possible errors in the API. When set,
  it is never cleared. Thus, please connect to the signal
  `errorChanged` to trigger error processing and don't rely on
  clearance of errorString property.

* `real `**`metersPerPixel`** Meters per pixel at the center of the map
  given for the pixel density as specified by `pixelRatio`.


### Queries and Signals

### Methods

Map methods are classified and listed in the following sub-sections.

#### General methods

* `void `**`fitView`**`(const QVariantList &coordinates)`

  Finds zoom and the center that would allow to fit the given list of
  coordinates in the map view taking into account current `margins`,
  `pitch`, and `bearing`. Note that the expected list elements are
  coordinates as given by `QGeoCoordinate` (`QtPositioning.coordinate`
  in QML).

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


#### Map sources

* `void `**`addSource`**`(const QString &sourceID, const QVariantMap& params)`

  Adds a source _id_ to the map as specified by the
    [Mapbox style specification](https://www.mapbox.com/mapbox-gl-js/style-spec/#sources)
    with _params_. For example, GeoJSON type can be used to specify
    road, position on the map, or many other features that can added
    and later modified by application.
    
    The following snippet adds a point on the map
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
    
    and the following adds a small line source
    
    ```javascript
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
