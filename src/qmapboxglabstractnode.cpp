#include "qmapboxglabstractnode.h"

#include "macros.h"

#include <math.h>

//////////////////////////////////////////
/// QMapboxGLAbstractNode

QMapboxGLAbstractNode::QMapboxGLAbstractNode(const QMapLibre::Settings &settings, const QSize &size,
                                             qreal devicePixelRatio, qreal pixelRatio,
                                             QQuickItem *item)
    : QObject(), m_map_size(size), m_item_size(size), m_pixel_ratio(pixelRatio),
      m_device_pixel_ratio(devicePixelRatio) {
    m_map.reset(
        new QMapLibre::Map(nullptr, settings, size.expandedTo(MIN_TEXTURE_SIZE), pixelRatio));

    QObject::connect(m_map.data(), &QMapLibre::Map::needsRendering, item, &QQuickItem::update);
    QObject::connect(m_map.data(), &QMapLibre::Map::copyrightsChanged, item, &QQuickItem::update);
}

void QMapboxGLAbstractNode::resize(const QSize &size, qreal pixelRatio) {
    m_item_size = size;
    m_pixel_ratio = pixelRatio;
}

float QMapboxGLAbstractNode::mapToQtPixelRatio() const {
    return 0.5 * (width() / m_item_size.width() + height() / m_item_size.height());
}

/////////////////////////////////
/// queries

void QMapboxGLAbstractNode::querySourceExists(const QString &sourceID) {
    emit replySourceExists(sourceID, m_map->sourceExists(sourceID));
}

void QMapboxGLAbstractNode::queryLayerExists(const QString &sourceID) {
    emit replyLayerExists(sourceID, m_map->layerExists(sourceID));
}

void QMapboxGLAbstractNode::queryCoordinateForPixel(QPointF p, const QVariant &tag) {
    float rx = ((float)m_map_size.width()) / ((float)m_item_size.width());
    float ry = ((float)m_map_size.height()) / ((float)m_item_size.height());

    p.setX(p.x() * rx);
    p.setY(p.y() * ry);
    QMapLibre::Coordinate mbc = m_map->coordinateForPixel(p);
    QGeoCoordinate coor(mbc.first, mbc.second);

    // get sensitivity of coordinates to the changes in pixel coordinates
    double bearing = m_map->bearing() / 180. * M_PI;
    double sinB = sin(bearing);
    double cosB = cos(bearing);
    p += QPointF(cosB + sinB, -sinB + cosB);
    QMapLibre::Coordinate mbc_shift = m_map->coordinateForPixel(p);

    qreal degLatPerPixel = fabs(mbc_shift.first - mbc.first) * rx;
    qreal degLonPerPixel = fabs(mbc_shift.second - mbc.second) * ry;

    emit replyCoordinateForPixel(p, coor, degLatPerPixel, degLonPerPixel, tag);
}
