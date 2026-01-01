#ifndef QT5_RENDERNODE_H
#define QT5_RENDERNODE_H

#include "macros.h"

#if IS_QT5

#if HAS_SGRENDERNODE
#include <QSGRenderNode>

#include "basenode.h"

namespace MLNQT5 {

class RenderNode : public BaseNode, public QSGRenderNode {
    Q_OBJECT

  public:
    RenderNode(const QMapLibre::Settings &, const QSize &, qreal devicePixelRatio, qreal pixelRatio,
               QQuickItem *item);

    void resize(const QSize &size, qreal pixelRatio);

    // QSGRenderNode
    void render(const RenderState *state) override;
    StateFlags changedStates() const override;
};

} // namespace MLNQT5

#endif

#endif

#endif // QT5_RENDERNODE_H