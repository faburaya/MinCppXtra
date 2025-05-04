#include "internal/pch.h"
#include "console.hpp"

namespace mincpp
{
	/// <summary>
	/// Performs initialization tasks for the application.
	/// </summary>
	struct StaticInitializer
	{
		StaticInitializer()
		{
			// allow UTF-8 in standard output
			SetConsoleOutputCP(CP_UTF8);
		}
	};

	static StaticInitializer initializer;
}