#pragma once

#include <memory>

namespace mincpp
{
	/// <summary>
	/// Creates a scope for access of the call stack.
	/// </summary>
	class CallStackAccessScope
	{
	private:

		class Impl;
		std::unique_ptr<Impl> m_pimpl;

	public:

		/// <summary>
		/// Creates the scope.
		/// </summary>
		CallStackAccessScope();

		~CallStackAccessScope();
	};
}
