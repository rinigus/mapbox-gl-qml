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
        QOpenGLFramebufferObjectFormat fmt;
        fmt.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
        fmt.setTextureTarget(GL_TEXTURE_2D);
        fmt.setInternalTextureFormat(GL_RGBA);

        m_fbo.reset(new QOpenGLFramebufferObject(fbSize, fmt));
        m_texture.reset();
    }

    m_map->setOpenGLFramebufferObject(static_cast<quint32>(m_fbo->handle()), fbSize);
}

void TextureNodeOpenGL::render(QQuickWindow *window) {
    if (!m_map || m_map_size.isEmpty())
        return;

    const QOpenGLContext *glContext = QOpenGLContext::currentContext();
    if (glContext == nullptr) {
        qWarning() << "TextureNodeOpenGL::render: no current QOpenGLContext";
        return;
    }

    // Ensure renderer is created first
    if (!m_renderer_bound) {
        m_map->createRenderer();
        m_renderer_bound = true;
    }

    QQuickOpenGLUtils::resetOpenGLState();

    m_map->render();

    // setup texture if it is missing
    if (!m_texture) {
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
    }

    markDirty(QSGNode::DirtyMaterial);

    QQuickOpenGLUtils::resetOpenGLState();
}

#endif
