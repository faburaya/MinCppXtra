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
#include "win32_errors.hpp"
#include "win32_api_strings.hpp"

#include <sstream>
#include <comdef.h>

namespace mincpp
{
    std::ostream& Win32Errors::AppendErrorMessage(
        uint32_t errCode,
        const char* funcName,
        std::ostream& oss)
    {
        if (funcName != nullptr && funcName[0] != 0)
            oss << funcName << " returned error ";
        else
            oss << "Win32 API error code ";

        oss << errCode;

        HRESULT hr = HRESULT_FROM_WIN32(errCode);
        _com_error err(hr);
        return oss << ": " << Win32ApiStrings::ToUtf8(err.ErrorMessage());
    }

    std::string Win32Errors::GetErrorMessage(uint32_t errCode, const char* funcName)
    {
        std::ostringstream oss;
        AppendErrorMessage(errCode, funcName, oss);
        return oss.str();
    }
}