# Unofficial Mapbox GL Native bindings for Qt QML: API

This is a description of an API that was written for platforms lacking
QtLocation 5.9 support. I would like to acknowledge the use of QtLocation 5.9
Mapbox GL plugin documentation while this API documentation.

## QQuickItemMapboxGL (C++) / MapboxMap (QML)

QML Quick Item for displaying maps using Mapbox GL.

### Properties

Map properties are listed classified and listed in the following
sub-sections.

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

* `int cacheDatabaseMaximalSize` Returns the cache database maximum
    hard size in bytes. The database will grow until the limit is
    reached. Setting a maximum size smaller than the current size of
    an existing database results in undefined behavior

    By default, it is set to 50 MB.

#### Map rendering

#### Mangling of URLs 

### Queries and Signals

### Methods
