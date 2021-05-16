#include "stdafx.h"

#include <graphics/bufferbinding.h>
#include <graphics/materials.h>

#include <infrastructure/vfs.h>
#include <infrastructure/binaryreader.h>

#include "clipping.h"
#include "graphics/shaders.h"
#include "clippingmesh.h"
#include "../../gameview.h"
#include "config/config.h"

using namespace gfx;

class ClippingSystem::Impl {
public:
	explicit Impl(RenderingDevice& g, WorldCamera& camera) : 
		device(g),
		camera(camera),
		material(CreateMaterial(g)),
		debugMaterial(CreateDebugMaterial(g)),
		mBufferBinding(material.GetVertexShader())
	{
		mBufferBinding.AddBuffer(nullptr, 0, sizeof(XMFLOAT3))
			.AddElement(VertexElementType::Float3, VertexElementSemantic::Position);
	}

	std::vector<std::unique_ptr<ClippingMesh>> mClippingMeshes;

	RenderingDevice& device;
	WorldCamera& camera;
	Material material;
	Material debugMaterial;
	BufferBinding mBufferBinding;
	bool debug = false;
	size_t rendered = 0;

	static Material CreateMaterial(RenderingDevice &device);
	static Material CreateDebugMaterial(RenderingDevice &device);
};

Material ClippingSystem::Impl::CreateMaterial(RenderingDevice& device) {
	RasterizerSpec rasterizerState;
	rasterizerState.cullMode = CullMode::None;

	BlendSpec blendState;
	// Clipping geometry does not write color
	blendState.writeAlpha = false;
	blendState.writeRed = false;
	blendState.writeGreen = false;
	blendState.writeBlue = false;

	auto vs(device.GetShaders().LoadVertexShader("clipping_vs"));
	auto ps(device.GetShaders().LoadPixelShader("clipping_ps"));

	return device.CreateMaterial(blendState, {}, rasterizerState, {}, vs, ps);

}

Material ClippingSystem::Impl::CreateDebugMaterial(RenderingDevice& device) {

	RasterizerSpec rasterizerState;
	rasterizerState.wireframe = true;

	auto vs(device.GetShaders().LoadVertexShader("clipping_vs"));
	auto ps(device.GetShaders().LoadPixelShader("clipping_ps"));

	return device.CreateMaterial({}, {}, rasterizerState, {}, vs, ps);

}

ClippingSystem::ClippingSystem(RenderingDevice& g, WorldCamera &camera) 
	: mImpl(std::make_unique<Impl>(g, camera)) {
}

ClippingSystem::~ClippingSystem() {
}

void ClippingSystem::Load(const std::string& directory) {

	Unload();

	LoadMeshes(directory);
	LoadObjects(directory);

}

void ClippingSystem::Unload() {
	mImpl->mClippingMeshes.clear();
}

void ClippingSystem::SetDebug(bool enable) {
	mImpl->debug = enable;
}

bool ClippingSystem::IsDebug() const {
	return mImpl->debug;
}

size_t ClippingSystem::GetTotal() const
{
	size_t total = 0;
	for (auto &mesh : mImpl->mClippingMeshes) {
		if (mesh) {
			total += mesh->GetInstances().size();
		}
	}
	return total;
}

size_t ClippingSystem::GetRenderered() const
{
	return mImpl->rendered;
}

const std::string & ClippingSystem::GetName() const
{
	static std::string sName = "Clipping";
	return sName;
}

void ClippingSystem::LoadMeshes(const std::string& directory) {

	auto filename(fmt::format("{}\\clipping.cgf", directory));
	auto data(vfs->ReadAsBinary(filename));

	BinaryReader reader(data);

	auto count = reader.Read<int32_t>();
	mImpl->mClippingMeshes.reserve(count);

	for (auto i = 0; i < count; ++i) {
		auto meshFilename(reader.ReadFixedString(260));
		if (meshFilename.empty()) {
			logger->error("Failed to read filename of clipping mesh #{} from {}", i, filename);
			mImpl->mClippingMeshes.push_back({});
			continue;
		}

		try {
			mImpl->mClippingMeshes.emplace_back(
				std::make_unique<ClippingMesh>(mImpl->device, meshFilename)
			);
		} catch (TempleException &e) {
			logger->error("Failed to load clipping mesh {}: {}", filename, e.what());
			mImpl->mClippingMeshes.push_back({});
		}
	}

}

