
#include "stdafx.h"
#include "fade.h"

Fade fade;

static struct FadeAddresses : AddressTable {
	bool (__cdecl *FadeAndTeleport)(const FadeAndTeleportArgs &args);
	void (__cdecl *Fade)(const FadeArgs &args);

	FadeAddresses() {
		rebase(FadeAndTeleport, 0x10084A50);
		rebase(Fade, 0x10051C00);
	}
} addresses;

bool Fade::FadeAndTeleport(const FadeAndTeleportArgs& args) {
	return addresses.FadeAndTeleport(args);
}

void Fade::PerformFade(const FadeArgs& args) {
	addresses.Fade(args);
}
