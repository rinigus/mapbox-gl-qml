#ifndef QSGMAPBOXGLTEXTURENODE_H
#define QSGMAPBOXGLTEXTURENODE_H

#include "qmapboxglabstractnode.h"

#include <QtGui/QOpenGLFramebufferObject>
#include <QtQuick/QSGSimpleTextureNode>

class QSGMapboxGLTextureNode : public QMapboxGLAbstractNode, public QSGSimpleTextureNode {
    Q_OBJECT

  public:
    QSGMapboxGLTextureNode(const QMapLibre::Settings &, const QSize &, qreal devicePixelRatio,
                           qreal pixelRatio, QQuickItem *item);
    ~QSGMapboxGLTextureNode();

    QMapLibre::Map *map() const { return m_map.data(); }

    void resize(const QSize &size, qreal pixelRatio) override;
    void render(QQuickWindow *) override;

  private:
    QScopedPointer<QOpenGLFramebufferObject> m_fbo;
};

#endif // QSGMAPBOXGLTEXTURENODE_H
