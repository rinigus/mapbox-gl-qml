Summary: Mapbox GL Native QML plugin
Name: mapboxgl-qml
Version: 1.1.0
Release: 1%{?dist}
License: LGPLv3
Group: Libraries/Geosciences
URL: https://github.com/rinigus/mapbox-gl-qml

Source: %{name}-%{version}.tar.gz

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

BuildRequires: opt-gcc6 gcc-c++
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Location)
BuildRequires: pkgconfig(Qt5Positioning)
BuildRequires: pkgconfig(libcurl)
BuildRequires: qmapboxgl

%description
QML plugin for Mapbox GL Native.

%prep
%setup -q -n %{name}-%{version}

%build

%qmake5 mapbox-gl-qml.pro \
    VERSION='%{version}-%{release}'

%{__make} CXX=/opt/gcc6/bin/g++ CC=/opt/gcc6/bin/gcc LINK=/opt/gcc6/bin/g++ %{?_smp_mflags}

%install
%{__rm} -rf %{buildroot}
%qmake5_install

%clean
%{__rm} -rf %{buildroot}

%pre

%files
%files
%defattr(-, root, root, 0755)
%{_libdir}/qt5/qml/MapboxMap/libqmlmapboxglplugin.so
%{_libdir}/qt5/qml/MapboxMap/qmldir
%{_libdir}/qt5/qml/MapboxMap/MapboxMapGestureArea.qml

%changelog
* Thu Sep 28 2017 rinigus <rinigus.git@gmail.com> - 1.0.0-1
- initial packaging release for SFOS
