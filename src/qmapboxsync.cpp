#include "qmapboxsync_p.h"

using namespace QMapboxSync;

/// Source

SourceList::SourceAction::SourceAction(Type t, const QString id, const QVariantMap params):
  Action(t),
  m_asset(id, params)
{
}

void SourceList::SourceAction::apply(QMapboxGL *map)
{
  if (type() == Add)
    map->addSource(m_asset.id, m_asset.params);
  else if (type() == Update)
    map->updateSource(m_asset.id, m_asset.params);
  else if (type() == Remove)
    map->removeSource(m_asset.id);
  else
    Q_ASSERT(0);
}

void SourceList::add(const QString &id, const QVariantMap &params)
{
  m_action_stack.append( SourceAction(Action::Add, id, params) );
}

void SourceList::remove(const QString &id)
{
  m_action_stack.append( SourceAction(Action::Remove, id) );
}

void SourceList::update(const QString &id, const QVariantMap &params)
{
  m_action_stack.append( SourceAction(Action::Update, id, params) );
}

void SourceList::apply(QMapboxGL *map)
{
  for (SourceAction &action: m_action_stack)
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

      else if (action.type() == Action::Update)
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

LayerList::LayerAction::LayerAction(Type t, const QString id, const QVariantMap params):
  Action(t),
  m_asset(id, params)
{
  m_asset.params["id"] = id;
}

void LayerList::LayerAction::apply(QMapboxGL *map)
{
  if (type() == Add)
    map->addLayer(m_asset.params);
  else if (type() == Remove)
    map->removeLayer(m_asset.id);
  else
    Q_ASSERT(0);
}

void LayerList::add(const QString &id, const QVariantMap &params)
{
  m_action_stack.append( LayerAction(Action::Add, id, params) );
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
      LayerAction action(Action::Add, asset.id, asset.params);
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
