
#include <D3DX9Shader.h>
#include <infrastructure/logging.h>
#include <infrastructure/exception.h>
#include <infrastructure/vfs.h>

#include "shaders_compiler.h"

namespace gfx {
	
	class VfsShaderIncludeHandler : public ID3DXInclude {
	public:
		HRESULT __stdcall Open(D3DXINCLUDE_TYPE IncludeType, 
			LPCSTR pFileName, 
			LPCVOID pParentData, 
			LPCVOID *ppData, 
			UINT *pBytes) override;
		HRESULT __stdcall Close(LPCVOID pData) override;
	};

	VertexShaderPtr gfx::ShaderCompiler::CompileVertexShader(RenderingDevice &device)
	{
		auto code(CompileShaderCode("vs_3_0"));

		return std::make_shared<VertexShader>(device, mName, code);
	}

	PixelShaderPtr ShaderCompiler::CompilePixelShader(RenderingDevice &device)
	{
		auto code(CompileShaderCode("ps_3_0"));

		return std::make_shared<PixelShader>(device, mName, code);
	}

	std::vector<uint8_t> ShaderCompiler::CompileShaderCode(const std::string &profile)
	{
		VfsShaderIncludeHandler includeHandler;

		// Convert the defines
		std::vector<D3DXMACRO> macros;
		for (auto &define : mDefines) {
			macros.push_back({
				define.first.c_str(), 
				define.second.c_str()
			});
		}
		macros.push_back({ nullptr, nullptr }); // Null terminated array

		// Debug flags
		DWORD flags = D3DXSHADER_SKIPOPTIMIZATION;
#ifndef NDEBUG
		flags |= D3DXSHADER_DEBUG | D3DXSHADER_SKIPOPTIMIZATION;
#endif

		CComPtr<ID3DXBuffer> codeBuffer;
		CComPtr<ID3DXBuffer> errorBuffer;
		CComPtr<ID3DXConstantTable> constantTable;
		auto result = D3DXCompileShader(
			&mSourceCode[0], 
			mSourceCode.length(),
			&macros[0],
			&includeHandler,
			"main",
			profile.c_str(),
			flags,
			&codeBuffer,
			&errorBuffer,
			&constantTable);
		

		// Copy the errors and warnings into a readable string
		std::string errors;
		if (errorBuffer) {
			errors = std::string(reinterpret_cast<char*>(errorBuffer->GetBufferPointer()),
				errorBuffer->GetBufferSize());
		}

		if (result != D3D_OK) {
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

	HRESULT VfsShaderIncludeHandler::Open(D3DXINCLUDE_TYPE IncludeType, 
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
