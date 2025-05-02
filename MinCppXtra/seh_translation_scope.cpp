#include "internal/pch.h"
#include "seh_translation_scope.hpp"
#include "win32_exception.hpp"

namespace mincpp
{
    static void TranslateWin32Exception(unsigned int code, EXCEPTION_POINTERS* exceptionPointers)
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
}