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

#include "qsgd3d11renderloop_p.h"
#include "qsgd3d11engine_p.h"
#include "qsgd3d11context_p.h"
#include "qsgd3d11rendercontext_p.h"
#include <private/qquickwindow_p.h>
#include <private/qquickprofiler_p.h>
#include <private/qquickanimatorcontroller_p.h>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QScreen>

QT_BEGIN_NAMESPACE

// NOTE: Avoid categorized logging. It is slow.

#define DECLARE_DEBUG_VAR(variable) \
    static bool debug_ ## variable() \
    { static bool value = qgetenv("QSG_RENDERER_DEBUG").contains(QT_STRINGIFY(variable)); return value; }

DECLARE_DEBUG_VAR(loop)
DECLARE_DEBUG_VAR(time)


// This render loop operates on the gui (main) thread.
// Conceptually it matches the OpenGL 'windows' render loop.

static inline int qsgrl_animation_interval()
{
    const qreal refreshRate = QGuiApplication::primaryScreen() ? QGuiApplication::primaryScreen()->refreshRate() : 0;
    return refreshRate < 1 ? 16 : int(1000 / refreshRate);
}

QSGD3D11RenderLoop::QSGD3D11RenderLoop(ID3D11Device *device, ID3D11DeviceContext *context, IDXGISwapChain *swapChain) 
	: m_device(device), m_context(context), m_swapChain(swapChain)
{
    sg = new QSGD3D11Context;

    m_anims = sg->createAnimationDriver(this);
    connect(m_anims, &QAnimationDriver::started, this, &QSGD3D11RenderLoop::onAnimationStarted);
    connect(m_anims, &QAnimationDriver::stopped, this, &QSGD3D11RenderLoop::onAnimationStopped);
    m_anims->install();

    m_vsyncDelta = qsgrl_animation_interval();
}

QSGD3D11RenderLoop::~QSGD3D11RenderLoop()
{
    delete sg;
}

void QSGD3D11RenderLoop::show(QQuickWindow *window)
{
    if (Q_UNLIKELY(debug_loop()))
        qDebug() << "show" << window;
}

void QSGD3D11RenderLoop::hide(QQuickWindow *window)
{
    if (Q_UNLIKELY(debug_loop()))
        qDebug() << "hide" << window;
}

void QSGD3D11RenderLoop::resize(QQuickWindow *window)
{
    if (!m_windows.contains(window) || window->size().isEmpty())
        return;

    if (Q_UNLIKELY(debug_loop()))
        qDebug() << "resize" << window;

    const WindowData &data(m_windows[window]);

    if (!data.exposed)
        return;

    if (data.engine)
        data.engine->setWindowSize(window->size(), window->effectiveDevicePixelRatio());
}

void QSGD3D11RenderLoop::windowDestroyed(QQuickWindow *window)
{
    if (Q_UNLIKELY(debug_loop()))
        qDebug() << "window destroyed" << window;

    if (!m_windows.contains(window))
        return;

    QQuickWindowPrivate *wd = QQuickWindowPrivate::get(window);
    wd->fireAboutToStop();

    WindowData &data(m_windows[window]);
    QSGD3D11Engine *engine = data.engine;
    QSGD3D11RenderContext *rc = data.rc;
    m_windows.remove(window);

    // Bye bye nodes...
    wd->cleanupNodesOnShutdown();

    rc->invalidate();

    if (m_windows.isEmpty())
        QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);

    delete rc;
    delete engine;

    delete wd->animationController;
}

void QSGD3D11RenderLoop::exposeWindow(QQuickWindow *window)
{
    WindowData data;
    data.exposed = true;
    data.engine = new QSGD3D11Engine(m_device.Get(), m_context.Get(), m_swapChain.Get());
    data.rc = static_cast<QSGD3D11RenderContext *>(QQuickWindowPrivate::get(window)->context);
    data.rc->setEngine(data.engine);
    m_windows[window] = data;

    const int samples = window->format().samples();
    const bool alpha = window->format().alphaBufferSize() > 0;
    const qreal dpr = window->effectiveDevicePixelRatio();

    if (Q_UNLIKELY(debug_loop()))
        qDebug() << "initializing D3D11 engine" << window << window->size() << dpr << samples << alpha;

    data.engine->attachToWindow(window->size(), dpr, samples, alpha);
}

void QSGD3D11RenderLoop::obscureWindow(QQuickWindow *window)
{
    m_windows[window].exposed = false;
    QQuickWindowPrivate *wd = QQuickWindowPrivate::get(window);
    wd->fireAboutToStop();
}

