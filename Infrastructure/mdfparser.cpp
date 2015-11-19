#include <regex>

#include "infrastructure/mdfparser.h"
#include "infrastructure/stringutil.h"
#include "infrastructure/tokenizer.h"

namespace gfx {

	MdfParser::MdfParser(const std::string& filename, const std::string& content)
		: mFilename(filename), mContent(content), mIn(mContent) {
	}

	std::unique_ptr<MdfMaterial> MdfParser::Parse() {
		auto type = ParseMaterialType();

		switch (type) {
		case MdfType::Textured:
			return ParseTextured();
		case MdfType::General:
			return ParseGeneral();
		case MdfType::Clipper:
			return ParseClipper();
		}

		return std::unique_ptr<MdfMaterial>();
	}

	void MdfParser::SetStrict(bool strict) {
		mStrict = strict;
	}

	bool MdfParser::GetLine() {
		if (std::getline(mIn, mLine)) {
			// Skip trailing windows newline characters in case the /
			// MDF file has been read in binary mode
			if (!mLine.empty() && mLine.back() == '\r') {
				mLine.resize(mLine.size() - 1);
			}
			mLineNo++;
			return true;
		}
		return false;
	}

	MdfType MdfParser::ParseMaterialType() {
		if (!GetLine()) {
			throw CreateError("File is empty");
		}

		auto input(tolower(mLine));

		if (input == "textured") {
			return MdfType::Textured;
		} else if (input == "general") {
			return MdfType::General;
		} else if (input == "clipper") {
			return MdfType::Clipper;
		} else {
			throw CreateError("Unrecognized material type '{}'", input);
		}
	}

	std::unique_ptr<MdfMaterial> MdfParser::ParseClipper() {
		auto result(std::make_unique<MdfMaterial>(MdfType::Clipper));

		result->enableZWrite = false;
		result->enableColorWrite = false;

		Tokenizer tokenizer(mContent);
		tokenizer.NextToken(); // Skip material type

		while (tokenizer.NextToken()) {
			if (tokenizer.IsIdentifier("wire")) {
				result->wireframe = true;
				result->enableColorWrite = true;
			} else if (tokenizer.IsIdentifier("zfill")) {
				result->enableZWrite = true;
			} else if (tokenizer.IsIdentifier("outline")) {
				result->outline = true;
				result->enableColorWrite = true;
			} else {
				if (mStrict) {
					throw CreateError("Unrecognized token '{}'", tokenizer.GetTokenText());
				}
			}
		}

		return result;
	}

	std::unique_ptr<MdfMaterial> MdfParser::ParseTextured() {

		auto result(std::make_unique<MdfMaterial>(MdfType::Textured));

		Tokenizer tokenizer(mContent);
		/*
			For some reason ToEE doesn't use the tokenizer for this 
			shader type normally. So we disable escape sequences to
			get some form of compatibility.
		*/
		tokenizer.SetEnableEscapes(false);
		tokenizer.NextToken(); // Skip material type

		while (tokenizer.NextToken()) {
			if (tokenizer.IsIdentifier("color")) {
				if (!ParseRgba(tokenizer, "Color", result->diffuse)) {
					throw CreateError("Unable to parse diffuse color");
				}
			} else if (tokenizer.IsIdentifier("texture")) {
				if (!tokenizer.NextToken() || !tokenizer.IsQuotedString()) {
					throw CreateError("Missing filename for texture");
				}

				result->samplers[0].filename = tokenizer.GetTokenText();
			} else if (tokenizer.IsIdentifier("colorfillonly")) {
				result->enableZWrite = false;
				result->enableColorWrite = true;
			} else if (tokenizer.IsIdentifier("notlit")) {
				result->notLit = true;
			} else if (tokenizer.IsIdentifier("notlite")) {
				// The original ToEE parser only does prefix parsing, which is why 
				// "notlite" was accepted as "notlit".
				result->notLit = true;
			} else if (tokenizer.IsIdentifier("disablez")) {
				result->disableZ = true;
			} else if (tokenizer.IsIdentifier("double")) {
				result->faceCulling = false;
			} else if (tokenizer.IsIdentifier("clamp")) {
				result->clamp = true;
			} else {
				if (mStrict) {
					throw CreateError("Unrecognized token '{}'", tokenizer.GetTokenText());
				}
			}
		}

		return result;

	}

