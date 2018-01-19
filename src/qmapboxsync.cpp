#include "qmapboxsync_p.h"

#include <QJsonDocument>

#include <QDebug>

using namespace QMapboxSync;

/// Source

SourceList::SourceAction::SourceAction(Type t, const QString id, const QVariantMap params):
  Action(t),
  m_asset(id, params)
{
}

void SourceList::SourceAction::apply(QMapboxGL *map)
{
  // special treatment of "data" field
  if (m_asset.params.contains("data"))
    {
      QVariant data_orig = m_asset.params["data"];
      if (data_orig.type() == QVariant::String)
        m_asset.params["data"] = data_orig.toString().toUtf8();
      else if (data_orig.type() == QVariant::Map)
        m_asset.params["data"] = QJsonDocument::fromVariant(data_orig).toJson();
    }

  // apply
  if (type() == Add || type() == Update)
   {
      // we can always use update since it will be added if needed.
      // however, using update avoids race conditions where add is called
      // after update. such race condition can happen if QML objects are
      // created in non-expected order, for example.
      //
      // The functionality of add_or_replace is dependent on this assumption.
      // If Add and Update are considered different, change that method as well
      //
      map->updateSource(m_asset.id, m_asset.params);
    }
  else if (type() == Remove)
    map->removeSource(m_asset.id);
  else
    Q_ASSERT(0);
}

void SourceList::add(const QString &id, const QVariantMap &params)
{
  add_to_stack(Action::Add, id, params);
}

void SourceList::remove(const QString &id)
{
  add_to_stack(Action::Remove, id, QVariantMap());
}

void SourceList::update(const QString &id, const QVariantMap &params)
{
  add_to_stack(Action::Update, id, params);
}

/// To avoid populating stack of added sources (for example during
/// initialization or longer CPU sleep or background activity without
/// OpenGL calls), replace the last added source in the stack with the
/// new one. If the source has been removed earlier in the stack,
/// drop that remove as well
void SourceList::add_to_stack(Action::Type t, const QString &id, const QVariantMap &params)
{
  // remove previous source action if existing
  QMutableListIterator<SourceAction> i(m_action_stack);
  while (i.hasNext())
    if (i.next().asset().id == id)
      i.remove();

  m_action_stack.append( SourceAction(t, id, params) );
}

void SourceList::apply(QMapboxGL *map)
{
  for (SourceAction &action: m_action_stack)
    {
      action.apply(map);

      if (action.type() == Action::Remove)
        {
          QMutableListIterator<Asset> i(m_assets);
          while (i.hasNext())
            if (i.next().id == action.asset().id)
              i.remove();
        }

      else if (action.type() == Action::Add || action.type() == Action::Update)
        {
          Asset update = action.asset();
          bool updated = false;
          for (Asset &asset: m_assets)
            if (update.id == asset.id)
              {
                updated = true;
                for (QVariantMap::const_iterator iter = update.params.constBegin();
                     iter != update.params.constEnd(); ++iter)
                  asset.params[ iter.key() ] = iter.value();
              }

          // treat update equal to add if such source does not exist
          if (!updated)
            m_assets.append(update);
        }
    }

  m_action_stack.clear();
}

void SourceList::setup(QMapboxGL *map)
{
  for (Asset &asset: m_assets)
    {
      SourceAction action(Action::Add, asset.id, asset.params);
      action.apply(map);
    }
}


/// Layer

LayerList::LayerAction::LayerAction(Type t, const QString id, const QVariantMap params, const QString before):
  Action(t),
  m_asset(id, params, before)
{
  m_asset.params["id"] = id;
}

void LayerList::LayerAction::apply(QMapboxGL *map)
{
  if (type() == Add)
    {
      if (map->layerExists(m_asset.id))
        map->removeLayer(m_asset.id);
      map->addLayer(m_asset.params, m_asset.before);
    }
  else if (type() == Remove)
    map->removeLayer(m_asset.id);
  else
    Q_ASSERT(0);
}

void LayerList::add(const QString &id, const QVariantMap &params, const QString &before)
{
  m_action_stack.append( LayerAction(Action::Add, id, params, before) );
}

void LayerList::remove(const QString &id)
{
  m_action_stack.append( LayerAction(Action::Remove, id) );
}

void LayerList::apply(QMapboxGL *map)
{
  for (LayerAction &action: m_action_stack)
    {
      action.apply(map);

      if (action.type() == Action::Add) m_assets.append(action.asset());

      else if (action.type() == Action::Remove)
        {
          QMutableListIterator<Asset> i(m_assets);
          while (i.hasNext())
            if (i.next().id == action.asset().id)
              i.remove();
        }
    }

  m_action_stack.clear();
}

void LayerList::setup(QMapboxGL *map)
{
  for (Asset &asset: m_assets)
    {
      LayerAction action(Action::Add, asset.id, asset.params, asset.before);
      action.apply(map);
    }
}

/// Properties

void PropertyList::add(const QString &layer, const QString &property, const QVariant& value)
{
  m_action_stack.append( Property(layer, property, value) );
}

void PropertyList::apply(QMapboxGL *map)
{
  for (Property &p: m_action_stack)
    {
      this->apply_property(map, p);
      m_properties.append(p);
    }

  m_action_stack.clear();
}

void PropertyList::setup(QMapboxGL *map)
{
  for (Property &p: m_properties)
    {
      this->apply_property(map, p);
    }
}

void LayoutPropertyList::apply_property(QMapboxGL *map, Property &p)
{
  map->setLayoutProperty(p.layer, p.property, p.value);
}

void PaintPropertyList::apply_property(QMapboxGL *map, Property &p)
{
  map->setPaintProperty(p.layer, p.property, p.value);
}


/// Images

/// Layer

ImageList::ImageAction::ImageAction(Type t, const QString id, const QImage im):
  Action(t),
  m_image(id, im)
{
}

void ImageList::ImageAction::apply(QMapboxGL *map)
{
  if (type() == Add)
    map->addImage(m_image.id, m_image.image);
  else if (type() == Remove)
    map->removeImage(m_image.id);
  else
    Q_ASSERT(0);
}

void ImageList::add(const QString &id, const QImage &sprite)
{
  m_action_stack.append( ImageAction(Action::Add, id, sprite) );
}

void ImageList::remove(const QString &id)
{
  m_action_stack.append( ImageAction(Action::Remove, id) );
}

void ImageList::apply(QMapboxGL *map)
{
  for (ImageAction &action: m_action_stack)
    {
      action.apply(map);

      if (action.type() == Action::Add) m_images.append(action.image());

      else if (action.type() == Action::Remove)
        {
          QMutableListIterator<Image> i(m_images);
          while (i.hasNext())
            if (i.next().id == action.image().id)
              i.remove();
        }
    }

  m_action_stack.clear();
}

void ImageList::setup(QMapboxGL *map)
{
  for (Image &image: m_images)
    {
      ImageAction action(Action::Add, image.id, image.image);
      action.apply(map);
    }
}
