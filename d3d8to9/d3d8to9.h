#ifndef D3D8TO9_H
#define D3D8TO9_H

struct Direct3DVertexBuffer8Adapter;
struct Direct3DIndexBuffer8Adapter;
struct Direct3DTexture8Adapter;
struct Direct3DDevice8Adapter;
struct Direct3DSurface8Adapter;
struct Direct3D8Adapter;

#include "d3d8to9_rootobj.h"
#include "d3d8to9_device.h"
#include "d3d8to9_vertexbuffer.h"

extern bool enableLinearPresent;

#endif // D3D8TO9_H

