#include "pch.h"
#include "utils.hpp"

#include <Windows.h>
#include <iostream>

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

	/// <summary>
	/// Performs initialization tasks for this program.
	/// </summary>
	struct StaticInitializer {
		StaticInitializer() {
			SetConsoleOutputCP(CP_UTF8);
			std::cout << "Static initializer called!" << std::endl;
		}
	};

	static StaticInitializer initializer;
}