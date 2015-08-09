#pragma once

#include <exception>
#include <string>

class D3D8AdapterException : std::exception
{
public:
	explicit D3D8AdapterException(const std::string &msg) : mMsg(msg) {}

	const char* what() const override {
		return mMsg.c_str();
	}
private:
	std::string mMsg;
};

void SetDirect3d9Ex(bool enable);

// Forward declarations for all adapters in an attempt to not require the d3d8 headers in the main program
struct Direct3DVertexBuffer8Adapter;
struct Direct3DIndexBuffer8Adapter;
struct Direct3DTexture8Adapter;
struct Direct3DDevice8Adapter;
struct Direct3DSurface8Adapter;
struct Direct3D8Adapter;

// Accessor functions to get underlying delegates
struct IDirect3DTexture9;
IDirect3DTexture9 *GetTextureDelegate(Direct3DTexture8Adapter *adapter);
Direct3DTexture8Adapter *CreateTextureAdapter();

struct IDirect3DDevice9;
IDirect3DDevice9 *GetDeviceDelegate(Direct3DDevice8Adapter *adapter);

Direct3DVertexBuffer8Adapter* CreateVertexBufferAdapter();

Direct3DIndexBuffer8Adapter* CreateIndexBufferAdapter();
