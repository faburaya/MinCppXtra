#include "internal/pch.h"
#include "call_stack_access_scope.hpp"
#include "win32_errors.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>
#include <DbgHelp.h>

namespace mincpp
{
    static void ReportLastError(const char* functionName)
    {
        Win32Errors::AppendErrorMessage(
            GetLastError(), functionName, std::cerr) << std::endl;
    }

    HANDLE GetThisProcessHandle()
    {
        static HANDLE handle = nullptr;

        static std::once_flag initialization;
        std::call_once(initialization, []()
        {
            HANDLE curProcessHandle = GetCurrentProcess();

            if (NOT_OK(DuplicateHandle(
                    curProcessHandle,
                    curProcessHandle,
                    curProcessHandle,
                    &handle,
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS)))
            {
                ReportLastError(NAMEOF(SymInitialize));
                handle = curProcessHandle;
            }
        });

        return handle;
    }

	class CallStackAccessScope::Impl
	{
    public:

        Impl()
        {
            SymSetOptions(
                SymGetOptions()
                    | SYMOPT_UNDNAME
                    | SYMOPT_DEFERRED_LOADS
                    | SYMOPT_LOAD_LINES);

            if (NOT_OK(SymInitialize(GetThisProcessHandle(), nullptr, TRUE)))
            {
                ReportLastError(NAMEOF(SymInitialize));
            }
        }

        ~Impl()
        {
            if (NOT_OK(SymCleanup(GetThisProcessHandle())))
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