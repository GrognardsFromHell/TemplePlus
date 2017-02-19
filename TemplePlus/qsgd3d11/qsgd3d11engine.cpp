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

#include "qsgd3d11engine_p.h"
#include "qsgd3d11engine_p_p.h"
#include "cs_mipmapgen.hlslh"
#include <QString>
#include <QColor>
#include <QLoggingCategory>
#include <qmath.h>
#include <qalgorithms.h>

// Comment out to disable DeviceLossTester functionality in order to reduce
// code size and improve startup perf a tiny bit.
#define DEVLOSS_TEST

#ifdef DEVLOSS_TEST
#include "cs_tdr.hlslh"
#endif

#ifdef Q_OS_WINRT
#include <QtCore/private/qeventdispatcher_winrt_p.h>
#include <functional>
#include <windows.ui.xaml.h>
#include <windows.ui.xaml.media.dxinterop.h>
#endif

#include <comdef.h>

QT_BEGIN_NAMESPACE

// NOTE: Avoid categorized logging. It is slow.

#define DECLARE_DEBUG_VAR(variable) \
    static bool debug_ ## variable() \
    { static bool value = qgetenv("QSG_RENDERER_DEBUG").contains(QT_STRINGIFY(variable)); return value; }

DECLARE_DEBUG_VAR(render)
DECLARE_DEBUG_VAR(descheap)
DECLARE_DEBUG_VAR(buffer)
DECLARE_DEBUG_VAR(texture)

// Except for system info on startup.
Q_LOGGING_CATEGORY(QSG_LOG_INFO_GENERAL, "qt.scenegraph.general")


// Any changes to the defaults below must be reflected in adaptations.qdoc as
// well and proven by qmlbench or similar.

static const int DEFAULT_SWAP_CHAIN_BUFFER_COUNT = 3;
static const int DEFAULT_FRAME_IN_FLIGHT_COUNT = 2;
static const int DEFAULT_WAITABLE_SWAP_CHAIN_MAX_LATENCY = 0;

static const int MAX_DRAW_CALLS_PER_LIST = 4096;

static const int MAX_CACHED_ROOTSIG = 16;
static const int MAX_CACHED_PSO = 64;

static const int GPU_CBVSRVUAV_DESCRIPTORS = 512;

static const DXGI_FORMAT RT_COLOR_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;

static const int BUCKETS_PER_HEAP = 8; // must match freeMap
static const int DESCRIPTORS_PER_BUCKET = 32; // the bit map (freeMap) is quint32
static const int MAX_DESCRIPTORS_PER_HEAP = BUCKETS_PER_HEAP * DESCRIPTORS_PER_BUCKET;

static bool d3dDebugLayer() {
    static const bool debugLayer = qEnvironmentVariableIntValue("QT_D3D_DEBUG") != 0;
    return debugLayer;
}

static QString comErrorMessage(HRESULT hr)
{
#ifndef Q_OS_WINRT
    const _com_error comError(hr);
#else
    const _com_error comError(hr, nullptr);
#endif
    QString result = QLatin1String("Error 0x") + QString::number(ulong(hr), 16);
    if (const wchar_t *msg = comError.ErrorMessage())
        result += QLatin1String(": ") + QString::fromWCharArray(msg);
    return result;
}

// One device per process, one everything else (engine) per window.
Q_GLOBAL_STATIC(QSGD3D11DeviceManager, deviceManager)

void QSGD3D11DeviceManager::deviceLossDetected()
{
    for (DeviceLossObserver *observer : qAsConst(m_observers))
        observer->deviceLost();

    // Nothing else to do here. All windows are expected to release their
    // resources and call unref() in response immediately.
}

void QSGD3D11DeviceManager::registerDeviceLossObserver(DeviceLossObserver *observer)
{
    if (!m_observers.contains(observer))
        m_observers.append(observer);
}

QSGD3D11Engine::QSGD3D11Engine(ID3D11Device *device, ID3D11DeviceContext *context, IDXGISwapChain *swapChain)
{
    d = new QSGD3D11EnginePrivate(device, context, swapChain);
}

QSGD3D11Engine::~QSGD3D11Engine()
{    
    d->releaseResources();
    delete d;
}

bool QSGD3D11Engine::attachToWindow(const QSize &size, float dpr, int surfaceFormatSamples, bool alpha)
{
    if (d->isInitialized()) {
        qWarning("QSGD3D11Engine: Cannot attach active engine to window");
        return false;
    }

    d->initialize(size, dpr, surfaceFormatSamples, alpha);
    return d->isInitialized();
}

void QSGD3D11Engine::releaseResources()
{
    d->releaseResources();
}

bool QSGD3D11Engine::hasResources() const
{
    // An explicit releaseResources() or a device loss results in initialized == false.
    return d->isInitialized();
}

void QSGD3D11Engine::setWindowSize(const QSize &size, float dpr)
{
    d->setWindowSize(size, dpr);
}

QSize QSGD3D11Engine::windowSize() const
{
    return d->currentWindowSize();
}

float QSGD3D11Engine::windowDevicePixelRatio() const
{
    return d->currentWindowDpr();
}

uint QSGD3D11Engine::windowSamples() const
{
    return d->currentWindowSamples();
}

void QSGD3D11Engine::beginFrame()
{
    d->beginFrame();
}

void QSGD3D11Engine::endFrame()
{
    d->endFrame();
}

void QSGD3D11Engine::beginLayer()
{
    d->beginLayer();
}

void QSGD3D11Engine::endLayer()
{
    d->endLayer();
}

void QSGD3D11Engine::invalidateCachedFrameState()
{
    d->invalidateCachedFrameState();
}

void QSGD3D11Engine::restoreFrameState(bool minimal)
{
    d->restoreFrameState(minimal);
}

void QSGD3D11Engine::finalizePipeline(const QSGD3D11PipelineState &pipelineState)
{
    d->finalizePipeline(pipelineState);
}

uint QSGD3D11Engine::genVertexBuffer()
{
    return d->genBuffer(D3D11_BIND_VERTEX_BUFFER);
}

uint QSGD3D11Engine::genIndexBuffer()
{
    return d->genBuffer(D3D11_BIND_INDEX_BUFFER);
}

uint QSGD3D11Engine::genConstantBuffer()
{
    return d->genBuffer(D3D11_BIND_CONSTANT_BUFFER);
}

void QSGD3D11Engine::releaseBuffer(uint id)
{
    d->releaseBuffer(id);
}

void QSGD3D11Engine::resetBuffer(uint id, const quint8 *data, int size)
{
    d->resetBuffer(id, data, size);
}

void QSGD3D11Engine::markBufferDirty(uint id, int offset, int size)
{
    d->markBufferDirty(id, offset, size);
}

void QSGD3D11Engine::queueViewport(const QRect &rect)
{
    d->queueViewport(rect);
}

void QSGD3D11Engine::queueScissor(const QRect &rect)
{
    d->queueScissor(rect);
}

void QSGD3D11Engine::queueSetRenderTarget(uint id)
{
    d->queueSetRenderTarget(id);
}

void QSGD3D11Engine::queueClearRenderTarget(const QColor &color)
{
    d->queueClearRenderTarget(color);
}

void QSGD3D11Engine::queueClearDepthStencil(float depthValue, quint8 stencilValue, ClearFlags which)
{
    d->queueClearDepthStencil(depthValue, stencilValue, which);
}

void QSGD3D11Engine::queueSetBlendFactor(const QVector4D &factor)
{
    d->queueSetBlendFactor(factor);
}

void QSGD3D11Engine::queueSetStencilRef(quint32 ref)
{
    d->queueSetStencilRef(ref);
}

void QSGD3D11Engine::queueDraw(const DrawParams &params)
{
    d->queueDraw(params);
}

void QSGD3D11Engine::present()
{
    d->present();
}

uint QSGD3D11Engine::genTexture()
{
    return d->genTexture();
}

void QSGD3D11Engine::createTexture(uint id, const QSize &size, QImage::Format format, TextureCreateFlags flags)
{
    d->createTexture(id, size, format, flags);
}

void QSGD3D11Engine::queueTextureResize(uint id, const QSize &size)
{
    d->queueTextureResize(id, size);
}

void QSGD3D11Engine::queueTextureUpload(uint id, const QImage &image, const QPoint &dstPos, TextureUploadFlags flags)
{
    d->queueTextureUpload(id, QVector<QImage>() << image, QVector<QPoint>() << dstPos, flags);
}

void QSGD3D11Engine::queueTextureUpload(uint id, const QVector<QImage> &images, const QVector<QPoint> &dstPos,
                                        TextureUploadFlags flags)
{
    d->queueTextureUpload(id, images, dstPos, flags);
}

void QSGD3D11Engine::releaseTexture(uint id)
{
    d->releaseTexture(id);
}

void QSGD3D11Engine::useTexture(uint id)
{
    d->useTexture(id);
}

uint QSGD3D11Engine::genRenderTarget()
{
    return d->genRenderTarget();
}

void QSGD3D11Engine::createRenderTarget(uint id, const QSize &size, uint samples)
{
    d->createRenderTarget(id, size, samples);
}

void QSGD3D11Engine::releaseRenderTarget(uint id)
{
    d->releaseRenderTarget(id);
}

void QSGD3D11Engine::useRenderTargetAsTexture(uint id)
{
    d->useRenderTargetAsTexture(id);
}

uint QSGD3D11Engine::activeRenderTarget() const
{
    return d->activeRenderTarget();
}

QImage QSGD3D11Engine::executeAndWaitReadbackRenderTarget(uint id)
{
    return d->executeAndWaitReadbackRenderTarget(id);
}

void QSGD3D11Engine::simulateDeviceLoss()
{
    d->simulateDeviceLoss();
}

void *QSGD3D11Engine::getResource(QQuickWindow *, QSGRendererInterface::Resource resource) const
{
    return d->getResource(resource);
}

