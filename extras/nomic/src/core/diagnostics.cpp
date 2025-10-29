/*
 * Nomic C Semantic Source Code Analyzer
 * (C) 2020-2026 Manny Peterson <manny@heliosproj.org>
 *
 * This file is part of Nomic.
 *
 * Nomic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nomic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Nomic. If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <sstream>
#include <spdlog/spdlog.h>

namespace nomic {

class DiagnosticEngine {
public:
    enum Level {
        Debug,
        Info,
        Warning,
        Error,
        Fatal
    };

    struct Diagnostic {
        Level level;
        std::string message;
        std::string file;
        unsigned line = 0;
        unsigned column = 0;
    };

    void report(const Diagnostic& diag) {
        switch (diag.level) {
            case Debug:
                spdlog::debug("{}", diag.message);
                break;
            case Info:
                spdlog::info("{}", diag.message);
                break;
            case Warning:
                spdlog::warn("{}", formatDiagnostic(diag));
                break;
            case Error:
                spdlog::error("{}", formatDiagnostic(diag));
                break;
            case Fatal:
                spdlog::critical("{}", formatDiagnostic(diag));
                break;
        }
    }

private:
    std::string formatDiagnostic(const Diagnostic& diag) {
        std::ostringstream oss;
        if (!diag.file.empty()) {
            oss << diag.file;
            if (diag.line > 0) {
                oss << ":" << diag.line;
                if (diag.column > 0) {
                    oss << ":" << diag.column;
                }
            }
            oss << ": ";
        }
        oss << diag.message;
        return oss.str();
    }
};

} // namespace nomic