#include "pch.h"
#include <MinCppXtra/call_stack_access_scope.hpp>
#include <MinCppXtra/traceable_exception.hpp>
#include "utils.hpp"

#include <string>

namespace unit_tests
{
	class MyTestException : public mincpp::TraceableException
	{
	public:

		MyTestException(
			const std::string& message,
			const std::exception& innerException)
			: mincpp::TraceableException(message, innerException)
		{
		}
	};

	static const char* EXPECTED_EX_MESSAGE = "I am an exception.";
	static const char* EXPECTED_INNER_EX_MESSAGE = "And this is the inner exception.";

	static __declspec(noinline) void ThrowTraceableException()
	{
		try
		{
			throw std::runtime_error(EXPECTED_INNER_EX_MESSAGE);
		}
		catch (std::exception& ex)
		{
			throw MyTestException(EXPECTED_EX_MESSAGE, ex);
		}
	}

	TEST(TraceableException, CatchException)
	{
		mincpp::TraceableException::UseColorsOnStackTrace(true);

		try
		{
			mincpp::CallStackAccessScope scope;
			ThrowTraceableException();
		}
		catch (mincpp::TraceableException& ex)
		{
			std::string cst = ex.GetCallStackTrace();
			const char* line = NAMEOF(unit_tests::ThrowTraceableException);
			EXPECT_EQ(1, CountMatches(line, cst)) << ex.Serialize();

			std::string serializedEx = ex.Serialize();
			EXPECT_EQ(1, CountMatches(EXPECTED_EX_MESSAGE, ex.what()));

			std::optional<std::exception> innerEx = ex.GetInnerException();
			EXPECT_TRUE(innerEx.has_value());
			EXPECT_STREQ(EXPECTED_INNER_EX_MESSAGE, innerEx.value().what());
			EXPECT_EQ(1, CountMatches(innerEx.value().what(), serializedEx));
		}
	}

	TEST(TraceableException, PrintException)
	{
		mincpp::TraceableException::UseColorsOnStackTrace(true);

		try
		{
			mincpp::CallStackAccessScope scope;
			ThrowTraceableException();
		}
		catch (mincpp::TraceableException& ex)
		{
			std::cout << ex.Serialize();
		}
	}
}