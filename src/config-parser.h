#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

// Module for Parsing Config files
// In simple, text-proto format:

#include <vector>
#include <string>

class ConfigParser {
	class Object {
		std::vector<std::pair<std::string, std::string>> properties;
		
	};
public:
	ConfigParser();

};

#endif