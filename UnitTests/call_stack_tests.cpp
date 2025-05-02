#include "pch.h"
#include <MinCppXtra/call_stack.hpp>
#include <MinCppXtra/call_stack_access_scope.hpp>

#include <string>

#define NAMEOF(x) #x

namespace unit_tests
{
	class CallStackTestFixture
		: public ::testing::TestWithParam<int>
	{
	};

	static __declspec(noinline) std::string GetCallStackTrace(int depth, bool coloured)
	{
		return (depth <= 1)
			? mincpp::CallStack::GetTrace(coloured)
			: GetCallStackTrace(depth - 1, coloured);
	}

	TEST(CallStack, PrintCallStackTrace)
	{
		mincpp::CallStackAccessScope scope;
		std::cout << GetCallStackTrace(1, true);
	}

	static int CountMatches(const std::string_view& x, const std::string& text)
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

	TEST_P(CallStackTestFixture, GetTrace)
	{
		mincpp::CallStackAccessScope scope;
		int depth = GetParam();
		std::string cst = GetCallStackTrace(depth, false);
		const char* line = NAMEOF(unit_tests) "::" NAMEOF(GetCallStackTrace);
		int expectedMatchCount = depth;
		EXPECT_EQ(expectedMatchCount, CountMatches(line, cst));
		
	}

	INSTANTIATE_TEST_CASE_P(
		GetCallStackTraceWithVaryingDepth,
		CallStackTestFixture,
		::testing::Values(1, 2));
}