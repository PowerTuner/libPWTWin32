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
#include "win32Svc.h"
#include <tlhelp32.h>
#include <format>
#include <thread>

#include "elapsedTimer.h"

#define LOG(msg) if (logcb) logcb(msg);

namespace PWTSVC {
    bool stopService(LPCWSTR name, const std::function<void(const std::wstring &)> &logcb) {
        using namespace std::chrono_literals;

        SERVICE_STATUS_PROCESS svcProcStat {};
        SERVICE_STATUS svcStat {};
        PWTW32::ElapsedTimer stopTimer {};
        SC_HANDLE scManager;
        SC_HANDLE service;
        DWORD bytesNeed = 0;
        HANDLE svcProc;

        scManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
        if (scManager == nullptr) {
            LOG(std::format(L"failed to open sc manager: {}", GetLastError()));
            return false;
        }

        service = OpenServiceW(scManager, name, SERVICE_STOP | SERVICE_QUERY_STATUS);
        if (service == nullptr) {
            const DWORD lastErr = GetLastError();

            if (lastErr != ERROR_SERVICE_DOES_NOT_EXIST)
                LOG(std::format(L"failed to open {} service: {}", name, lastErr))

            CloseServiceHandle(scManager);
            return lastErr == ERROR_SERVICE_DOES_NOT_EXIST;
        }

        if (!QueryServiceStatus(service, &svcStat)) {
            LOG(std::format(L"failed to query {} service: {}", name, GetLastError()))
            goto err_abort;
        }

        if (svcStat.dwCurrentState == SERVICE_STOPPED) {
            CloseServiceHandle(service);
            CloseServiceHandle(scManager);
            return true;
        }

        if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&svcProcStat), sizeof(SERVICE_STATUS_PROCESS), &bytesNeed)) {
            LOG(std::format(L"failed to query {} service process: {}", name, GetLastError()))
            goto err_abort;
        }

        svcProc = OpenProcess(SYNCHRONIZE, FALSE, svcProcStat.dwProcessId);

        if (!ControlService(service, SERVICE_CONTROL_STOP, &svcStat)) {
            LOG(std::format(L"failed to send stop command to {} service: {}", name, GetLastError()))
            CloseHandle(svcProc);
            goto err_abort;
        }

        stopTimer.start();

        while (svcStat.dwCurrentState != SERVICE_STOPPED) {
            if (stopTimer.hasExpired(4000ms)) {
                LOG(std::format(L"failed to stop {} service: timed out", name))
                CloseHandle(svcProc);
                goto err_abort;
            }

            std::this_thread::sleep_for(15ms);

            if (!QueryServiceStatus(service, &svcStat)) {
                LOG(std::format(L"failed to query {} service status: {}", name, GetLastError()))
                CloseHandle(svcProc);
                goto err_abort;
            }
        }

        WaitForSingleObject(svcProc, 4000);
        CloseHandle(svcProc);
        CloseServiceHandle(service);
        CloseServiceHandle(scManager);
        return true;

    err_abort:
        CloseServiceHandle(service);
        CloseServiceHandle(scManager);
        return false;
    }

    bool startService(LPCWSTR name, const std::function<void(const std::wstring &)> &logcb) {
        using namespace std::chrono_literals;

        SERVICE_STATUS svcStat {};
        PWTW32::ElapsedTimer startTimer {};
        SC_HANDLE scManager;
        SC_HANDLE service;

        scManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
        if (scManager == nullptr) {
            LOG(std::format(L"failed to open sc manager: {}", GetLastError()))
            return false;
        }

        service = OpenServiceW(scManager, name, SERVICE_START | SERVICE_QUERY_STATUS);
        if (service == nullptr) {
            LOG(std::format(L"failed to open {} service: {}", name, GetLastError()))
            CloseServiceHandle(scManager);
            return false;
        }

        if (!QueryServiceStatus(service, &svcStat)) {
            LOG(std::format(L"failed to query {} service: {}", name, GetLastError()))
            goto err_abort;
        }

        if (svcStat.dwCurrentState == SERVICE_RUNNING) {
            CloseServiceHandle(service);
            CloseServiceHandle(scManager);
            return true;
        }

        if (!StartServiceW(service, 0, nullptr)) {
            LOG(std::format(L"failed to start {} service: {}", name, GetLastError()))
            goto err_abort;
        }

        startTimer.start();

        while (svcStat.dwCurrentState != SERVICE_RUNNING) {
            if (startTimer.hasExpired(4000ms)) {
                LOG(std::format(L"Failed to start {} service, timed out", name))
                goto err_abort;
            }

            std::this_thread::sleep_for(15ms);

            if (!QueryServiceStatus(service, &svcStat)) {
                LOG(std::format(L"Failed to query {} service status: {}", name, GetLastError()))
                goto err_abort;
            }
        }

        CloseServiceHandle(service);
        CloseServiceHandle(scManager);
        return true;

    err_abort:
        CloseServiceHandle(service);
        CloseServiceHandle(scManager);
        return false;
    }

    bool deleteService(LPCWSTR name, const std::function<void(const std::wstring &)> &logcb) {
        SC_HANDLE scManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
        if (scManager == nullptr) {
            LOG(std::format(L"failed to open sc manager: {}", GetLastError()))
            return false;
        }

        SC_HANDLE service = OpenServiceW(scManager, name, DELETE);
        if (service == nullptr) {
            const DWORD lastErr = GetLastError();

            if (lastErr != ERROR_SERVICE_DOES_NOT_EXIST)
                LOG(std::format(L"failed to open {} service: {}", name, lastErr))

            CloseServiceHandle(scManager);
            return lastErr == ERROR_SERVICE_DOES_NOT_EXIST;
        }

        const bool res = DeleteService(service) == TRUE;

        if (!res)
            LOG(std::format(L"failed to delete {} service: {}", name, GetLastError()))

        CloseServiceHandle(service);
        CloseServiceHandle(scManager);
        return res;
    }

    bool installService(LPCWSTR name, const std::wstring &exePath, const std::function<void(const std::wstring &)> &logcb) {
        wchar_t description[] = L"PowerTuner daemon allows you to control various aspects of your device to better fit your needs: battery life, performance or a balance of both.";
        SERVICE_DESCRIPTION svcDesc {.lpDescription = description};
        SC_HANDLE scManager;
        SC_HANDLE service;

        scManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
        if (scManager == nullptr) {
            LOG(std::format(L"failed to open sc manager: {}", GetLastError()))
            return false;
        }

        service = OpenServiceW(scManager, name, SERVICE_QUERY_STATUS);
        if (service != nullptr || GetLastError() != ERROR_SERVICE_DOES_NOT_EXIST) {
            const DWORD lastErr = GetLastError();

            CloseServiceHandle(service);
            CloseServiceHandle(scManager);

            if (lastErr != ERROR_SUCCESS) {
                LOG(std::format(L"failed to check {} service existence: {}", name, GetLastError()))
                return false;
            }

            return true;
        }

        service = CreateServiceW(scManager, name, name, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, exePath.c_str(), nullptr, nullptr, nullptr, nullptr, nullptr);
        if (service == nullptr) {
            LOG(std::format(L"failed to create {} service: {}", name, GetLastError()))
            CloseServiceHandle(scManager);
            return false;
        }

        ChangeServiceConfig2W(service, SERVICE_CONFIG_DESCRIPTION, &svcDesc);
        CloseServiceHandle(service);
        CloseServiceHandle(scManager);
        return true;
    }

    bool closeRunningApps(const std::vector<std::wstring> &list, const std::function<void(const std::wstring &)> &logcb) {
        HANDLE procsShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32 procEntry;

        if (procsShot == INVALID_HANDLE_VALUE) {
            LOG(std::format(L"failed to get process list snapshot, code: {}", GetLastError()))
            return false;
        }

        procEntry.dwSize = sizeof(PROCESSENTRY32);

        if (!Process32FirstW(procsShot, &procEntry)) {
            LOG(std::format(L"failed to read first process entry, code: {}", GetLastError()))
            CloseHandle(procsShot);
            return false;
        }

        do {
            if (std::find(list.begin(), list.end(), procEntry.szExeFile) == list.end())
                continue;

            HANDLE proc = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, procEntry.th32ProcessID);

            if (proc == nullptr) {
                LOG(std::format(L"failed to grab {} process, code: {}", procEntry.szExeFile, GetLastError()))
                CloseHandle(procsShot);
                return false;
            }

            if (!TerminateProcess(proc, 0)) {
                LOG(std::format(L"failed to terminate {} process, code: {}", procEntry.szExeFile, GetLastError()))
                CloseHandle(proc);
                CloseHandle(procsShot);
                return false;
            }

            WaitForSingleObject(proc, 10 * 1000);
            CloseHandle(proc);
        } while (Process32NextW(procsShot, &procEntry));

        CloseHandle(procsShot);
        return true;
    }

    std::optional<bool> isServiceRunning(LPCWSTR name, const std::function<void(const std::wstring &)> &logcb) {
        SERVICE_STATUS svcStat {};
        SC_HANDLE scManager;
        SC_HANDLE service;

        scManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
        if (scManager == nullptr) {
            LOG(std::format(L"failed to open sc manager: {}", GetLastError()))
            return {};
        }

        service = OpenServiceW(scManager, name, SERVICE_QUERY_STATUS);
        if (service == nullptr) {
            const DWORD lastErr = GetLastError();

            if (lastErr != ERROR_SERVICE_DOES_NOT_EXIST) {
                LOG(std::format(L"failed to open {} service: {}", name, lastErr))
                return {};
            }

            CloseServiceHandle(scManager);
            return false;
        }

        if (!QueryServiceStatus(service, &svcStat)) {
            LOG(std::format(L"failed to query {} service: {}", name, GetLastError()))
            CloseServiceHandle(service);
            CloseServiceHandle(scManager);
            return {};
        }

        CloseServiceHandle(service);
        CloseServiceHandle(scManager);

        return svcStat.dwCurrentState == SERVICE_RUNNING;
    }
}
