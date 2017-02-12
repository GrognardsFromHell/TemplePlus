/****************************************************************************
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

#include "qsgd3d11context_p.h"
#include "qsgd3d11rendercontext_p.h"
#include "qsgd3d11internalrectanglenode_p.h"
#include "qsgd3d11internalimagenode_p.h"
#include "qsgd3d11glyphnode_p.h"
#include "qsgd3d11layer_p.h"
#include "qsgd3d11painternode_p.h"
#include "qsgd3d11publicnodes_p.h"
#include "qsgd3d11spritenode_p.h"
#include <QtQuick/qquickwindow.h>

QT_BEGIN_NAMESPACE

QSGRenderContext *QSGD3D11Context::createRenderContext()
{
    return new QSGD3D11RenderContext(this);
}

QSGInternalRectangleNode *QSGD3D11Context::createInternalRectangleNode()
{
    return new QSGD3D11InternalRectangleNode;
}

QSGInternalImageNode *QSGD3D11Context::createInternalImageNode()
{
    return new QSGD3D11InternalImageNode;
}

QSGPainterNode *QSGD3D11Context::createPainterNode(QQuickPaintedItem *item)
{
    return new QSGD3D11PainterNode(item);
}

QSGGlyphNode *QSGD3D11Context::createGlyphNode(QSGRenderContext *renderContext, bool preferNativeGlyphNode)
{
    Q_UNUSED(preferNativeGlyphNode);
    // ### distance field text rendering is not supported atm

    QSGD3D11RenderContext *rc = static_cast<QSGD3D11RenderContext *>(renderContext);
    return new QSGD3D11GlyphNode(rc);
}

QSGLayer *QSGD3D11Context::createLayer(QSGRenderContext *renderContext)
{
    QSGD3D11RenderContext *rc = static_cast<QSGD3D11RenderContext *>(renderContext);
    return new QSGD3D11Layer(rc);
}

QSGGuiThreadShaderEffectManager *QSGD3D11Context::createGuiThreadShaderEffectManager()
{
    return nullptr;
}

QSGShaderEffectNode *QSGD3D11Context::createShaderEffectNode(QSGRenderContext *renderContext,
                                                             QSGGuiThreadShaderEffectManager *mgr)
{
    return nullptr;
}

QSize QSGD3D11Context::minimumFBOSize() const
{
    return QSize(16, 16);
}

QSurfaceFormat QSGD3D11Context::defaultSurfaceFormat() const
{
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();

    if (QQuickWindow::hasDefaultAlphaBuffer())
        format.setAlphaBufferSize(8);

    return format;
}

QSGRendererInterface *QSGD3D11Context::rendererInterface(QSGRenderContext *renderContext)
{
    return static_cast<QSGD3D11RenderContext *>(renderContext);
}

QSGRectangleNode *QSGD3D11Context::createRectangleNode()
{
    return new QSGD3D11RectangleNode;
}

QSGImageNode *QSGD3D11Context::createImageNode()
{
    return new QSGD3D11ImageNode;
}

QSGNinePatchNode *QSGD3D11Context::createNinePatchNode()
{
    return new QSGD3D11NinePatchNode;
}

QSGSpriteNode *QSGD3D11Context::createSpriteNode()
{
    return new QSGD3D11SpriteNode;
}

QT_END_NAMESPACE
