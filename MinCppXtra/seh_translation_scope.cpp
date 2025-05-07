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
#include "seh_translation_scope.hpp"
#include "win32_exception.hpp"

namespace mincpp
{
    static void __cdecl TranslateWin32Exception(unsigned int code, EXCEPTION_POINTERS* exceptionPointers)
    {
        throw Win32Exception(exceptionPointers);
    }

    class SehTranslationScope::Impl
    {
    private:

        _se_translator_function m_prevTranslatorFunc;

    public:

        Impl()
        {
            m_prevTranslatorFunc =
                _set_se_translator(TranslateWin32Exception);
        }

        ~Impl()
        {
            _set_se_translator(m_prevTranslatorFunc);
        }
    };

    SehTranslationScope::SehTranslationScope()
        : m_pimpl(new SehTranslationScope::Impl())
    {
    }

    SehTranslationScope::~SehTranslationScope() = default;
}