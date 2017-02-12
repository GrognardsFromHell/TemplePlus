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

#ifndef QSGD3D11ENGINE_P_P_H
#define QSGD3D11ENGINE_P_P_H

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

#include "qsgd3d11engine_p.h"
#include <QCache>

#include <d3d11.h>
#include <d3d11_1.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;

// No moc-related features (Q_OBJECT, signals, etc.) can be used here to due
// moc-generated code failing to compile when combined with COM stuff.

// Recommended reading before moving further: https://github.com/Microsoft/DirectXTK/wiki/ComPtr
// Note esp. operator= vs. Attach and operator& vs. GetAddressOf

// ID3D12* is never passed to Qt containers directly. Always use ComPtr and put it into a struct.

QT_BEGIN_NAMESPACE

class QSGD3D11DeviceManager
{
public:
    void deviceLossDetected();

    struct DeviceLossObserver {
        virtual void deviceLost() = 0;
    };
    void registerDeviceLossObserver(DeviceLossObserver *observer);

private:
    QAtomicInt m_ref;
    QVector<DeviceLossObserver *> m_observers;
};

class QSGD3D11EnginePrivate : public QSGD3D11DeviceManager::DeviceLossObserver
{
public:
		
	QSGD3D11EnginePrivate(ID3D11Device *device, ID3D11DeviceContext *context, IDXGISwapChain *swapChain) : device(device), swapChain(swapChain) {
		if (FAILED(context->QueryInterface(IID_PPV_ARGS(&this->context)))) {
			qWarning("Unable to retrieve ID3D11DeviceContext1 from given device!");
		}
	}

    void initialize(const QSize &size, float dpr, int surfaceFormatSamples, bool alpha);
    bool isInitialized() const { return initialized; }
    void releaseResources();
    void setWindowSize(const QSize &size, float dpr);
    QSize currentWindowSize() const { return windowSize; }
    float currentWindowDpr() const { return windowDpr; }
    uint currentWindowSamples() const { return windowSamples; }

    void beginFrame();
    void endFrame();
    void beginLayer();
    void endLayer();
    void invalidateCachedFrameState();
    void restoreFrameState(bool minimal = false);

    uint genBuffer(UINT bindFlags);
    void releaseBuffer(uint id);
    void resetBuffer(uint id, const quint8 *data, int size);
    void markBufferDirty(uint id, int offset, int size);

    void queueViewport(const QRect &rect);
    void queueScissor(const QRect &rect);
    void queueSetRenderTarget(uint id);
    void queueClearRenderTarget(const QColor &color);
    void queueClearDepthStencil(float depthValue, quint8 stencilValue, QSGD3D11Engine::ClearFlags which);
    void queueSetBlendFactor(const QVector4D &factor);
    void queueSetStencilRef(quint32 ref);

    void finalizePipeline(const QSGD3D11PipelineState &pipelineState);

    void queueDraw(const QSGD3D11Engine::DrawParams &params);

    void present();

    uint genTexture();
    void createTexture(uint id, const QSize &size, QImage::Format format, QSGD3D11Engine::TextureCreateFlags flags);
    void queueTextureResize(uint id, const QSize &size);
    void queueTextureUpload(uint id, const QVector<QImage> &images, const QVector<QPoint> &dstPos,
                            QSGD3D11Engine::TextureUploadFlags flags);
    void releaseTexture(uint id);
    void useTexture(uint id);

    uint genRenderTarget();
    void createRenderTarget(uint id, const QSize &size, uint samples);
    void releaseRenderTarget(uint id);
    void useRenderTargetAsTexture(uint id);
    uint activeRenderTarget() const { return currentRenderTarget; }

    QImage executeAndWaitReadbackRenderTarget(uint id);

    void simulateDeviceLoss();

    void *getResource(QSGRendererInterface::Resource resource) const;

    // the device is intentionally hidden here. all resources have to go
    // through the engine and, unlike with GL, cannot just be created in random
    // places due to the need for proper tracking, managing and releasing.
private:
    void ensureDevice();
    void setupDefaultRenderTargets();
    void deviceLost() override;

