// Based on code from MapLibre Native Qt

// Copyright (C) 2023 MapLibre contributors
// Copyright (C) 2026 Rinigus

// SPDX-License-Identifier: BSD-2-Clause

#ifndef QT6_TEXTURENODEOPENGL_H
#define QT6_TEXTURENODEOPENGL_H

#include "macros.h"

#if IS_QT6

#include "basetexturenode.h"

#include <QOpenGLFramebufferObject>
#include <QtGui/qopengl.h>

#include <memory>

namespace MLNQT6 {

class TextureNodeOpenGL final : public BaseTextureNode {
  public:
    using BaseTextureNode::BaseTextureNode;
    ~TextureNodeOpenGL() final;

    void resize(const QSize &size, qreal pixelRatio) final;
    void render(QQuickWindow *window) final;

  private:
    bool m_renderer_bound{};
    std::unique_ptr<QOpenGLFramebufferObject> m_fbo{};
    std::unique_ptr<QSGTexture> m_texture{};
    std::unique_ptr<QOpenGLFramebufferObject> m_prev_fbo{};
    std::unique_ptr<QSGTexture> m_prev_texture{};
};

} // namespace MLNQT6

#endif
#endif
