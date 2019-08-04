
#include "platform/d3d.h"

#include "infrastructure/images.h"
#include "infrastructure/vfs.h"
#include "infrastructure/logging.h"
#include "infrastructure/stringutil.h"

#include "graphics/textures.h"
#include "graphics/device.h"

namespace gfx {

class FileTexture;

class TextureLoader {
public:
	explicit TextureLoader(RenderingDevice &device, size_t memoryBudget)
		: mDevice(device), mMemoryBudget(memoryBudget) {
	}

	CComPtr<ID3D11ShaderResourceView> Load(const std::string& filename,
	                                gfx::ContentRect& contentRectOut,
	                                gfx::Size& sizeOut);

	void Unload(const gfx::Size &size) {
		mLoaded--;

		mEstimatedUsage -= size.width * size.height * 4;
	}

	int GetLoaded() const {
		return mLoaded;
	}

	size_t GetEstimatedUsage() const {
		return mEstimatedUsage;
	}

	size_t GetMemoryBudget() const {
		return mMemoryBudget;
	}

	void FreeUnusedTextures();
	FileTexture *mLeastRecentlyUsed = nullptr;
	FileTexture *mMostRecentlyUsed = nullptr;
private:
	RenderingDevice &mDevice;

	int mLoaded = 0;
	size_t mEstimatedUsage = 0;
	size_t mMemoryBudget;
};

CComPtr<ID3D11ShaderResourceView> TextureLoader::Load(const std::string& filename, gfx::ContentRect& contentRectOut, gfx::Size& sizeOut) {

	CComPtr<ID3D11ShaderResourceView> result;

	auto textureData(vfs->ReadAsBinary(filename));

	try {
		gfx::DecodedImage image;
		
		if (endsWith(tolower(filename), ".img") && textureData.size() == 4) {
			image = std::move(gfx::DecodeCombinedImage(filename, textureData));
		} else {
			image = std::move(gfx::DecodeImage(textureData));
		}	

		auto texWidth = image.info.width;
		auto texHeight = image.info.height;

		contentRectOut.x = 0;
		contentRectOut.y = 0;
		contentRectOut.width = image.info.width;
		contentRectOut.height = image.info.height;

		sizeOut.width = texWidth;
		sizeOut.height = texHeight;

		// Load the D3D11 one
		CD3D11_TEXTURE2D_DESC textureDesc(DXGI_FORMAT_B8G8R8A8_UNORM, texWidth, texHeight, 1, 1,
			D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_IMMUTABLE);

		D3D11_SUBRESOURCE_DATA initialData;
		memset(&initialData, 0, sizeof(initialData));
		initialData.pSysMem = image.data.get();
		initialData.SysMemPitch = image.info.width * 4;

		CComPtr<ID3D11Texture2D> texture;
		D3DVERIFY(mDevice.mD3d11Device->CreateTexture2D(&textureDesc, &initialData, &texture));
		
		if (mDevice.IsDebugDevice()) {
			texture->SetPrivateData(WKPDID_D3DDebugObjectName, filename.size(), filename.c_str());
		}

		// Make a shader resource view for the texture since that's the only thing we're interested in here		
		CD3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc(texture, D3D_SRV_DIMENSION_TEXTURE2D);
		D3DVERIFY(mDevice.mD3d11Device->CreateShaderResourceView(texture, &resourceViewDesc, &result));

		mLoaded++;
		mEstimatedUsage += texWidth * texHeight * 4;

	} catch (std::exception& e) {
		logger->error("Unable to load texture {}: {}", filename, e.what());
	}

	return result;

}


class FileTexture : public gfx::Texture {
friend class TextureManager;
friend class TextureLoader;
public:

	explicit FileTexture(std::shared_ptr<TextureLoader> loader, int id, const std::string& name)
		: mLoader(loader), mId(id), mFilename(name) {
	}
	~FileTexture();

	int GetId() const override {
		return mId;
	}

	const std::string& GetName() const override {
		return mFilename;
	}

	const gfx::ContentRect& GetContentRect() const override {
		if (!mMetadataValid) {
			const_cast<FileTexture*>(this)->Load();
		}
		return mContentRect;
	}

	const gfx::Size& GetSize() const override {
		if (!mMetadataValid) {
			const_cast<FileTexture*>(this)->Load();
		}
		return mSize;
	}

