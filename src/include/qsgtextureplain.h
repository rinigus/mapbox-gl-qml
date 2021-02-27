/****************************************************************************
**
** Based on the private implementation of QSGTexturePrivate by Qt as in
** https://github.com/qt/qtdeclarative/blob/5.9/src/quick/scenegraph/util/qsgtexture_p.h
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

#ifndef QSGTEXTUREPLAIN_H
#define QSGTEXTUREPLAIN_H

#include <QSGTexture>

class QSGTexturePlain : public QSGTexture
{
    Q_OBJECT
public:
    QSGTexturePlain();
    virtual ~QSGTexturePlain();

    void setOwnsTexture(bool owns) { m_owns_texture = owns; }
    bool ownsTexture() const { return m_owns_texture; }

    void setTextureId(int id);
    int textureId() const override;
    void setTextureSize(const QSize &size) { m_texture_size = size; }
    QSize textureSize() const override { return m_texture_size; }

    void setHasAlphaChannel(bool alpha) { m_has_alpha = alpha; }
    bool hasAlphaChannel() const override { return m_has_alpha; }

    bool hasMipmaps() const override { return mipmapFiltering() != QSGTexture::None; }

    void bind() override;

protected:
    uint m_texture_id;
    QSize m_texture_size;
    QRectF m_texture_rect;

    uint m_has_alpha : 1;
    uint m_dirty_texture : 1;
    uint m_dirty_bind_options : 1;
    uint m_owns_texture : 1;
    uint m_mipmaps_generated : 1;
    uint m_retain_image: 1;
};

#endif // QSGTEXTUREPLAIN_H
