
#include <infrastructure/logging.h>
#include <infrastructure/exception.h>
#include <infrastructure/vfs.h>

#include "shaders_compiler.h"

#include <D3Dcompiler.h>
#include <D3DCompiler.inl>

#include <EASTL/fixed_vector.h>

namespace gfx {
	
	class VfsShaderIncludeHandler : public ID3DInclude {
	public:
		HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType,
			LPCSTR pFileName, 
			LPCVOID pParentData, 
			LPCVOID *ppData, 
			UINT *pBytes) override;
		HRESULT __stdcall Close(LPCVOID pData) override;
	};
	
	VertexShaderPtr gfx::ShaderCompiler::CompileVertexShader(RenderingDevice &device)
	{
		auto code(CompileShaderCode("vs_4_0"));
		return std::make_shared<VertexShader>(device, mName, code);
	}

	PixelShaderPtr ShaderCompiler::CompilePixelShader(RenderingDevice &device)
	{
		auto code(CompileShaderCode("ps_4_0"));
		return std::make_shared<PixelShader>(device, mName, code);
	}

	std::vector<uint8_t> ShaderCompiler::CompileShaderCode(const std::string &profile)
	{
		VfsShaderIncludeHandler includeHandler;

		// Convert the defines
		eastl::fixed_vector<D3D_SHADER_MACRO, 16> macros;
		for (auto &define : mDefines) {
			macros.push_back({
				define.first.c_str(), 
				define.second.c_str()
			});
		}

		// Add a D3D11 define if we compile for newer targets
		if (profile == "vs_4_0" || profile == "ps_4_0") {
			macros.push_back({
				"D3D11", "1"
			});
		}

		macros.push_back({ nullptr, nullptr }); // Null terminated array

		// Debug flags
		DWORD flags = D3DCOMPILE_SKIP_OPTIMIZATION;
#ifndef NDEBUG
		flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_PREFER_FLOW_CONTROL;
#endif

		CComPtr<ID3DBlob> codeBuffer;
		CComPtr<ID3DBlob> errorBuffer;		
		auto result = D3DCompile(
			&mSourceCode[0], 
			mSourceCode.length(),
			("C:\\TemplePlus\\TemplePlus\\tpdata\\shaders\\" + mName + ".hlsl").c_str(),
			&macros[0],
			&includeHandler,
			"main",
			profile.c_str(),
			flags,
			0,
			&codeBuffer,
			&errorBuffer);		

		// Copy the errors and warnings into a readable string
		std::string errors;
		if (errorBuffer) {
			errors = std::string(reinterpret_cast<char*>(errorBuffer->GetBufferPointer()),
				errorBuffer->GetBufferSize());
		}

		if (!SUCCEEDED(result)) {
			// Unable to compile the actual shader
			throw TempleException("Unable to compile shader {}: {}", mName, errors);
		}

		if (!errors.empty()) {
			logger->warn("Errors/Warnings compiling shader {}: {}", mName,
				errors);
		}

		auto sourceData(reinterpret_cast<uint8_t*>(codeBuffer->GetBufferPointer()));
		std::vector<uint8_t> shaderData(sourceData, sourceData + codeBuffer->GetBufferSize());
		return shaderData;
	}

	HRESULT VfsShaderIncludeHandler::Open(D3D_INCLUDE_TYPE IncludeType,
		LPCSTR pFileName, 
		LPCVOID pParentData, 
		LPCVOID * ppData, 
		UINT * pBytes)
	{
		auto filename(fmt::format("shaders/{}", pFileName));
		try {
			auto content(vfs->ReadAsBinary(filename));
			
			auto data(malloc(content.size()));
			memcpy(data, content.data(), content.size());
			*ppData = data;
			*pBytes = content.size();
		} catch (std::exception &e) {
			logger->error("Unable to include file {}: {}", filename, e.what());
			return E_FAIL;
		}

		return S_OK;
	}

	HRESULT VfsShaderIncludeHandler::Close(LPCVOID pData)
	{
		free(const_cast<void*>(pData));
		return S_OK;
	}

}
