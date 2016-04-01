#include "stdafx.h"
#include "util/fixes.h"
#include "common.h"
#include "tig/tig_tabparser.h"
#include "gamesystems/legacymapsystems.h"
#include "gamesystems/gamesystems.h"

class ProtosHooks : public TempleFix{
public: 
	const char* name() override { 
		return "Protos Hooks";
	} 
	
	

	void apply() override 
	{
		
		// replaces the proto parser for supporting extensions
		static int (*orgProtoParser)(TigTabParser*, int, const char**) = 
			replaceFunction<int(__cdecl)(TigTabParser*, int, const char**)>(0x1003B640, [](TigTabParser* parser, int lineIdx, const char** cols)->int{

			static std::vector<int> parsedProtoIds;

			auto & parsedIds = parsedProtoIds;
			auto protoNum = atol(cols[0]);
			auto foundProto = std::find(parsedIds.begin(), parsedIds.end(), protoNum);
			if (foundProto == parsedIds.end()){
				parsedIds.push_back(protoNum);
				return orgProtoParser(parser, lineIdx, cols);
			} else
			{
				logger->info("Skipping duplicate proto {}", protoNum);
			}

			return 0;
			
		});
	}
} protosHooks;
