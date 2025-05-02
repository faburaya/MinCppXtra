#pragma once

#include <cinttypes>
#include <ostream>

namespace mincpp
{
	class Win32Errors
	{
    public:

        static void AppendErrorMessage(
            uint32_t errCode,
            const char* funcName,
            std::ostream& oss);

        static std::string GetErrorMessage(uint32_t errCode, const char* funcName);
	};
}
