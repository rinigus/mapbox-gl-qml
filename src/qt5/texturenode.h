#ifndef TEXTURENODE_H
#define TEXTURENODE_H

#include "macros.h"

#if IS_QT5

#include "basenode.h"

#include <QtGui/QOpenGLFramebufferObject>
#include <QtQuick/QSGSimpleTextureNode>

namespace MLNQT5 {

class TextureNode : public BaseNode, public QSGSimpleTextureNode {
    Q_OBJECT

  public:
    TextureNode(const QMapLibre::Settings &, const QSize &, qreal devicePixelRatio,
                           qreal pixelRatio, QQuickItem *item);
    ~TextureNode();

    void resize(const QSize &size, qreal pixelRatio) override;
    void render(QQuickWindow *) override;

  private:
    QScopedPointer<QOpenGLFramebufferObject> m_fbo;
};

}

#endif

#endif // TEXTURENODE_H