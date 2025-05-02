#pragma once

#include <string>

namespace mincpp
{
	/// <summary>
	/// Provides call stack information.
	/// </summary>
	class CallStack
	{
	public:

		/// <summary>
		/// Gets the current call stack trace.
		/// </summary>
		/// <param name="isConsole"> Whether the text should be visual appealing for the console.</param>
		/// <returns>The current call stack trace, UTF-8 encoded.</returns>
		static std::string GetTrace(bool isConsole = false);
	};
}