static inline quint32 alignedSize(quint32 size, quint32 byteAlign)
{
    return (size + byteAlign - 1) & ~(byteAlign - 1);
}

quint32 QSGD3D11Engine::alignedConstantBufferSize(quint32 size)
{
    return alignedSize(size, 256);
}

QSGD3D11Format QSGD3D11Engine::toDXGIFormat(QSGGeometry::Type sgtype, int tupleSize, int *size)
{
    QSGD3D11Format format = FmtUnknown;

    static const QSGD3D11Format formatMap_ub[] = { FmtUnknown,
                                                   FmtUNormByte,
                                                   FmtUNormByte2,
                                                   FmtUnknown,
                                                   FmtUNormByte4 };

    static const QSGD3D11Format formatMap_f[] = { FmtUnknown,
                                                  FmtFloat,
                                                  FmtFloat2,
                                                  FmtFloat3,
                                                  FmtFloat4 };

    switch (sgtype) {
    case QSGGeometry::UnsignedByteType:
        format = formatMap_ub[tupleSize];
        if (size)
            *size = tupleSize;
        break;
    case QSGGeometry::FloatType:
        format = formatMap_f[tupleSize];
        if (size)
            *size = sizeof(float) * tupleSize;
        break;

    case QSGGeometry::UnsignedShortType:
        format = FmtUnsignedShort;
        if (size)
            *size = sizeof(ushort) * tupleSize;
        break;
    case QSGGeometry::UnsignedIntType:
        format = FmtUnsignedInt;
        if (size)
            *size = sizeof(uint) * tupleSize;
        break;

    case QSGGeometry::ByteType:
    case QSGGeometry::IntType:
    case QSGGeometry::ShortType:
        qWarning("no mapping for GL type 0x%x", sgtype);
        break;

    default:
        qWarning("unknown GL type 0x%x", sgtype);
        break;
    }

    return format;
}

int QSGD3D11Engine::mipMapLevels(const QSize &size)
{
    return ceil(log2(qMax(size.width(), size.height()))) + 1;
}

inline static bool isPowerOfTwo(int x)
{
    // Assumption: x >= 1
    return x == (x & -x);
}

QSize QSGD3D11Engine::mipMapAdjustedSourceSize(const QSize &size)
{
    if (size.isEmpty())
        return size;

    QSize adjustedSize = size;

    // ### for now only power-of-two sizes are mipmap-capable
    if (!isPowerOfTwo(size.width()))
        adjustedSize.setWidth(qNextPowerOfTwo(size.width()));
    if (!isPowerOfTwo(size.height()))
        adjustedSize.setHeight(qNextPowerOfTwo(size.height()));

    return adjustedSize;
}

void QSGD3D11EnginePrivate::releaseResources()
{
    if (!initialized)
        return;

    mipmapper.releaseResources();
    devLossTest.releaseResources();
	
    buffers.clear();
    textures.clear();
    renderTargets.clear();
	
    initialized = false;

    // 'window' must be kept, may just be a device loss
}

void QSGD3D11EnginePrivate::initialize(const QSize &size, float dpr, int surfaceFormatSamples, bool alpha)
{
    if (initialized)
        return;

    windowSize = size;
    windowDpr = dpr;
    windowSamples = qMax(1, surfaceFormatSamples); // may be -1 or 0, whereas windowSamples is uint and >= 1
    windowAlpha = alpha;

    swapChainBufferCount = qMin(qEnvironmentVariableIntValue("QT_D3D_BUFFER_COUNT"), MAX_SWAP_CHAIN_BUFFER_COUNT);
    if (swapChainBufferCount < 2)
        swapChainBufferCount = DEFAULT_SWAP_CHAIN_BUFFER_COUNT;

    frameInFlightCount = qMin(qEnvironmentVariableIntValue("QT_D3D_FRAME_COUNT"), MAX_FRAME_IN_FLIGHT_COUNT);
    if (frameInFlightCount < 1)
        frameInFlightCount = DEFAULT_FRAME_IN_FLIGHT_COUNT;

    static const char *latReqEnvVar = "QT_D3D_WAITABLE_SWAP_CHAIN_MAX_LATENCY";
    if (!qEnvironmentVariableIsSet(latReqEnvVar))
        waitableSwapChainMaxLatency = DEFAULT_WAITABLE_SWAP_CHAIN_MAX_LATENCY;
    else
        waitableSwapChainMaxLatency = qBound(0, qEnvironmentVariableIntValue(latReqEnvVar), 16);

    if (qEnvironmentVariableIsSet("QSG_INFO"))
        const_cast<QLoggingCategory &>(QSG_LOG_INFO_GENERAL()).setEnabled(QtDebugMsg, true);

    qCDebug(QSG_LOG_INFO_GENERAL, "d3d12 engine init. swap chain buffer count %d, max frames prepared without blocking %d",
            swapChainBufferCount, frameInFlightCount);
    if (waitableSwapChainMaxLatency)
        qCDebug(QSG_LOG_INFO_GENERAL, "Swap chain frame latency waitable object enabled. Frame latency is %d", waitableSwapChainMaxLatency);

	deviceManager()->registerDeviceLossObserver(this);

    if (d3dDebugLayer()) {

    }

    setupDefaultRenderTargets();

    ComPtr<ID3D11DeviceContext> deferredContext;
    if (FAILED(device->CreateDeferredContext(0, &deferredContext))) {
        qWarning("Unable to create deferred context");
        return;
    }
    if (FAILED(deferredContext.CopyTo(IID_PPV_ARGS(&drawContext)))) {
        qWarning("Unable to get ID3D11DeviceContext1 for deferred draw context");
        return;
    }

    frameIndex = 0;

    if (!mipmapper.initialize(this))
        return;

    if (!devLossTest.initialize(this))
        return;

    currentRenderTarget = 0;

    initialized = true;
}

DXGI_SAMPLE_DESC QSGD3D11EnginePrivate::makeSampleDesc(DXGI_FORMAT format, uint samples)
{
    DXGI_SAMPLE_DESC sampleDesc;
    sampleDesc.Count = 1;
    sampleDesc.Quality = 0;

    if (samples > 1) {
        UINT numQualityLevels;

        if (SUCCEEDED(device->CheckMultisampleQualityLevels(format, samples, &numQualityLevels))) {
            if (numQualityLevels > 0) {
                sampleDesc.Count = samples;
                sampleDesc.Quality = numQualityLevels - 1;
            } else {
                qWarning("No quality levels for multisampling with sample count %d", samples);
            }
        } else {
            qWarning("Failed to query multisample quality levels for sample count %d", samples);
        }
    }

    return sampleDesc;
}

auto QSGD3D11EnginePrivate::createColorBuffer(const QSize &size, uint samples) -> TextureAndRTV
{
    CD3D11_TEXTURE2D_DESC rtDesc(
        RT_COLOR_FORMAT,
        size.width(),
        size.height()
    );
    rtDesc.MipLevels = 1;
    rtDesc.SampleDesc = makeSampleDesc(rtDesc.Format, samples);
    if (samples <= 1) {
        // Multi-sampled RTs cannot be used directly as shader resources, they have to be resolved first
        rtDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    } else {
        rtDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
    }

    TextureAndRTV result;
    if (FAILED(device->CreateTexture2D(&rtDesc, nullptr, &result.first))) {
        qWarning("Failed to create offscreen render target of size %dx%d", size.width(), size.height());
        return result;
    }

    if (FAILED(device->CreateRenderTargetView(result.first.Get(), nullptr, &result.second))) {
        qWarning("Failed to create offscreen render target view of size %dx%d", size.width(), size.height());
        return result;
    }

    return result;
}

auto QSGD3D11EnginePrivate::createDepthStencil(const QSize &size, uint samples) -> TextureAndDSV
{
    CD3D11_TEXTURE2D_DESC bufDesc(
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        size.width(),
        size.height()
    );
    bufDesc.MipLevels = 1;
    bufDesc.SampleDesc = makeSampleDesc(bufDesc.Format, samples);
    bufDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    TextureAndDSV result;
    if (FAILED(device->CreateTexture2D(&bufDesc, nullptr, &result.first))) {
        qWarning("Failed to create depth-stencil buffer of size %dx%d", size.width(), size.height());
        return result;
    }

    CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDesc(
        bufDesc.SampleDesc.Count <= 1 ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS,
        DXGI_FORMAT_D24_UNORM_S8_UINT
    );

    if (FAILED(device->CreateDepthStencilView(result.first.Get(), &depthStencilDesc, &result.second))) {
        qWarning("Failed to create depth-stencil view of size %dx%d", size.width(), size.height());
        return result;
    }

    return result;
}

void QSGD3D11EnginePrivate::setupDefaultRenderTargets()
{
    const QSize size(windowSize.width() * windowDpr, windowSize.height() * windowDpr);
    
	defaultRenderTarget = genRenderTarget();

	createRenderTarget(defaultRenderTarget, size, windowSamples);

    presentFrameIndex = 0;
}

void QSGD3D11EnginePrivate::setWindowSize(const QSize &size, float dpr)
{
    if (!initialized || (windowSize == size && windowDpr == dpr))
        return;

    windowSize = size;
    windowDpr = dpr;

    if (Q_UNLIKELY(debug_render()))
        qDebug() << "resize" << size << dpr;

    // Clear these, otherwise resizing will fail.
	releaseRenderTarget(defaultRenderTarget);

	setupDefaultRenderTargets();
}

void QSGD3D11EnginePrivate::deviceLost()
{
    qWarning("D3D device lost, will attempt to reinitialize");

    // Release all resources. This is important because otherwise reinitialization may fail.
    releaseResources();

    // Now in uninitialized state (but 'window' is still valid). Will recreate
    // all the resources on the next beginFrame().
}

void QSGD3D11EnginePrivate::resolveMultisampledTarget(ID3D11Resource *msaa,
                                                      ID3D11Resource *resolve) const
{
    drawContext->ResolveSubresource(resolve, 0, msaa, 0, RT_COLOR_FORMAT);
}

