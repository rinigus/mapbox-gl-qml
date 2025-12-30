#ifndef QSGMAPBOXGLRENDERNODE_H
#define QSGMAPBOXGLRENDERNODE_H

#include "macros.h"

#if HAS_SGRENDERNODE
#include <QSGRenderNode>

#include "qmapboxglabstractnode.h"

class QSGMapboxGLRenderNode : public QMapboxGLAbstractNode, public QSGRenderNode {
    Q_OBJECT

  public:
    QSGMapboxGLRenderNode(const QMapLibre::Settings &, const QSize &, qreal devicePixelRatio,
                          qreal pixelRatio, QQuickItem *item);

    QMapLibre::Map *map() const;

    void resize(const QSize &size, qreal pixelRatio);

    // QSGRenderNode
    void render(const RenderState *state) override;
    StateFlags changedStates() const override;
};
#endif

#endif // QSGMAPBOXGLRENDERNODE_H
