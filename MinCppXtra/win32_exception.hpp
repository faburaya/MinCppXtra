#pragma once

#include <stdexcept>

namespace mincpp
{
	/// <summary>
	/// Represents a Win32 exception that has been translated from SEH to C++.
	/// </summary>
	class Win32Exception : public std::runtime_error
	{
	public:

		/// <summary>
		/// Creates an instance of Win32Exception.
		/// </summary>
		/// <param name="exceptionPointers">
		/// This is provided by the caught Win32 exception.
		/// (The erased type is PEXCEPTION_POINTERS).
		/// </param>
		Win32Exception(void* exceptionPointers);
	};
}