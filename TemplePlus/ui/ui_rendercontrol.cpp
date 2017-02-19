
#include "stdafx.h"

#include <QtQuick/private/qquickanimatorcontroller_p.h>
#include <QtQuick/private/qsgcontext_p.h>
#include <QtQuick/private/qsgcontextplugin_p.h>
#include <QQuickView>
#include <QQuickWindow>
#include <QtGui/private/qguiapplication_p.h>
#include <QPluginLoader>
#include <QtPlugin>
#include <QByteArray>
#include <QLibraryInfo>
#include <QStringList>
#include "ui_rendercontrol.h"
#include "../tig/tig_startup.h"
#include <graphics/device.h>
#include <graphics/shaperenderer2d.h>
#include "../qsgd3d11/qsgd3d11renderloop_p.h"
#include "../qml/networkaccessmanager.h"
#include <infrastructure/vfs.h>
#include <qpa/qplatformintegration.h>
#include <qpa/qplatformfontdatabase.h>

struct UiRenderControl::Impl {
	std::unique_ptr<QQmlEngine> engine;
	TPNetworkAccessManagerFactory namFactory;
	
	std::unique_ptr<QSGContext> context;
	std::unique_ptr<QSGRenderContext> renderContext;
	QSGD3D11RenderLoop *renderLoop = nullptr;
};

Q_IMPORT_PLUGIN(QSGD3D11Adaptation)

#include <QtQml/5.8.0/QtQml/private/qqmlengine_p.h>
#include <QtQml/5.8.0/QtQml/private/qqmlimport_p.h>

UiRenderControl::UiRenderControl() : impl(std::make_unique<Impl>())
{
	QQuickWindow::setSceneGraphBackend("d3d11");
	QQuickWindow::setDefaultAlphaBuffer(true);

	auto device = tig->GetRenderingDevice().GetDevice();
	auto context = tig->GetRenderingDevice().GetContext();
	auto swapChain = tig->GetRenderingDevice().GetSwapChain();
	
	impl->renderLoop = new QSGD3D11RenderLoop(device, context, swapChain);
	QSGRenderLoop::setInstance(impl->renderLoop);

	impl->engine = std::make_unique<QQmlEngine>();
	static TPNetworkAccessManagerFactory namFactory;
	impl->engine->setNetworkAccessManagerFactory(&namFactory);
	impl->engine->setBaseUrl(QUrl("tio:///"));

	// This ensures we dont accidentally pick up the QtCreatorPlugin for the "TemplePlus" URI
	// because by default the exe directory is also in the search path
	QStringList importDirs = QStringList()
		<< QLibraryInfo::location(QLibraryInfo::Qml2ImportsPath)
		<< "tio:///qml/";
	impl->engine->setImportPathList(importDirs);

	QObject::connect(impl->engine.get(), &QQmlEngine::warnings, [=](const QList<QQmlError> &warnings) {
		for (auto &error : warnings) {
			logger->warn("{}", error.toString().toStdString());
		}
	});
	
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

QQuickView * UiRenderControl::CreateView(const std::string & mainFile)
{
	auto view = new QQuickView(impl->engine.get(), nullptr);
	view->setClearBeforeRendering(true);
	view->setColor(QColor(Qt::transparent));

	QObject::connect(view, &QQuickView::statusChanged, [=](QQuickView::Status status) {
		if (status == QQuickView::Error) {
			for (auto &error : view->errors()) {
				logger->warn("{}", error.toString().toStdString());
			}
		}
	});

	view->setSource(QUrl(QString::fromStdString(mainFile)));

	return view;
}