	TextureType GetType() const override {
		return TextureType::File;
	}

	void FreeDeviceTexture() override {
		if (mResourceView) {
			mResourceView.Release();
			mLoader->Unload(mSize);
			DisconnectMru();
		}
	}

	ID3D11ShaderResourceView* GetResourceView() override {
		if (!mResourceView && !mResourceView) {
			Load();
		}
		MarkUsed();
		return mResourceView;
	}

private:

	void MarkUsed();

	void Load() {
		Expects(!mResourceView);
		mResourceView = mLoader->Load(mFilename, mContentRect, mSize);
		
		if (mResourceView) {
			mMetadataValid = true;
						
			// The texture should not be in the MRU cache at this point
			Expects(!this->mNextMoreRecentlyUsed);
			Expects(!this->mNextLessRecentlyUsed);
			MakeMru();
		}
	}

	void MakeMru();
	void DisconnectMru();

	bool mUsedThisFrame = false;
	std::shared_ptr<TextureLoader> mLoader;
	int mId;
	std::string mFilename;
	mutable bool mMetadataValid = false;
	mutable gfx::ContentRect mContentRect;
	mutable gfx::Size mSize;
	mutable CComPtr<ID3D11ShaderResourceView> mResourceView;

	FileTexture* mNextMoreRecentlyUsed = nullptr;
	FileTexture* mNextLessRecentlyUsed = nullptr;
	
};

void TextureLoader::FreeUnusedTextures() {
	
	// Start with the least recently used texture
	auto texture = mLeastRecentlyUsed;
	while (texture) {

		if (texture->mUsedThisFrame) {
			break;
		}

		auto aboutToDelete = texture;

		texture = texture->mNextMoreRecentlyUsed;		
		
		if (mEstimatedUsage > mMemoryBudget) {
			aboutToDelete->FreeDeviceTexture();
		}

	}

	// Reset the rest of the textures to not be used this frame
	while (texture) {
		texture->mUsedThisFrame = false;
		texture = texture->mNextMoreRecentlyUsed;
	}
}

FileTexture::~FileTexture() {
	DisconnectMru();
}

void FileTexture::MarkUsed() {
	mUsedThisFrame = true;
		
	if (mLoader->mMostRecentlyUsed == this)
		return; // Already MRU

	// Disconnect from current position of MRU list
	DisconnectMru();

	// Insert to front of MRU list
	MakeMru();
}

void FileTexture::MakeMru() {
	if (mLoader->mMostRecentlyUsed) {
		Expects(!mLoader->mMostRecentlyUsed->mNextMoreRecentlyUsed);
		mLoader->mMostRecentlyUsed->mNextMoreRecentlyUsed = this;		
	}

	mNextLessRecentlyUsed = mLoader->mMostRecentlyUsed;

	mLoader->mMostRecentlyUsed = this;

	if (!mLoader->mLeastRecentlyUsed) {
		mLoader->mLeastRecentlyUsed = this;
	}
}

void FileTexture::DisconnectMru() {
	if (mNextLessRecentlyUsed) {
		Expects(mNextLessRecentlyUsed->mNextMoreRecentlyUsed == this);
		mNextLessRecentlyUsed->mNextMoreRecentlyUsed = mNextMoreRecentlyUsed;
	}
	if (mNextMoreRecentlyUsed) {
		Expects(mNextMoreRecentlyUsed->mNextLessRecentlyUsed == this);
		mNextMoreRecentlyUsed->mNextLessRecentlyUsed = mNextLessRecentlyUsed;
	}
	if (mLoader->mLeastRecentlyUsed == this) {
		mLoader->mLeastRecentlyUsed = mNextMoreRecentlyUsed;
		Expects(!mNextMoreRecentlyUsed
			|| mLoader->mLeastRecentlyUsed->mNextLessRecentlyUsed == nullptr);
	}
	if (mLoader->mMostRecentlyUsed == this) {
		mLoader->mMostRecentlyUsed = mNextLessRecentlyUsed;
		Expects(!mNextLessRecentlyUsed
			|| mLoader->mMostRecentlyUsed->mNextMoreRecentlyUsed == nullptr);
	}
	mNextLessRecentlyUsed = nullptr;
	mNextMoreRecentlyUsed = nullptr;
}

Textures::Textures(RenderingDevice& device, size_t memoryBudget)
	: mLoader(std::make_shared<TextureLoader>(device, memoryBudget)) {
}

void Textures::FreeUnusedTextures() {
	
	mLoader->FreeUnusedTextures();

}

void Textures::FreeAllTextures() {

	for (auto& entry : mTexturesByName) {
		entry.second->FreeDeviceTexture();
	}

}

gfx::TextureRef Textures::Resolve(const std::string& filename, bool withMipMaps) {

	auto filenameLower = tolower(filename);

	auto it = mTexturesByName.find(filenameLower);
	if (it != mTexturesByName.end()) {
		return it->second;
	}

	// Texture is not registered yet, so let's do that
	if (!vfs->FileExists(filename)) {
		logger->error("Cannot register texture '{}', because it does not exist.", filename);
		auto result = Texture::GetInvalidTexture();
		mTexturesByName[filenameLower] = result;
		return result;
	}

	auto id = mNextFreeId++;

	auto texture(std::make_shared<FileTexture>(mLoader, id, filename));

	Expects(mTexturesByName.find(filenameLower) == mTexturesByName.end());
	Expects(mTexturesById.find(id) == mTexturesById.end());
	mTexturesByName[filenameLower] = texture;
	mTexturesById[id] = texture;

	return texture;

}

gfx::TextureRef Textures::Override(const std::string & filename, bool withMipMaps)
{
	auto filenameLower = tolower(filename);

	auto it = mTexturesByName.find(filenameLower);
	auto id = -1;
	if (it != mTexturesByName.end()) {
		//return it->second;
		id = it->second.get()->GetId();
	}

	// Texture is not registered yet, so let's do that
	if (!vfs->FileExists(filename)) {
		logger->error("Cannot register texture '{}', because it does not exist.", filename);
		auto result = Texture::GetInvalidTexture();
		mTexturesByName[filenameLower] = result;
		return result;
	}

	if (id == -1)
		id = mNextFreeId++;

	auto texture(std::make_shared<FileTexture>(mLoader, id, filename));

	//Expects(mTexturesByName.find(filenameLower) == mTexturesByName.end());
	//Expects(mTexturesById.find(id) == mTexturesById.end());
	mTexturesByName[filenameLower] = texture;
	mTexturesById[id] = texture;

	return texture;
}

gfx::TextureRef Textures::ResolveUncached(const std::string & filename, bool withMipMaps)
{
	// Texture is not registered yet, so let's do that
	if (!vfs->FileExists(filename)) {
		logger->error("Cannot laod texture '{}', because it does not exist.", filename);
		return Texture::GetInvalidTexture();
	}

	return std::make_shared<FileTexture>(mLoader, -1, filename);
}

gfx::TextureRef Textures::GetById(int textureId) {

	if (textureId == -1) {
		return gfx::Texture::GetInvalidTexture();
	}

	auto it = mTexturesById.find(textureId);

	if (it != mTexturesById.end()) {
		return it->second;
	}

	logger->info("Trying to retrieve unknown texture id {}", textureId);
	return Texture::GetInvalidTexture();

}

int Textures::GetLoaded() {
	return mLoader->GetLoaded();
}

int Textures::GetRegistered() {
	return mTexturesById.size();
}

size_t Textures::GetUsageEstimate() {
	return mLoader->GetEstimatedUsage();
}

size_t Textures::GetMemoryBudget() {
	return mLoader->GetMemoryBudget();
}

class InvalidTexture : public gfx::Texture {
public:
	const std::string& GetName() const override {
		static std::string sInvalidName("<invalid>");
		return sInvalidName;
	}

	int GetId() const override {
		return -1;
	}

	const gfx::ContentRect& GetContentRect() const override {
		static gfx::ContentRect rect{ 0,0,1,1 };
		return rect;
	}

	const gfx::Size& GetSize() const override {
		static gfx::Size size{ 1, 1 };
		return size;
	}

	void FreeDeviceTexture() override {
	}

	TextureType GetType() const override {
		return TextureType::Invalid;
	}

	ID3D11ShaderResourceView* GetResourceView() override {
		return nullptr;
	}

	bool IsValid() const override {
		return false;
	}
};

std::shared_ptr<class Texture> Texture::GetInvalidTexture() {
	static std::shared_ptr<Texture> instance(std::make_shared<InvalidTexture>());
	return instance;
}

}
