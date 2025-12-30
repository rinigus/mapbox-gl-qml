#include "qsgmapboxgltexturenode.h"
#include "qsgtextureplain.h"

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>

#include <math.h>

#include <QDebug>

static const QSize minTextureSize = QSize(16, 16);

//////////////////////////////////////////
/// QSGMapboxGLTextureNode

QSGMapboxGLTextureNode::QSGMapboxGLTextureNode(const QMapLibre::Settings &settings,
                                               const QSize &size, qreal devicePixelRatio,
                                               qreal pixelRatio, QQuickItem *item)
    : QMapboxGLAbstractNode(settings, size, devicePixelRatio, pixelRatio, item),
      QSGSimpleTextureNode() {
    qInfo() << "Using QSGMapboxGLTextureNode for map rendering."
            << "devicePixelRatio:" << devicePixelRatio;

    setTextureCoordinatesTransform(QSGSimpleTextureNode::MirrorVertically);
    setFiltering(QSGTexture::Linear);

    resize(size, pixelRatio); // to fill and attach fbo
}

QSGMapboxGLTextureNode::~QSGMapboxGLTextureNode() {}

void QSGMapboxGLTextureNode::resize(const QSize &size, qreal pixelRatio) {
    const QSize minSize = size.expandedTo(minTextureSize);
    QMapboxGLAbstractNode::resize(minSize, pixelRatio);

    const QSize fbSize = minSize * m_device_pixel_ratio;         // physical pixels
    m_map_size = minSize * m_device_pixel_ratio / m_pixel_ratio; // ensure zoom

    m_map->resize(m_map_size);

    m_fbo.reset(
        new QOpenGLFramebufferObject(fbSize, QOpenGLFramebufferObject::CombinedDepthStencil));
    m_map->setOpenGLFramebufferObject(m_fbo->handle(), fbSize);

    QSGTexturePlain *fboTexture = static_cast<QSGTexturePlain *>(texture());
    if (!fboTexture)
        fboTexture = new QSGTexturePlain;

    fboTexture->setTextureId(m_fbo->texture());
    fboTexture->setTextureSize(fbSize);

    if (!texture()) {
        setTexture(fboTexture);
        setOwnsTexture(true);
    }

    setRect(QRectF(QPointF(), minSize));
}

void QSGMapboxGLTextureNode::render(QQuickWindow *window) {
    QOpenGLFunctions *f = window->openglContext()->functions();
    f->glViewport(0, 0, m_fbo->width(), m_fbo->height());

    GLint alignment;
    f->glGetIntegerv(GL_UNPACK_ALIGNMENT, &alignment);

    m_fbo->bind();
    m_map->render();
    //    m_logo.render();
    m_fbo->release();

    // QTBUG-62861
    f->glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    f->glDepthRangef(0, 1);

    window->resetOpenGLState();
}
