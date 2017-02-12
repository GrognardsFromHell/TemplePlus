
#include "stdafx.h"

#include <QtQuick/private/qquickanimatorcontroller_p.h>
#include <QtQuick/private/qsgcontext_p.h>
#include <QtQuick/private/qsgcontextplugin_p.h>
#include <QQuickWindow>
#include <QtGui/private/qguiapplication_p.h>
#include <QPluginLoader>
#include <QtPlugin>
#include "ui_rendercontrol.h"
#include "../tig/tig_startup.h"
#include <graphics/device.h>
#include <graphics/shaperenderer2d.h>
#include "../qsgd3d11/qsgd3d11renderloop_p.h"

struct UiRenderControl::Impl {
	std::unique_ptr<QSGContext> context;
	std::unique_ptr<QSGRenderContext> renderContext;
	QSGD3D11RenderLoop *renderLoop = nullptr;
};

Q_IMPORT_PLUGIN(QSGD3D11Adaptation)

UiRenderControl::UiRenderControl() : impl(std::make_unique<Impl>())
{
	
	QFontDatabase::addApplicationFont("C:/TemplePlus/TemplePlus/tpdata/fonts/Junicode.ttf");
	QFontDatabase::addApplicationFont("C:/TemplePlus/TemplePlus/tpdata/fonts/Junicode-Bold.ttf");
	QFontDatabase::addApplicationFont("C:/TemplePlus/TemplePlus/tpdata/fonts/Junicode-BoldItalic.ttf");
	QFontDatabase::addApplicationFont("C:/TemplePlus/TemplePlus/tpdata/fonts/Junicode-Italic.ttf");

	QQuickWindow::setSceneGraphBackend("d3d11");
	QQuickWindow::setDefaultAlphaBuffer(true);

	auto device = tig->GetRenderingDevice().GetDevice();
	auto context = tig->GetRenderingDevice().GetContext();
	auto swapChain = tig->GetRenderingDevice().GetSwapChain();
	
	impl->renderLoop = new QSGD3D11RenderLoop(device, context, swapChain);
	QSGRenderLoop::setInstance(impl->renderLoop);

}

class BorrowedTexture : public gfx::Texture {
public:

	BorrowedTexture(const std::string &name, int w, int h, ID3D11ShaderResourceView *srv) : mName(name), mSrv(srv) {
		mSize.width = w;
		mSize.height = h;
		mContentRect.x = 0;
		mContentRect.y = 0;
		mContentRect.width = w;
		mContentRect.height = h;
	}

	int GetId() const override {
		return -1;
	}

	const std::string& GetName() const override {
		return mName;
	}

	const gfx::ContentRect& GetContentRect() const override {
		return mContentRect;
	}

	const gfx::Size& GetSize() const override {
		return mSize;
	}

	void FreeDeviceTexture() {
		// No-op since we do not own the texture
	}

	ID3D11ShaderResourceView* GetResourceView() {
		return mSrv;
	}

	gfx::TextureType GetType() const override {
		return gfx::TextureType::Custom;
	}

private:
	std::string mName;
	ID3D11ShaderResourceView *mSrv;	
	gfx::ContentRect mContentRect;
	gfx::Size mSize;
};

UiRenderControl::~UiRenderControl()
{
}

void UiRenderControl::Render(QQuickWindow *window) {

	auto &device = tig->GetRenderingDevice();
	auto &shapeRenderer = tig->GetShapeRenderer2d();

	auto srv = static_cast<ID3D11ShaderResourceView*>(window->rendererInterface()->getResource(window, QSGRendererInterface::PainterResource));
	if (srv) {
		BorrowedTexture texture("quick-surface", window->width(), window->height(), srv);
		shapeRenderer.DrawRectangle(0, 0, window->width(), window->height(), texture);
	}
	
}

void UiRenderControl::ProcessEvents()
{
	QGuiApplication::processEvents();

	auto &device = tig->GetRenderingDevice();
	device.RestoreState();
}