void QSGD3D11RenderLoop::exposureChanged(QQuickWindow *window)
{
    if (Q_UNLIKELY(debug_loop()))
        qDebug() << "exposure changed" << window << window->isExposed();

    if (window->isExposed()) {
        if (!m_windows.contains(window))
            exposeWindow(window);

        // Stop non-visual animation timer as we now have a window rendering.
        if (m_animationTimer && somethingVisible()) {
            killTimer(m_animationTimer);
            m_animationTimer = 0;
        }
        // If we have a pending timer and we get an expose, we need to stop it.
        // Otherwise we get two frames and two animation ticks in the same time interval.
        if (m_updateTimer) {
            killTimer(m_updateTimer);
            m_updateTimer = 0;
        }

        WindowData &data(m_windows[window]);
        data.exposed = true;
        data.updatePending = true;

        render();

    } else if (m_windows.contains(window)) {
        obscureWindow(window);

        // Potentially start the non-visual animation timer if nobody is rendering.
        if (m_anims->isRunning() && !somethingVisible() && !m_animationTimer)
            m_animationTimer = startTimer(m_vsyncDelta);
    }
}

QImage QSGD3D11RenderLoop::grab(QQuickWindow *window)
{
    const bool tempExpose = !m_windows.contains(window);
    if (tempExpose)
        exposeWindow(window);

    m_windows[window].grabOnly = true;

    renderWindow(window);

    QImage grabbed = m_grabContent;
    m_grabContent = QImage();

    if (tempExpose)
        obscureWindow(window);

    return grabbed;
}

bool QSGD3D11RenderLoop::somethingVisible() const
{
    for (auto it = m_windows.constBegin(), itEnd = m_windows.constEnd(); it != itEnd; ++it) {
        if (it.key()->isVisible() && it.key()->isExposed())
            return true;
    }
    return false;
}

void QSGD3D11RenderLoop::maybePostUpdateTimer()
{
    if (!m_updateTimer) {
        if (Q_UNLIKELY(debug_loop()))
            qDebug("starting update timer");
        m_updateTimer = startTimer(m_vsyncDelta / 3);
    }
}

void QSGD3D11RenderLoop::update(QQuickWindow *window)
{
    maybeUpdate(window);
}

void QSGD3D11RenderLoop::maybeUpdate(QQuickWindow *window)
{
    if (!m_windows.contains(window) || !somethingVisible())
        return;

    m_windows[window].updatePending = true;
    maybePostUpdateTimer();
}

QAnimationDriver *QSGD3D11RenderLoop::animationDriver() const
{
    return m_anims;
}

QSGContext *QSGD3D11RenderLoop::sceneGraphContext() const
{
    return sg;
}

QSGRenderContext *QSGD3D11RenderLoop::createRenderContext(QSGContext *) const
{
    // The rendercontext and engine are per-window, like with the threaded
    // loop, but unlike the non-threaded OpenGL variants.
    return sg->createRenderContext();
}

void QSGD3D11RenderLoop::releaseResources(QQuickWindow *window)
{
    if (Q_UNLIKELY(debug_loop()))
        qDebug() << "releaseResources" << window;
}

void QSGD3D11RenderLoop::postJob(QQuickWindow *window, QRunnable *job)
{
    Q_UNUSED(window);
    Q_ASSERT(job);
    Q_ASSERT(window);
    job->run();
    delete job;
}

QSurface::SurfaceType QSGD3D11RenderLoop::windowSurfaceType() const
{
    return QSurface::OpenGLSurface;
}

bool QSGD3D11RenderLoop::interleaveIncubation() const
{
    return m_anims->isRunning() && somethingVisible();
}

void QSGD3D11RenderLoop::onAnimationStarted()
{
    if (!somethingVisible()) {
        if (!m_animationTimer) {
            if (Q_UNLIKELY(debug_loop()))
                qDebug("starting non-visual animation timer");
            m_animationTimer = startTimer(m_vsyncDelta);
        }
    } else {
        maybePostUpdateTimer();
    }
}

void QSGD3D11RenderLoop::onAnimationStopped()
{
    if (m_animationTimer) {
        if (Q_UNLIKELY(debug_loop()))
            qDebug("stopping non-visual animation timer");
        killTimer(m_animationTimer);
        m_animationTimer = 0;
    }
}

bool QSGD3D11RenderLoop::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::Timer:
    {
        QTimerEvent *te = static_cast<QTimerEvent *>(event);
        if (te->timerId() == m_animationTimer) {
            if (Q_UNLIKELY(debug_loop()))
                qDebug("animation tick while no windows exposed");
            m_anims->advance();
        } else if (te->timerId() == m_updateTimer) {
            if (Q_UNLIKELY(debug_loop()))
                qDebug("update timeout - rendering");
            killTimer(m_updateTimer);
            m_updateTimer = 0;
            render();
        }
        return true;
    }
    default:
        break;
    }

    return QObject::event(event);
}

