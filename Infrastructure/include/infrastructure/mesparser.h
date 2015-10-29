
#pragma once

#include <string>
#include <map>

class MesFile {
public:

	// This has to be ordered
	using Content = std::map<int, std::string>;

	static Content ParseFile(const std::string &filename);

	static Content ParseString(const std::string &content, const std::string& filename = "<string>");

};
