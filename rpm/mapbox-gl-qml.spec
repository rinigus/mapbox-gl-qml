Summary: Mapbox GL Native QML plugin
Name: mapboxgl-qml
Version: 2.0.1
Release: 1%{?dist}
License: LGPLv3
Group: Libraries/Geosciences
URL: https://github.com/rinigus/mapbox-gl-qml

Source: %{name}-%{version}.tar.gz

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

BuildRequires: gcc-c++
BuildRequires: cmake
BuildRequires: pkgconfig(Qt5Core)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Location)
BuildRequires: pkgconfig(Qt5Positioning)
BuildRequires: pkgconfig(Qt5Svg)
BuildRequires: pkgconfig(libcurl)
BuildRequires: pkgconfig(openssl)
%if 0%{?fedora_version} >= 29 || 0%{?centos_version} >= 800
BuildRequires: qt5-qtbase-devel
%endif
%if 0%{?suse_version} >= 1500
BuildRequires: libqt5-qtbase-devel
%endif
BuildRequires: qmaplibregl-devel

%description
QML plugin for Maplibre GL Native, Mapbox GL Native fork.

PackageName: Mapbox GL QML Plugin
Categories:
  - Maps
  - Science

%prep
%setup -q -n %{name}-%{version}

%build

%if 0%{?suse_version} >= 1500 || 0%{?fedora_version} >= 29 || 0%{?centos_version} >= 800
%cmake
%else
%cmake -DUSE_CURL_SSL=ON
%endif

%{__make} %{?_smp_mflags}

%install
%{__rm} -rf %{buildroot}
%{__make} install DESTDIR=%{buildroot}

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
* Sun Jul 28 2019 rinigus <rinigus.git@gmail.com> - 1.5.0.2
- Expose runtime changes in styleJson

* Sat Nov 7 2018 rinigus <rinigus.git@gmail.com> - 1.3.2-1
- Add OpenSSL locks during widget initialization
- Add Linux compilation instructions

* Sat Sep 15 2018 rinigus <rinigus.git@gmail.com> - 1.3.1-1
- Add threshold for metersPerPixel property

* Sun Mar 10 2018 rinigus <rinigus.git@gmail.com> - 1.3.0-1
- Add pan threshold
- Update to QMapboxGL qt-1.3.0 API

* Sun Mar 4 2018 rinigus <rinigus.git@gmail.com> - 1.2.0-1
- Add property to snap to integer zoom levels
- Allow cache cleaning

* Fri Dec 15 2017 rinigus <rinigus.git@gmail.com> - 1.1.1-1
- bugfix: remove hard-coded certificate file name

* Thu Sep 28 2017 rinigus <rinigus.git@gmail.com> - 1.0.0-1
- initial packaging release for SFOS