void QSGD3D11RenderLoop::present()
{

	bool rendered = false;
	for (auto it = m_windows.begin(), itEnd = m_windows.end(); it != itEnd; ++it) {
		auto window = it.key();
		if (!it->exposed || !window->isVisible()) {
			continue;
		}

		it->engine->present();
	}

}

void QSGD3D11RenderLoop::render()
{
    bool rendered = false;
    for (auto it = m_windows.begin(), itEnd = m_windows.end(); it != itEnd; ++it) {
        if (it->updatePending) {
            it->updatePending = false;
            renderWindow(it.key());
            rendered = true;
        }
    }

    if (!rendered) {
        if (Q_UNLIKELY(debug_loop()))
            qDebug("render - no changes, sleep");
        QThread::msleep(m_vsyncDelta);
    }

    if (m_anims->isRunning()) {
        if (Q_UNLIKELY(debug_loop()))
            qDebug("render - advancing animations");

        m_anims->advance();

        // It is not given that animations triggered another maybeUpdate()
        // and thus another render pass, so to keep things running,
        // make sure there is another frame pending.
        maybePostUpdateTimer();

        emit timeToIncubate();
    }
}

void QSGD3D11RenderLoop::renderWindow(QQuickWindow *window)
{
    if (Q_UNLIKELY(debug_loop()))
        qDebug() << "renderWindow" << window;

    QQuickWindowPrivate *wd = QQuickWindowPrivate::get(window);
    if (!m_windows.contains(window) || !window->geometry().isValid())
        return;

    WindowData &data(m_windows[window]);
    if (!data.exposed) { // not the same as window->isExposed(), when grabbing invisible windows for instance
        if (Q_UNLIKELY(debug_loop()))
            qDebug("renderWindow - not exposed, abort");
        return;
    }

    if (!data.grabOnly)
        wd->flushFrameSynchronousEvents();

    QElapsedTimer renderTimer;
    qint64 renderTime = 0, syncTime = 0, polishTime = 0;
    const bool profileFrames = debug_time();
    if (profileFrames)
        renderTimer.start();
    Q_QUICK_SG_PROFILE_START(QQuickProfiler::SceneGraphPolishFrame);

    wd->polishItems();

    if (profileFrames)
        polishTime = renderTimer.nsecsElapsed();
    Q_QUICK_SG_PROFILE_SWITCH(QQuickProfiler::SceneGraphPolishFrame,
                              QQuickProfiler::SceneGraphRenderLoopFrame,
                              QQuickProfiler::SceneGraphPolishPolish);

    emit window->afterAnimating();

    // Recover from device loss.
    if (!data.engine->hasResources()) {
        if (Q_UNLIKELY(debug_loop()))
            qDebug("sync - device was lost, resetting scenegraph");
        wd->cleanupNodesOnShutdown();
        data.rc->invalidate();
    }

    data.rc->initialize(nullptr);

    wd->syncSceneGraph();

    if (profileFrames)
        syncTime = renderTimer.nsecsElapsed();
    Q_QUICK_SG_PROFILE_RECORD(QQuickProfiler::SceneGraphRenderLoopFrame,
                              QQuickProfiler::SceneGraphRenderLoopSync);

    wd->renderSceneGraph(window->size());

    if (profileFrames)
        renderTime = renderTimer.nsecsElapsed();
    Q_QUICK_SG_PROFILE_RECORD(QQuickProfiler::SceneGraphRenderLoopFrame,
                              QQuickProfiler::SceneGraphRenderLoopRender);

    if (data.grabOnly) {
        m_grabContent = data.engine->executeAndWaitReadbackRenderTarget();
        data.grabOnly = false;
    }

    Q_QUICK_SG_PROFILE_END(QQuickProfiler::SceneGraphRenderLoopFrame,
                           QQuickProfiler::SceneGraphRenderLoopSwap);

    if (Q_UNLIKELY(debug_time())) {
        static QTime lastFrameTime = QTime::currentTime();
        qDebug("Frame rendered with 'd3d12' renderloop in %dms, polish=%d, sync=%d, render=%d, frameDelta=%d",
               int(polishTime / 1000000),
               int((syncTime - polishTime) / 1000000),
               int((renderTime - syncTime) / 1000000),
               int(lastFrameTime.msecsTo(QTime::currentTime())));
        lastFrameTime = QTime::currentTime();
    }

    // Simulate device loss if requested.
    static int devLossTest = qEnvironmentVariableIntValue("QT_D3D_TEST_DEVICE_LOSS");
    if (devLossTest > 0) {
        static QElapsedTimer kt;
        static bool timerRunning = false;
        if (!timerRunning) {
            kt.start();
            timerRunning = true;
        } else if (kt.elapsed() > 5000) {
            --devLossTest;
            kt.restart();
            data.engine->simulateDeviceLoss();
        }
    }
}

int QSGD3D11RenderLoop::flags() const
{
    return SupportsGrabWithoutExpose;
}

QT_END_NAMESPACE
