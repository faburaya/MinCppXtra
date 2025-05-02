#include "internal/pch.h"
#include "call_stack_access_scope.hpp"
#include "win32_errors.hpp"

#include <iostream>
#include <DbgHelp.h>

namespace mincpp
{
	class CallStackAccessScope::Impl
	{
    private:

        static void ReportLastError(const char* functionName)
        {
            Win32Errors::AppendErrorMessage(GetLastError(), functionName, std::cerr);
        }

    public:

        Impl()
        {
            SymSetOptions(SymGetOptions() | SYMOPT_LOAD_LINES);

            if (NOT_OK(SymInitialize(GetCurrentProcess(), nullptr, TRUE)))
            {
                ReportLastError(NAMEOF(SymInitialize));
            }
        }

        ~Impl()
        {
            if (NOT_OK(SymCleanup(GetCurrentProcess())))
            {
                ReportLastError(NAMEOF(SymCleanup));
            }
        }
	};

    CallStackAccessScope::CallStackAccessScope()
        : m_pimpl(std::make_unique<CallStackAccessScope::Impl>())
    {
    }


    CallStackAccessScope::~CallStackAccessScope() = default;
}