    DXGI_SAMPLE_DESC makeSampleDesc(DXGI_FORMAT format, uint samples);

    using TextureAndRTV = QPair<ComPtr<ID3D11Texture2D>, ComPtr<ID3D11RenderTargetView>>;
    using TextureAndDSV = QPair<ComPtr<ID3D11Texture2D>, ComPtr<ID3D11DepthStencilView>>;

    TextureAndRTV createColorBuffer(const QSize &size, uint samples);
    TextureAndDSV createDepthStencil(const QSize &size, uint samples);

    void resolveMultisampledTarget(ID3D11Resource *msaa, ID3D11Resource *resolve) const;

    ID3D11Buffer *createBuffer(int size, UINT usage);

    typedef QVector<QPair<int, int> > DirtyList;
    void addDirtyRange(DirtyList *dirty, int offset, int size, int bufferSize);

    struct PersistentFrameData {
        QSet<uint> pendingTextureUploads;
        QSet<uint> pendingTextureMipMap;
        QSet<uint> buffersUsedInDrawCallSet;
        QSet<uint> buffersUsedInFrame;
        struct PendingRelease {
            enum Type {
                TypeTexture,
                TypeBuffer
            };
            Type type = TypeTexture;
            uint id = 0;
            PendingRelease(Type type, uint id) : type(type), id(id) { }
            PendingRelease() { }
            bool operator==(const PendingRelease &other) const { return type == other.type && id == other.id; }
        };
        QSet<PendingRelease> pendingReleases;
        QSet<PendingRelease> outOfFramePendingReleases;
    };
    friend uint qHash(const PersistentFrameData::PendingRelease &pr, uint seed);

    struct Buffer;
    void ensureBuffer(Buffer *buf);
    void updateBuffer(Buffer *buf);

    void beginDrawCalls();
    void beginFrameDraw();
    void endDrawCalls(bool lastInFrame = false);

    static const int MAX_SWAP_CHAIN_BUFFER_COUNT = 4;
    static const int MAX_FRAME_IN_FLIGHT_COUNT = 4;

    bool initialized = false;
    bool inFrame = false;
    QSize windowSize;
    float windowDpr;
    uint windowSamples;
    bool windowAlpha;
    int swapChainBufferCount;
    int frameInFlightCount;
    int waitableSwapChainMaxLatency;
	ComPtr<IDXGISwapChain> swapChain;
    ID3D11Device *device;    
    ID3D11DeviceContext1 *context;

    /*
     * We use a deferred context like this to queue up all
     * actual draw commands and then do a batched resource upload
     * at the end immediately before executing all queued up commands.
     */
    ComPtr<ID3D11DeviceContext1> drawContext;

    quint64 presentFrameIndex;
    quint64 frameIndex;

    PersistentFrameData pframeData[MAX_FRAME_IN_FLIGHT_COUNT];
    int currentPFrameIndex;
    int activeLayers = 0;
    int currentLayerDepth = 0;

    float blendFactor[4];

    struct PSOCacheEntry {
        ComPtr<ID3D11VertexShader> vs;
        ComPtr<ID3D11PixelShader> ps;
        ComPtr<ID3D11InputLayout> inputLayout;
        ComPtr<ID3D11BlendState> blendState;
        ComPtr<ID3D11RasterizerState> rasterizerState;
        ComPtr<ID3D11DepthStencilState> depthStencilState;
        UINT sampleMask;
        D3D11_PRIMITIVE_TOPOLOGY primitiveTopology;
    };
    QCache<QSGD3D11PipelineState, PSOCacheEntry> psoCache;

    struct SamplerStateCacheEntry {
        ComPtr<ID3D11SamplerState> samplerStates[QSGD3D11_MAX_TEXTURE_VIEWS];
    };
    QCache<QSGD3D11RootSignature, SamplerStateCacheEntry> samplerStateCache;

