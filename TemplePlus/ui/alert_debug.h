#pragma once
struct objHndl;
namespace alertDebug {
	bool Enabled();
	void SetEnabled(bool enabled);
	void Record(objHndl center, objHndl recipient, int range, const char* reason);
	void Render();
}
