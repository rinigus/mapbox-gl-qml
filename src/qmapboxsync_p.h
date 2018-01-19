#ifndef QMAPBOXSYNC_H
#define QMAPBOXSYNC_H

#include <QMapboxGL>

#include <QString>
#include <QVariantMap>
#include <QList>
#include <QImage>

namespace QMapboxSync
{
  //////////////////////////////////////////////////////////////////////////
  /// QMapboxSync namespace contains classes that are responsible
  /// for application of dynamic settings, such as manipulation with
  /// additional sources, layers, paint properties. All supported
  /// assets have similar approach. On addition/update/removal, the request
  /// is recorded into an action list. To apply the action list on the map,
  /// call method apply. On application, all manipulations are also recorded
  /// into an object variable that contains the list of all active assets.
  /// This allows to setup all the assets (add sources, layers, ...) when
  /// the map has been destroyed and recreated again. For that, call setup method.
  ///
  /// Externally, users are expected to use SourceList, LayerList, ... classes

  //////////////////////////////////////////////////////////
  /// General action that covers application of new setting

  class Action
  {
  public:
    enum Type { Add, Update, Remove };

  public:
    Action(Type t): m_type(t) {}

    virtual void apply(QMapboxGL *map) = 0;

    Type type() const { return m_type; }

  protected:
    Type m_type;
  };

  //////////////////////////////////////////////////////////
  /// General asset that covers source and layer support

  class Asset {
  public:
    Asset(const QString i, const QVariantMap p, const QString b = QString()):
      id(i), params(p), before(b) {}

  public:
    QString id;
    QVariantMap params;
    QString before;
  };

  //////////////////////////////////////////////////////////
  /// Source support

  class SourceList
  {
  public:
    SourceList() {}

    void add(const QString &id, const QVariantMap& params);
    void update(const QString &id, const QVariantMap& params);
    void remove(const QString &id);

    void apply(QMapboxGL *map);
    void setup(QMapboxGL *map);

  protected:

    class SourceAction: public Action {
    public:
      SourceAction(Type t, const QString id, const QVariantMap params = QVariantMap());
      virtual void apply(QMapboxGL *map);
      Asset& asset() { return m_asset; }

    protected:
      Asset m_asset;
    };

    void add_to_stack(Action::Type t, const QString &id, const QVariantMap& params);

  protected:
    QList<Asset> m_assets;
    QList<SourceAction> m_action_stack;
  };

  //////////////////////////////////////////////////////////
  /// Layer support

  class LayerList
  {
  public:
    LayerList() {}

    void add(const QString &id, const QVariantMap& params, const QString &before);
    void remove(const QString &id);

    void apply(QMapboxGL *map);
    void setup(QMapboxGL *map);

  protected:

    class LayerAction: public Action {
    public:
      LayerAction(Type t, const QString id, const QVariantMap params = QVariantMap(), const QString before = QString());
      virtual void apply(QMapboxGL *map);
      Asset& asset() { return m_asset; }

    protected:
      Asset m_asset;
    };

  protected:
    QList<Asset> m_assets;
    QList<LayerAction> m_action_stack;
  };

  ///////////////////////////////////////////////////////////
  /// Properties support

  class Property {
  public:
    Property(const QString &l, const QString &p, const QVariant &v):
      layer(l), property(p), value(v) {}

  public:
    QString layer;
    QString property;
    QVariant value;
  };

  class PropertyList {
  public:
    PropertyList() {}

    void add(const QString &layer, const QString &property, const QVariant& value);

    void apply(QMapboxGL *map);
    void setup(QMapboxGL *map);

  protected:
    virtual void apply_property(QMapboxGL *map, Property &p) = 0;

  protected:
    QList<Property> m_properties;
    QList<Property> m_action_stack;
  };

  class LayoutPropertyList: public PropertyList {
  public:
    LayoutPropertyList(): PropertyList() {}
  protected:
    virtual void apply_property(QMapboxGL *map, Property &p);
  };

  class PaintPropertyList: public PropertyList {
  public:
    PaintPropertyList(): PropertyList() {}
  protected:
    virtual void apply_property(QMapboxGL *map, Property &p);
  };

  ///////////////////////////////////////////////////////////
  /// Images support

  class Image {
  public:
    Image(const QString &i, const QImage &im):
      id(i), image(im) {}

  public:
    QString id;
    QImage image;
    QVariant value;
  };

  class ImageList {
  public:
    ImageList() {}

    void add(const QString &id, const QImage &sprite);
    void remove(const QString &id);

    void apply(QMapboxGL *map);
    void setup(QMapboxGL *map);

  protected:
    class ImageAction: public Action {
    public:
      ImageAction(Type t, const QString id, const QImage image = QImage());
      virtual void apply(QMapboxGL *map);
      Image& image() { return m_image; }

    protected:
      Image m_image;
    };


  protected:
    QList<Image> m_images;
    QList<ImageAction> m_action_stack;
  };


}

#endif // QMAPBOXSYNC_H
