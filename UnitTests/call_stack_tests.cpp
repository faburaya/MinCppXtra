#include "pch.h"
#include <MinCppXtra/call_stack.hpp>
#include <MinCppXtra/call_stack_access_scope.hpp>
#include "utils.hpp"

#include <string>

namespace unit_tests
{
	static __declspec(noinline) std::string GetCallStackTrace(int depth, bool coloured)
	{
		return (depth <= 1)
			? mincpp::CallStack::GetTrace(coloured)
			: GetCallStackTrace(depth - 1, coloured);
	}

	class CallStackTestFixture
		: public ::testing::TestWithParam<int>
	{
	};

	TEST_P(CallStackTestFixture, GetTrace)
	{
		mincpp::CallStackAccessScope scope;
		int depth = GetParam();
		std::string cst = GetCallStackTrace(depth, false);
		const char* line = NAMEOF(unit_tests::GetCallStackTrace);
		int expectedMatchCount = depth;
		EXPECT_EQ(expectedMatchCount, CountMatches(line, cst));
	}

	INSTANTIATE_TEST_CASE_P(
		GetCallStackTraceWithVaryingDepth,
		CallStackTestFixture,
		::testing::Values(1, 2));
}