ID3D11Buffer *QSGD3D11EnginePrivate::createBuffer(int size, UINT usage)
{
    ID3D11Buffer *buf;

    CD3D11_BUFFER_DESC bufDesc(size, usage);
    bufDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    HRESULT hr = device->CreateBuffer(&bufDesc, nullptr, &buf);
    if (FAILED(hr))
        qWarning("Failed to create buffer resource: %s", qPrintable(comErrorMessage(hr)));

    return buf;
}

void QSGD3D11EnginePrivate::ensureBuffer(Buffer *buf)
{
    Buffer::InFlightData &bfd(buf->d[currentPFrameIndex]);
    // Only enlarge, never shrink
    const bool newBufferNeeded = bfd.buffer ? (buf->cpuDataRef.size > bfd.resourceSize) : true;
    if (newBufferNeeded) {
        // Round it up and overallocate a little bit so that a subsequent
        // buffer contents rebuild with a slightly larger total size does
        // not lead to creating a new buffer.
        const quint32 sz = alignedSize(buf->cpuDataRef.size, 4096);
        if (Q_UNLIKELY(debug_buffer()))
            qDebug("new buffer[pf=%d] of size %d (actual data size %d)", currentPFrameIndex, sz, buf->cpuDataRef.size);
        bfd.buffer.Attach(createBuffer(sz, buf->bindFlags));
        bfd.resourceSize = sz;
    }
    // Cache the actual data size in the per-in-flight-frame data as well.
    bfd.dataSize = buf->cpuDataRef.size;
}

