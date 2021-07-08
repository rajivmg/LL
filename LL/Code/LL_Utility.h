#pragma once

#include "pch.h"

#include <memory>
#include <string>

#include "LL_Common.h"

namespace utility
{
    typedef std::shared_ptr<std::vector<std::wstring> > filelist;

    filelist GetFileList(std::wstring const &Directory, std::wstring const &Pattern = L"*.*");

    std::string ToString(std::wstring WS);

    template <typename t>
    inline t Sign(t Number)
    {
        return (Number > 0) - (Number < 0);
    }

    template <>
    inline f32 Sign(f32 Number)
    {
        return std::signbit(Number) ? -1.0f : 1.0f;
    }

    template <>
    inline f64 Sign(f64 Number)
    {
        return std::signbit(Number) ? -1.0 : 1.0;
    }
}