    struct Texture {
        enum Flag {
            EntryInUse = 0x01,
            Alpha = 0x02,
            MipMap = 0x04
        };
        int flags = 0;
        bool entryInUse() const { return flags & EntryInUse; }
        bool alpha() const { return flags & Alpha; }
        bool mipmap() const { return flags & MipMap; }
        ComPtr<ID3D11Texture2D> texture;
        ComPtr<ID3D11ShaderResourceView> srv;
        quint64 fenceValue = 0;
        quint64 lastWaitFenceValue = 0;
        struct StagingBuffer {
            ComPtr<ID3D11Resource> buffer;
        };
        QVector<StagingBuffer> stagingBuffers;
        QVector<ComPtr<ID3D11UnorderedAccessView>> mipUAVs;
    };

    QVector<Texture> textures;

    struct TransientFrameData {
        QSGGeometry::DrawingMode drawingMode;
        uint currentIndexBuffer;
        struct ActiveTexture {
            enum Type {
                TypeTexture,
                TypeRenderTarget
            };
            Type type = TypeTexture;
            uint id = 0;
            ActiveTexture(Type type, uint id) : type(type), id(id) { }
            ActiveTexture() { }
        };
        int activeTextureCount;
        ActiveTexture activeTextures[QSGD3D11_MAX_TEXTURE_VIEWS];
        int drawCount;
        bool descHeapSet;

        QRect viewport;
        QRect scissor;
        QVector4D blendFactor = QVector4D(1, 1, 1, 1);
        quint32 stencilRef = 1;
        QSGD3D11PipelineState pipelineState;
    };
    TransientFrameData tframeData;

    struct MipMapGen {
        bool initialize(QSGD3D11EnginePrivate *enginePriv);
        void releaseResources();
        void generate(const Texture &t);

        QSGD3D11EnginePrivate *engine;
    };

    MipMapGen mipmapper;

    struct RenderTarget {
        enum Flag {
            EntryInUse = 0x01,
            NeedsReadBarrier = 0x02,
            Multisample = 0x04
        };
        int flags = 0;
        bool entryInUse() const { return flags & EntryInUse; }
        ComPtr<ID3D11Texture2D> color;
        ComPtr<ID3D11Texture2D> colorResolve;
        ComPtr<ID3D11RenderTargetView> rtv;
        ComPtr<ID3D11Texture2D> ds;
        ComPtr<ID3D11DepthStencilView> dsv;
        ComPtr<ID3D11ShaderResourceView> srv;
    };

    QVector<RenderTarget> renderTargets;
    uint currentRenderTarget;
	uint defaultRenderTarget;

	ID3D11ShaderResourceView *backBuffer();
		
    struct CPUBufferRef {
        const quint8 *p = nullptr;
        quint32 size = 0;
        DirtyList dirty;
        CPUBufferRef() { dirty.reserve(16); }
    };

    struct Buffer {
        enum Flag {
            EntryInUse = 0x01
        };
        int flags = 0;
        bool entryInUse() const { return flags & EntryInUse; }
        struct InFlightData {
            ComPtr<ID3D11Buffer> buffer;
            DirtyList dirty;
            quint32 dataSize = 0;
            quint32 resourceSize = 0;
            InFlightData() { dirty.reserve(16); }
        };
        InFlightData d[MAX_FRAME_IN_FLIGHT_COUNT];
        CPUBufferRef cpuDataRef;
        UINT bindFlags;
    };

    QVector<Buffer> buffers;

    struct DeviceLossTester {
        bool initialize(QSGD3D11EnginePrivate *enginePriv);
        void releaseResources();
        void killDevice();

        QSGD3D11EnginePrivate *engine;
        ComPtr<ID3D11ComputeShader> computeShader;
    };

    DeviceLossTester devLossTest;

};

inline uint qHash(const QSGD3D11EnginePrivate::PersistentFrameData::PendingRelease &pr, uint seed = 0)
{
    Q_UNUSED(seed);
    return pr.id + pr.type;
}

QT_END_NAMESPACE

#endif
