/*
 * This file is part of PWTWin32.
 * Copyright (C) 2025 kylon
 *
 * PWTWin32 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PWTWin32 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "win.h"
#include <winreg.h>
#include <string>
#include <memory>
#include <functional>
#include <optional>

#include "export.h"

namespace PWTREG {
    [[nodiscard]] LIB_EXPORT std::optional<std::wstring> regReadSZ(HKEY hkey, LPCWSTR subKey, LPCWSTR value, const std::function<void(const std::wstring &)> &logcb);
    [[nodiscard]] LIB_EXPORT std::optional<DWORD> regReadDword(HKEY hkey, LPCWSTR subKey, LPCWSTR value, const std::function<void(const std::wstring &)> &logcb);
    [[nodiscard]] LIB_EXPORT std::unique_ptr<BYTE[]> regReadBinary(HKEY hkey, LPCWSTR subKey, LPCWSTR value, LPDWORD size, const std::function<void(const std::wstring &)> &logcb);
    [[nodiscard]] LIB_EXPORT bool regWriteSZ(HKEY hkey, LPCWSTR subKey, LPCWSTR value, DWORD type, const std::wstring &sz, const std::function<void(const std::wstring &)> &logcb);
    [[nodiscard]] LIB_EXPORT bool regDeleteVal(HKEY hkey, LPCWSTR subKey, LPCWSTR value, const std::function<void(const std::wstring &)> &logcb);
    [[nodiscard]] LIB_EXPORT std::optional<bool> regKeyHasValue(HKEY hkey, LPCWSTR subKey, LPCWSTR value, const std::function<void(const std::wstring &)> &logcb);
}
