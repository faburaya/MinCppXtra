/*
 * MinCppXtra - A minimalistic C++ utility library
 *
 * Author: Felipe Vieira Aburaya, 2025
 * License: The Unlicense (public domain)
 * Repository: https://github.com/faburaya/MinCppXtra
 *
 * This software is released into the public domain.
 * You can freely use, modify, and distribute it without restrictions.
 *
 * For more details, see: https://unlicense.org
 */

#include "internal/pch.h"
#include "traceable_exception.hpp"

#include "call_stack.hpp"
#include "console.hpp"

#include <sstream>

namespace mincpp
{
	class TraceableException::Impl
	{
	private:

		std::string m_callStackTrace;
		std::optional<std::exception> m_innerException;

	public:

		Impl(std::optional<std::exception>&& innerException)
			: m_innerException(innerException)
			, m_callStackTrace(
				CallStack::GetTrace(TraceableException::s_useColorsOnStackTrace))
		{
		}

		Impl(const void* exceptionContextHandle, bool isStackTraceEnabled)
			: m_callStackTrace(isStackTraceEnabled
				? CallStack::GetTrace(
					exceptionContextHandle, TraceableException::s_useColorsOnStackTrace)
				: "(disabled in this build)")
		{
		}

		const std::string& GetCallStackTrace() const
		{
			return m_callStackTrace;
		}

		const std::optional<std::exception>& GetInnerException() const
		{
			return m_innerException;
		}
	};

	bool TraceableException::s_useColorsOnStackTrace = false;

	void TraceableException::UseColorsOnStackTrace(bool enable)
	{
		s_useColorsOnStackTrace = enable;
	}

	TraceableException::TraceableException(
		const std::string& message,
		std::optional<std::exception>&& innerException)
		: std::runtime_error(message)
		, m_pimpl(new Impl(std::move(innerException)))
	{
	}

	TraceableException::TraceableException(
		const std::string& message,
		const void* exceptionContextHandle,
		bool isStackTraceEnabled)
		: std::runtime_error(message)
		, m_pimpl(new Impl(exceptionContextHandle, isStackTraceEnabled))
	{
	}

	TraceableException::~TraceableException() = default;

	const std::string& TraceableException::GetCallStackTrace() const
	{
		return m_pimpl->GetCallStackTrace();
	}

	const std::optional<std::exception>& TraceableException::GetInnerException() const
	{
		return m_pimpl->GetInnerException();
	}

	std::string_view TraceableException::GetTypeName() const
	{
		constexpr std::string_view prefix = "class ";
		static const std::string_view rttiTypeName = typeid(*this).name();
		static const std::string_view typeName =
			rttiTypeName.substr(rttiTypeName.starts_with(prefix) ? prefix.length() : 0);

		return typeName;
	}

	std::string TraceableException::Serialize() const
	{
		std::ostringstream oss;
		oss << Console::Color(s_useColorsOnStackTrace).BrightRed()
			<< GetTypeName() << ": " << what()
			<< Console::Color(s_useColorsOnStackTrace).Reset()
			<< std::endl;

		if (m_pimpl->GetInnerException())
		{
			oss << Console::Color(s_useColorsOnStackTrace).Red() 
				<< "  ∟ " << m_pimpl->GetInnerException().value().what()
				<< Console::Color(s_useColorsOnStackTrace).Reset()
				<< std::endl;
		}

		oss << "=== CALL STACK TRACE ===" << std::endl;
		oss << m_pimpl->GetCallStackTrace() << std::endl;
		return oss.str();
	}
}