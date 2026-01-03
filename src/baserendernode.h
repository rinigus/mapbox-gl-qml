#ifndef BASERENDERNODE_H
#define BASERENDERNODE_H

#include "macros.h"

#if HAS_SGRENDERNODE
#include <QSGRenderNode>

#include "basenode.h"

class BaseRenderNode : public BaseNode, public QSGRenderNode {
    Q_OBJECT

  public:
    BaseRenderNode(const QMapLibre::Settings &, const QSize &, qreal devicePixelRatio,
                   qreal pixelRatio, QQuickItem *item);

    // QSGRenderNode
    StateFlags changedStates() const override;
};

#endif

#endif // QT5_RENDERNODE_H