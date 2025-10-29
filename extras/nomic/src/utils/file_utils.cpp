/*
 * Nomic C Semantic Source Code Analyzer
 * (C) 2020-2026 Manny Peterson <manny@heliosproject.org>
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

#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <spdlog/spdlog.h>

namespace nomic::utils {

class FileUtils {
public:
    static std::string readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            spdlog::error("Cannot open file: {}", path);
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    static std::vector<std::string> readLines(const std::string& path) {
        std::vector<std::string> lines;
        std::ifstream file(path);

        if (!file.is_open()) {
            spdlog::error("Cannot open file: {}", path);
            return lines;
        }

        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }

        return lines;
    }

    static bool writeFile(const std::string& path, const std::string& content) {
        std::ofstream file(path);
        if (!file.is_open()) {
            spdlog::error("Cannot open file for writing: {}", path);
            return false;
        }

        file << content;
        return file.good();
    }

    static bool fileExists(const std::string& path) {
        return std::filesystem::exists(path) &&
               std::filesystem::is_regular_file(path);
    }

    static bool directoryExists(const std::string& path) {
        return std::filesystem::exists(path) &&
               std::filesystem::is_directory(path);
    }

    static std::vector<std::string> findFiles(const std::string& directory,
                                             const std::string& extension,
                                             bool recursive = true) {
        std::vector<std::string> files;

        try {
            if (recursive) {
                for (const auto& entry :
                     std::filesystem::recursive_directory_iterator(directory)) {
                    if (entry.is_regular_file() &&
                        entry.path().extension() == extension) {
                        files.push_back(entry.path().string());
                    }
                }
            } else {
                for (const auto& entry :
                     std::filesystem::directory_iterator(directory)) {
                    if (entry.is_regular_file() &&
                        entry.path().extension() == extension) {
                        files.push_back(entry.path().string());
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            spdlog::error("Error scanning directory {}: {}", directory, e.what());
        }

        return files;
    }

    static std::string getAbsolutePath(const std::string& path) {
        try {
            return std::filesystem::absolute(path).string();
        } catch (const std::filesystem::filesystem_error& e) {
            spdlog::error("Error getting absolute path for {}: {}", path, e.what());
            return path;
        }
    }

    static std::string getDirectory(const std::string& path) {
        return std::filesystem::path(path).parent_path().string();
    }

    static std::string getFilename(const std::string& path) {
        return std::filesystem::path(path).filename().string();
    }

    static std::string getBasename(const std::string& path) {
        return std::filesystem::path(path).stem().string();
    }

    static std::string getExtension(const std::string& path) {
        return std::filesystem::path(path).extension().string();
    }

    static bool createDirectory(const std::string& path) {
        try {
            return std::filesystem::create_directories(path);
        } catch (const std::filesystem::filesystem_error& e) {
            spdlog::error("Error creating directory {}: {}", path, e.what());
            return false;
        }
    }

    static size_t getFileSize(const std::string& path) {
        try {
            return std::filesystem::file_size(path);
        } catch (const std::filesystem::filesystem_error& e) {
            spdlog::error("Error getting file size for {}: {}", path, e.what());
            return 0;
        }
    }

    static std::filesystem::file_time_type getLastModifiedTime(const std::string& path) {
        try {
            return std::filesystem::last_write_time(path);
        } catch (const std::filesystem::filesystem_error& e) {
            spdlog::error("Error getting last modified time for {}: {}", path, e.what());
            return {};
        }
    }
};

} // namespace nomic::utils