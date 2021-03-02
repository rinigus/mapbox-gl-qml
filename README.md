# Unofficial Mapbox GL Native bindings for Qt QML

These bindings were originally developed for supporting Mapbox GL
Native for platforms that lack Qt/QML 5.9 support, such as Sailfish
OS, and application that are targeting Mapbox GL Native maps
specifically. These bindings provide an alternative API that can be
used to access Mapbox GL Native functionality in QML applications.

These bindings are used by few applications, such as [Pure
Maps](https://github.com/rinigus/pure-maps) and
[Laufhelden](https://github.com/jdrescher2006/Laufhelden).

The code is based on QtLocation 5.9 Mapbox GL plugin, Qt/QML bindings
developed as a part of Mapbox GL Native by Mapbox team before merging
QML bindings with QtLocation. Please note that the origin of each file
is noted in the comment at the top of the file.

Since QtLocation 5.9 is distributed with LGPL3.0, the code written for
these bindings is distributed with the same license. The original code
developed as a part of Mapbox GL Native has a license at
https://github.com/mapbox/mapbox-gl-native/blob/master/LICENSE.md

For Sailfish, the bindings are available as a package at
OpenRepos. For desktop Linux, unless packaged for your distribution,
compilation from source is needed. Instructions for compilation are
[here](https://github.com/rinigus/mapbox-gl-qml/blob/master/source_install.md)


The description of API: [api.md](https://github.com/rinigus/mapbox-gl-qml/blob/master/api.md)

Demo for Sailfish OS is at the separate repository: https://github.com/rinigus/mapbox-demo-sfos

Included demo (app subfolder) is for C++. Note that if the plugin is
installed in the system, its easier to use it from QML directly. Just
add `import MapboxMap 1.0` to your QML and you would have access to it
without any C++ code requirements.

To use Mapbox GL tiles and styles provided by Mapbox, please register
on their site and obtain access key. The key can be either used to
specify `accessToken` property of MapboxMap (see [API
description](https://github.com/rinigus/mapbox-gl-qml/blob/master/api.md))
or using environment variable `MAPBOX_ACCESS_TOKEN`.
