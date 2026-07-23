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

#include <chrono>

namespace PWTW32 {
    class ElapsedTimer final {
    private:
        std::chrono::time_point<std::chrono::steady_clock> startTime;

    public:
        void start() {
            startTime = std::chrono::steady_clock::now();
        }

        [[nodiscard]]
        bool hasExpired(const std::chrono::milliseconds &ms) const {
            const auto now = std::chrono::steady_clock::now();

            return std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime) < ms;
        }
    };
}
