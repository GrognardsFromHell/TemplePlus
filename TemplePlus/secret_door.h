#pragma once
#include "common.h"
#include <temple/dll.h>
#include "d20.h"
#include "gamesystems/gamesystem.h"

class SecretdoorSystem : public GameSystem, public SaveGameAwareGameSystem, public ResetAwareGameSystem {
	friend class SecretDoorSys;
public:
	static constexpr auto Name = "Secretdoor";
	SecretdoorSystem(const GameSystemConf& config);
	void Reset() override;
	bool SaveGame(TioFile* file) override;
	bool LoadGame(GameSystemSaveFile* saveFile) override;
	const std::string& GetName() const override;

private:
	int* mNamesSeen = temple::GetRef<int[100]>(0x109DD880);
};

class SecretDoorSys : temple::AddressTable
{
public:
	bool isSecretDoor(objHndl obj);
	int getSecretDoorDC(objHndl obj);
	uint32_t secretDoorIsRevealed(objHndl secDoor);
	int SearchConcentratedPerformFunc(D20Actn *d20a);
	int SecretDoorSkillCheck(objHndl seeker, objHndl *foundObject);
	int SearchEventSecretDoorDetect(objHndl sd, objHndl seeker);
	int SecretDoorRollAndReveal(objHndl secdoor, objHndl seeker, BonusList* bonList);
	BOOL SecretDoorDetect(objHndl sd, objHndl seeker);
	int SecretDoorGetDCAutoDetect(objHndl secdoor);

	bool TaggedSceneryWasSeen(objHndl scenery);
	void MarkTaggedScenerySeenAndPlayVoice(objHndl scenery);

	SecretDoorSys();
};


extern SecretDoorSys secretdoorSys;


bool _isSecretDoor(objHndl obj);
int _getSecretDoorDC(objHndl obj);
uint32_t _secretDoorIsRevealed(objHndl secDoor);