	std::unique_ptr<MdfMaterial> MdfParser::ParseGeneral() {
		auto result(std::make_unique<MdfMaterial>(MdfType::General));

		Tokenizer tokenizer(mContent);
		tokenizer.NextToken(); // Skip material type

		while (tokenizer.NextToken()) {

			if (!tokenizer.IsIdentifier()) {
				if (mStrict) {
					throw CreateError("Unexpected token: {}", tokenizer.GetTokenText());
				}
				continue;
			}

			if (tokenizer.IsIdentifier("highquality")) {
				// In no case is the GPU the bottleneck anymore,
				// so we will always parse the high quality section
				// Previously, it did cancel here if the GPU supported
				// less than 4 textures
				continue;
			}

			if (tokenizer.IsIdentifier("texture")) {
				if (!ParseTextureStageId(tokenizer)) {
					continue;
				}

				auto samplerNo = tokenizer.GetTokenInt();
				if (ParseFilename(tokenizer, "Texture")) {
					result->samplers[samplerNo].filename = tokenizer.GetTokenText();
				}
				continue;
			}

			if (tokenizer.IsIdentifier("glossmap")) {
				if (ParseFilename(tokenizer, "GlossMap")) {
					result->glossmap = tokenizer.GetTokenText();
				}
				continue;
			}

			if (tokenizer.IsIdentifier("uvtype")) {
				if (!ParseTextureStageId(tokenizer)) {
					continue;
				}

				auto samplerNo = tokenizer.GetTokenInt();

				if (!ParseIdentifier(tokenizer, "UvType")) {
					continue;
				}

				MdfUvType uvType;
				if (tokenizer.IsIdentifier("mesh")) {
					uvType = MdfUvType::Mesh;
				} else if (tokenizer.IsIdentifier("environment")) {
					uvType = MdfUvType::Environment;
				} else if (tokenizer.IsIdentifier("drift")) {
					uvType = MdfUvType::Drift;
				} else if (tokenizer.IsIdentifier("swirl")) {
					uvType = MdfUvType::Swirl;
				} else if (tokenizer.IsIdentifier("wavey")) {
					uvType = MdfUvType::Wavey;
				} else {
					if (mStrict) {
						throw CreateError("Unrecognized UvType: {}", tokenizer.GetTokenText());
					}
					continue;
				}
				result->samplers[samplerNo].uvType = uvType;
				continue;
			}

			if (tokenizer.IsIdentifier("blendtype")) {
				if (!ParseTextureStageId(tokenizer)) {
					continue;
				}

				auto samplerNo = tokenizer.GetTokenInt();

				if (!ParseIdentifier(tokenizer, "BlendType")) {
					continue;
				}

				MdfTextureBlendType blendType;
				if (tokenizer.IsIdentifier("modulate")) {
					blendType = MdfTextureBlendType::Modulate;
				} else if (tokenizer.IsIdentifier("add")) {
					blendType = MdfTextureBlendType::Add;
				} else if (tokenizer.IsIdentifier("texturealpha")) {
					blendType = MdfTextureBlendType::TextureAlpha;
				} else if (tokenizer.IsIdentifier("currentalpha")) {
					blendType = MdfTextureBlendType::CurrentAlpha;
				} else if (tokenizer.IsIdentifier("currentalphaadd")) {
					blendType = MdfTextureBlendType::CurrentAlphaAdd;
				} else {
					if (mStrict) {
						throw CreateError("Unrecognized BlendType: {}", tokenizer.GetTokenText());
					}
					continue;
				}
				result->samplers[samplerNo].blendType = blendType;
				continue;
			}

			if (tokenizer.IsIdentifier("color")) {
				uint32_t argbColor;
				if (ParseRgba(tokenizer, "Color", argbColor)) {
					result->diffuse = argbColor;
				}
				continue;
			}

			if (tokenizer.IsIdentifier("specular")) {
				uint32_t argbColor;
				if (ParseRgba(tokenizer, "Specular", argbColor)) {
					result->specular = argbColor;
				}
				continue;
			}

			if (tokenizer.IsIdentifier("specularpower")) {
				if (!tokenizer.NextToken()) {
					if (mStrict) {
						throw CreateError("Unexpected end of file after SpecularPower");
					}
				} else if (!tokenizer.IsNumber()) {
					if (mStrict) {
						throw CreateError("Expected number after SpecularPower, but got: {}",
						                  tokenizer.GetTokenText());
					}
				} else {
					result->specularPower = tokenizer.GetTokenFloat();
				}
				continue;
			}

			if (tokenizer.IsIdentifier("materialblendtype")) {
				if (!ParseIdentifier(tokenizer, "MaterialBlendType")) {
					continue;
				}

				if (tokenizer.IsIdentifier("none")) {
					result->blendType = MdfBlendType::None;
				} else if (tokenizer.IsIdentifier("alpha")) {
					result->blendType = MdfBlendType::Alpha;
				} else if (tokenizer.IsIdentifier("add")) {
					result->blendType = MdfBlendType::Add;
				} else if (tokenizer.IsIdentifier("alphaadd")) {
					result->blendType = MdfBlendType::AlphaAdd;
				} else {
					if (mStrict) {
						throw CreateError("Unrecognized MaterialBlendType: {}",
						                  tokenizer.GetTokenText());
					}
				}
				continue;
			}

			if (tokenizer.IsIdentifier("speed")) {
				if (!ParseNumber(tokenizer, "Speed")) {
					continue;
				}

				auto speed = tokenizer.GetTokenFloat() * 60.0f;

				// Set the speed for all texture stages and for both U and V
				for (auto& sampler : result->samplers) {
					sampler.speedU = speed;
					sampler.speedV = speed;
				}
				continue;
			}

			if (tokenizer.IsIdentifier("speedu")) {
				if (!ParseTextureStageId(tokenizer)) {
					continue;
				}

				auto samplerNo = tokenizer.GetTokenInt();

				if (!ParseNumber(tokenizer, "SpeedU")) {
					continue;
				}

				auto speed = tokenizer.GetTokenFloat() * 60.0f;
				result->samplers[samplerNo].speedU = speed;
				continue;
			}

			if (tokenizer.IsIdentifier("speedv")) {
				if (!ParseTextureStageId(tokenizer)) {
					continue;
				}

				auto samplerNo = tokenizer.GetTokenInt();

				if (!ParseNumber(tokenizer, "SpeedV")) {
					continue;
				}

				auto speed = tokenizer.GetTokenFloat() * 60.0f;
				result->samplers[samplerNo].speedV = speed;
				continue;
			}

			if (tokenizer.IsIdentifier("double")) {
				result->faceCulling = false;
				continue;
			}

			if (tokenizer.IsIdentifier("linearfiltering")) {
				result->linearFiltering = true;
				continue;
			}

			if (tokenizer.IsIdentifier("recalculatenormals")) {
				result->recalculateNormals = true;
				continue;
			}

			if (tokenizer.IsIdentifier("zfillonly")) {
				result->enableColorWrite = false;
				result->enableZWrite = true;
				continue;
			}

			if (tokenizer.IsIdentifier("colorfillonly")) {
				result->enableColorWrite = true;
				result->enableZWrite = false;
				continue;
			}

			if (tokenizer.IsIdentifier("notlit")) {
				result->notLit = true;
				continue;
			}

			if (tokenizer.IsIdentifier("disablez")) {
				result->disableZ = true;
				continue;
			}

			if (mStrict) {
				throw CreateError("Unrecognized token: {}", tokenizer.GetTokenText());
			}

		}

		return result;
	}

