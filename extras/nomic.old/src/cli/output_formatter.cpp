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

#include "nomic/core/rule_engine.h"
#include <iostream>
#include <iomanip>
#include <sstream>

namespace nomic::cli {

class OutputFormatter {
public:
    enum class Color {
        RESET,
        RED,
        YELLOW,
        GREEN,
        BLUE,
        MAGENTA,
        CYAN,
        WHITE,
        BOLD
    };

    OutputFormatter(bool use_color = true) : use_color_(use_color) {}

    void printViolation(const Violation& violation, std::ostream& out) {
        // Format: file:line:col: severity: message [rule-id]
        const auto& loc = violation.getLocation().getStart();

        out << colorize(loc.getFile() + ":" +
                       std::to_string(loc.getLine()) + ":" +
                       std::to_string(loc.getColumn()) + ": ",
                       Color::CYAN);

        Color severity_color = getSeverityColor(violation.getSeverity());
        out << colorize(severityToString(violation.getSeverity()) + ": ",
                       severity_color);

        out << violation.getMessage();

        out << colorize(" [" + violation.getRuleId() + "]", Color::BLUE);

        out << std::endl;

        // Print source snippet if available
        if (violation.getSnippet().has_value()) {
            printSnippet(violation.getSnippet().value(), loc.getLine(), out);
        }

        // Print suggested fix if available
        if (violation.getSuggestedFixDescription().has_value()) {
            out << colorize("    Suggested fix: ", Color::GREEN)
                << violation.getSuggestedFixDescription().value()
                << std::endl;
        }
    }

    void printSummary(const std::vector<Violation>& violations, std::ostream& out) {
        int errors = 0, warnings = 0, infos = 0;

        for (const auto& v : violations) {
            switch (v.getSeverity()) {
                case Severity::Error:
                    errors++;
                    break;
                case Severity::Warning:
                    warnings++;
                    break;
                case Severity::Info:
                    infos++;
                    break;
            }
        }

        out << std::endl;
        out << colorize("Summary: ", Color::BOLD);

        if (errors > 0) {
            out << colorize(std::to_string(errors) + " error" +
                          (errors > 1 ? "s" : ""), Color::RED);
        }

        if (warnings > 0) {
            if (errors > 0) out << ", ";
            out << colorize(std::to_string(warnings) + " warning" +
                          (warnings > 1 ? "s" : ""), Color::YELLOW);
        }

        if (infos > 0) {
            if (errors > 0 || warnings > 0) out << ", ";
            out << std::to_string(infos) + " info" + (infos > 1 ? "s" : "");
        }

        if (violations.empty()) {
            out << colorize("No issues found", Color::GREEN);
        }

        out << std::endl;
    }

private:
    void printSnippet(const std::string& snippet, unsigned highlight_line,
                     std::ostream& out) {
        std::istringstream iss(snippet);
        std::string line;
        unsigned line_num = highlight_line - 2;  // Assuming snippet starts 2 lines before

        while (std::getline(iss, line)) {
            if (line_num == highlight_line) {
                out << colorize(" >> ", Color::RED);
            } else {
                out << "    ";
            }

            out << std::setw(4) << line_num << " | " << line << std::endl;
            line_num++;
        }
    }

    std::string colorize(const std::string& text, Color color) {
        if (!use_color_) {
            return text;
        }

        std::string code;
        switch (color) {
            case Color::RESET:
                code = "\033[0m";
                break;
            case Color::RED:
                code = "\033[31m";
                break;
            case Color::YELLOW:
                code = "\033[33m";
                break;
            case Color::GREEN:
                code = "\033[32m";
                break;
            case Color::BLUE:
                code = "\033[34m";
                break;
            case Color::MAGENTA:
                code = "\033[35m";
                break;
            case Color::CYAN:
                code = "\033[36m";
                break;
            case Color::WHITE:
                code = "\033[37m";
                break;
            case Color::BOLD:
                code = "\033[1m";
                break;
        }

        return code + text + "\033[0m";
    }

    Color getSeverityColor(Severity severity) {
        switch (severity) {
            case Severity::Error:
                return Color::RED;
            case Severity::Warning:
                return Color::YELLOW;
            case Severity::Info:
                return Color::BLUE;
            default:
                return Color::RESET;
        }
    }

    std::string severityToString(Severity severity) {
        switch (severity) {
            case Severity::Error:
                return "error";
            case Severity::Warning:
                return "warning";
            case Severity::Info:
                return "info";
            default:
                return "unknown";
        }
    }

private:
    bool use_color_;
};

} // namespace nomic::cli