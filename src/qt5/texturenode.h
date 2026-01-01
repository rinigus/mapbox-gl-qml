#ifndef QT5_TEXTURENODE_H
#define QT5_TEXTURENODE_H

#include "macros.h"

#if IS_QT5

#include "basetexturenode.h"

#include <QtGui/QOpenGLFramebufferObject>

namespace MLNQT5 {

class TextureNode : public BaseTextureNode {
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

} // namespace MLNQT5

#endif

#endif // QT5_TEXTURENODE_H