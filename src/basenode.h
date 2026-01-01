#ifndef BASENODE_H
#define BASENODE_H

#include <QGeoCoordinate>
#include <QQuickItem>

#include <QMapLibre/Map>
#include <QMapLibre/Settings>

class BaseNode : public QObject {
    Q_OBJECT

  public:
    BaseNode(const QMapLibre::Settings &, const QSize &, qreal devicePixelRatio,
                          qreal pixelRatio, QQuickItem *item);

    QMapLibre::Map *map() const { return m_map.data(); }
    float height() const { return m_map_size.height(); }
    float width() const { return m_map_size.width(); }
    float mapToQtPixelRatio() const;

    virtual void resize(const QSize &size, qreal pixelRatio);
    virtual void render(QQuickWindow *) {}

  public slots:
    void querySourceExists(const QString &id);
    void queryLayerExists(const QString &id);
    void queryCoordinateForPixel(QPointF p, const QVariant &tag);

  signals:
    void replySourceExists(const QString id, bool exists);
    void replyLayerExists(const QString id, bool exists);
    void replyCoordinateForPixel(const QPointF p, QGeoCoordinate geo, qreal degLatPerPixel,
                                 qreal degLonPerPixel, const QVariant &tag);

  protected:
    QScopedPointer<QMapLibre::Map> m_map;
    QSize m_map_size;  ///<- size as set for map
    QSize m_item_size; ///<- size of Qt item in Qt logical pixels units
    qreal m_pixel_ratio;
    qreal m_device_pixel_ratio{1};
};

#endif // BASENODE_H