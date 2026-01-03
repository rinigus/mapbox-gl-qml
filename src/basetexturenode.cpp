#include "basetexturenode.h"

BaseTextureNode::BaseTextureNode(const QMapLibre::Settings &settings, const QSize &size,
                                 qreal devicePixelRatio, qreal pixelRatio, QQuickItem *item)
    : BaseNode(settings, size, devicePixelRatio, pixelRatio, item), QSGSimpleTextureNode() {
    setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
    setFiltering(QSGTexture::Linear);
}

BaseTextureNode::~BaseTextureNode() {}
