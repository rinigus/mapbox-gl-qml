#ifndef BASETEXTURENODE_H
#define BASETEXTURENODE_H

#include "basenode.h"

#include <QtQuick/QSGSimpleTextureNode>

class BaseTextureNode : public BaseNode, public QSGSimpleTextureNode {
    Q_OBJECT

  public:
    BaseTextureNode(const QMapLibre::Settings &, const QSize &, qreal devicePixelRatio,
                    qreal pixelRatio, QQuickItem *item);
    ~BaseTextureNode();
};

#endif // TEXTURENODE_H