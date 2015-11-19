
#pragma once

#include "graphics/shaders.h"

namespace gfx {

class ShaderCompiler {
public:

	void SetSource(const std::string &sourceCode) {
		mSourceCode = sourceCode;
	}

	void SetDefines(const Shaders::ShaderDefines &defines) {
		mDefines = defines;
	}

	void SetName(const std::string &name) {
		mName = name;
	}

	VertexShaderPtr CompileVertexShader(RenderingDevice &device);

	PixelShaderPtr CompilePixelShader(RenderingDevice &device);

private:
	Shaders::ShaderDefines mDefines;
	std::string mSourceCode;
	std::string mName;

	std::vector<uint8_t> CompileShaderCode(const std::string &profile);
};

}
