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
#include "win.h"

#include <tlhelp32.h>
#include <QThread>
#include <QElapsedTimer>

#include "win32Svc.h"

#define SVCNAME TEXT("PowerTunerDaemon")

namespace PWTW32 {
    bool stopService(QString &error) {
        SERVICE_STATUS_PROCESS svcProcStat {};
        SERVICE_STATUS svcStat {};
        QElapsedTimer stopTimer {};
        SC_HANDLE scManager;
        SC_HANDLE service;
        DWORD bytesNeed = 0;
        HANDLE svcProc;

        scManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
        if (scManager == nullptr) {
            error = QString("failed to open sc manager: %1").arg(GetLastError());
            return false;
        }

        service = OpenService(scManager, SVCNAME, SERVICE_STOP | SERVICE_QUERY_STATUS);
        if (service == nullptr) {
            const DWORD lastErr = GetLastError();

            if (lastErr != ERROR_SERVICE_DOES_NOT_EXIST)
                error = QString("failed to open PowerTuner service: %1").arg(lastErr);

            CloseServiceHandle(scManager);
            return lastErr == ERROR_SERVICE_DOES_NOT_EXIST;
        }

        if (!QueryServiceStatus(service, &svcStat)) {
            error = QString("failed to query PowerTuner service: %1").arg(GetLastError());
            goto err_abort;
        }

        if (svcStat.dwCurrentState == SERVICE_STOPPED) {
            CloseServiceHandle(service);
            CloseServiceHandle(scManager);
            return true;
        }

        if (!QueryServiceStatusEx(service, SC_STATUS_PROCESS_INFO, reinterpret_cast<LPBYTE>(&svcProcStat), sizeof(SERVICE_STATUS_PROCESS), &bytesNeed)) {
            error = QString("failed to query PowerTuner service process: %1").arg(GetLastError());
            goto err_abort;
        }

        svcProc = OpenProcess(SYNCHRONIZE, FALSE, svcProcStat.dwProcessId);

        if (!ControlService(service, SERVICE_CONTROL_STOP, &svcStat)) {
            error = QString("failed to send stop command to PowerTuner service: %1").arg(GetLastError());
            CloseHandle(svcProc);
            goto err_abort;
        }

        stopTimer.start();

        while (svcStat.dwCurrentState != SERVICE_STOPPED) {
            if (stopTimer.hasExpired(4000)) {
                error = "failed to stop PowerTuner service: timed out";
                CloseHandle(svcProc);
                goto err_abort;
            }

            QThread::msleep(15);

            if (!QueryServiceStatus(service, &svcStat)) {
                error = QString("failed to query PowerTuner service status: %1").arg(GetLastError());
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

    bool startService(QString &error) {
        SERVICE_STATUS svcStat {};
        QElapsedTimer startTimer {};
        SC_HANDLE scManager;
        SC_HANDLE service;

        scManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
        if (scManager == nullptr) {
            error = QString("failed to open sc manager: %1").arg(GetLastError());
            return false;
        }

        service = OpenService(scManager, SVCNAME, SERVICE_START | SERVICE_QUERY_STATUS);
        if (service == nullptr) {
            error = QString("failed to open service: %1").arg(GetLastError());
            CloseServiceHandle(scManager);
            return false;
        }

        if (!QueryServiceStatus(service, &svcStat)) {
            error = QString("failed to query PowerTuner service: %1").arg(GetLastError());
            goto err_abort;
        }

        if (svcStat.dwCurrentState == SERVICE_RUNNING) {
            CloseServiceHandle(service);
            CloseServiceHandle(scManager);
            return true;
        }

        if (!StartService(service, 0, nullptr)) {
            error = QString("failed to start service: %1").arg(GetLastError());
            goto err_abort;
        }

        startTimer.start();

        while (svcStat.dwCurrentState != SERVICE_RUNNING) {
            if (startTimer.hasExpired(4000)) {
                error = "Failed to start the service, timed out";
                goto err_abort;
            }

            QThread::msleep(15);

            if (!QueryServiceStatus(service, &svcStat)) {
                error = QString("Failed to query service status: %1").arg(GetLastError());
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

    bool deleteService(QString &error) {
        SC_HANDLE scManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
        if (scManager == nullptr) {
            error = QString("failed to open sc manager: %1").arg(GetLastError());
            return false;
        }

        SC_HANDLE service = OpenService(scManager, SVCNAME, DELETE);
        if (service == nullptr) {
            const DWORD lastErr = GetLastError();

            if (lastErr != ERROR_SERVICE_DOES_NOT_EXIST)
                error = QString("failed to open PowerTuner service: %1").arg(lastErr);

            CloseServiceHandle(scManager);
            return lastErr == ERROR_SERVICE_DOES_NOT_EXIST;
        }

        const bool res = DeleteService(service) == TRUE;

        if (!res)
            error = QString("failed to delete old service: %1").arg(GetLastError());

        CloseServiceHandle(service);
        CloseServiceHandle(scManager);
        return res;
    }

    bool installService(const QString &exePath, QString &error) {
        TCHAR description[] = L"PowerTuner daemon allows you to control various aspects of your device to better fit your needs: battery life, performance or a balance of both.";
        SERVICE_DESCRIPTION svcDesc {.lpDescription = description};
        SC_HANDLE scManager;
        SC_HANDLE service;

        scManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
        if (scManager == nullptr) {
            error = QString("failed to open sc manager: %1").arg(GetLastError());
            return false;
        }

        service = OpenService(scManager, SVCNAME, SERVICE_QUERY_STATUS);
        if (service != nullptr || GetLastError() != ERROR_SERVICE_DOES_NOT_EXIST) {
            const DWORD lastErr = GetLastError();

            CloseServiceHandle(service);
            CloseServiceHandle(scManager);

            if (lastErr != ERROR_SUCCESS) {
                error = QString("failed to check service existence: %1").arg(GetLastError());
                return false;
            }

            return true;
        }

        service = CreateService(scManager, SVCNAME, SVCNAME, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, exePath.toStdWString().c_str(), nullptr, nullptr, nullptr, nullptr, nullptr);
        if (service == nullptr) {
            error = QString("failed to create service: %1").arg(GetLastError());
            CloseServiceHandle(scManager);
            return false;
        }

        ChangeServiceConfig2(service, SERVICE_CONFIG_DESCRIPTION, &svcDesc);
        CloseServiceHandle(service);
        CloseServiceHandle(scManager);
        return true;
    }

    bool closeRunningApps(QString &error) {
        static constexpr std::array<std::wstring_view, 4> procList = {{
            L"PowerTunerDaemon.exe", L"PowerTunerCLI.exe", L"PowerTunerClient.exe", L"PowerTunerConsole.exe"
        }};
        HANDLE procsShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        PROCESSENTRY32 procEntry;

        if (procsShot == INVALID_HANDLE_VALUE) {
            error = QString("failed to get process list snapshot, code %1").arg(GetLastError());
            return false;
        }

        procEntry.dwSize = sizeof(PROCESSENTRY32);

        if (!Process32First(procsShot, &procEntry)) {
            error = QString("failed to read first process entry, code %1").arg(GetLastError());
            CloseHandle(procsShot);
            return false;
        }

        do {
            if (std::find(procList.begin(), procList.end(), procEntry.szExeFile) == procList.end())
                continue;

            HANDLE proc = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, FALSE, procEntry.th32ProcessID);

            if (proc == nullptr) {
                error = QString("failed to grab %1 process, code %2").arg(procEntry.szExeFile).arg(GetLastError());
                CloseHandle(procsShot);
                return false;
            }

            if (!TerminateProcess(proc, 0)) {
                error = QString("failed to terminate %1 process, code %2").arg(procEntry.szExeFile).arg(GetLastError());
                CloseHandle(proc);
                CloseHandle(procsShot);
                return false;
            }

            WaitForSingleObject(proc, 10 * 1000);
            CloseHandle(proc);
        } while (Process32Next(procsShot, &procEntry));

        CloseHandle(procsShot);
        return true;
    }

    bool isServiceRunning(QString &error) {
        SERVICE_STATUS svcStat {};
        SC_HANDLE scManager;
        SC_HANDLE service;

        scManager = OpenSCManager(nullptr, nullptr, SC_MANAGER_CONNECT);
        if (scManager == nullptr) {
            error = QString("failed to open sc manager: %1").arg(GetLastError());
            return false;
        }

        service = OpenService(scManager, SVCNAME, SERVICE_QUERY_STATUS);
        if (service == nullptr) {
            const DWORD lastErr = GetLastError();

            if (lastErr != ERROR_SERVICE_DOES_NOT_EXIST)
                error = QString("failed to open PowerTuner service: %1").arg(lastErr);

            CloseServiceHandle(scManager);
            return false;
        }

        if (!QueryServiceStatus(service, &svcStat)) {
            error = QString("failed to query PowerTuner service: %1").arg(GetLastError());
            CloseServiceHandle(service);
            CloseServiceHandle(scManager);
            return false;
        }

        CloseServiceHandle(service);
        CloseServiceHandle(scManager);

        return svcStat.dwCurrentState == SERVICE_RUNNING;
    }
}