void QSGD3D11EnginePrivate::updateBuffer(Buffer *buf)
{
    if (buf->cpuDataRef.dirty.isEmpty())
        return;

    Buffer::InFlightData &bfd(buf->d[currentPFrameIndex]);
    D3D11_MAPPED_SUBRESOURCE mapped;
    if (FAILED(context->Map(bfd.buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        qWarning("Unable to map buffer for update.");
    }

    auto data = reinterpret_cast<quint8*>(mapped.pData);
    memcpy(data, buf->cpuDataRef.p, buf->cpuDataRef.size);

    // D3D12 can use partial updates here since it can guarantee that
    // the buffer is not being used by the GPU while it's writing to it,
    // but since D3D11 has no such guarantees (and no way to guarantee it),
    // we need to replace the entirety of the buffer and let the graphics driver
    // handle it...
    /*for (auto &dirty : buf->cpuDataRef.dirty) {
        auto offset = dirty.first;
        auto size = dirty.second;
        memcpy(data + offset, buf->cpuDataRef.p + offset, size);
    }*/

    context->Unmap(bfd.buffer.Get(), 0);

    buf->cpuDataRef.dirty.clear();
}

void QSGD3D11EnginePrivate::ensureDevice()
{
    if (!initialized)
        initialize(windowSize, windowDpr, windowSamples, windowAlpha);
}

void QSGD3D11EnginePrivate::beginFrame()
{
    if (inFrame && !activeLayers)
        qFatal("beginFrame called again without an endFrame, frame index was %d", frameIndex);

    if (Q_UNLIKELY(debug_render()))
        qDebug() << "***** begin frame, logical" << frameIndex << "present" << presentFrameIndex << "layer" << activeLayers;

    if (inFrame && activeLayers) {
        if (Q_UNLIKELY(debug_render()))
            qDebug("frame %d already in progress", frameIndex);
        if (!currentLayerDepth) {
            // There are layers and the real frame preparation starts now. Prepare for present.
            beginFrameDraw();
        }
        return;
    }

    inFrame = true;

    // The device may have been lost. This is the point to attempt to start
    // again from scratch. Except when it is not. Operations that can happen
    // out of frame (e.g. textures, render targets) may trigger reinit earlier
    // than beginFrame.
    ensureDevice();

    // Block if needed. With 2 frames in flight frame N waits for frame N - 2, but not N - 1, to finish.
    currentPFrameIndex = frameIndex % frameInFlightCount;

    PersistentFrameData &pfd(pframeData[currentPFrameIndex]);

    for (Buffer &b : buffers) {
        if (b.entryInUse())
            b.d[currentPFrameIndex].dirty.clear();
    }

    if (frameIndex >= frameInFlightCount - 1) {
        // Now sync the buffer changes from the previous, potentially still in
        // flight, frames. This is done by taking the ranges dirtied in those
        // frames and adding them to the global CPU-side buffer's dirty list,
        // as if this frame changed those ranges. (however, dirty ranges
        // inherited this way are not added to this frame's persistent
        // per-frame dirty list because the next frame after this one should
        // inherit this frame's genuine changes only, the rest will come from
        // the earlier ones)
        for (int delta = frameInFlightCount - 1; delta >= 1; --delta) {
            const int prevPFrameIndex = (frameIndex - delta) % frameInFlightCount;
            PersistentFrameData &prevFrameData(pframeData[prevPFrameIndex]);
            for (uint id : qAsConst(prevFrameData.buffersUsedInFrame)) {
                Buffer &b(buffers[id - 1]);
                if (b.d[currentPFrameIndex].buffer && b.d[currentPFrameIndex].dataSize == b.cpuDataRef.size) {
                    if (Q_UNLIKELY(debug_buffer()))
                        qDebug() << "frame" << frameIndex << "takes dirty" << b.d[prevPFrameIndex].dirty
                                 << "from frame" << frameIndex - delta << "for buffer" << id;
                    for (const auto &range : qAsConst(b.d[prevPFrameIndex].dirty))
                        addDirtyRange(&b.cpuDataRef.dirty, range.first, range.second, b.cpuDataRef.size);
                } else {
                    if (Q_UNLIKELY(debug_buffer()))
                        qDebug() << "frame" << frameIndex << "makes all dirty from frame" << frameIndex - delta
                                 << "for buffer" << id;
                    addDirtyRange(&b.cpuDataRef.dirty, 0, b.cpuDataRef.size, b.cpuDataRef.size);
                }
            }
        }
    }

    if (frameIndex >= frameInFlightCount) {
        // Do some texture upload bookkeeping.
        const quint64 finishedFrameIndex = frameIndex - frameInFlightCount; // we know since we just blocked for this
        // pfd conveniently refers to the same slot that was used by that frame
        if (!pfd.pendingTextureUploads.isEmpty()) {
            if (Q_UNLIKELY(debug_texture()))
                qDebug("Removing texture upload data for frame %d", finishedFrameIndex);
            for (uint id : qAsConst(pfd.pendingTextureUploads)) {
                const int idx = id - 1;
                Texture &t(textures[idx]);
                // fenceValue is 0 when the previous frame cleared it, skip in
                // this case. Skip also when fenceValue > the value it was when
                // adding the last GPU wait - this is the case when more
                // uploads were queued for the same texture in the meantime.
                if (t.fenceValue && t.fenceValue == t.lastWaitFenceValue) {
                    t.fenceValue = 0;
                    t.lastWaitFenceValue = 0;
                    t.stagingBuffers.clear();
                    if (Q_UNLIKELY(debug_texture()))
                        qDebug("Cleaned staging data for texture %u", id);
                }
            }
            pfd.pendingTextureUploads.clear();
            if (!pfd.pendingTextureMipMap.isEmpty()) {
                if (Q_UNLIKELY(debug_texture()))
                    qDebug() << "cleaning mipmap generation data for " << pfd.pendingTextureMipMap;
                // no special cleanup is needed as mipmap generation uses the frame's resources
                pfd.pendingTextureMipMap.clear();
            }
            bool hasPending = false;
            for (int delta = 1; delta < frameInFlightCount; ++delta) {
                const PersistentFrameData &prevFrameData(pframeData[(frameIndex - delta) % frameInFlightCount]);
                if (!prevFrameData.pendingTextureUploads.isEmpty()) {
                    hasPending = true;
                    break;
                }
            }
        }

        // Mark released texture, buffer, etc. slots free.
        if (!pfd.pendingReleases.isEmpty()) {
            for (const auto &pr : qAsConst(pfd.pendingReleases)) {
                Q_ASSERT(pr.id);
                if (pr.type == PersistentFrameData::PendingRelease::TypeTexture) {
                    Texture &t(textures[pr.id - 1]);
                    Q_ASSERT(t.entryInUse());
                    t.flags &= ~RenderTarget::EntryInUse; // createTexture() can now reuse this entry
                    t.texture = nullptr;
                } else if (pr.type == PersistentFrameData::PendingRelease::TypeBuffer) {
                    Buffer &b(buffers[pr.id - 1]);
                    Q_ASSERT(b.entryInUse());
                    b.flags &= ~Buffer::EntryInUse;
                    for (int i = 0; i < frameInFlightCount; ++i)
                        b.d[i].buffer = nullptr;
                } else {
                    qFatal("Corrupt pending release list, type %d", pr.type);
                }
            }
            pfd.pendingReleases.clear();
        }
        if (!pfd.outOfFramePendingReleases.isEmpty()) {
            pfd.pendingReleases = pfd.outOfFramePendingReleases;
            pfd.outOfFramePendingReleases.clear();
        }
    }

    pfd.buffersUsedInFrame.clear();

    beginDrawCalls();

    // Prepare for present if this is a frame without layers.
    if (!activeLayers)
        beginFrameDraw();
}

void QSGD3D11EnginePrivate::beginDrawCalls()
{
    // TODO: Fix
    //frameCommandList->Reset(frameCommandAllocator[frameIndex % frameInFlightCount].Get(), nullptr);
    //commandList = frameCommandList.Get();
    invalidateCachedFrameState();
}

void QSGD3D11EnginePrivate::invalidateCachedFrameState()
{
    tframeData.drawingMode = QSGGeometry::DrawingMode(-1);
    tframeData.currentIndexBuffer = 0;
    tframeData.activeTextureCount = 0;
    tframeData.drawCount = 0;
    tframeData.descHeapSet = false;
}

void QSGD3D11EnginePrivate::restoreFrameState(bool minimal)
{
    queueSetRenderTarget(currentRenderTarget);
    if (!minimal) {
        queueViewport(tframeData.viewport);
        queueScissor(tframeData.scissor);
        queueSetBlendFactor(tframeData.blendFactor);
        queueSetStencilRef(tframeData.stencilRef);
    }
    finalizePipeline(tframeData.pipelineState);
}

void QSGD3D11EnginePrivate::beginFrameDraw()
{
	queueSetRenderTarget(defaultRenderTarget);
}

void QSGD3D11EnginePrivate::endFrame()
{
    if (!inFrame)
        qFatal("endFrame called without beginFrame, frame index %d", frameIndex);

    if (Q_UNLIKELY(debug_render()))
        qDebug("***** end frame");

    endDrawCalls(true);

    // TODO commandQueue->Signal(frameFence[frameIndex % frameInFlightCount]->fence.Get(), frameIndex + 1);
    ++frameIndex;

    inFrame = false;
}

void QSGD3D11EnginePrivate::endDrawCalls(bool lastInFrame)
{
    PersistentFrameData &pfd(pframeData[currentPFrameIndex]);

    // Now is the time to sync all the changed areas in the buffers.
    if (Q_UNLIKELY(debug_buffer()))
        qDebug() << "buffers used in drawcall set" << pfd.buffersUsedInDrawCallSet;
    for (uint id : qAsConst(pfd.buffersUsedInDrawCallSet))
        updateBuffer(&buffers[id - 1]);

    pfd.buffersUsedInFrame += pfd.buffersUsedInDrawCallSet;
    pfd.buffersUsedInDrawCallSet.clear();

    if (!pfd.pendingTextureUploads.isEmpty()) {
        for (uint id : qAsConst(pfd.pendingTextureUploads)) {
            const int idx = id - 1;
            Texture &t(textures[idx]);
            if (t.mipmap())
                pfd.pendingTextureMipMap.insert(id);
        }

        // Generate mipmaps when necessary.
        if (!pfd.pendingTextureMipMap.isEmpty()) {
            if (Q_UNLIKELY(debug_texture()))
                qDebug() << "starting mipmap generation for" << pfd.pendingTextureMipMap;
            for (uint id : qAsConst(pfd.pendingTextureMipMap))
                mipmapper.generate(textures[id - 1]);
        }
    }

    // Fire off our deferred draw commands
    ComPtr<ID3D11CommandList> commandList;
    if (FAILED(drawContext->FinishCommandList(FALSE, &commandList))) {
        qWarning("Unable to finish command list!");
        return;
    }
    context->ExecuteCommandList(commandList.Get(), FALSE);

    if (lastInFrame) {
        if (activeLayers) {
            if (Q_UNLIKELY(debug_render()))
                qDebug("this frame had %d layers", activeLayers);
            activeLayers = 0;
        }
    }
}

ID3D11ShaderResourceView * QSGD3D11EnginePrivate::backBuffer()
{
	Q_ASSERT(defaultRenderTarget);
	const int idx = defaultRenderTarget - 1;
	Q_ASSERT(idx < renderTargets.count());
	RenderTarget &rt(renderTargets[idx]);
	Q_ASSERT(rt.entryInUse() && rt.color);

	if (rt.flags & RenderTarget::NeedsReadBarrier) {
		rt.flags &= ~RenderTarget::NeedsReadBarrier;
		if (rt.flags & RenderTarget::Multisample) {
			resolveMultisampledTarget(rt.color.Get(), rt.colorResolve.Get());
		}
	}

	return rt.srv.Get();
}

void QSGD3D11EnginePrivate::beginLayer()
{
    if (inFrame && !activeLayers)
        qFatal("Layer rendering cannot be started while a frame is active");

    if (Q_UNLIKELY(debug_render()))
        qDebug("===== beginLayer active %d depth %d (inFrame=%d)", activeLayers, currentLayerDepth, inFrame);

    ++activeLayers;
    ++currentLayerDepth;

    // Do an early beginFrame. With multiple layers this results in
    // beginLayer - beginFrame - endLayer - beginLayer - beginFrame - endLayer - ... - (*) beginFrame - endFrame
    // where (*) denotes the start of the preparation of the actual, non-layer frame.

    if (activeLayers == 1)
        beginFrame();
}

void QSGD3D11EnginePrivate::endLayer()
{
    if (!inFrame || !activeLayers || !currentLayerDepth)
        qFatal("Mismatched endLayer");

    if (Q_UNLIKELY(debug_render()))
        qDebug("===== endLayer active %d depth %d", activeLayers, currentLayerDepth);

    --currentLayerDepth;

    // Do not touch activeLayers. It remains valid until endFrame.
}

// Root signature:
// [0] CBV - always present
// [1] table with one SRV per texture (must be a table since root descriptor SRVs cannot be textures) - optional
// one static sampler per texture - optional
//
// SRVs can be created freely via QSGD3D11CPUDescriptorHeapManager and stored
// in QSGD3D11TextureView. The engine will copy them onto a dedicated,
// shader-visible CBV-SRV-UAV heap in the correct order.

void QSGD3D11EnginePrivate::finalizePipeline(const QSGD3D11PipelineState &pipelineState)
{
    if (!inFrame) {
        qWarning("%s: Cannot be called outside begin/endFrame", __FUNCTION__);
        return;
    }

    tframeData.pipelineState = pipelineState;

    auto cachedSamplerState = samplerStateCache[pipelineState.shaders.rootSig];
    if (!cachedSamplerState) {
        if (Q_UNLIKELY(debug_render()))
            qDebug("NEW SAMPLER STATE");

        cachedSamplerState = new SamplerStateCacheEntry;

        // Mixing up samplers and resource views in QSGD3D11TextureView means
        // that the number of static samplers has to match the number of
        // textures. This is not really ideal in general but works for Quick's use cases.
        // The shaders can still choose to declare and use fewer samplers, if they want to.
            Q_ASSERT(pipelineState.shaders.rootSig.textureViewCount <= _countof(cachedSamplerState->samplerStates));
            for (int i = 0; i < pipelineState.shaders.rootSig.textureViewCount; ++i) {
                const QSGD3D11TextureView &tv(pipelineState.shaders.rootSig.textureViews[i]);
                D3D11_SAMPLER_DESC sd = {};
                sd.Filter = D3D11_FILTER(tv.filter);
                sd.AddressU = D3D11_TEXTURE_ADDRESS_MODE(tv.addressModeHoriz);
                sd.AddressV = D3D11_TEXTURE_ADDRESS_MODE(tv.addressModeVert);
                sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
                sd.MinLOD = 0.0f;
                sd.MaxLOD = D3D11_FLOAT32_MAX;


                if (FAILED(device->CreateSamplerState(&sd, &cachedSamplerState->samplerStates[i]))) {
                    qWarning("Unable to create sampler state");
                }

            }
//        desc.pStaticSamplers = staticSamplers;
//        desc.Flags = D3D11_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

//        ComPtr<ID3DBlob> signature;
//        ComPtr<ID3DBlob> error;
//        if (FAILED(D3D11SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error))) {
//            QByteArray msg(static_cast<const char *>(error->GetBufferPointer()), error->GetBufferSize());
//            qWarning("Failed to serialize root signature: %s", qPrintable(msg));
//            return;
//        }
//        if (FAILED(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
//                                               IID_PPV_ARGS(&cachedRootSig->rootSig)))) {
//            qWarning("Failed to create root signature");
//            return;
//        }

        samplerStateCache.insert(pipelineState.shaders.rootSig, cachedSamplerState);
    }

    PSOCacheEntry *cachedPso = psoCache[pipelineState];
    if (!cachedPso) {
        if (Q_UNLIKELY(debug_render()))
            qDebug("NEW PSO");

        cachedPso = new PSOCacheEntry;

        D3D11_INPUT_ELEMENT_DESC inputElements[QSGD3D11_MAX_INPUT_ELEMENTS];
        int ieIdx = 0;
        for (int i = 0; i < pipelineState.inputElementCount; ++i) {
            const QSGD3D11InputElement &ie(pipelineState.inputElements[i]);
            D3D11_INPUT_ELEMENT_DESC ieDesc = {};
            ieDesc.SemanticName = ie.semanticName;
            ieDesc.SemanticIndex = ie.semanticIndex;
            ieDesc.Format = DXGI_FORMAT(ie.format);
            ieDesc.InputSlot = ie.slot;
            ieDesc.AlignedByteOffset = ie.offset;
            ieDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
            if (Q_UNLIKELY(debug_render()))
                qDebug("input [%d]: %s %d 0x%x %d", ieIdx, ie.semanticName, ie.offset, ie.format, ie.slot);
            inputElements[ieIdx++] = ieDesc;
        }

        if (FAILED(device->CreateInputLayout(inputElements, UINT(ieIdx), pipelineState.shaders.vs, pipelineState.shaders.vsSize, &cachedPso->inputLayout))) {
            qWarning("Unable to create input layout for pipeline state.");
            return;
        }

        if (FAILED(device->CreateVertexShader(pipelineState.shaders.vs, pipelineState.shaders.vsSize, nullptr, &cachedPso->vs))) {
            qWarning("Unable to compile vertex shader");
            return;
        }

        if (FAILED(device->CreatePixelShader(pipelineState.shaders.ps, pipelineState.shaders.psSize, nullptr, &cachedPso->ps))) {
            qWarning("Unable to compile vertex shader");
            return;
        }

        D3D11_RASTERIZER_DESC rastDesc = {};
        rastDesc.FillMode = D3D11_FILL_SOLID;
        rastDesc.CullMode = D3D11_CULL_MODE(pipelineState.cullMode);
        rastDesc.FrontCounterClockwise = pipelineState.frontCCW;
        rastDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
        rastDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
        rastDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rastDesc.DepthClipEnable = TRUE;
        rastDesc.ScissorEnable = TRUE;

        if (FAILED(device->CreateRasterizerState(&rastDesc, &cachedPso->rasterizerState))) {
            qWarning("Unable to create rasterizer state");
            return;
        }

        D3D11_BLEND_DESC blendDesc = {};
        if (pipelineState.blend == QSGD3D11PipelineState::BlendNone) {
            D3D11_RENDER_TARGET_BLEND_DESC noBlendDesc = {};
            noBlendDesc.RenderTargetWriteMask = pipelineState.colorWrite ? D3D11_COLOR_WRITE_ENABLE_ALL : 0;
            blendDesc.RenderTarget[0] = noBlendDesc;
        } else if (pipelineState.blend == QSGD3D11PipelineState::BlendPremul) {
            const D3D11_RENDER_TARGET_BLEND_DESC premulBlendDesc = {
                TRUE,
                D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
                D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
                UINT8(pipelineState.colorWrite ? D3D11_COLOR_WRITE_ENABLE_ALL : 0)
            };
            blendDesc.RenderTarget[0] = premulBlendDesc;
        } else if (pipelineState.blend == QSGD3D11PipelineState::BlendColor) {
            const D3D11_RENDER_TARGET_BLEND_DESC colorBlendDesc = {
                TRUE,
                D3D11_BLEND_BLEND_FACTOR, D3D11_BLEND_INV_SRC_COLOR, D3D11_BLEND_OP_ADD,
                D3D11_BLEND_BLEND_FACTOR, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD,
                UINT8(pipelineState.colorWrite ? D3D11_COLOR_WRITE_ENABLE_ALL : 0)
            };
            blendDesc.RenderTarget[0] = colorBlendDesc;
        }

        if (FAILED(device->CreateBlendState(&blendDesc, &cachedPso->blendState))) {
            qWarning("Unable to create blend state");
            return;
        }

        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable = pipelineState.depthEnable;
        depthStencilDesc.DepthWriteMask = pipelineState.depthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        depthStencilDesc.DepthFunc = D3D11_COMPARISON_FUNC(pipelineState.depthFunc);

        depthStencilDesc.StencilEnable = pipelineState.stencilEnable ? TRUE : FALSE;
        depthStencilDesc.StencilReadMask = depthStencilDesc.StencilWriteMask = 0xFF;
        D3D11_DEPTH_STENCILOP_DESC stencilOpDesc = {
            D3D11_STENCIL_OP(pipelineState.stencilFailOp),
            D3D11_STENCIL_OP(pipelineState.stencilDepthFailOp),
            D3D11_STENCIL_OP(pipelineState.stencilPassOp),
            D3D11_COMPARISON_FUNC(pipelineState.stencilFunc)
        };
        depthStencilDesc.FrontFace = depthStencilDesc.BackFace = stencilOpDesc;

        if (FAILED(device->CreateDepthStencilState(&depthStencilDesc, &cachedPso->depthStencilState))) {
            qWarning("Unable to create depth stencil state");
            return;
        }

        cachedPso->sampleMask = UINT_MAX;
        /*psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = RT_COLOR_FORMAT;
        psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        psoDesc.SampleDesc = defaultRT->GetDesc().SampleDesc;

        HRESULT hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&cachedPso->pso));
        if (FAILED(hr)) {
            qWarning("Failed to create graphics pipeline state: %s",
                     qPrintable(comErrorMessage(hr)));
            return;
        }*/

        psoCache.insert(pipelineState, cachedPso);
    }

    drawContext->VSSetShader(cachedPso->vs.Get(), nullptr, 0);
    drawContext->PSSetShader(cachedPso->ps.Get(), nullptr, 0);
    drawContext->IASetInputLayout(cachedPso->inputLayout.Get());
    // drawContext->IASetPrimitiveTopology(cachedPso->primitiveTopology);

    const float blendFactors[4] = {
        tframeData.blendFactor.x(),
        tframeData.blendFactor.y(),
        tframeData.blendFactor.z(),
        tframeData.blendFactor.w()
    };
    drawContext->OMSetBlendState(cachedPso->blendState.Get(), blendFactors, cachedPso->sampleMask);
    drawContext->RSSetState(cachedPso->rasterizerState.Get());
    drawContext->OMSetDepthStencilState(cachedPso->depthStencilState.Get(), tframeData.stencilRef);

    ID3D11SamplerState* samplerStates[QSGD3D11_MAX_TEXTURE_VIEWS];
    for (int i = 0; i < pipelineState.shaders.rootSig.textureViewCount; i++) {
        samplerStates[i] = cachedSamplerState->samplerStates[i].Get();
    }

    drawContext->PSSetSamplers(0, pipelineState.shaders.rootSig.textureViewCount, samplerStates);

//    if (cachedRootSig->rootSig.Get() != tframeData.lastRootSig) {
//        tframeData.lastRootSig = cachedRootSig->rootSig.Get();
//        commandList->SetGraphicsRootSignature(tframeData.lastRootSig);
//    }

//    if (pipelineState.shaders.rootSig.textureViewCount > 0)
//        setDescriptorHeaps();
}

void QSGD3D11EnginePrivate::queueViewport(const QRect &rect)
{
    if (!inFrame) {
        qWarning("%s: Cannot be called outside begin/endFrame", __FUNCTION__);
        return;
    }

    tframeData.viewport = rect;
    const D3D11_VIEWPORT viewport = { float(rect.x()), float(rect.y()), float(rect.width()), float(rect.height()), 0, 1 };
    drawContext->RSSetViewports(1, &viewport);
}

void QSGD3D11EnginePrivate::queueScissor(const QRect &rect)
{
    if (!inFrame) {
        qWarning("%s: Cannot be called outside begin/endFrame", __FUNCTION__);
        return;
    }

    tframeData.scissor = rect;
    const D3D11_RECT scissorRect = { rect.x(), rect.y(), rect.x() + rect.width(), rect.y() + rect.height() };
    drawContext->RSSetScissorRects(1, &scissorRect);
}

void QSGD3D11EnginePrivate::queueSetRenderTarget(uint id)
{
    if (!inFrame) {
        qWarning("%s: Cannot be called outside begin/endFrame", __FUNCTION__);
        return;
    }

    ID3D11RenderTargetView* rtvHandle;
    ID3D11DepthStencilView* dsvHandle;

    if (!id) {
		queueSetRenderTarget(defaultRenderTarget);
    } else {
        const int idx = id - 1;
        Q_ASSERT(idx < renderTargets.count() && renderTargets[idx].entryInUse());
        RenderTarget &rt(renderTargets[idx]);
        rtvHandle = rt.rtv.Get();
        dsvHandle = rt.dsv.Get();
		Q_ASSERT(rtvHandle);
		drawContext->OMSetRenderTargets(1, &rtvHandle, dsvHandle);
		currentRenderTarget = id;
    }
	   
}

void QSGD3D11EnginePrivate::queueClearRenderTarget(const QColor &color)
{
    if (!inFrame) {
        qWarning("%s: Cannot be called outside begin/endFrame", __FUNCTION__);
        return;
    }

    const float clearColor[] = { float(color.redF()), float(color.blueF()), float(color.greenF()), float(color.alphaF()) };
	if (currentRenderTarget) {
		auto rtv = renderTargets[currentRenderTarget - 1].rtv.Get();
		drawContext->ClearRenderTargetView(rtv, clearColor);
	}
}

void QSGD3D11EnginePrivate::queueClearDepthStencil(float depthValue, quint8 stencilValue, QSGD3D11Engine::ClearFlags which)
{
    if (!inFrame) {
        qWarning("%s: Cannot be called outside begin/endFrame", __FUNCTION__);
        return;
    }

	if (currentRenderTarget) {
		auto dsv = renderTargets[currentRenderTarget - 1].dsv.Get();
		drawContext->ClearDepthStencilView(dsv, UINT(which), depthValue, stencilValue);
	}
}

void QSGD3D11EnginePrivate::queueSetBlendFactor(const QVector4D &factor)
{
    if (!inFrame) {
        qWarning("%s: Cannot be called outside begin/endFrame", __FUNCTION__);
        return;
    }

    tframeData.blendFactor = factor;
}

void QSGD3D11EnginePrivate::queueSetStencilRef(quint32 ref)
{
    if (!inFrame) {
        qWarning("%s: Cannot be called outside begin/endFrame", __FUNCTION__);
        return;
    }

    tframeData.stencilRef = ref;
}

void QSGD3D11EnginePrivate::queueDraw(const QSGD3D11Engine::DrawParams &params)
{
    if (!inFrame) {
        qWarning("%s: Cannot be called outside begin/endFrame", __FUNCTION__);
        return;
    }

    const bool skip = tframeData.scissor.isEmpty();

    PersistentFrameData &pfd(pframeData[currentPFrameIndex]);

    pfd.buffersUsedInDrawCallSet.insert(params.vertexBuf);
    const int vertexBufIdx = params.vertexBuf - 1;
    Q_ASSERT(params.vertexBuf && vertexBufIdx < buffers.count() && buffers[vertexBufIdx].entryInUse());

    pfd.buffersUsedInDrawCallSet.insert(params.constantBuf);
    const int constantBufIdx = params.constantBuf - 1;
    Q_ASSERT(params.constantBuf && constantBufIdx < buffers.count() && buffers[constantBufIdx].entryInUse());

    int indexBufIdx = -1;
    if (params.indexBuf) {
        pfd.buffersUsedInDrawCallSet.insert(params.indexBuf);
        indexBufIdx = params.indexBuf - 1;
        Q_ASSERT(indexBufIdx < buffers.count() && buffers[indexBufIdx].entryInUse());
    }

    // Ensure buffers are created but do not copy the data here, leave that to endDrawCalls().
    ensureBuffer(&buffers[vertexBufIdx]);
    ensureBuffer(&buffers[constantBufIdx]);
    if (indexBufIdx >= 0) {
        ensureBuffer(&buffers[indexBufIdx]);
    }

    // Set the CBV.
    if (!skip && params.cboOffset >= 0) {
        ID3D11Buffer *cbuf = buffers[constantBufIdx].d[currentPFrameIndex].buffer.Get();
        if (cbuf) {
            Q_ASSERT(params.cboOffset % 16 == 0);

            UINT first = params.cboOffset / 16;
            UINT size = 16;

            drawContext->VSSetConstantBuffers1(0, 1, &cbuf, &first, &size);
            drawContext->PSSetConstantBuffers1(0, 1, &cbuf, &first, &size);
        }
    }

    // Set up vertex and index buffers.
    ID3D11Buffer *vbuf = buffers[vertexBufIdx].d[currentPFrameIndex].buffer.Get();
    ID3D11Buffer *ibuf = indexBufIdx >= 0 && params.startIndexIndex >= 0
            ? buffers[indexBufIdx].d[currentPFrameIndex].buffer.Get() : nullptr;

    if (!skip && params.mode != tframeData.drawingMode) {
        D3D_PRIMITIVE_TOPOLOGY topology;
        switch (params.mode) {
        case QSGGeometry::DrawPoints:
            topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
            break;
        case QSGGeometry::DrawLines:
            topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
            break;
        case QSGGeometry::DrawLineStrip:
            topology = D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
            break;
        case QSGGeometry::DrawTriangles:
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
            break;
        case QSGGeometry::DrawTriangleStrip:
            topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
            break;
        default:
            qFatal("Unsupported drawing mode 0x%x", params.mode);
            break;
        }
        drawContext->IASetPrimitiveTopology(topology);
        tframeData.drawingMode = params.mode;
    }

    if (!skip) {
        // must be set after the topology
        UINT stride = params.vboStride;
        UINT offset = params.vboOffset;
        drawContext->IASetVertexBuffers(0, 1, &vbuf, &stride, &offset);
    }

    if (!skip && params.startIndexIndex >= 0 && ibuf && tframeData.currentIndexBuffer != params.indexBuf) {
        tframeData.currentIndexBuffer = params.indexBuf;
        auto format = DXGI_FORMAT(params.indexFormat);
        drawContext->IASetIndexBuffer(ibuf, format, 0);
    }

    // Copy the SRVs to a drawcall-dedicated area of the shader-visible descriptor heap.
    Q_ASSERT(tframeData.activeTextureCount == tframeData.pipelineState.shaders.rootSig.textureViewCount);
    if (tframeData.activeTextureCount > 0) {
        if (!skip) {
            ID3D11ShaderResourceView *srvs[QSGD3D11_MAX_TEXTURE_VIEWS];
            for (int i = 0; i < tframeData.activeTextureCount; ++i) {
                const TransientFrameData::ActiveTexture &t(tframeData.activeTextures[i]);
                Q_ASSERT(t.id);
                const int idx = t.id - 1;
                const bool isTex = t.type == TransientFrameData::ActiveTexture::TypeTexture;
                srvs[i] = isTex ? textures[idx].srv.Get() : renderTargets[idx].srv.Get();
            }
            drawContext->PSSetShaderResources(0, tframeData.activeTextureCount, srvs);
        }
        tframeData.activeTextureCount = 0;
    }

    // Add the draw call.
    if (!skip) {
        ++tframeData.drawCount;
        if (params.startIndexIndex >= 0)
            drawContext->DrawIndexedInstanced(params.count, 1, params.startIndexIndex, 0, 0);
        else
            drawContext->DrawInstanced(params.count, 1, 0, 0);
    }

    if (tframeData.drawCount == MAX_DRAW_CALLS_PER_LIST) {
        if (Q_UNLIKELY(debug_render()))
            qDebug("Limit of %d draw calls reached, executing command list", MAX_DRAW_CALLS_PER_LIST);
        // submit the command list
        endDrawCalls();
        // start a new one
        beginDrawCalls();
        // prepare for the upcoming drawcalls
        restoreFrameState();
    }
}

void QSGD3D11EnginePrivate::present()
{
    if (!initialized)
        return;

    if (Q_UNLIKELY(debug_render()))
        qDebug("--- present with vsync ---");
	
	QSGD3D11PipelineState pipelineState;
	
	auto &rootSig = pipelineState.shaders.rootSig;
	rootSig.textureViewCount = 1;
	rootSig.textureViews[0].filter = QSGD3D11TextureView::FilterNearest;
	rootSig.textureViews[0].addressModeHoriz = QSGD3D11TextureView::AddressClamp;
	rootSig.textureViews[0].addressModeVert = QSGD3D11TextureView::AddressClamp;

	//inFrame = true;
	//finalizePipeline(pipelineState);
	//useRenderTargetAsTexture(defaultRenderTarget);
	//queueDraw(drawParams);
	//inFrame = false;
		
    ++presentFrameIndex;
}

template<class T> uint newId(T *tbl)
{
    uint id = 0;
    for (int i = 0; i < tbl->count(); ++i) {
        if (!(*tbl)[i].entryInUse()) {
            id = i + 1;
            break;
        }
    }

    if (!id) {
        tbl->resize(tbl->size() + 1);
        id = tbl->count();
    }

    (*tbl)[id - 1].flags = 0x01; // reset flags and set EntryInUse

    return id;
}

template<class T> void syncEntryFlags(T *e, int flag, bool b)
{
    if (b)
        e->flags |= flag;
    else
        e->flags &= ~flag;
}

uint QSGD3D11EnginePrivate::genBuffer(UINT bindFlags)
{
    auto id = newId(&buffers);
    buffers[id - 1].bindFlags = bindFlags;
    return id;
}

void QSGD3D11EnginePrivate::releaseBuffer(uint id)
{
    if (!id || !initialized)
        return;

    const int idx = id - 1;
    Q_ASSERT(idx < buffers.count());

    if (Q_UNLIKELY(debug_buffer()))
        qDebug("releasing buffer %u", id);

    Buffer &b(buffers[idx]);
    if (!b.entryInUse())
        return;

    // Do not null out and do not mark the entry reusable yet.
    // Do that only when the frames potentially in flight have finished for sure.

    for (int i = 0; i < frameInFlightCount; ++i) {
        if (b.d[i].buffer)
            b.d[i].buffer.Reset();
    }

    QSet<PersistentFrameData::PendingRelease> *pendingReleasesSet = inFrame
            ? &pframeData[currentPFrameIndex].pendingReleases
            : &pframeData[(currentPFrameIndex + 1) % frameInFlightCount].outOfFramePendingReleases;

    pendingReleasesSet->insert(PersistentFrameData::PendingRelease(PersistentFrameData::PendingRelease::TypeBuffer, id));
}

void QSGD3D11EnginePrivate::resetBuffer(uint id, const quint8 *data, int size)
{
    if (!inFrame) {
        qWarning("%s: Cannot be called outside begin/endFrame", __FUNCTION__);
        return;
    }

    Q_ASSERT(id);
    const int idx = id - 1;
    Q_ASSERT(idx < buffers.count() && buffers[idx].entryInUse());
    Buffer &b(buffers[idx]);

    if (Q_UNLIKELY(debug_buffer()))
        qDebug("reset buffer %u, size %d", id, size);

    b.cpuDataRef.p = data;
    b.cpuDataRef.size = size;

    b.cpuDataRef.dirty.clear();
    b.d[currentPFrameIndex].dirty.clear();

    if (size > 0) {
        const QPair<int, int> range = qMakePair(0, size);
        b.cpuDataRef.dirty.append(range);
        b.d[currentPFrameIndex].dirty.append(range);
    }
}

void QSGD3D11EnginePrivate::addDirtyRange(DirtyList *dirty, int offset, int size, int bufferSize)
{
    // Bail out when the dirty list already spans the entire buffer.
    if (!dirty->isEmpty()) {
        if (dirty->at(0).first == 0 && dirty->at(0).second == bufferSize)
            return;
    }

    const QPair<int, int> range = qMakePair(offset, size);
    if (!dirty->contains(range))
        dirty->append(range);
}

void QSGD3D11EnginePrivate::markBufferDirty(uint id, int offset, int size)
{
    if (!inFrame) {
        qWarning("%s: Cannot be called outside begin/endFrame", __FUNCTION__);
        return;
    }

    Q_ASSERT(id);
    const int idx = id - 1;
    Q_ASSERT(idx < buffers.count() && buffers[idx].entryInUse());
    Buffer &b(buffers[idx]);

    addDirtyRange(&b.cpuDataRef.dirty, offset, size, b.cpuDataRef.size);
    addDirtyRange(&b.d[currentPFrameIndex].dirty, offset, size, b.cpuDataRef.size);
}

uint QSGD3D11EnginePrivate::genTexture()
{
    const uint id = newId(&textures);
    textures[id - 1].fenceValue = 0;
    return id;
}

static inline DXGI_FORMAT textureFormat(QImage::Format format, bool wantsAlpha, bool mipmap, bool force32bit,
                                        QImage::Format *imageFormat, int *bytesPerPixel)
{
    DXGI_FORMAT f = DXGI_FORMAT_R8G8B8A8_UNORM;
    QImage::Format convFormat = format;
    int bpp = 4;

    if (!mipmap) {
        switch (format) {
        case QImage::Format_Grayscale8:
        case QImage::Format_Indexed8:
        case QImage::Format_Alpha8:
            if (!force32bit) {
                f = DXGI_FORMAT_R8_UNORM;
                bpp = 1;
            } else {
                convFormat = QImage::Format_RGBA8888;
            }
            break;
        case QImage::Format_RGB32:
            f = DXGI_FORMAT_B8G8R8A8_UNORM;
            break;
        case QImage::Format_ARGB32:
            f = DXGI_FORMAT_B8G8R8A8_UNORM;
            convFormat = wantsAlpha ? QImage::Format_ARGB32_Premultiplied : QImage::Format_RGB32;
            break;
        case QImage::Format_ARGB32_Premultiplied:
            f = DXGI_FORMAT_B8G8R8A8_UNORM;
            convFormat = wantsAlpha ? format : QImage::Format_RGB32;
            break;
        default:
            convFormat = wantsAlpha ? QImage::Format_RGBA8888_Premultiplied : QImage::Format_RGBX8888;
            break;
        }
    } else {
        // Mipmap generation needs unordered access and BGRA is not an option for that. Stick to RGBA.
        convFormat = wantsAlpha ? QImage::Format_RGBA8888_Premultiplied : QImage::Format_RGBX8888;
    }

    if (imageFormat)
        *imageFormat = convFormat;

    if (bytesPerPixel)
        *bytesPerPixel = bpp;

    return f;
}

static inline QImage::Format imageFormatForTexture(DXGI_FORMAT format)
{
    QImage::Format f = QImage::Format_Invalid;

    switch (format) {
    case DXGI_FORMAT_R8G8B8A8_UNORM:
        f = QImage::Format_RGBA8888_Premultiplied;
        break;
    case DXGI_FORMAT_B8G8R8A8_UNORM:
        f = QImage::Format_ARGB32_Premultiplied;
        break;
    case DXGI_FORMAT_R8_UNORM:
        f = QImage::Format_Grayscale8;
        break;
    default:
        break;
    }

    return f;
}

void QSGD3D11EnginePrivate::createTexture(uint id, const QSize &size, QImage::Format format,
                                          QSGD3D11Engine::TextureCreateFlags createFlags)
{
    ensureDevice();

    Q_ASSERT(id);
    const int idx = id - 1;
    Q_ASSERT(idx < textures.count() && textures[idx].entryInUse());
    Texture &t(textures[idx]);

    syncEntryFlags(&t, Texture::Alpha, createFlags & QSGD3D11Engine::TextureWithAlpha);
    syncEntryFlags(&t, Texture::MipMap, createFlags & QSGD3D11Engine::TextureWithMipMaps);

    const QSize adjustedSize = !t.mipmap() ? size : QSGD3D11Engine::mipMapAdjustedSourceSize(size);

    auto realFormat = textureFormat(format, t.alpha(), t.mipmap(),
                                createFlags.testFlag(QSGD3D11Engine::TextureAlways32Bit),
                                nullptr, nullptr);
    auto mipLevels = !t.mipmap() ? 1 : QSGD3D11Engine::mipMapLevels(adjustedSize);

    CD3D11_TEXTURE2D_DESC textureDesc(
                realFormat,
                adjustedSize.width(),
                adjustedSize.height(),
                1, // array size
                mipLevels
    );

    if (t.mipmap())
        textureDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;

    HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, &t.texture);
    if (FAILED(hr)) {
        qWarning("Failed to create texture resource: %s", qPrintable(comErrorMessage(hr)));
        return;
    }

    CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(
        D3D11_SRV_DIMENSION_TEXTURE2D,
        textureDesc.Format,
        0,
        textureDesc.MipLevels
    );

    hr = device->CreateShaderResourceView(t.texture.Get(), &srvDesc, &t.srv);
    if (FAILED(hr)) {
        qWarning("Failed to create shader resource view for texture: %s", qPrintable(comErrorMessage(hr)));
        return;
    }

    if (t.mipmap()) {
        // Mipmap generation will need an UAV for each level that needs to be generated.
        t.mipUAVs.clear();
        for (UINT level = 1; level < textureDesc.MipLevels; ++level) {
            CD3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc(
                D3D11_UAV_DIMENSION_TEXTURE2D,
                textureDesc.Format,
                level
            );
            ComPtr<ID3D11UnorderedAccessView> h;
            device->CreateUnorderedAccessView(t.texture.Get(), &uavDesc, &h);
            t.mipUAVs.append(h);
        }
    }

    if (Q_UNLIKELY(debug_texture()))
        qDebug("created texture %u, size %dx%d, miplevels %d", id, adjustedSize.width(), adjustedSize.height(), textureDesc.MipLevels);
}

void QSGD3D11EnginePrivate::queueTextureResize(uint id, const QSize &size)
{
    Q_ASSERT(id);
    const int idx = id - 1;
    Q_ASSERT(idx < textures.count() && textures[idx].entryInUse());
    Texture &t(textures[idx]);

    if (!t.texture) {
        qWarning("Cannot resize non-created texture %u", id);
        return;
    }

    if (t.mipmap()) {
        qWarning("Cannot resize mipmapped texture %u", id);
        return;
    }

    if (Q_UNLIKELY(debug_texture()))
        qDebug("resizing texture %u, size %dx%d", id, size.width(), size.height());

    D3D11_TEXTURE2D_DESC textureDesc;
    t.texture->GetDesc(&textureDesc);
    textureDesc.Width = size.width();
    textureDesc.Height = size.height();

    // Release the old resources
    auto oldTexture = t.texture;
    t.texture.Reset();
    t.srv.Reset();

    HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, &t.texture);
    if (FAILED(hr)) {
        qWarning("Failed to create resized texture resource: %s",
                 qPrintable(comErrorMessage(hr)));
        return;
    }

    CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(
        D3D11_SRV_DIMENSION_TEXTURE2D,
        textureDesc.Format,
        0,
        textureDesc.MipLevels
    );

    hr = device->CreateShaderResourceView(t.texture.Get(), &srvDesc, &t.srv);
    if (FAILED(hr)) {
        qWarning("Failed to create shader resource view for texture: %s", qPrintable(comErrorMessage(hr)));
        return;
    }

    drawContext->CopySubresourceRegion(t.texture.Get(), 0, 0, 0, 0, oldTexture.Get(), 0, nullptr);

    if (Q_UNLIKELY(debug_texture()))
        qDebug("submitted old content copy for texture %u on the copy queue", id);
}

