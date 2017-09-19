#ifndef QMAPBOXSYNC_H
#define QMAPBOXSYNC_H

#include <QMapboxGL>

#include <QString>
#include <QVariantMap>
#include <QList>

namespace QMapboxSync
{

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


  class Source {
  public:
    Source(const QString sourceID, const QVariantMap p = QVariantMap()):
      id(sourceID), params(p) {}

  public:
    QString id;
    QVariantMap params;
  };

  class SourceList
  {
  public:
    SourceList() {}

    void add(const QString &sourceID, const QVariantMap& params);
    void update(const QString &sourceID, const QVariantMap& params);
    void remove(const QString &sourceID);

    void apply(QMapboxGL *map);
    void setup(QMapboxGL *map);

  protected:

    class SourceAction: public Action {
    public:
      SourceAction(Type t, const QString sourceID, const QVariantMap params = QVariantMap());
      virtual void apply(QMapboxGL *map);
      Source& source() { return m_source; }

    protected:
      Source m_source;
    };

  protected:
    QList<Source> m_sources;
    QList<SourceAction> m_action_stack;
  };
}

#endif // QMAPBOXSYNC_H
