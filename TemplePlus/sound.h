
#pragma once

#include "obj.h"


struct MilesSoundSthg {
	char isUsed;
	char field1;
	char field2;
	char field3;
	int flags;
	int field8;
	int fieldC;
	int field10;
	void* streamSthg;
	int quickLoadMemSthg;
	int m3dSampleSthg;
	int field20;
	int field24[65];
	int field128;
	int curVolume;
	int field130;
	int field134;
	int field138;
	int field13c;
	int64_t x;
	int64_t y;
	int field150;
	int field154;
};

/*
	High level sound and music wrappers around tig_sound.
*/
class Sound {
	friend class SoundHooks;
	friend class SoundGameSystem;
public:

	/*
		Plays a sound with the given sound id (from the mes files).
		Returns the tig_sound stream id.
	*/
	int PlaySound(int soundId, int loopCount = 1);

	/*
		Plays a sound at the location of the given object.
		Otherwise works as PlaySound
	*/
	int PlaySoundAtObj(int soundId, objHndl obj, int loopCount = 1);
	
	/*
		Plays a sound at the given tile.
		Otherwise works as PlaySound
	*/
	int PlaySoundAtLoc(int soundId, locXY loc, int loopCount = 1);

	/*
	low level sound player?
	*/
	int MssPlaySound(int soundId);

	void MssFreeStream(int streamId);

private:
	void Init();
	static std::map<int, std::string> mUserSounds;
	MesHandle tpSounds = -1;
};

extern Sound sound;
