#include "baserendernode.h"

#if HAS_SGRENDERNODE

//////////////////////////////////////////
/// RenderNode

BaseRenderNode::BaseRenderNode(const QMapLibre::Settings &settings, const QSize &size,
                               qreal devicePixelRatio, qreal pixelRatio, QQuickItem *item)
    : BaseNode(settings, size, devicePixelRatio, pixelRatio, item) {}

QSGRenderNode::StateFlags BaseRenderNode::changedStates() const {
    return QSGRenderNode::DepthState | QSGRenderNode::StencilState | QSGRenderNode::ScissorState |
           QSGRenderNode::ColorState | QSGRenderNode::BlendState | QSGRenderNode::ViewportState |
           QSGRenderNode::RenderTargetState;
}

#endif
