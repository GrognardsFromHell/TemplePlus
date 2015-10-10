#include "stdafx.h"

#include <atlcomcli.h>
#include <infrastructure/images.h>

#include "textures.h"

#include <tio/tio.h>
#include <util/config.h>

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
		static gfx::ContentRect rect{0,0,1,1};
		return rect;
	}

	const gfx::Size& GetSize() const override {
		static gfx::Size size{1, 1};
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

class Texture;

class TextureLoader {
public:
	explicit TextureLoader(const CComPtr<IDirect3DDevice9>& iDirect3DDevice9, size_t memoryBudget)
		: mDevice(iDirect3DDevice9), mMemoryBudget(memoryBudget) {
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
	Texture *mLeastRecentlyUsed = nullptr;
	Texture *mMostRecentlyUsed = nullptr;
private:
	CComPtr<IDirect3DDevice9> mDevice;

	int mLoaded = 0;
	size_t mEstimatedUsage = 0;
	size_t mMemoryBudget;
};

static void RoundUpToPow2(int& dimension) {

	static int sPow2Dimensions[] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048};

	for (auto powDimension : sPow2Dimensions) {
		if (powDimension >= dimension) {
			dimension = powDimension;
			return;
		}
	}

}

CComPtr<IDirect3DTexture9> TextureLoader::Load(const std::string& filename, gfx::ContentRect& contentRectOut, gfx::Size& sizeOut) {

	auto file = tio_fopen(filename.c_str(), "rb");
	if (!file) {
		logger->info("Cannot open texture {}", filename);
		return nullptr;
	}

	size_t size = tio_filelength(file);
	std::unique_ptr<uint8_t[]> data(new uint8_t[size]);
	if (tio_fread(data.get(), size, 1, file) != 1) {
		logger->info("Cannot read texture {}", filename);
		tio_fclose(file);
	}
	tio_fclose(file);

	try {
		auto image(gfx::DecodeImage({data.get(), size}));

		auto texWidth = image.info.width;
		RoundUpToPow2(texWidth);
		auto texHeight = image.info.height;
		RoundUpToPow2(texHeight);

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
		if (!config.useDirect3d9Ex) {
			pool = D3DPOOL_MANAGED;
			usage = 0;
		}

		auto result = D3DLOG(mDevice->CreateTexture(texWidth, texHeight, 1, usage, format, pool, &texture, nullptr));
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


class Texture : public gfx::Texture {
friend class TextureManager;
friend class TextureLoader;
public:

	explicit Texture(std::shared_ptr<TextureLoader> loader, int id, const std::string& name)
		: mLoader(loader), mId(id), mFilename(name) {
	}

	~Texture();

	int GetId() const override {
		return mId;
	}

	const std::string& GetName() const override {
		return mFilename;
	}

	const gfx::ContentRect& GetContentRect() const override {
		if (!mMetadataValid) {
			const_cast<Texture*>(this)->Load();
		}
		return mContentRect;
	}

	const gfx::Size& GetSize() const override {
		if (!mMetadataValid) {
			const_cast<Texture*>(this)->Load();
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

	Texture* mNextMoreRecentlyUsed = nullptr;
	Texture* mNextLessRecentlyUsed = nullptr;
	
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

Texture::~Texture() {
	DisconnectMru();
}

void Texture::MarkUsed() {
	mUsedThisFrame = true;
		
	if (mLoader->mMostRecentlyUsed == this)
		return; // Already MRU

	// Disconnect from current position of MRU list
	DisconnectMru();

	// Insert to front of MRU list
	MakeMru();
}

void Texture::MakeMru() {
	if (mLoader->mMostRecentlyUsed) {
		mLoader->mMostRecentlyUsed->mNextMoreRecentlyUsed = this;
	}

	mLoader->mMostRecentlyUsed = this;

	if (!mLoader->mLeastRecentlyUsed) {
		mLoader->mLeastRecentlyUsed = this;
	}
}

void Texture::DisconnectMru() {
	if (mNextLessRecentlyUsed) {
		mNextLessRecentlyUsed->mNextMoreRecentlyUsed = mNextMoreRecentlyUsed;
	}
	if (mNextMoreRecentlyUsed) {
		mNextMoreRecentlyUsed->mNextLessRecentlyUsed = mNextLessRecentlyUsed;
	}
	if (mLoader->mLeastRecentlyUsed == this) {
		mLoader->mLeastRecentlyUsed = mNextLessRecentlyUsed;
	}
	if (mLoader->mMostRecentlyUsed == this) {
		mLoader->mMostRecentlyUsed = mNextMoreRecentlyUsed;
	}
	mNextLessRecentlyUsed = nullptr;
	mNextMoreRecentlyUsed = nullptr;
}

static gfx::TextureRef sInvalidTexture(std::make_shared<InvalidTexture>());

TextureManager::TextureManager(IDirect3DDevice9* device, size_t memoryBudget)
	: mLoader(std::make_shared<TextureLoader>(device, memoryBudget)) {
}

void TextureManager::FreeUnusedTextures() {
	
	mLoader->FreeUnusedTextures();

}

void TextureManager::FreeAllTextures() {

	for (auto& entry : mTexturesByName) {
		entry.second->FreeDeviceTexture();
	}

}

gfx::TextureRef TextureManager::Resolve(const std::string& filename, bool withMipMaps) {

	auto filenameLower = tolower(filename);

	auto it = mTexturesByName.find(filenameLower);
	if (it != mTexturesByName.end()) {
		return it->second;
	}

	// Texture is not registered yet, so let's do that
	if (!tio_fileexists(filename.c_str())) {
		logger->error("Cannot register texture '{}', because it does not exist.", filename);
		mTexturesByName[filenameLower] = sInvalidTexture;
		return sInvalidTexture;
	}

	auto id = mNextFreeId++;

	auto texture(std::make_shared<Texture>(mLoader, id, filename));

	Expects(mTexturesByName.find(filenameLower) == mTexturesByName.end());
	Expects(mTexturesById.find(id) == mTexturesById.end());
	mTexturesByName[filenameLower] = texture;
	mTexturesById[id] = texture;

	return texture;

}

gfx::TextureRef TextureManager::GetById(int textureId) {

	auto it = mTexturesById.find(textureId);

	if (it != mTexturesById.end()) {
		return it->second;
	}

	return sInvalidTexture;

}

int TextureManager::GetLoaded() {
	return mLoader->GetLoaded();
}

int TextureManager::GetRegistered() {
	return mTexturesById.size();
}

size_t TextureManager::GetUsageEstimate() {
	return mLoader->GetEstimatedUsage();
}

size_t TextureManager::GetMemoryBudget() {
	return mLoader->GetMemoryBudget();
}
