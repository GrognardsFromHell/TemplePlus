
#include "infrastructure/logging.h"

#include "platform/d3d.h"
#include "platform/dxerr.h"

void LogD3dError(const char* method, HRESULT result) {
	logger->warn("Direct3D Error @ {}: {}", method, DXGetErrorString(result));
}
