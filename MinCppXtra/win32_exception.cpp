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
#include "win32_exception.hpp"

#include <map>
#include <mutex>
#include <sstream>
#include <string>

#define ADD_MESSAGE(map, code) map[code] = #code

namespace mincpp
{
    static const char* GetExceptionCodeDetails(uint32_t exCode)
    {
        static std::once_flag messagesInitialization;
        static std::map<uint32_t, const char*> messageByCode;

        std::call_once(messagesInitialization, []()
        {
            ADD_MESSAGE(messageByCode, EXCEPTION_ACCESS_VIOLATION);
            ADD_MESSAGE(messageByCode, EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
            ADD_MESSAGE(messageByCode, EXCEPTION_BREAKPOINT);
            ADD_MESSAGE(messageByCode, EXCEPTION_DATATYPE_MISALIGNMENT);
            ADD_MESSAGE(messageByCode, EXCEPTION_FLT_DENORMAL_OPERAND);
            ADD_MESSAGE(messageByCode, EXCEPTION_FLT_DIVIDE_BY_ZERO);
            ADD_MESSAGE(messageByCode, EXCEPTION_FLT_INEXACT_RESULT);
            ADD_MESSAGE(messageByCode, EXCEPTION_FLT_INVALID_OPERATION);
            ADD_MESSAGE(messageByCode, EXCEPTION_FLT_OVERFLOW);
            ADD_MESSAGE(messageByCode, EXCEPTION_FLT_STACK_CHECK);
            ADD_MESSAGE(messageByCode, EXCEPTION_FLT_UNDERFLOW);
            ADD_MESSAGE(messageByCode, EXCEPTION_ILLEGAL_INSTRUCTION);
            ADD_MESSAGE(messageByCode, EXCEPTION_IN_PAGE_ERROR);
            ADD_MESSAGE(messageByCode, EXCEPTION_INT_DIVIDE_BY_ZERO);
            ADD_MESSAGE(messageByCode, EXCEPTION_INT_OVERFLOW);
            ADD_MESSAGE(messageByCode, EXCEPTION_INVALID_DISPOSITION);
            ADD_MESSAGE(messageByCode, EXCEPTION_NONCONTINUABLE_EXCEPTION);
            ADD_MESSAGE(messageByCode, EXCEPTION_PRIV_INSTRUCTION);
            ADD_MESSAGE(messageByCode, EXCEPTION_SINGLE_STEP);
            ADD_MESSAGE(messageByCode, EXCEPTION_STACK_OVERFLOW);
        });

        auto iter = messageByCode.find(exCode);
        if (iter != messageByCode.end())
        {
            return iter->second;
        }
        return "Unknown Win32 exception";
    }

    static void AppendExceptionMessage(
        const EXCEPTION_RECORD* exRecord,
        std::ostringstream& oss)
    {
        const ULONG_PTR* params = exRecord->ExceptionInformation;
        const DWORD exCode = exRecord->ExceptionCode;

        oss << GetExceptionCodeDetails(exCode);

        switch (exCode)
        {
        case EXCEPTION_ACCESS_VIOLATION:
        case EXCEPTION_IN_PAGE_ERROR:
            _ASSERTE(exRecord->NumberParameters >= 2);
            oss << " - ";

            switch (params[0])
            {
            case 0:
                oss << "Read access violation";
                break;

            case 1:
                oss << "Write access violation";
                break;

            case 8:
                oss << "User-mode DEP violation";
                break;

            default:
                oss << "Unknown operation type";
                break;
            }

            oss << " on address 0x" << std::hex << params[1];
            if (exRecord->NumberParameters >= 3)
            {
                oss << ", NTSTATUS code " << params[2];
            }
            break;

        default:
            break;
        }

        oss << " (code 0x" << std::hex << exCode << ')';
    }

    static std::string CreateExceptionMessage(EXCEPTION_RECORD* exRecord)
    {
        std::ostringstream oss;
        AppendExceptionMessage(exRecord, oss);
        while ((exRecord = exRecord->ExceptionRecord) != nullptr)
        {
            oss << std::endl << "  ∟ ";
            AppendExceptionMessage(exRecord, oss);
        }
        oss << std::endl
            << "See https://learn.microsoft.com/en-us/windows/win32/debug/getexceptioncode";

        return oss.str();
    }

#ifdef NDEBUG
    // release build crashes when walking stack upon SEH translation
#   define ENABLE_STACK_TRACE false
#else
#   define ENABLE_STACK_TRACE true
#endif

    Win32Exception::Win32Exception(const void* exceptionPointers)
        : TraceableException(
            CreateExceptionMessage(
                static_cast<const EXCEPTION_POINTERS*>(exceptionPointers)->ExceptionRecord),
            static_cast<const EXCEPTION_POINTERS*>(exceptionPointers)->ContextRecord,
            ENABLE_STACK_TRACE)
    {
    }
}