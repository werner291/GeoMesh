

#ifndef HTTP_STRING_UTILS_H
#define HTTP_STRING_UTILS_H

#include <string>
#include <string.h>

struct CaseInsensitiveStringIsEqual
{
    bool operator()(const std::string &lhs, const std::string &rhs) const
    {
        if (lhs.length() != rhs.length())
            return false;
        else
            return strncasecmp(lhs.data(), rhs.data(), lhs.length()) < 0;
    }
};

struct CaseInsensitiveStringHasher
{
    size_t operator()(const std::string &toHash) const
    {
    	size_t hash = 0;

    	for (char c : toHash)
    	{
    		c = tolower(c);

    		hash = hash << 1 ^ c;
    	}

    	return hash;
    }
};

#endif
