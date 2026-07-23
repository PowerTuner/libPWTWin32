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
#include <functional>
#include <string>
#include <vector>
#include <optional>

#include "export.h"

namespace PWTSVC {
    [[nodiscard]] LIB_EXPORT bool stopService(LPCWSTR name, const std::function<void(const std::wstring &)> &logcb);
    [[nodiscard]] LIB_EXPORT bool startService(LPCWSTR name, const std::function<void(const std::wstring &)> &logcb);
    [[nodiscard]] LIB_EXPORT bool deleteService(LPCWSTR name, const std::function<void(const std::wstring &)> &logcb);
    [[nodiscard]] LIB_EXPORT bool installService(LPCWSTR name, const std::wstring &exePath, const std::function<void(const std::wstring &)> &logcb);
    [[nodiscard]] LIB_EXPORT bool closeRunningApps(const std::vector<std::wstring> &list, const std::function<void(const std::wstring &)> &logcb);
    [[nodiscard]] LIB_EXPORT std::optional<bool> isServiceRunning(LPCWSTR name, const std::function<void(const std::wstring &)> &logcb);
}
