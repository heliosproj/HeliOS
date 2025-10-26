/*
 * Nomic C Semantic Source Code Analyzer
 * Copyright (C) 2025 Manny Peterson <me@mannypeterson.com>
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
#include <chrono>
#include <iomanip>
#include <spdlog/spdlog.h>

namespace nomic::cli {

class ProgressReporter {
public:
    ProgressReporter(size_t total_items, bool enable = true)
        : total_items_(total_items), current_item_(0), enabled_(enable) {
        start_time_ = std::chrono::steady_clock::now();
    }

    void update(size_t current, const std::string& message = "") {
        if (!enabled_) return;

        current_item_ = current;
        float progress = static_cast<float>(current) / total_items_;

        // Calculate elapsed time and ETA
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);

        std::chrono::seconds eta(0);
        if (current > 0) {
            auto total_estimated = elapsed.count() / progress;
            eta = std::chrono::seconds(static_cast<int>(total_estimated - elapsed.count()));
        }

        // Print progress bar
        printProgress(progress, message, elapsed, eta);
    }

    void increment(const std::string& message = "") {
        update(current_item_ + 1, message);
    }

    void finish(const std::string& message = "Complete") {
        if (!enabled_) return;

        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);

        std::cout << "\r[====================] 100% " << message
                 << " (Total: " << formatTime(elapsed) << ")          " << std::endl;
    }

    void setEnabled(bool enabled) {
        enabled_ = enabled;
    }

private:
    void printProgress(float progress, const std::string& message,
                       const std::chrono::seconds& elapsed,
                       const std::chrono::seconds& eta) {
        const int bar_width = 20;
        int filled = static_cast<int>(bar_width * progress);

        std::cout << "\r[";
        for (int i = 0; i < bar_width; ++i) {
            if (i < filled) {
                std::cout << "=";
            } else {
                std::cout << " ";
            }
        }

        std::cout << "] " << std::fixed << std::setprecision(1)
                 << (progress * 100.0) << "%";

        if (!message.empty()) {
            std::cout << " " << message;
        }

        std::cout << " (Elapsed: " << formatTime(elapsed);
        if (eta.count() > 0) {
            std::cout << ", ETA: " << formatTime(eta);
        }
        std::cout << ")    ";  // Extra spaces to clear previous longer text

        std::cout << std::flush;
    }

    std::string formatTime(const std::chrono::seconds& seconds) {
        int hrs = seconds.count() / 3600;
        int mins = (seconds.count() % 3600) / 60;
        int secs = seconds.count() % 60;

        std::ostringstream oss;
        if (hrs > 0) {
            oss << hrs << "h " << mins << "m " << secs << "s";
        } else if (mins > 0) {
            oss << mins << "m " << secs << "s";
        } else {
            oss << secs << "s";
        }
        return oss.str();
    }

private:
    size_t total_items_;
    size_t current_item_;
    bool enabled_;
    std::chrono::steady_clock::time_point start_time_;
};

} // namespace nomic::cli