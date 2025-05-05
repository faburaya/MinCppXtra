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

#include "call_stack.hpp"
#include "console.hpp"
#include "traceable_exception.hpp"
#include "win32_api_strings.hpp"
#include "win32_errors.hpp"

#include <array>
#include <cinttypes>
#include <mutex>
#include <regex>
#include <sstream>
#include <utility>
#include <vector>

#include <comdef.h>
#include <DbgHelp.h>

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
                GetThisProcessHandle(),
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
        std::string function;
        std::string fileName;
        uint32_t lineNumber;
    };

    static ResolvedFrame Resolve(const STACKFRAME& frame)
    {
        char buffer[sizeof(SYMBOL_INFOW) + MAX_SYM_NAME * sizeof(wchar_t)]{};
        SYMBOL_INFOW* symbol = reinterpret_cast<SYMBOL_INFOW*>(buffer);
        symbol->SizeOfStruct = sizeof * symbol;
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 d64;
        ResolvedFrame result;
        if (NOT_OK(SymFromAddrW(GetThisProcessHandle(), frame.AddrPC.Offset, &d64, symbol)))
        {
            result.status = GetLastError();
            return result;
        }

        result.status = ERROR_SUCCESS;
        result.function = Win32ApiStrings::ToUtf8(symbol->Name, symbol->NameLen);

        DWORD d32;
        IMAGEHLP_LINEW64 line{};
        line.SizeOfStruct = sizeof line;
        if (OK(SymGetLineFromAddrW64(GetThisProcessHandle(), frame.AddrPC.Offset, &d32, &line)))
        {
            result.fileName = Win32ApiStrings::ToUtf8(line.FileName);
            result.lineNumber = line.LineNumber;
        }

        return result;
    }

    static std::vector<ResolvedFrame> FilterFrames(const std::vector<ResolvedFrame>& frames)
    {
        static const auto topIrrelevantSymbols =
            std::to_array<const char*>(
            {
                NAMEOF(mincpp::CallStack::GetTrace),
                NAMEOF(mincpp::TraceableException::),
                "_CxxFrameHandler",
                "RtlCaptureContext",
            });

        // drop irrelevant frames on top of the stack:
        auto iterBegin = frames.cbegin();
        for (int idx = 0, lastMatchIdx = -1, matchCount = 0; idx < frames.size(); ++idx)
        {
            auto& frame = frames[idx];

            if (std::any_of(
                    topIrrelevantSymbols.cbegin(),
                    topIrrelevantSymbols.cend(),
                    [&frame](const char* name) {
                        return frame.status == ERROR_INVALID_ADDRESS ||
                            frame.function.starts_with(name) ||
                            frame.function.find("::catch$") != std::string::npos;
                    }))
            {
                ++matchCount;
                lastMatchIdx = idx;
            }

            int itemCount = idx + 1;
            if (itemCount >= matchCount + 2)
            {
                iterBegin += ++lastMatchIdx;
                break;
            }
        }

        static const auto bottomIrrelevantSymbols =
            std::to_array<const char*>(
                {
                    "__scrt_common_main",
                    "mainCRTStartup",
                    "BaseThreadInitThunk",
                    "RtlUserThreadStart",
                });

        // drop irrelevant frames in the bottom of the stack:
        auto revIterBegin = frames.crbegin();
        while (revIterBegin != frames.crend() &&
            std::any_of(
                bottomIrrelevantSymbols.cbegin(),
                bottomIrrelevantSymbols.cend(),
                [&revIterBegin](const char* name) {
                    return revIterBegin->function.starts_with(name);
                }))
        {
            ++revIterBegin;
        }

        auto iterEnd = revIterBegin.base();
        return std::vector<ResolvedFrame>(iterBegin, iterEnd);
    }

    static std::string SerializeStackTrace(
        const std::vector<ResolvedFrame>& frames, bool isConsole)
    {
        std::regex mangledLambdaRegEx("lambda_\\w+");
        std::ostringstream oss;

        int idx = 0;
        uint32_t prevStatus = ERROR_SUCCESS;

        for (const auto& frame : frames)
        {
            if (frame.status != ERROR_SUCCESS
                && prevStatus == frame.status)
            {
                continue;
            }

            oss << '#' << std::dec << idx++ << ' ';

            switch (frame.status)
            {
            case ERROR_SUCCESS:
                oss << Console::Color(isConsole).Yellow()
                    << frame.function;

                if (!frame.fileName.empty())
                {
                    oss << std::endl
                        << Console::Color(isConsole).BrightBlack()
                        << "  in " << frame.fileName
                        << ", line " << frame.lineNumber;
                }
                break;

            case ERROR_MOD_NOT_FOUND:
                oss << "[.NET managed?] cannot resolve symbol for frame(s)";
                break;

            default:
                oss << "cannot resolve symbol for frame(s) - ";
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

    std::string CallStack::GetTrace(const void* currentContextHandle, bool isConsole)
    {
        std::vector<STACKFRAME> allStackFrames =
            BackTraceStackFrames(static_cast<const CONTEXT*>(currentContextHandle));

        std::vector<ResolvedFrame> resolvedFrames;
        resolvedFrames.reserve(allStackFrames.size());

        std::transform(
            allStackFrames.cbegin(),
            allStackFrames.cend(),
            std::back_inserter(resolvedFrames),
            [isConsole](const STACKFRAME& frame)
            {
                return Resolve(frame);
            });

        const auto filteredFrames = FilterFrames(resolvedFrames);
        return SerializeStackTrace(filteredFrames, isConsole);
    }

    std::string CallStack::GetTrace(bool isConsole)
    {
        CONTEXT currentContext;
        RtlCaptureContext(&currentContext);
        return GetTrace(&currentContext, isConsole);
    }
}