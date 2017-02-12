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

#ifndef QSGD3D11RENDERLOOP_P_H
#define QSGD3D11RENDERLOOP_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <private/qsgrenderloop_p.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

QT_BEGIN_NAMESPACE

class QSGD3D11Engine;
class QSGD3D11Context;
class QSGD3D11RenderContext;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;

class QSGD3D11RenderLoop : public QSGRenderLoop
{
    Q_OBJECT

public:
    QSGD3D11RenderLoop(ID3D11Device *device, ID3D11DeviceContext *context, IDXGISwapChain *swapChain);
    ~QSGD3D11RenderLoop();

    void show(QQuickWindow *window) override;
    void hide(QQuickWindow *window) override;
    void resize(QQuickWindow *window) override;

    void windowDestroyed(QQuickWindow *window) override;

    void exposureChanged(QQuickWindow *window) override;

    QImage grab(QQuickWindow *window) override;

    void update(QQuickWindow *window) override;
    void maybeUpdate(QQuickWindow *window) override;

    QAnimationDriver *animationDriver() const override;

    QSGContext *sceneGraphContext() const override;
    QSGRenderContext *createRenderContext(QSGContext *) const override;

    void releaseResources(QQuickWindow *window) override;
    void postJob(QQuickWindow *window, QRunnable *job) override;

    QSurface::SurfaceType windowSurfaceType() const override;
    bool interleaveIncubation() const override;
    int flags() const override;

    bool event(QEvent *event) override;

	void present();

public Q_SLOTS:
    void onAnimationStarted();
    void onAnimationStopped();

private:
    void exposeWindow(QQuickWindow *window);
    void obscureWindow(QQuickWindow *window);
    void renderWindow(QQuickWindow *window);
    void render();
    void maybePostUpdateTimer();
    bool somethingVisible() const;

    QSGD3D11Context *sg;
    QAnimationDriver *m_anims;
    int m_vsyncDelta;
    int m_updateTimer = 0;
    int m_animationTimer = 0;

    struct WindowData {
        QSGD3D11RenderContext *rc = nullptr;
        QSGD3D11Engine *engine = nullptr;
        bool updatePending = false;
        bool grabOnly = false;
        bool exposed = false;
    };

    QHash<QQuickWindow *, WindowData> m_windows;

    QImage m_grabContent;

	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;
	ComPtr<IDXGISwapChain> m_swapChain;
};

QT_END_NAMESPACE

#endif // QSGD3D11RENDERLOOP_P_H
