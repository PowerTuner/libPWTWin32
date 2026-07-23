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
#include "win32Reg.h"
#include <format>

#define LOG(msg) if (logcb) logcb(msg);

namespace PWTREG {
    std::optional<std::wstring> regReadSZ(HKEY hkey, LPCWSTR subKey, LPCWSTR value, const std::function<void(const std::wstring &)> &logcb) {
        std::unique_ptr<wchar_t[]> buf;
        DWORD bufSz;
        HKEY rkey;
        LSTATUS ret;

        ret = RegOpenKeyExW(hkey, subKey, 0, KEY_READ, &rkey);
        if (ret != ERROR_SUCCESS) {
            LOG(std::format(L"failed to open {}, value: {}, code: {}", subKey, value, ret))
            return {};
        }

        ret = RegGetValueW(rkey, nullptr, value, RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ, nullptr, nullptr, &bufSz);
        if (ret != ERROR_SUCCESS) {
            LOG(std::format(L"failed to read size for {}, value: {}, code: {}", subKey, value, ret))
            RegCloseKey(rkey);
            return {};
        }

        bufSz += sizeof(wchar_t);
        buf = std::make_unique<wchar_t[]>(bufSz);

        ret = RegGetValueW(rkey, nullptr, value, RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ, nullptr, buf.get(), &bufSz);
        if (ret != ERROR_SUCCESS) {
            LOG(std::format(L"failed to read {}, value: {}, code: {}", subKey, value, ret))
            RegCloseKey(rkey);
            return {};
        }

        RegCloseKey(rkey);
        return std::wstring(buf.get());
    }

    std::optional<DWORD> regReadDword(HKEY hkey, LPCWSTR subKey, LPCWSTR value, const std::function<void(const std::wstring &)> &logcb) {
        DWORD dwSize = sizeof(DWORD);
        DWORD regVal;
        HKEY rkey;
        LSTATUS ret;

        ret = RegOpenKeyExW(hkey, subKey, 0, KEY_READ, &rkey);
        if (ret != ERROR_SUCCESS) {
            LOG(std::format(L"failed to open {}, value: {}, code: {}", subKey, value, ret))
            return {};
        }

        ret = RegGetValueW(rkey, nullptr, value, RRF_RT_REG_DWORD, nullptr, &regVal, &dwSize);
        if (ret != ERROR_SUCCESS) {
            LOG(std::format(L"failed to read {}, value: {}, code: {}", subKey, value, ret))
            return {};
        }

        RegCloseKey(rkey);
        return regVal;
    }

    std::unique_ptr<BYTE[]> regReadBinary(HKEY hkey, LPCWSTR subKey, LPCWSTR value, LPDWORD size, const std::function<void(const std::wstring &)> &logcb) {
        std::unique_ptr<BYTE[]> buf;
        DWORD bufSz;
        HKEY rkey;
        LSTATUS ret;

        ret = RegOpenKeyExW(hkey, subKey, 0, KEY_READ, &rkey);
        if (ret != ERROR_SUCCESS) {
            LOG(std::format(L"failed to open {}, value: {}, code: {}", subKey, value, ret))
            return nullptr;
        }

        ret = RegGetValueW(rkey, nullptr, value, RRF_RT_REG_BINARY, nullptr, nullptr, &bufSz);
        if (ret != ERROR_SUCCESS) {
            LOG(std::format(L"failed to read size for {}, value: {}, code: {}", subKey, value, ret))
            RegCloseKey(rkey);
            return nullptr;
        }

        *size = bufSz;
        buf = std::make_unique<BYTE[]>(bufSz);

        ret = RegGetValueW(rkey, nullptr, value, RRF_RT_REG_BINARY, nullptr, buf.get(), &bufSz);
        if (ret != ERROR_SUCCESS) {
            LOG(std::format(L"failed to read {}, value: {}, code: {}", subKey, value, ret))
            RegCloseKey(rkey);
            return nullptr;
        }

        RegCloseKey(rkey);
        return buf;
    }

    bool regWriteSZ(HKEY hkey, LPCWSTR subKey, LPCWSTR value, const DWORD type, const std::wstring &sz, const std::function<void(const std::wstring &)> &logcb) {
        HKEY rkey;
        LSTATUS ret;

        ret = RegOpenKeyExW(hkey, subKey, 0, KEY_SET_VALUE, &rkey);
        if (ret != ERROR_SUCCESS) {
            LOG(std::format(L"failed to open {}, value: {}, code: {}", subKey, value, ret))
            return false;
        }

        ret = RegSetValueExW(rkey, value, 0, type, reinterpret_cast<const BYTE *>(sz.c_str()), (sz.size() + 1) * sizeof(wchar_t));
        if (ret != ERROR_SUCCESS) {
            LOG(std::format(L"failed to write {}, value: {}, code: {}", subKey, value, ret))
            RegCloseKey(rkey);
            return false;
        }

        RegCloseKey(rkey);
        return true;
    }

    bool regDeleteVal(HKEY hkey, LPCWSTR subKey, LPCWSTR value, const std::function<void(const std::wstring &)> &logcb) {
        HKEY rkey;
        LSTATUS ret;

        ret = RegOpenKeyExW(hkey, subKey, 0, KEY_SET_VALUE, &rkey);
        if (ret != ERROR_SUCCESS) {
            LOG(std::format(L"Failed to open {}, value: {}, code: {}", subKey, value, ret))
            return false;
        }

        ret = RegDeleteValueW(rkey, value);
        if (ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND) {
            LOG(std::format(L"Failed to unset {}, value: {}, code: {}", subKey, value, ret))
            RegCloseKey(rkey);
            return false;
        }

        RegCloseKey(rkey);
        return true;
    }

    std::optional<bool> regKeyHasValue(HKEY hkey, LPCWSTR subKey, LPCWSTR value, const std::function<void(const std::wstring &)> &logcb) {
        HKEY rkey;
        LSTATUS ret;

        ret = RegOpenKeyExW(hkey, subKey, 0, KEY_QUERY_VALUE, &rkey);
        if (ret != ERROR_SUCCESS) {
            LOG(std::format(L"Failed to open {}, value: {}, code: {}", subKey, value, ret))
            return {};
        }

        ret = RegQueryValueExW(rkey, value, nullptr, nullptr, nullptr, nullptr);
        if (ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND) {
            LOG(std::format(L"Failed to query {}, value: {}, code: {}", subKey, value, ret))
            return {};
        }

        RegCloseKey(rkey);
        return ret == ERROR_SUCCESS;
    }
}
