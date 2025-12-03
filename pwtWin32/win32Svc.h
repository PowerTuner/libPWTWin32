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

#include <QString>

#include "libExport.h"

namespace PWTW32 {
    [[nodiscard]] PWTWIN32_EXPORT bool stopService(QString &error);
    [[nodiscard]] PWTWIN32_EXPORT bool startService(QString &error);
    [[nodiscard]] PWTWIN32_EXPORT bool deleteService(QString &error);
    [[nodiscard]] PWTWIN32_EXPORT bool installService(const QString &exePath, QString &error);
    [[nodiscard]] PWTWIN32_EXPORT bool closeRunningApps(QString &error);
    [[nodiscard]] PWTWIN32_EXPORT bool isServiceRunning(QString &error);
}