void QSGD3D11EnginePrivate::queueTextureUpload(uint id, const QVector<QImage> &images, const QVector<QPoint> &dstPos,
                                               QSGD3D11Engine::TextureUploadFlags flags)
{
    Q_ASSERT(id);
    Q_ASSERT(images.count() == dstPos.count());
    if (images.isEmpty())
        return;

    const int idx = id - 1;
    Q_ASSERT(idx < textures.count() && textures[idx].entryInUse());
    Texture &t(textures[idx]);
    Q_ASSERT(t.texture);

    // When mipmapping is not in use, image can be smaller than the size passed
    // to createTexture() and dstPos can specify a non-zero destination position.

    if (t.mipmap() && (images.count() != 1 || dstPos.count() != 1 || !dstPos[0].isNull())) {
        qWarning("Mipmapped textures (%u) do not support partial uploads", id);
        return;
    }

    // Make life simpler by disallowing queuing a new mipmapped upload before the previous one finishes.
    if (t.mipmap() && t.fenceValue) {
        qWarning("Attempted to queue mipmapped texture upload (%u) while a previous upload is still in progress", id);
        return;
    }

    if (Q_UNLIKELY(debug_texture()))
        qDebug("adding upload for texture %u on the copy queue", id);

    D3D11_TEXTURE2D_DESC textureDesc;
    t.texture->GetDesc(&textureDesc);
    const QSize adjustedTextureSize(textureDesc.Width, textureDesc.Height);

    for (int i = 0; i < images.count(); ++i) {
        QImage::Format convFormat;
        int bytesPerPixel;
        auto format = textureFormat(images[i].format(), t.alpha(), t.mipmap(),
                      flags.testFlag(QSGD3D11Engine::TextureUploadAlways32Bit),
                      &convFormat, &bytesPerPixel);
        if (Q_UNLIKELY(debug_texture() && i == 0))
            qDebug("source image format %d, target format %d, bpp %d", images[i].format(), convFormat, bytesPerPixel);

        QImage convImage = images[i].format() == convFormat ? images[i] : images[i].convertToFormat(convFormat);

        if (t.mipmap() && adjustedTextureSize != convImage.size())
            convImage = convImage.scaled(adjustedTextureSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        CD3D11_TEXTURE2D_DESC texDesc(
            format,
            convImage.width(),
            convImage.height()
        );
        texDesc.MipLevels = 1;
        texDesc.Usage = D3D11_USAGE_IMMUTABLE; // We're never going to read/change this

        D3D11_SUBRESOURCE_DATA texData;
        texData.pSysMem = convImage.constBits();
        texData.SysMemPitch = convImage.bytesPerLine();

        ComPtr<ID3D11Texture2D> stagingTex;
        if (FAILED(device->CreateTexture2D(&texDesc, &texData, &stagingTex))) {
            qWarning("Failed to create texture upload buffer");
            return;
        }

        drawContext->CopySubresourceRegion(t.texture.Get(), 0, dstPos[i].x(), dstPos[i].y(), 0, stagingTex.Get(), 0, nullptr);
    }
}

void QSGD3D11EnginePrivate::releaseTexture(uint id)
{
    if (!id || !initialized)
        return;

    const int idx = id - 1;
    Q_ASSERT(idx < textures.count());

    if (Q_UNLIKELY(debug_texture()))
        qDebug("releasing texture %d", id);

    Texture &t(textures[idx]);
    if (!t.entryInUse())
        return;

    if (t.texture) {
        t.texture.Reset();
        t.srv.Reset();
        for (auto h : t.mipUAVs)
            h.Reset();
    }

    QSet<PersistentFrameData::PendingRelease> *pendingReleasesSet = inFrame
            ? &pframeData[currentPFrameIndex].pendingReleases
            : &pframeData[(currentPFrameIndex + 1) % frameInFlightCount].outOfFramePendingReleases;

    pendingReleasesSet->insert(PersistentFrameData::PendingRelease(PersistentFrameData::PendingRelease::TypeTexture, id));
}

void QSGD3D11EnginePrivate::useTexture(uint id)
{
    if (!inFrame) {
        qWarning("%s: Cannot be called outside begin/endFrame", __FUNCTION__);
        return;
    }

    Q_ASSERT(id);
    const int idx = id - 1;
    Q_ASSERT(idx < textures.count() && textures[idx].entryInUse());

    // Within one frame the order of calling this function determines the
    // texture register (0, 1, ...) so fill up activeTextures accordingly.
    tframeData.activeTextures[tframeData.activeTextureCount++]
            = TransientFrameData::ActiveTexture(TransientFrameData::ActiveTexture::TypeTexture, id);

    if (textures[idx].fenceValue)
        pframeData[currentPFrameIndex].pendingTextureUploads.insert(id);
}

bool QSGD3D11EnginePrivate::MipMapGen::initialize(QSGD3D11EnginePrivate *enginePriv)
{
    engine = enginePriv;
    return true;
}

void QSGD3D11EnginePrivate::MipMapGen::releaseResources()
{
}

// The mipmap generator is used to insert commands on the main 3D queue. It is
// guaranteed that the queue has a wait for the base texture level upload
// before invoking generate(). There can be any number of invocations
// without waiting for earlier ones to finish. finished() is invoked when it is
// known for sure that frame containing the upload and mipmap generation has
// finished on the GPU.

void QSGD3D11EnginePrivate::MipMapGen::generate(const Texture &t)
{
    engine->context->GenerateMips(t.srv.Get());
}

uint QSGD3D11EnginePrivate::genRenderTarget()
{
    return newId(&renderTargets);
}

void QSGD3D11EnginePrivate::createRenderTarget(uint id, const QSize &size, uint samples)
{
    Q_ASSERT(id);
    const int idx = id - 1;
    Q_ASSERT(idx < renderTargets.count() && renderTargets[idx].entryInUse());
    RenderTarget &rt(renderTargets[idx]);

    auto rtv = createColorBuffer(size, samples);
    rt.color = rtv.first;
    rt.rtv = rtv.second;

    auto dsres = createDepthStencil(size, samples);
    rt.ds = dsres.first;
    rt.dsv = dsres.second;

    D3D11_TEXTURE2D_DESC rtDesc;
    rt.color->GetDesc(&rtDesc);
    const bool multisample = rtDesc.SampleDesc.Count > 1;
    syncEntryFlags(&rt, RenderTarget::Multisample, multisample);

    if (!multisample) {
        if (FAILED(device->CreateShaderResourceView(rt.color.Get(), nullptr, &rt.srv))) {
            qWarning("Unable to create SRV for new render target");
        }
    } else {
        CD3D11_TEXTURE2D_DESC textureDesc(
                    RT_COLOR_FORMAT,
                    size.width(),
                    size.height(),
                    1,
                    1
        );

        HRESULT hr = device->CreateTexture2D(&textureDesc, nullptr, &rt.colorResolve);
        if (FAILED(hr)) {
            qWarning("Failed to create resolve buffer: %s",
                     qPrintable(comErrorMessage(hr)));
            return;
        }

        if (FAILED(device->CreateShaderResourceView(rt.colorResolve.Get(), nullptr, &rt.srv))) {
            qWarning("Unable to create SRV for new render target");
        }
    }

    if (Q_UNLIKELY(debug_render()))
        qDebug("created new render target %u, size %dx%d, samples %d", id, size.width(), size.height(), samples);
}

void QSGD3D11EnginePrivate::releaseRenderTarget(uint id)
{
    if (!id || !initialized)
        return;

    const int idx = id - 1;
    Q_ASSERT(idx < renderTargets.count());
    RenderTarget &rt(renderTargets[idx]);
    if (!rt.entryInUse())
        return;

    if (Q_UNLIKELY(debug_render()))
        qDebug("releasing render target %u", id);

    if (rt.colorResolve) {
        rt.colorResolve = nullptr;
    }
    if (rt.color) {        
        rt.color = nullptr;
        rt.rtv = nullptr;
        rt.srv = nullptr;
    }
    if (rt.ds) {
        rt.ds = nullptr;
        rt.dsv = nullptr;
    }

    rt.flags &= ~RenderTarget::EntryInUse;
}

void QSGD3D11EnginePrivate::useRenderTargetAsTexture(uint id)
{
    if (!inFrame) {
        qWarning("%s: Cannot be called outside begin/endFrame", __FUNCTION__);
        return;
    }

    Q_ASSERT(id);
    const int idx = id - 1;
    Q_ASSERT(idx < renderTargets.count());
    RenderTarget &rt(renderTargets[idx]);
    Q_ASSERT(rt.entryInUse() && rt.color);

    if (rt.flags & RenderTarget::NeedsReadBarrier) {
        rt.flags &= ~RenderTarget::NeedsReadBarrier;
        if (rt.flags & RenderTarget::Multisample) {
            resolveMultisampledTarget(rt.color.Get(), rt.colorResolve.Get());
        }
    }

    tframeData.activeTextures[tframeData.activeTextureCount++] =
            TransientFrameData::ActiveTexture::ActiveTexture(TransientFrameData::ActiveTexture::TypeRenderTarget, id);
}

QImage QSGD3D11EnginePrivate::executeAndWaitReadbackRenderTarget(uint id)
{
    // Readback due to QQuickWindow::grabWindow() happens outside
    // begin-endFrame, but QQuickItemGrabResult leads to rendering a layer
    // without a real frame afterwards and triggering readback. This has to be
    // supported as well.
    if (inFrame && (!activeLayers || currentLayerDepth)) {
        qWarning("%s: Cannot be called while frame preparation is active", __FUNCTION__);
        return QImage();
    }

    // Due to the above we insert a fake "real" frame when a layer was just rendered into.
    if (inFrame) {
        beginFrame();
        endFrame();
    }

    ID3D11Texture2D *rtRes;

	if (!id) {
		id = defaultRenderTarget;
	}

    const int idx = id - 1;
    Q_ASSERT(idx < renderTargets.count());
    RenderTarget &rt(renderTargets[idx]);
    Q_ASSERT(rt.entryInUse() && rt.color);

    if (rt.flags & RenderTarget::Multisample) {
        resolveMultisampledTarget(rt.color.Get(), rt.colorResolve.Get());
        rtRes = rt.colorResolve.Get();
    } else {
        rtRes = rt.color.Get();
    }

    D3D11_TEXTURE2D_DESC rtDesc;
    rtRes->GetDesc(&rtDesc);

    CD3D11_TEXTURE2D_DESC bufDesc(
                rtDesc.Format,
                rtDesc.Width,
                rtDesc.Height,
                1,
                1,
                0, // No bind
                D3D11_USAGE_STAGING,
                D3D11_CPU_ACCESS_READ
                );

    ComPtr<ID3D11Texture2D> readbackBuf;
    if (FAILED(device->CreateTexture2D(&bufDesc, nullptr, &readbackBuf))) {
        qWarning("Failed to create staging texture for readback");
        return QImage();
    }

    context->CopyResource(readbackBuf.Get(), rtRes);

    QImage::Format fmt = imageFormatForTexture(rtDesc.Format);
    if (fmt == QImage::Format_Invalid) {
        qWarning("Could not map render target format %d to a QImage format", rtDesc.Format);
        return QImage();
    }
    QImage img(rtDesc.Width, rtDesc.Height, fmt);
    quint8 *p = nullptr;

    // TODO: Synchronize over immediate context (?)

    D3D11_MAPPED_SUBRESOURCE mapped;
    if (FAILED(context->Map(readbackBuf.Get(), 0, D3D11_MAP_READ, 0, &mapped))) {
        qWarning("Mapping the readback buffer failed");
        return QImage();
    }
    p = reinterpret_cast<quint8*>(mapped.pData);

    const int bpp = 4; // ###
    if (id == 0) {
        for (UINT y = 0; y < rtDesc.Height; ++y) {
            quint8 *dst = img.scanLine(y);
            memcpy(dst, p, rtDesc.Width * bpp);
            p += mapped.RowPitch;
        }
    } else {
        for (int y = rtDesc.Height - 1; y >= 0; --y) {
            quint8 *dst = img.scanLine(y);
            memcpy(dst, p, rtDesc.Width * bpp);
            p += mapped.RowPitch;
        }
    }
    context->Unmap(readbackBuf.Get(), 0);

    return img;
}

void QSGD3D11EnginePrivate::simulateDeviceLoss()
{
    qWarning("QSGD3D11Engine: Triggering device loss via TDR");
    devLossTest.killDevice();
}

bool QSGD3D11EnginePrivate::DeviceLossTester::initialize(QSGD3D11EnginePrivate *enginePriv)
{
    engine = enginePriv;

#ifdef DEVLOSS_TEST
    HRESULT hr = engine->device->CreateComputeShader(g_timeout, sizeof(g_timeout), nullptr, &computeShader);
    if (FAILED(hr)) {
        qWarning("Unable to create compute shader: %s", qPrintable(comErrorMessage(hr)));
        return false;
    }
#endif

    return true;
}

void QSGD3D11EnginePrivate::DeviceLossTester::releaseResources()
{
#ifdef DEVLOSS_TEST
    computeShader = nullptr;
#endif
}

void QSGD3D11EnginePrivate::DeviceLossTester::killDevice()
{
#ifdef DEVLOSS_TEST
    engine->context->CSSetShader(computeShader.Get(), nullptr, 0);
    engine->context->Dispatch(256, 1, 1);
#endif
}

void *QSGD3D11EnginePrivate::getResource(QSGRendererInterface::Resource resource) const
{
    switch (resource) {
    case QSGRendererInterface::DeviceResource:
        return device;
	case QSGRendererInterface::PainterResource:
		return const_cast<QSGD3D11EnginePrivate*>(this)->backBuffer();
    default:
        break;
    }
    return nullptr;
}

QT_END_NAMESPACE
