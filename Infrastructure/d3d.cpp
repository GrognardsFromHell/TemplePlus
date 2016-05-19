
#include "infrastructure/logging.h"
#include "infrastructure/exception.h"

#include "platform/d3d.h"

void LogD3dError(const char* method, HRESULT result) {

	static char sMessageBuffer[65535];

	auto written = FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM,
		0,
		result,
		0,
		sMessageBuffer,
		65535,
		nullptr
	);

	sMessageBuffer[written] = '\0';

	logger->warn("Direct3D Error 0x{:x} @ {}: {}", result, method, sMessageBuffer);

}

void ThrowD3dAssertion(const char *method, HRESULT result)
{
	LogD3dError(method, result);
	throw new TempleException("Failed Direct3D Operation: {} (Result = 0x{:x})", method, result);
}
