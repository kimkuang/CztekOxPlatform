#ifndef __StdStringUtils_h__
#define __StdStringUtils_h__
#include <string>
#include <vector>

class StdStringUtils
{
public:
	static std::string& ltrim(std::string &s);
	static std::string& rtrim(std::string &s);
	static std::string& trim(std::string &s);
	static std::vector<std::string> split(const std::string &str, const std::string &spliter, bool skipEmptyParts);
};

#endif /* __StdStringUtils_h__ */
