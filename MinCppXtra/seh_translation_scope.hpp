#pragma once

#include <memory>

namespace mincpp
{
	/// <summary>
	/// Creates a scope where any Win32 exceptions are translated
	/// from SEH to C++ (mincpp::Win32Exception).
	/// </summary>
	class SehTranslationScope
	{
	private:

		class Impl;
		std::unique_ptr<Impl> m_pimpl;

	public:

		/// <summary>
		/// Creates the scope.
		/// </summary>
		SehTranslationScope();
	};
}