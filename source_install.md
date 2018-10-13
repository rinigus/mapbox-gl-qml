# Installation from source

To install unofficial Mapbox GL Native bindings for Qt QML from
sources, you would have to install Qt version of MapboxGL native
library. For that, please follow instructions at
https://github.com/rinigus/pkg-mapbox-gl-native

After QMapboxGL is installed, proceed as follows.

Clone this repository
```
git clone https://github.com/rinigus/mapbox-gl-qml.git
```

Change to the directory, make build folder and start building:
```
cd mapbox-gl-qml
mkdir build && cd build
qmake ../mapbox-gl-qml.pro
make -j4
```

Install from build folder
```
sudo make install
```

