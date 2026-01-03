// Based on code from MapLibre Native Qt and earlier codes from Qt5 support
//
// Copyright (C) 2023 MapLibre contributors
// Copyright (C) 2026 Rinigus

// SPDX-License-Identifier: BSD-2-Clause

#include "texturenodeopengl.h"

#if IS_QT6

#include <QtGui/qopenglcontext.h>

#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>
#include <QtQuick/QQuickOpenGLUtils>

using namespace MLNQT6;

TextureNodeOpenGL::~TextureNodeOpenGL() {}

void TextureNodeOpenGL::resize(const QSize &size, qreal pixelRatio) {
    if (!m_map) {
        return;
    }

    const QSize minSize = size.expandedTo(MIN_TEXTURE_SIZE);
    BaseNode::resize(minSize, pixelRatio);
    setRect(QRectF(QPointF(0, 0), m_item_size));

    const QSize fbSize = minSize * m_device_pixel_ratio;         // physical pixels
    m_map_size = minSize * m_device_pixel_ratio / m_pixel_ratio; // ensure zoom

    m_map->resize(m_map_size);

    if (!m_fbo || m_fbo->size() != fbSize) {
        // check if we have GL context
        const QOpenGLContext *glContext = QOpenGLContext::currentContext();
        if (glContext == nullptr) {
            qWarning() << "TextureNodeOpenGL::render: no current QOpenGLContext";
            return;
        }

        QOpenGLFramebufferObjectFormat fmt;
        fmt.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        fmt.setTextureTarget(GL_TEXTURE_2D);
        fmt.setInternalTextureFormat(GL_RGBA);

        // keep old texture/fbo before we set new texture in render
        m_prev_texture.reset(m_texture.release());
        m_prev_fbo.reset(m_fbo.release());
        m_fbo.reset(new QOpenGLFramebufferObject(fbSize, fmt));

        if (!m_fbo || !m_fbo->isValid()) {
            qWarning() << "Failed to create FBO or FBO is invalid after creation";
            m_fbo.reset();
            return;
        }
    }

    m_map->setOpenGLFramebufferObject(static_cast<quint32>(m_fbo->handle()), fbSize);
}

void TextureNodeOpenGL::render(QQuickWindow *window) {
    if (!m_map || m_map_size.isEmpty() || !m_fbo)
        return;

    // Ensure renderer is created first
    if (!m_renderer_bound) {
        m_map->createRenderer();
        m_renderer_bound = true;
    }

    // setup texture if it is missing
    if (!m_texture) {
        // check if we have GL context
        const QOpenGLContext *glContext = QOpenGLContext::currentContext();
        if (glContext == nullptr) {
            qWarning() << "TextureNodeOpenGL::render: no current QOpenGLContext";
            return;
        }

        const GLuint maplibreTextureId = static_cast<GLuint>(m_fbo->texture());

        if (maplibreTextureId == 0) {
            qDebug() << "Missing FBO texture ID";
            return;
        }

        m_texture.reset(QNativeInterface::QSGOpenGLTexture::fromNative(
            maplibreTextureId, window, m_map_size, QQuickWindow::TextureHasAlphaChannel));

        if (!m_texture) {
            qWarning()
                << "TextureNodeOpenGL::render: failed to create QSG texture from native GL texture";
            return;
        }

        setTexture(m_texture.get());
        setOwnsTexture(false);

        // drop unused objects
        m_prev_texture.reset();
        m_prev_fbo.reset();
    }

    QQuickOpenGLUtils::resetOpenGLState();

    m_map->render();

    markDirty(QSGNode::DirtyMaterial);

    QQuickOpenGLUtils::resetOpenGLState();
}

#endif
