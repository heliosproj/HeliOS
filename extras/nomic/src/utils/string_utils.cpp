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

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace nomic::utils {

class StringUtils {
public:
    static std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;

        while (std::getline(ss, token, delimiter)) {
            if (!token.empty()) {
                tokens.push_back(token);
            }
        }

        return tokens;
    }

    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) {
            return "";
        }

        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }

    static std::string toLower(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    static std::string toUpper(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                      [](unsigned char c) { return std::toupper(c); });
        return result;
    }

    static bool startsWith(const std::string& str, const std::string& prefix) {
        return str.size() >= prefix.size() &&
               str.compare(0, prefix.size(), prefix) == 0;
    }

    static bool endsWith(const std::string& str, const std::string& suffix) {
        return str.size() >= suffix.size() &&
               str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    static std::string replace(const std::string& str,
                              const std::string& from,
                              const std::string& to) {
        std::string result = str;
        size_t pos = 0;

        while ((pos = result.find(from, pos)) != std::string::npos) {
            result.replace(pos, from.length(), to);
            pos += to.length();
        }

        return result;
    }

    static std::string join(const std::vector<std::string>& strings,
                           const std::string& delimiter) {
        std::ostringstream oss;

        for (size_t i = 0; i < strings.size(); ++i) {
            if (i > 0) {
                oss << delimiter;
            }
            oss << strings[i];
        }

        return oss.str();
    }

    static bool isWhitespaceOnly(const std::string& str) {
        return str.find_first_not_of(" \t\n\r") == std::string::npos;
    }

    static std::string escapeRegex(const std::string& str) {
        std::string escaped;
        const std::string special_chars = "^$.*+?()[]{}|\\";

        for (char c : str) {
            if (special_chars.find(c) != std::string::npos) {
                escaped += '\\';
            }
            escaped += c;
        }

        return escaped;
    }
};

} // namespace nomic::utils