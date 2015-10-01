#pragma once

#include <string>
#include <sstream>
#include <memory>

#include "exception.h"
#include "mdfmaterial.h"

class Tokenizer;

namespace gfx {

	class MdfParser {
	public:
		MdfParser(const std::string& filename, const std::string& content);

		std::unique_ptr<MdfMaterial> Parse();

		void SetStrict(bool strict);

	private:
		std::string mFilename;

		// The line last read using GetLine. Is trimmed of whitespace at start and end
		std::string mLine;
		int mLineNo = 0;
		std::string mContent;
		std::istringstream mIn;
		
		bool mStrict = false;

		bool GetLine();
		MdfType ParseMaterialType();
		std::unique_ptr<MdfClipperMaterial> ParseClipper();
		std::unique_ptr<MdfTexturedMaterial> ParseTextured();
		std::unique_ptr<MdfGeneralMaterial> ParseGeneral();

		bool ParseTextureStageId(Tokenizer &tokenizer) const;
		bool ParseFilename(Tokenizer &tokenizer, const char *logMsg) const;
		bool ParseIdentifier(Tokenizer &tokenizer, const char *logMsg) const;
		bool ParseRgba(Tokenizer &tokenizer, const char *logMsg, uint32_t &diffuseOut) const;
		bool ParseNumber(Tokenizer &tokenizer, const char *logMsg) const;

		// Creates a better error message with context
		template <typename... T>
		TempleException CreateError(const char* format, T ... args) const;
	};
}
