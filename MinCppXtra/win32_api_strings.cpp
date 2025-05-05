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
#include "win32_api_strings.hpp"

#include <vector>
#include <utf8/cpp20.h>

namespace mincpp
{
    Win32ApiStrings::StaticInitializer s_initializer;

    Win32ApiStrings::StaticInitializer::StaticInitializer()
    {
        // allow UTF-8 in standard output
        SetConsoleOutputCP(CP_UTF8);
    }

    std::string Win32ApiStrings::ToUtf8(const wchar_t* utf16str, size_t wideCharCount)
    {
        static_assert(sizeof(wchar_t) == sizeof(char16_t));
        const auto begin = reinterpret_cast<const char16_t*>(utf16str);
        const auto end = begin + wideCharCount;
        return utf8::utf16to8(std::u16string_view(begin, end));
    }

    std::string Win32ApiStrings::ToUtf8(const wchar_t* utf16str)
    {
        return ToUtf8(utf16str, wcslen(utf16str));
    }

    std::wstring Win32ApiStrings::ToUtf16(const std::u8string_view utf8str)
    {
        std::u16string utf16str = utf8::utf8to16(utf8str);
        static_assert(sizeof(wchar_t) == sizeof(char16_t));
        const auto begin = reinterpret_cast<const wchar_t*>(utf16str.data());
        const auto end = begin + utf16str.length();
        return std::wstring(begin, end);
    }

    std::wstring Win32ApiStrings::ToUtf16(const char* utf8str, size_t charCount)
    {
        const auto begin = reinterpret_cast<const char8_t*>(utf8str);
        const auto end = begin + charCount;
        return ToUtf16(std::u8string_view(begin, end));
    }

    std::wstring Win32ApiStrings::ToUtf16(const char* utf8str)
    {
        return ToUtf16(utf8str, strlen(utf8str));
    }
}