void ClippingSystem::LoadObjects(const std::string& directory) {

	auto indexFileName(fmt::format("{}\\clipping.cif", directory));
	auto data(vfs->ReadAsBinary(indexFileName));

	BinaryReader reader(data);

	auto count = reader.Read<int32_t>();

	for (auto i = 0; i < count; ++i) {
		LoadObject(reader);
	}
}

void ClippingSystem::LoadObject(BinaryReader& reader) {

	auto meshIdx = reader.Read<size_t>();

	auto mesh = mImpl->mClippingMeshes[meshIdx].get();
	
	ClippingMeshObj obj;
	obj.posX = reader.Read<float>();
	obj.posY = reader.Read<float>();
	obj.posZ = reader.Read<float>();
	obj.scaleX = reader.Read<float>();
	obj.scaleY = reader.Read<float>();
	obj.scaleZ = reader.Read<float>();
	obj.rotation = reader.Read<float>();

	if (!mesh) {
		logger->warn("Discarding clipping instance for invalid mesh #{}", meshIdx);
		return;
	}

	mesh->AddInstance(obj);
}

struct ClippingGlobals {
	XMFLOAT4X4 viewProj;
	XMFLOAT4 rotCos;
	XMFLOAT4 rotSin;
	XMFLOAT4 pos;
	XMFLOAT4 scale;
};

void ClippingSystem::Render() {

	gfx::PerfGroup perfGroup(mImpl->device, "Clipping");

	mImpl->rendered = 0;

	if (mImpl->mClippingMeshes.empty()) {
		return;
	}
	
	if (mImpl->debug) {
		mImpl->device.SetMaterial(mImpl->debugMaterial);
	} else {
		mImpl->device.SetMaterial(mImpl->material);
	}

	auto& camera = mImpl->camera;

	ClippingGlobals globals;
	globals.viewProj = camera.GetViewProj();

	// For clipping purposes
	auto screenCenterWorld = gameView->GetScreenCenterInWorld3d();

	for (auto& mesh : mImpl->mClippingMeshes) {
				
		if (!mesh) {
			continue; // This is a mesh that failed to load
		}
	
		mImpl->mBufferBinding.SetBuffer(0, mesh->GetVertexBuffer());
		mImpl->mBufferBinding.Bind();
		mImpl->device.SetIndexBuffer(*mesh->GetIndexBuffer());
		
		for (auto& obj : mesh->GetInstances()) {
			XMFLOAT3 sphereCenter = mesh->GetBoundingSphereOrigin();

			// Sphere pos relative to computed screen center
			auto relXPos = obj.posX - sphereCenter.x - screenCenterWorld.x;
			auto relZPos = obj.posZ - sphereCenter.y - screenCenterWorld.z;

			// Distance of the sphere center in screen coordinates from the screen center
			auto distX = fabs(relZPos * 0.70710599f - relXPos * 0.70710599f);
			auto distY = fabs(relZPos * 0.50497407f + relXPos * 0.50497407f - (sphereCenter.z + obj.posY) * 0.7f);
			
			auto maxScale = std::max(std::max(obj.scaleX, obj.scaleY), obj.scaleZ);
			auto scaledRadius = maxScale * mesh->GetBoundingSphereRadius();
			bool culled = (distX > (long double)(camera.GetScreenWidth() / 2.0f) + scaledRadius
				|| distY > (camera.GetScreenHeight() / 2.0f) + scaledRadius);

			if (culled) {
				continue;
			}
				
			mImpl->rendered++;

			globals.rotCos.x = cosf(obj.rotation);
			globals.rotSin.x = sinf(obj.rotation);

			globals.pos.x = obj.posX;
			globals.pos.y = obj.posY;
			globals.pos.z = obj.posZ;

			globals.scale.x = obj.scaleX;
			globals.scale.y = obj.scaleY;
			globals.scale.z = obj.scaleZ;
			
			mImpl->device.SetVertexShaderConstants(0, globals);

			mImpl->device.DrawIndexed(
				gfx::PrimitiveType::TriangleList,
				mesh->GetVertexCount(),
				mesh->GetTriCount() * 3
			);
		}

	}

}
