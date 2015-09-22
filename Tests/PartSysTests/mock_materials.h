
#pragma once

#include <gmock/gmock.h>
#include <materials.h>

class MaterialsMock : public gfx::MaterialManager {
public:
	MOCK_METHOD1(Resolve, gfx::MaterialRef(const std::string& materialName));

};

class MaterialMock : public gfx::Material {
public:
	MOCK_METHOD0(IsValid, bool());
	MOCK_CONST_METHOD0(GetName, std::string());
	MOCK_METHOD0(GetPrimaryTexture, gfx::TextureRef());
};
