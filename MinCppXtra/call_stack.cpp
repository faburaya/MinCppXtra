#include "internal/pch.h"
#include "call_stack.hpp"
#include "console.hpp"
#include "win32_api_strings.hpp"
#include "win32_errors.hpp"

#include <array>
#include <cinttypes>
#include <mutex>
#include <regex>
#include <sstream>
#include <vector>

#include <comdef.h>
#include <DbgHelp.h>
#include <utf8.h>

namespace mincpp
{
    static void InitializeStackFrame64(STACKFRAME& frame, const CONTEXT* context)
    {
        frame = {};
        frame.AddrPC.Offset = context->Rip;
        frame.AddrPC.Mode = AddrModeFlat;
        frame.AddrFrame.Offset = context->Rsp;
        frame.AddrFrame.Mode = AddrModeFlat;
        frame.AddrStack.Offset = context->Rsp;
        frame.AddrStack.Mode = AddrModeFlat;
    }

    static bool GetNextStackFrame64(STACKFRAME& frame, const CONTEXT* context)
    {
        BOOL success =
            StackWalk64(
                IMAGE_FILE_MACHINE_AMD64,
                GetCurrentProcess(),
                GetCurrentThread(),
                &frame,
                (void*)context,
                nullptr,
                &SymFunctionTableAccess64,
                &SymGetModuleBase64,
                nullptr);

        return success == TRUE;
    }

    static std::vector<STACKFRAME> BackTraceStackFrames(const CONTEXT* context)
    {
        std::vector<STACKFRAME> stackFrames;
        static std::mutex symbolAccessMutex;
        {
            // lock access to stack walking because the API is not thread-safe
            std::lock_guard<std::mutex> lock(symbolAccessMutex);

            STACKFRAME frame;
            InitializeStackFrame64(frame, context);
            while (GetNextStackFrame64(frame, context))
            {
                stackFrames.push_back(frame);
            }
        }
        return stackFrames;
    }

    struct ResolvedFrame
    {
        uint32_t status;
        std::string description;
    };

    static ResolvedFrame Resolve(const STACKFRAME& frame, bool isConsole)
    {
        std::ostringstream oss;

        char buffer[sizeof(SYMBOL_INFOW) + MAX_SYM_NAME * sizeof(wchar_t)]{};
        SYMBOL_INFOW* symbol = reinterpret_cast<SYMBOL_INFOW*>(buffer);
        symbol->SizeOfStruct = sizeof * symbol;
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 d64;
        if (NOT_OK(SymFromAddrW(GetCurrentProcess(), frame.AddrPC.Offset, &d64, symbol)))
        {
            return { GetLastError(), std::string() };
        }

        oss << Console::Color(isConsole).Yellow()
            << Win32ApiStrings::ToUtf8(symbol->Name, symbol->NameLen)
            << Console::Color(isConsole).Reset();

        DWORD d32;
        IMAGEHLP_LINEW64 line{};
        line.SizeOfStruct = sizeof line;
        if (OK(SymGetLineFromAddrW64(GetCurrentProcess(), frame.AddrPC.Offset, &d32, &line)))
        {
            oss << std::endl
                << Console::Color(isConsole).BrightBlack()
                << "  in "
                << Win32ApiStrings::ToUtf8(line.FileName)
                << ", line " << line.LineNumber
                << Console::Color(isConsole).Reset();
        }

        return { ERROR_SUCCESS, oss.str() };
    }

    static std::string CreateStackTrace(
        const std::vector<ResolvedFrame>::const_iterator& framesBegin,
        const std::vector<ResolvedFrame>::const_iterator& framesEnd,
        bool isConsole)
    {
        std::regex mangledLambdaRegEx("lambda_\\w+");
        std::ostringstream oss;

        int idx = 0;
        uint32_t prevStatus = ERROR_SUCCESS;

        for (auto iter = framesBegin; iter != framesEnd; ++iter)
        {
            const ResolvedFrame& frame = *iter;

            if (frame.status == ERROR_MOD_NOT_FOUND
                && prevStatus == frame.status)
            {
                continue;
            }

            oss << '#' << std::dec << idx++ << ' ';

            switch (frame.status)
            {
            case ERROR_SUCCESS:
                oss << frame.description;
                break;

            case ERROR_MOD_NOT_FOUND:
                oss << "[managed code?] cannot resolve symbol for frame(s)";
                break;

            default:
                oss << "cannot resolve symbol - ";
                Win32Errors::AppendErrorMessage(frame.status, nullptr, oss);
                break;
            }

            oss << Console::Color(isConsole).Reset() << std::endl;
            if (!isConsole)
            {
                oss << "---" << std::endl;
            }

            prevStatus = frame.status;
        }

        return std::regex_replace(oss.str(), mangledLambdaRegEx, "lambda");
    }

    std::string CallStack::GetTrace(bool isConsole)
    {
        CONTEXT currentContext;
        RtlCaptureContext(&currentContext);
        std::vector<STACKFRAME> allStackFrames = BackTraceStackFrames(&currentContext);

        std::vector<ResolvedFrame> resolvedFrames;
        resolvedFrames.reserve(allStackFrames.size());

        std::transform(
            allStackFrames.cbegin(),
            allStackFrames.cend(),
            std::back_inserter(resolvedFrames),
            [isConsole](const STACKFRAME& frame)
            {
                return Resolve(frame, isConsole);
            });

        static std::array<const char*, 5> irrelevantSymbols =
        {
            "invoke_main",
            "__scrt_common_main",
            "mainCRTStartup",
            "BaseThreadInitThunk",
            "RtlUserThreadStart",
        };

        while (!resolvedFrames.empty() &&
            std::any_of(
                irrelevantSymbols.cbegin(),
                irrelevantSymbols.cend(),
                [&resolvedFrames](const std::string & name) {
                    return resolvedFrames.back()
                        .description.find(name) != std::string::npos;
                }))
        {
            resolvedFrames.pop_back();
        }

        const char* thisFrameName = NAMEOF(mincpp::CallStack::GetTrace);
        int skipCount =
            (resolvedFrames.front()
                .description.find(thisFrameName) != std::string::npos) ? 1 : 0;

        return CreateStackTrace(
            resolvedFrames.cbegin() + skipCount,
            resolvedFrames.cend(),
            isConsole);
    }
}