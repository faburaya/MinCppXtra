#include "pch.h"
#include <MinCppXtra/win32_api_strings.hpp>

#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <string>

namespace unit_tests
{
	TEST(Win32ApiStrings, ToUtf8_ASCII_only)
	{
		wchar_t given[] = L"whatever in English";
		const char* expected = "whatever in English";
		EXPECT_EQ(expected, mincpp::Win32ApiStrings::ToUtf8(given));
		EXPECT_EQ(expected, mincpp::Win32ApiStrings::ToUtf8(given, 19));
	}

	TEST(Win32ApiStrings, ToUtf8_not_ASCII)
	{
		const char16_t given[] = u"excluído ausgeschloßen";
		const auto expected = reinterpret_cast<const char*>(u8"excluído ausgeschloßen");
		EXPECT_EQ(expected, mincpp::Win32ApiStrings::ToUtf8(reinterpret_cast<const wchar_t*>(given)));
		EXPECT_EQ(expected, mincpp::Win32ApiStrings::ToUtf8(reinterpret_cast<const wchar_t*>(given), 22));
	}

	TEST(Win32ApiStrings, ToUtf16_ASCII_only)
	{
		const char given[] = "whatever in English";
		const wchar_t* expected = L"whatever in English";

		EXPECT_EQ(expected,
			mincpp::Win32ApiStrings::ToUtf16(
				std::u8string_view(
					reinterpret_cast<const char8_t*>(given))));

		EXPECT_EQ(expected, mincpp::Win32ApiStrings::ToUtf16(given));
		EXPECT_EQ(expected, mincpp::Win32ApiStrings::ToUtf16(given, 19));
	}

	TEST(Win32ApiStrings, ToUtf16_not_ASCII)
	{
		const char8_t given[] = u8"excluído ausgeschloßen";
		const auto expected = reinterpret_cast<const wchar_t*>(u"excluído ausgeschloßen");

		EXPECT_EQ(expected, mincpp::Win32ApiStrings::ToUtf16(std::u8string_view(given)));
		EXPECT_EQ(expected, mincpp::Win32ApiStrings::ToUtf16(reinterpret_cast<const char*>(given)));
		EXPECT_EQ(expected, mincpp::Win32ApiStrings::ToUtf16(reinterpret_cast<const char*>(given), 24));
	}

	TEST(Win32ApiStrings, UnicodeInStringStream)
	{
		int prevOutMode = _setmode(_fileno(stdout), _O_U16TEXT);

		std::wcout
			<< mincpp::Win32ApiStrings::ToUtf16(
				u8"Das ist ein gültiger Text auf Deutsch. Das heißt: er muss schön gezeigt werden.")
			<< std::endl;

		_setmode(_fileno(stdout), prevOutMode);
	}
}
