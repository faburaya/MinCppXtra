#include "pch.h"
#include "utils.hpp"

namespace unit_tests
{
	int CountMatches(const std::string_view& x, const std::string& text)
	{
		int count = 0;
		size_t matchPos, offset = 0;
		while ((matchPos = text.find(x, offset)) != std::string::npos)
		{
			++count;
			offset = matchPos + 1;
		}
		return count;
	}
}