#pragma once

#include <string>

#define NAMEOF(x) #x

namespace unit_tests
{
	int CountMatches(const std::string_view& x, const std::string& text);
}
