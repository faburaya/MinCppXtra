#include "pch.h"
#include "utils.hpp"

#include <MinCppXtra/call_stack_access_scope.hpp>
#include <MinCppXtra/seh_translation_scope.hpp>
#include <MinCppXtra/win32_exception.hpp>

namespace unit_tests
{
	static __declspec(noinline) int DividePerZero(int number)
	{
		return number / 0;
	}

#ifdef NDEBUG
	// release build crashes when walking stack upon SEH translation
#   define HAS_STACK_TRACE false
#else
#   define HAS_STACK_TRACE true
#endif

	TEST(Win32Exception, SehExceptionTranslation)
	{
		try
		{
			mincpp::CallStackAccessScope callStackAccessScope;
			mincpp::SehTranslationScope sehTranslationScope;
			int x = DividePerZero(1);
			std::cout << "division per zero returns " << x << std::endl;
		}
		catch (mincpp::Win32Exception& ex)
		{
			const char* line = NAMEOF(unit_tests::DividePerZero);
			std::string cst = ex.GetCallStackTrace();
			EXPECT_EQ(HAS_STACK_TRACE ? 1 : 0, CountMatches(line, cst)) << cst;
			return;
		}
		FAIL() << "Win32 exception has not been translated!";
	}

	TEST(Win32Exception, PrintTranslatedWin32Exception)
	{
		try
		{
			mincpp::CallStackAccessScope callStackAccessScope;
			mincpp::SehTranslationScope sehTranslationScope;
			int x = DividePerZero(1);
			std::cout << "division per zero returns " << x << std::endl;
		}
		catch (mincpp::Win32Exception& ex)
		{
			std::cout << ex.Serialize();
		}
	}
}