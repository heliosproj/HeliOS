#include "nomic/core/scope.h"
#include <algorithm>

namespace nomic {
namespace core {

Scope::Scope(ScopeKind kind)
    : kind_(kind) {
}

std::string Scope::getKindName() const {
    switch (kind_) {
        case ScopeKind::GLOBAL: return "Global";
        case ScopeKind::FILE: return "File";
        case ScopeKind::FUNCTION: return "Function";
        case ScopeKind::BLOCK: return "Block";
        case ScopeKind::STRUCT: return "Struct";
        case ScopeKind::UNION: return "Union";
        case ScopeKind::ENUM: return "Enum";
        case ScopeKind::NAMESPACE: return "Namespace";
        case ScopeKind::UNKNOWN:
        default: return "Unknown";
    }
}

} // namespace core
} // namespace nomic