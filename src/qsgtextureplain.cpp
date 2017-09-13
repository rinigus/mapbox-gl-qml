/****************************************************************************
**
** Based on the private implementation of QSGTexturePrivate by Qt as in
** https://github.com/qt/qtdeclarative/blob/5.9/src/quick/scenegraph/util/qsgtexture.cpp
**
** Here, the code is simplified and the class renamed to avoid confusion.
** The original code license is below
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtQuick module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qsgtextureplain.h"
#include <qopenglfunctions.h>
#include <QtGui/qopenglcontext.h>
#include <QtGui/qopenglfunctions.h>

QSGTexturePlain::QSGTexturePlain()
  : QSGTexture()
  , m_texture_id(0)
  , m_has_alpha(false)
  , m_dirty_texture(false)
  , m_dirty_bind_options(false)
  , m_owns_texture(true)
  , m_mipmaps_generated(false)
  , m_retain_image(false)
{
}


QSGTexturePlain::~QSGTexturePlain()
{
  if (m_texture_id && m_owns_texture && QOpenGLContext::currentContext())
    QOpenGLContext::currentContext()->functions()->glDeleteTextures(1, &m_texture_id);
}


int QSGTexturePlain::textureId() const
{
  if (m_dirty_texture) {
      // The actual texture and id will be updated/deleted in a later bind()
      // or ~QSGTexturePlain so just keep it minimal here.
      return 0;
    }
  return m_texture_id;
}

void QSGTexturePlain::setTextureId(int id)
{
  if (m_texture_id && m_owns_texture)
    QOpenGLContext::currentContext()->functions()->glDeleteTextures(1, &m_texture_id);

  m_texture_id = id;
  m_dirty_texture = false;
  m_dirty_bind_options = true;
  m_mipmaps_generated = false;
}

void QSGTexturePlain::bind()
{
  QOpenGLContext *context = QOpenGLContext::currentContext();
  QOpenGLFunctions *funcs = context->functions();
  if (!m_dirty_texture) {
      funcs->glBindTexture(GL_TEXTURE_2D, m_texture_id);
      if (mipmapFiltering() != QSGTexture::None && !m_mipmaps_generated) {
          funcs->glGenerateMipmap(GL_TEXTURE_2D);
          m_mipmaps_generated = true;
        }
      updateBindOptions(m_dirty_bind_options);
      m_dirty_bind_options = false;
      return;
    }

  m_dirty_texture = false;

  if (m_texture_id && m_owns_texture) {
      funcs->glDeleteTextures(1, &m_texture_id);
    }
  m_texture_id = 0;
  m_texture_size = QSize();
  m_has_alpha = false;

}
