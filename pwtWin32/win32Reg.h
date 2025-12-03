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

#include <QString>

#include "libExport.h"

namespace PWTW32 {
    [[nodiscard]] PWTWIN32_EXPORT bool regReadSZ(HKEY hkey, LPCWSTR subKey, LPCWSTR value, QString &result, QString &error, bool log = true);
    [[nodiscard]] PWTWIN32_EXPORT bool regReadDword(HKEY hkey, LPCWSTR subKey, LPCWSTR value, LPDWORD result, QString &error, bool log = true);
    [[nodiscard]] PWTWIN32_EXPORT bool regWriteSZ(HKEY hkey, LPCWSTR subKey, LPCWSTR value, DWORD type, const QString &str, QString &error, bool log = true);
    [[nodiscard]] PWTWIN32_EXPORT std::unique_ptr<BYTE[]> regReadBinary(HKEY hkey, LPCWSTR subKey, LPCWSTR value, LPDWORD size, QString &error, bool log = true);
    [[nodiscard]] PWTWIN32_EXPORT bool regDeleteVal(HKEY hkey, LPCWSTR subKey, LPCWSTR value, QString &error, bool log = true);
    [[nodiscard]] PWTWIN32_EXPORT bool regKeyHasValue(HKEY hkey, LPCWSTR subKey, LPCWSTR value, bool &result, QString &error, bool log = true);
}
