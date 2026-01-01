/****************************************************************************
**
** Based on the implementation of Mapbox GL Native QtLocation plugin at
** https://github.com/qt/qtlocation/tree/5.9/src/plugins/geoservices/mapboxgl
** and later versions. Developed further for integration with the plugin
**
** The original code license is below
**
** Copyright (C) 2017 The Qt Company Ltd.
** Copyright (C) 2017 Mapbox, Inc.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtLocation module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "rendernode.h"

#include "basenode.h"

#if IS_QT5

#include <QSize>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOpenGLFunctions>

#include <math.h>

#include <QDebug>

#if HAS_SGRENDERNODE

using namespace MLNQT5;

//////////////////////////////////////////
/// RenderNode

RenderNode::RenderNode(const QMapLibre::Settings &settings, const QSize &size,
                                             qreal devicePixelRatio, qreal pixelRatio,
                                             QQuickItem *item)
    : BaseNode(settings, size, devicePixelRatio, pixelRatio, item) {
    qInfo() << "Using RenderNode for map rendering. "
            << "devicePixelRatio:" << devicePixelRatio;
}

void RenderNode::resize(const QSize &size, qreal pixelRatio) {
    const QSize minSize = size.expandedTo(MIN_TEXTURE_SIZE);
    BaseNode::resize(size, pixelRatio);
    m_map_size = minSize / pixelRatio;
    m_map->resize(m_map_size);
}

void RenderNode::render(const RenderState *state) {
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

QSGRenderNode::StateFlags RenderNode::changedStates() const {
    return QSGRenderNode::DepthState | QSGRenderNode::StencilState | QSGRenderNode::ScissorState |
           QSGRenderNode::ColorState | QSGRenderNode::BlendState | QSGRenderNode::ViewportState |
           QSGRenderNode::RenderTargetState;
}

#endif
#endif