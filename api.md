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

Map properties are listed classified and listed in the following
sub-sections. Each property is given with the corresponding type in
front of it.

#### Map settings on initialization

The properties in this subsection should be set on initialization of
the map. If modified later, the properties may not be applied until
the map's frame buffer object is recreated. Some of them (such as
cache-related properties) would be used as they were specified for the 
first created map.

* `string accessToken` Mapbox-hosted vector tiles and styles require an API
    [access token](https://www.mapbox.com/help/define-access-token/),
    which you can obtain from the
    [Mapbox account page](https://www.mapbox.com/studio/account/tokens/). Access
    tokens associate requests to Mapbox's vector tile and style APIs
    with your Mapbox account. They also deter other developers from
    using your styles without your permission.

* `string apiBaseUrl` The API base URL is the URL that the `mapbox://`
    protocol will be resolved to. It defaults to
    "https://api.mapbox.com" but can be changed, for instance, to a
    tile cache server address.

* `string assetPath` Returns the asset path, which is the root directory from where
    the `asset://` scheme gets resolved in a style. `asset://` can be used
    for loading a resource from the disk in a style rather than fetching
    it from the network.

    By default, it is set to the value returned by
    `QCoreApplication::applicationDirPath()`.
    
* `string cacheDatabasePath` Returns the cache database path. The cache is used for storing
    recently used resources like tiles and also an offline tile database
    pre-populated by the [Offline Tool](https://github.com/mapbox/mapbox-gl-native/blob/master/bin/offline.sh).

    By default, it is set to `:memory:` meaning it will create an in-memory
    cache instead of a file on disk.
    
    Cache-related settings are shared between all QMapboxGL instances because different
    maps will share the same cache database file. The first map to configure cache properties
    such as size and path will force the configuration to all newly instantiated QMapboxGL
    objects.

* `int cacheDatabaseMaximalSize` Returns the cache database maximum
    hard size in bytes. The database will grow until the limit is
    reached. Setting a maximum size smaller than the current size of
    an existing database results in undefined behavior

    By default, it is set to 50 MB.
    
    Cache-related settings are shared between all QMapboxGL instances because different
    maps will share the same cache database file. The first map to configure cache properties
    such as size and path will force the configuration to all newly instantiated QMapboxGL
    objects.


#### Map rendering

* `real bearing` The map bearing angle in degrees. Negative values and
    values over 360 are valid and will wrap. The direction of the
    rotation is counterclockwise.

    When `center` is defined, the map will rotate around the center
    pixel coordinate respecting the margins if defined.
    
* `QGeoCoordinate center` Coordinates of the map center. Centers the
    map at a geographic coordinate respecting the margins, if set.
    
    In QML, use `QtPositioning.coordinate(latitude, longitude)`
    construct (see http://doc.qt.io/qt-5/qml-coordinate.html and
    http://doc.qt.io/qt-5/qgeocoordinate.html for details).
    
* `rect margins` Relative margins that determine position of the
    center on the map. Margins specify the relative active area of the
    widget and the position of the active area. For example, `margins`
    given with the rectangle (0.1 _x_, 0.2 _y_, 0.8 _width_, 0.3
    _height_) would determine that the active area is shifted
    `0.1*width` from the left, `0.2*height` up from the bottom, is
    `0.8*width` wide leaving `0.1*width` pixels to the right margin,
    and is `0.3*height` high leaving the top half of the widget as a
    margin. Through use of the margins one can shift the center within
    the widget and ensure that it is not covered by other GUI elements
    drawn on the top of the map, for example.
    
* `real maximumZoomLevel` Maximum zoom level allowed for the map.
    
* `real minimumZoomLevel` Minimum zoom level allowed for the map.
    
* `real pitch` Pitch toward the horizon measured in degrees, with 0
    resulting in a two-dimensional map. It is constrained at 60
    degrees.

* `real zoomLevel` The map zoom factor.  This property is used to zoom
    the map. When `center` is defined, the map will zoom in the
    direction of the center.


#### Mangling of URLs

#### Other properties



### Queries and Signals

### Methods
