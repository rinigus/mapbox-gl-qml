#ifndef MAPBOXGLEXTENSIONPLUGIN_H
#define MAPBOXGLEXTENSIONPLUGIN_H

#include <QQmlExtensionPlugin>

class MapboxGLExtensionPlugin : public QQmlExtensionPlugin
{
  Q_OBJECT
  Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
  MapboxGLExtensionPlugin(QObject *parent = Q_NULLPTR);

  virtual void registerTypes(const char *uri) override;
};

#endif // MAPBOXGLEXTENSIONPLUGIN_H
