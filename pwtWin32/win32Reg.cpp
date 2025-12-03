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

namespace PWTW32 {
    bool regReadSZ(HKEY hkey, LPCWSTR subKey, LPCWSTR value, QString &result, QString &error, const bool log) {
        std::unique_ptr<TCHAR[]> buf;
        DWORD bufSz;
        HKEY rkey;
        LSTATUS ret;

        ret = RegOpenKeyEx(hkey, subKey, 0, KEY_READ, &rkey);
        if (ret != ERROR_SUCCESS) {
            if (log)
                error = QString("failed to open %1, value: %2, code: %3").arg(subKey, value).arg(ret);

            return false;
        }

        ret = RegGetValue(rkey, nullptr, value, RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ, nullptr, nullptr, &bufSz);
        if (ret != ERROR_SUCCESS) {
            if (log)
                error = QString("failed to read size for %1, value: %2, code %3").arg(subKey, value).arg(ret);

            RegCloseKey(rkey);
            return false;
        }

        bufSz += sizeof(TCHAR);
        buf = std::make_unique<TCHAR[]>(bufSz);

        ret = RegGetValue(rkey, nullptr, value, RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ, nullptr, buf.get(), &bufSz);
        if (ret != ERROR_SUCCESS) {
            if (log)
                error = QString("failed to read %1, value: %2, code %3").arg(subKey, value).arg(ret);

            RegCloseKey(rkey);
            return false;
        }

        RegCloseKey(rkey);
        result.assign(buf.get());
        return true;
    }

    bool regReadDword(HKEY hkey, LPCWSTR subKey, LPCWSTR value, LPDWORD result, QString &error, const bool log) {
        DWORD dwSize = sizeof(DWORD);
        HKEY rkey;
        LSTATUS ret;

        ret = RegOpenKeyEx(hkey, subKey, 0, KEY_READ, &rkey);
        if (ret != ERROR_SUCCESS) {
            if (log)
                error = QString("failed to open %1, value: %2, code %3").arg(subKey, value).arg(ret);

            return false;
        }

        ret = RegGetValue(rkey, nullptr, value, RRF_RT_REG_DWORD, nullptr, result, &dwSize);
        if (ret != ERROR_SUCCESS) {
            if (log)
                error = QString("failed to read %1, value: %2, code %3").arg(subKey, value).arg(ret);

            return false;
        }

        RegCloseKey(rkey);
        return true;
    }

    bool regWriteSZ(HKEY hkey, LPCWSTR subKey, LPCWSTR value, const DWORD type, const QString &str, QString &error, const bool log) {
        HKEY rkey;
        LSTATUS ret;

        ret = RegOpenKeyEx(hkey, subKey, 0, KEY_SET_VALUE, &rkey);
        if (ret != ERROR_SUCCESS) {
            if (log)
                error = QString("failed to open %1, value: %2, code %3").arg(subKey, value).arg(ret);

            return false;
        }

        ret = RegSetValueEx(rkey, value, 0, type, reinterpret_cast<const BYTE *>(str.toStdWString().c_str()), (str.size() + 1) * sizeof(TCHAR));
        if (ret != ERROR_SUCCESS) {
            if (log)
                error = QString("failed to write %1, value: %2, code %3").arg(subKey, value).arg(ret);

            RegCloseKey(rkey);
            return false;
        }

        RegCloseKey(rkey);
        return true;
    }

    std::unique_ptr<BYTE[]> regReadBinary(HKEY hkey, LPCWSTR subKey, LPCWSTR value, LPDWORD size, QString &error, const bool log) {
        std::unique_ptr<BYTE[]> buf;
        DWORD bufSz;
        HKEY rkey;
        LSTATUS ret;

        ret = RegOpenKeyEx(hkey, subKey, 0, KEY_READ, &rkey);
        if (ret != ERROR_SUCCESS) {
            if (log)
                error = QString("failed to open %1, value: %2, code %3").arg(subKey, value).arg(ret);

            return nullptr;
        }

        ret = RegGetValue(rkey, nullptr, value, RRF_RT_REG_BINARY, nullptr, nullptr, &bufSz);
        if (ret != ERROR_SUCCESS) {
            if (log)
                error = QString("failed to read size for %1, value: %2, code %3").arg(subKey, value).arg(ret);

            RegCloseKey(rkey);
            return nullptr;
        }

        *size = bufSz;
        buf = std::make_unique<BYTE[]>(bufSz);

        ret = RegGetValue(rkey, nullptr, value, RRF_RT_REG_BINARY, nullptr, buf.get(), &bufSz);
        if (ret != ERROR_SUCCESS) {
            if (log)
                error = QString("failed to read %1, value: %2, code %3").arg(subKey, value).arg(ret);

            RegCloseKey(rkey);
            return nullptr;
        }

        RegCloseKey(rkey);
        return buf;
    }

    bool regDeleteVal(HKEY hkey, LPCWSTR subKey, LPCWSTR value, QString &error, const bool log) {
        HKEY rkey;
        LSTATUS ret;

        ret = RegOpenKeyEx(hkey, subKey, 0, KEY_SET_VALUE, &rkey);
        if (ret != ERROR_SUCCESS) {
            if (log)
                error = QString("Failed to open %1, value: %2, code %3").arg(subKey, value).arg(ret);

            return false;
        }

        ret = RegDeleteValue(rkey, value);
        if (ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND) {
            if (log)
                error = QString("Failed to unset %1, value: %2, code %3").arg(subKey, value).arg(ret);

            RegCloseKey(rkey);
            return false;
        }

        RegCloseKey(rkey);
        return true;
    }

    bool regKeyHasValue(HKEY hkey, LPCWSTR subKey, LPCWSTR value, bool &result, QString &error, const bool log) {
        HKEY rkey;
        LSTATUS ret;

        ret = RegOpenKeyEx(hkey, subKey, 0, KEY_QUERY_VALUE, &rkey);
        if (ret != ERROR_SUCCESS) {
            if (log)
                error = QString("Failed to open %1, value: %2, code %3").arg(subKey, value).arg(ret);

            return false;
        }

        ret = RegQueryValueEx(rkey, value, nullptr, nullptr, nullptr, nullptr);
        result = ret == ERROR_SUCCESS;

        RegCloseKey(rkey);
        return true;
    }
}