	bool MdfParser::ParseTextureStageId(Tokenizer& tokenizer) const {
		if (!tokenizer.NextToken()) {
			if (mStrict) {
				throw CreateError("Missing argument for texture");
			}
			return false;
		}
		if (!tokenizer.IsNumber() || tokenizer.GetTokenInt() < 0 || tokenizer.GetTokenInt() >= 4) {
			if (mStrict) {
				throw CreateError("Expected a texture stage between 0 and 3 as the second argument: {}",
				                  tokenizer.GetTokenText());
			}
			return false;
		}
		return true;
	}

	bool MdfParser::ParseFilename(Tokenizer& tokenizer, const char* logMsg) const {
		if (!tokenizer.NextToken()) {
			if (mStrict) {
				throw CreateError("Filename for {} is missing.", logMsg);
			}
			return false;
		} else if (!tokenizer.IsQuotedString()) {
			if (mStrict) {
				throw CreateError("Unexpected token instead of filename found for {}: {}",
				                  logMsg, tokenizer.GetTokenText());
			}
			return false;
		} else {
			return true;
		}
	}

	bool MdfParser::ParseIdentifier(Tokenizer& tokenizer, const char* logMsg) const {

		if (!tokenizer.NextToken()) {
			if (mStrict) {
				throw CreateError("Identifier after {} expected.", logMsg);
			}
			return false;
		}

		if (!tokenizer.IsIdentifier()) {
			if (mStrict) {
				throw CreateError("Identifier after {} expected, but got: {}",
				                  logMsg, tokenizer.GetTokenText());
			}
			return false;
		}

		return true;

	}

