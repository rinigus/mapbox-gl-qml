#include "qmapboxsync_p.h"

using namespace QMapboxSync;

SourceList::SourceAction::SourceAction(Type t, const QString sourceID, const QVariantMap params):
  Action(t),
  m_source(sourceID, params)
{
}

void SourceList::SourceAction::apply(QMapboxGL *map)
{
  if (type() == Add)
    map->addSource(m_source.id, m_source.params);
  else if (type() == Update)
    map->updateSource(m_source.id, m_source.params);
  else if (type() == Remove)
    map->removeSource(m_source.id);
  else
    Q_ASSERT(0);
}

void SourceList::add(const QString &sourceID, const QVariantMap &params)
{
  m_action_stack.append( SourceAction(Action::Add, sourceID, params) );
}

void SourceList::remove(const QString &sourceID)
{
  m_action_stack.append( SourceAction(Action::Remove, sourceID) );
}

void SourceList::update(const QString &sourceID, const QVariantMap &params)
{
  m_action_stack.append( SourceAction(Action::Update, sourceID, params) );
}

void SourceList::apply(QMapboxGL *map)
{
  for (SourceAction &action: m_action_stack)
    {
      action.apply(map);

      if (action.type() == Action::Add) m_sources.append(action.source());

      else if (action.type() == Action::Remove)
        {
          QMutableListIterator<Source> i(m_sources);
          while (i.hasNext())
            if (i.next().id == action.source().id)
              i.remove();
        }

      else if (action.type() == Action::Update)
        {
          Source update = action.source();
          bool updated = false;
          for (Source &source: m_sources)
            if (update.id == source.id)
              {
                updated = true;
                for (QVariantMap::const_iterator iter = update.params.constBegin();
                     iter != update.params.constEnd(); ++iter)
                  source.params[ iter.key() ] = iter.value();
              }

          // treat update equal to add if such source does not exist
          if (!updated)
            m_sources.append(update);
        }
    }

  m_action_stack.clear();
}

void SourceList::setup(QMapboxGL *map)
{
  for (Source &source: m_sources)
    {
      SourceAction action(Action::Add, source.id, source.params);
      action.apply(map);
    }
}
