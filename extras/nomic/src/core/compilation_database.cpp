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

#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <spdlog/spdlog.h>
#include <filesystem>
#include <memory>

namespace nomic {

class CompilationDatabaseManager {
public:
    static std::unique_ptr<clang::tooling::CompilationDatabase>
    loadDatabase(const std::string& build_path) {
        spdlog::info("Loading compilation database from: {}", build_path);

        std::string error_msg;
        auto db = clang::tooling::CompilationDatabase::loadFromDirectory(
            build_path, error_msg);

        if (!db) {
            spdlog::error("Failed to load compilation database: {}", error_msg);

            // Try to find compile_commands.json in parent directories
            std::filesystem::path path(build_path);
            while (!path.empty() && path != path.parent_path()) {
                std::filesystem::path json_path = path / "compile_commands.json";
                if (std::filesystem::exists(json_path)) {
                    spdlog::info("Found compile_commands.json at: {}", json_path.string());
                    db = clang::tooling::CompilationDatabase::loadFromDirectory(
                        path.string(), error_msg);
                    if (db) {
                        break;
                    }
                }
                path = path.parent_path();
            }
        }

        if (!db) {
            spdlog::warn("Creating fixed compilation database as fallback");
            // Create a fixed compilation database with common flags
            const char* fixed_compile_flags[] = {
                "-std=c11",
                "-Wall",
                "-I/usr/include",
                "-I/usr/local/include"
            };

            int argc = sizeof(fixed_compile_flags) / sizeof(fixed_compile_flags[0]);
            db = clang::tooling::FixedCompilationDatabase::loadFromCommandLine(
                argc,
                fixed_compile_flags,
                error_msg);
        }

        return db;
    }

    static std::vector<std::string>
    getSourceFiles(const clang::tooling::CompilationDatabase& db) {
        std::vector<std::string> files = db.getAllFiles();

        // Filter to only C files
        std::vector<std::string> c_files;
        for (const auto& file : files) {
            std::filesystem::path p(file);
            if (p.extension() == ".c" || p.extension() == ".h") {
                c_files.push_back(file);
            }
        }

        spdlog::info("Found {} C source files in compilation database", c_files.size());
        return c_files;
    }

    static clang::tooling::CompileCommand
    getCompileCommand(const clang::tooling::CompilationDatabase& db,
                      const std::string& file) {
        auto commands = db.getCompileCommands(file);

        if (commands.empty()) {
            spdlog::warn("No compile commands found for: {}", file);

            // Return a default compile command
            clang::tooling::CompileCommand cmd;
            cmd.Filename = file;
            cmd.Directory = std::filesystem::current_path().string();
            cmd.CommandLine = {"clang", "-std=c11", "-Wall", file};
            return cmd;
        }

        // Use the first compile command
        return commands[0];
    }

    static std::vector<std::string>
    extractIncludePaths(const clang::tooling::CompileCommand& cmd) {
        std::vector<std::string> include_paths;

        for (size_t i = 0; i < cmd.CommandLine.size(); ++i) {
            const std::string& arg = cmd.CommandLine[i];

            if (arg == "-I" && i + 1 < cmd.CommandLine.size()) {
                include_paths.push_back(cmd.CommandLine[i + 1]);
                ++i;
            } else if (arg.starts_with("-I")) {
                include_paths.push_back(arg.substr(2));
            } else if (arg == "-isystem" && i + 1 < cmd.CommandLine.size()) {
                include_paths.push_back(cmd.CommandLine[i + 1]);
                ++i;
            }
        }

        return include_paths;
    }

    static std::vector<std::string>
    extractDefines(const clang::tooling::CompileCommand& cmd) {
        std::vector<std::string> defines;

        for (size_t i = 0; i < cmd.CommandLine.size(); ++i) {
            const std::string& arg = cmd.CommandLine[i];

            if (arg == "-D" && i + 1 < cmd.CommandLine.size()) {
                defines.push_back(cmd.CommandLine[i + 1]);
                ++i;
            } else if (arg.starts_with("-D")) {
                defines.push_back(arg.substr(2));
            }
        }

        return defines;
    }

    static std::string
    extractStandard(const clang::tooling::CompileCommand& cmd) {
        for (const auto& arg : cmd.CommandLine) {
            if (arg.starts_with("-std=")) {
                return arg.substr(5);
            }
        }
        return "c11";  // Default to C11
    }

    static bool
    hasWarningsAsErrors(const clang::tooling::CompileCommand& cmd) {
        for (const auto& arg : cmd.CommandLine) {
            if (arg == "-Werror") {
                return true;
            }
        }
        return false;
    }
};

} // namespace nomic