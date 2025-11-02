// Utils Layer - String utilities placeholder

#include <string>
#include <algorithm>

namespace nomic {
namespace utils {

// Placeholder utility functions
std::string trim(const std::string& str) {
    auto start = str.begin();
    auto end = str.end();

    // Find first non-whitespace
    while (start != end && std::isspace(*start)) {
        ++start;
    }

    // Find last non-whitespace
    while (start != end && std::isspace(*(end - 1))) {
        --end;
    }

    return std::string(start, end);
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

} // namespace utils
} // namespace nomic