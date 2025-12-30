#include "qsgmapboxglrendernode.h"

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>

#include <math.h>

#include <QDebug>

static const QSize minTextureSize = QSize(16, 16);

#if HAS_SGRENDERNODE
//////////////////////////////////////////
/// QSGMapboxGLRenderNode

QSGMapboxGLRenderNode::QSGMapboxGLRenderNode(const QMapLibre::Settings &settings, const QSize &size,
                                             qreal devicePixelRatio, qreal pixelRatio,
                                             QQuickItem *item)
    : QMapboxGLAbstractNode(settings, size, devicePixelRatio, pixelRatio, item) {
    qInfo() << "Using QSGMapboxGLRenderNode for map rendering. "
            << "devicePixelRatio:" << devicePixelRatio;
}

void QSGMapboxGLRenderNode::resize(const QSize &size, qreal pixelRatio) {
    const QSize minSize = size.expandedTo(minTextureSize);
    QMapboxGLAbstractNode::resize(size, pixelRatio);
    m_map_size = minSize / pixelRatio;
    m_map->resize(m_map_size);
}

void QSGMapboxGLRenderNode::render(const RenderState *state) {
    // QMapLibre assumes we've prepared the viewport prior to render().
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glViewport(state->scissorRect().x(), state->scissorRect().y(), state->scissorRect().width(),
                  state->scissorRect().height());
    f->glScissor(state->scissorRect().x(), state->scissorRect().y(), state->scissorRect().width(),
                 state->scissorRect().height());
    f->glEnable(GL_SCISSOR_TEST);

    GLint alignment;
    f->glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);

    m_map->render();

    // QTBUG-62861
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    f->glDepthRangef(0, 1);
}

QSGRenderNode::StateFlags QSGMapboxGLRenderNode::changedStates() const {
    return QSGRenderNode::DepthState | QSGRenderNode::StencilState | QSGRenderNode::ScissorState |
           QSGRenderNode::ColorState | QSGRenderNode::BlendState | QSGRenderNode::ViewportState |
           QSGRenderNode::RenderTargetState;
}

#endif