	bool MdfParser::ParseRgba(Tokenizer& tokenizer, const char* logMsg, uint32_t& argbOut) const {
		// Color in the input is RGBA
		argbOut = 0; // The output is ARGB

		if (!tokenizer.NextToken() || !tokenizer.IsNumber()) {
			if (!mStrict) {
				return false;
			}
			throw CreateError("Missing red component for {}", logMsg);
		}
		argbOut |= (tokenizer.GetTokenInt() & 0xFF) << 16;

		if (!tokenizer.NextToken() || !tokenizer.IsNumber()) {
			if (!mStrict) {
				return false;
			}
			throw CreateError("Missing green component for {}", logMsg);
		}
		argbOut |= (tokenizer.GetTokenInt() & 0xFF) << 8;

		if (!tokenizer.NextToken() || !tokenizer.IsNumber()) {
			if (!mStrict) {
				return false;
			}
			throw CreateError("Missing blue component for {}", logMsg);
		}
		argbOut |= (tokenizer.GetTokenInt() & 0xFF);

		if (!tokenizer.NextToken() || !tokenizer.IsNumber()) {
			if (!mStrict) {
				return false;
			}
			throw CreateError("Missing alpha component for {}", logMsg);
		}
		argbOut |= (tokenizer.GetTokenInt() & 0xFF) << 24;

		return true;

	}

	bool MdfParser::ParseNumber(Tokenizer& tokenizer, const char* logMsg) const {
		if (!tokenizer.NextToken()) {
			if (mStrict) {
				throw CreateError("Unexpected end of file after {}", logMsg);
			}
			return false;
		} else if (!tokenizer.IsNumber()) {
			if (mStrict) {
				throw CreateError("Expected number after {}, but got: {}",
				                  logMsg, tokenizer.GetTokenText());
			}
			return false;
		}
		return true;
	}

	template <typename ... T>
	TempleException MdfParser::CreateError(const char* format, T ... args) const {
		auto msg(fmt::format(format, args...));

		return TempleException("Unable to parse MDF file {}:{}: {}",
		                       mFilename, mLineNo, msg);
	}

}
