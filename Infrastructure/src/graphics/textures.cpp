
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

	CComPtr<IDirect3DTexture9> Load(const std::string& filename,
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

/*static void RoundUpToPow2(int& dimension) {

	static int sPow2Dimensions[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048};

	for (auto powDimension : sPow2Dimensions) {
		if (powDimension >= dimension) {
			dimension = powDimension;
			return;
		}
	}

}*/

CComPtr<IDirect3DTexture9> TextureLoader::Load(const std::string& filename, gfx::ContentRect& contentRectOut, gfx::Size& sizeOut) {

	auto textureData(vfs->ReadAsBinary(filename));

	try {
		auto image(gfx::DecodeImage(textureData));

		auto texWidth = image.info.width;
		// RoundUpToPow2(texWidth);
		auto texHeight = image.info.height;
		// RoundUpToPow2(texHeight);

		contentRectOut.x = 0;
		contentRectOut.y = 0;
		contentRectOut.width = image.info.width;
		contentRectOut.height = image.info.height;

		sizeOut.width = texWidth;
		sizeOut.height = texHeight;

		CComPtr<IDirect3DTexture9> texture;
		auto format = D3DFMT_A8R8G8B8;
		auto pool = D3DPOOL_DEFAULT;
		auto usage = D3DUSAGE_DYNAMIC;

		auto device = mDevice.GetDevice();
		auto result = D3DLOG(device->CreateTexture(texWidth, texHeight, 1, usage, format, pool, &texture, nullptr));
		if (result != D3D_OK) {
			return nullptr;
		}

		D3DLOCKED_RECT locked;
		result = D3DLOG(texture->LockRect(0, &locked, nullptr, D3DLOCK_DISCARD));
		if (result != D3D_OK) {
			return nullptr;
		}

		auto dest = reinterpret_cast<uint8_t*>(locked.pBits);
		auto src = reinterpret_cast<uint8_t*>(image.data.get());
		for (int y = 0; y < image.info.height; y++) {
			memcpy(dest, src, image.info.width * 4);
			dest += locked.Pitch;
			src += image.info.width * 4;
		}

		D3DLOG(texture->UnlockRect(0));

		mLoaded++;
		mEstimatedUsage += texWidth * texHeight * 4;

		return texture;
	} catch (std::exception& e) {
		logger->error("Unable to load texture {}: {}", filename, e.what());
		return nullptr;
	}

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

	void FreeDeviceTexture() override {
		if (mDeviceTexture) {
			mDeviceTexture.Release();
			mLoader->Unload(mSize);
			DisconnectMru();
		}
	}

	IDirect3DTexture9* GetDeviceTexture() override {
		if (!mDeviceTexture) {
			Load();
		}
		MarkUsed();
		return mDeviceTexture;
	}

private:

	void MarkUsed();

	void Load() {
		Expects(!mDeviceTexture);
		mDeviceTexture = mLoader->Load(mFilename, mContentRect, mSize);
		if (mDeviceTexture) {
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
	mutable CComPtr<IDirect3DTexture9> mDeviceTexture;

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

	IDirect3DTexture9* GetDeviceTexture() override {
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
