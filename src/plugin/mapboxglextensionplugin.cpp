#include "mapboxglextensionplugin.h"
#include "qquickitemmapboxgl.h"

MapboxGLExtensionPlugin::MapboxGLExtensionPlugin(QObject *parent):
  QQmlExtensionPlugin(parent)
{
}

void MapboxGLExtensionPlugin::registerTypes(const char *uri)
{
  Q_ASSERT(uri == QLatin1String("MapboxMap"));
  qmlRegisterType<QQuickItemMapboxGL>(uri, 1, 0, "MapboxMap");
}
